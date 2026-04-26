#include <cstdint>
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "dbManager.h"
#include "event.h"
#include "task.h"

SqliteDb::SqliteDb(const std::string &filename) { open(filename); }

SqliteDb::~SqliteDb() {
  if (db_) {
    sqlite3_close(db_);
    db_ = nullptr;
  }
}

bool SqliteDb::open(const std::string &filename) {
  last_error_.clear();

  if (db_) {
    sqlite3_close(db_);
    db_ = nullptr;
  }

  int rc = sqlite3_open(filename.c_str(), &db_);
  if (rc != SQLITE_OK) {
    if (db_) {
      last_error_ = sqlite3_errmsg(db_);
      sqlite3_close(db_);
      db_ = nullptr;
    } else {
      last_error_ = "sqlite3_open failed";
    }
    return false;
  }

  return true;
}

bool SqliteDb::ok() const { return db_ != nullptr; }

const std::string &SqliteDb::last_error() const { return last_error_; }

bool SqliteDb::exec(const std::string &sql) {
  last_error_.clear();
  if (!db_) {
    last_error_ = "db is not open";
    return false;
  }

  char *err = nullptr;
  int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    if (err) {
      last_error_ = err;
      sqlite3_free(err);
    } else {
      last_error_ = sqlite3_errmsg(db_);
    }
    return false;
  }

  if (err) {
    sqlite3_free(err);
  }
  return true;
}

bool SqliteDb::create_tables() {
	// EVENTS
  std::string q1 =
      "CREATE TABLE IF NOT EXISTS events ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "start_unix INTEGER NOT NULL,"
      "end_unix INTEGER NOT NULL,"
      "name TEXT NOT NULL,"
      "description TEXT NOT NULL,"
      "is_checkable INTEGER NOT NULL,"
      "is_checked INTEGER NOT NULL"
      ");";

  if (!exec(q1)) {
    return false;
  }

	// TASKS
  std::string q2 =
      "CREATE TABLE IF NOT EXISTS tasks ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "event_id INTEGER NOT NULL,"
      "command TEXT NOT NULL,"
      "exit_code INTEGER NOT NULL,"
      "output TEXT NOT NULL,"
      "FOREIGN KEY(event_id) REFERENCES events(id)"
      ");";

  if (!exec(q2)) {
    return false;
  }

  return true;
}

bool SqliteDb::insert_event(Event &event) {
  last_error_.clear();
  if (!db_) {
    last_error_ = "db is not open";
    return false;
  }

	const char *q =
      "INSERT INTO events "
      "(id, start_unix, end_unix, name, description, is_checkable, is_checked) "
      "VALUES (?, ?, ?, ?, ?, ?, ?) "
      "ON CONFLICT(id) DO UPDATE SET "
      "start_unix=excluded.start_unix, "
      "end_unix=excluded.end_unix, "
      "name=excluded.name, "
      "description=excluded.description, "
      "is_checkable=excluded.is_checkable, "
      "is_checked=excluded.is_checked;";

  sqlite3_stmt *stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, q, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    last_error_ = sqlite3_errmsg(db_);
    return false;
  }

  if (event.id > 0) {
    sqlite3_bind_int64(stmt, 1, event.id);
  } else {
    sqlite3_bind_null(stmt, 1);
  }
  sqlite3_bind_int64(stmt, 2, to_unix_seconds(event.start));
  sqlite3_bind_int64(stmt, 3, to_unix_seconds(event.end));
  sqlite3_bind_text(stmt, 4, event.name.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 5, event.description.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 6, event.is_checkable);
  sqlite3_bind_int(stmt, 7, event.is_checked);

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    last_error_ = sqlite3_errmsg(db_);
    sqlite3_finalize(stmt);
    return false;
  }

  if (event.id == 0) {
    event.id = sqlite3_last_insert_rowid(db_);
  }

  sqlite3_finalize(stmt);
  return true;
}

bool SqliteDb::insert_task(Task &task, uint64_t &event_id) {
  last_error_.clear();
  if (!db_) {
    last_error_ = "db is not open";
    return false;
  }

	const char *q =
      "INSERT INTO tasks "
      "(id, event_id, command, exit_code, output) "
      "VALUES (?, ?, ?, ?, ?)"
      "ON CONFLICT(id) DO UPDATE SET "
      "event_id=excluded.event_id, "
      "command=excluded.command, "
      "exit_code=excluded.exit_code, "
      "output=excluded.output";

  sqlite3_stmt *stmt = nullptr;
  int rc = sqlite3_prepare_v2(db_, q, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    last_error_ = sqlite3_errmsg(db_);
    return false;
  }

  if (task.id > 0) {
    sqlite3_bind_int64(stmt, 1, task.id);
  } else {
    sqlite3_bind_null(stmt, 1);
  }
  sqlite3_bind_int64(stmt, 2, event_id);
  sqlite3_bind_text(stmt, 3, task.command.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 4, task.exit_code);
  sqlite3_bind_text(stmt, 5, task.output.c_str(), -1, SQLITE_TRANSIENT);

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    last_error_ = sqlite3_errmsg(db_);
    sqlite3_finalize(stmt);
    return false;
  }

	if (task.id == 0) {
		task.id = sqlite3_last_insert_rowid(db_);
	}

  sqlite3_finalize(stmt);
  return true;
}

std::vector<Event> SqliteDb::fetch_events() {
	std::vector<Event> result;

	last_error_.clear();
	if (!db_) {
		last_error_ = "db is not open";
		return result;
	}

	const char* q = "SELECT * FROM events";
		// "(id, start_unix, end_unix, name, description, is_checkable, is_checked) FROM events"

	sqlite3_stmt* stmt;

	int rc = sqlite3_prepare_v2(db_, q, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
    last_error_ = sqlite3_errmsg(db_);
		return result;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Event e;

		e.id = sqlite3_column_int(stmt, 0);
		e.start = from_unix_seconds(sqlite3_column_int(stmt, 1));
		e.end = from_unix_seconds(sqlite3_column_int(stmt, 2));
		e.name = reinterpret_cast<const char*>((sqlite3_column_text(stmt, 3)));
		e.description = reinterpret_cast<const char*>((sqlite3_column_text(stmt, 4)));
		e.is_checkable = sqlite3_column_int(stmt, 5);
		e.is_checked = sqlite3_column_int(stmt, 6);

		result.push_back(e);
	}

	sqlite3_finalize(stmt);

	return result;
}

std::vector<std::pair<uint64_t, Task>> SqliteDb::fetch_tasks() {
	std::vector<std::pair<uint64_t, Task>> result;

	last_error_.clear();
	if (!db_) {
		last_error_ = "db is not open";
		return result;
	}

	const char* q = "SELECT * FROM tasks";

	sqlite3_stmt* stmt;

	int rc = sqlite3_prepare_v2(db_, q, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
    last_error_ = sqlite3_errmsg(db_);
		return result;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Task t;

		t.id = sqlite3_column_int64(stmt, 0);
		uint64_t event_id = sqlite3_column_int(stmt, 1);
		t.command = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		t.exit_code = sqlite3_column_int(stmt, 3);
		t.output = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

		result.push_back({event_id, t});
	}

	sqlite3_finalize(stmt);

	return result;
}

std::vector<Task> SqliteDb::fetch_tasks(uint64_t event_id) {
	std::vector<Task> result;

	last_error_.clear();
	if (!db_) {
		last_error_ = "db is not open";
		return result;
	}

	const char* q = "SELECT * FROM tasks WHERE event_id = ?";

	sqlite3_stmt* stmt;

	int rc = sqlite3_prepare_v2(db_, q, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
    last_error_ = sqlite3_errmsg(db_);
		return result;
	}

	sqlite3_bind_int64(stmt, 1, event_id);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Task t;

		t.id = sqlite3_column_int64(stmt, 0);
		t.command = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		t.exit_code = sqlite3_column_int(stmt, 3);
		t.output = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

		result.push_back(t);
	}

	sqlite3_finalize(stmt);

	return result;
}
