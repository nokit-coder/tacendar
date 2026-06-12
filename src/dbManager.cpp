#include <cstdint>
#include <memory>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "../include/action.h"
#include "../include/dbManager.h"
#include "../include/event.h"

SqliteDb::SqliteDb(const std::string &filename) { open(filename); }

SqliteDb::~SqliteDb() {
  if (_db) {
    sqlite3_close(_db);
    _db = nullptr;
  }
}

bool SqliteDb::open(const std::string &filename) {
  last_error_.clear();

  if (_db) {
    sqlite3_close(_db);
    _db = nullptr;
  }

  int rc = sqlite3_open(filename.c_str(), &_db);
  if (rc != SQLITE_OK) {
    if (_db) {
      last_error_ = sqlite3_errmsg(_db);
      sqlite3_close(_db);
      _db = nullptr;
    } else {
      last_error_ = "sqlite3_open failed";
    }
    return false;
  }

  return true;
}

bool SqliteDb::ok() const { return _db != nullptr; }

const std::string &SqliteDb::last_error() const { return last_error_; }

bool SqliteDb::exec(const char sql[]) {
  last_error_.clear();
  if (!_db) {
    last_error_ = "db is not open";
    return false;
  }

  char *err = nullptr;
  int rc = sqlite3_exec(_db, sql, nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    if (err) {
      last_error_ = err;
      sqlite3_free(err);
    } else {
      last_error_ = sqlite3_errmsg(_db);
    }
    return false;
  }

  if (err) {
    sqlite3_free(err);
  }
  return true;
}
bool SqliteDb::exec(const std::string &sql) { return exec(sql.c_str()); }

bool SqliteDb::create_tables() {
  last_error_.clear();
  if (!_db) {
    last_error_ = "db is not open";
    return false;
  }

  // EVENTS
  const char *q1 = "CREATE TABLE IF NOT EXISTS events ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "start_unix INTEGER NOT NULL,"
                   "end_unix INTEGER NOT NULL,"
                   "name TEXT NOT NULL,"
                   "description TEXT NOT NULL,"
                   "is_checkable INTEGER NOT NULL,"
                   "is_checked INTEGER NOT NULL,"
                   "status INTEGER NOT NULL"
                   ");";

  if (!exec(q1)) {
    return false;
  }

  // ACTIONS
  const char *q2 = "CREATE TABLE IF NOT EXISTS actions ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "event_id INTEGER NOT NULL,"
                   "status INTEGER NOT NULL,"
                   "type INTEGER NOT NULL,"
                   "command TEXT NOT NULL DEFAULT '',"
                   "exit_code INTEGER NOT NULL DEFAULT 0,"
                   "output TEXT NOT NULL DEFAULT '',"
                   "FOREIGN KEY(event_id) REFERENCES events(id)"
                   ");";

  if (!exec(q2)) {
    return false;
  }

  return true;
}

bool SqliteDb::insert_event(Event &event) {
  last_error_.clear();
  if (!_db) {
    last_error_ = "db is not open";
    return false;
  }

  const char *q = "INSERT INTO events "
                  "(id, start_unix, end_unix, name, description, is_checkable, is_checked, status) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?) "
                  "ON CONFLICT(id) DO UPDATE SET "
                  "start_unix=excluded.start_unix, "
                  "end_unix=excluded.end_unix, "
                  "name=excluded.name, "
                  "description=excluded.description, "
                  "is_checkable=excluded.is_checkable, "
                  "is_checked=excluded.is_checked,"
                  "status=excluded.status;";

  sqlite3_stmt *stmt = nullptr;
  int rc = sqlite3_prepare_v2(_db, q, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    last_error_ = sqlite3_errmsg(_db);
    return false;
  }

  if (event.id > 0) {
    sqlite3_bind_int64(stmt, 1, event.id);
  } else {
    sqlite3_bind_null(stmt, 1);
  }
  sqlite3_bind_int64(  stmt, 2,  to_unix_seconds(event.start)                       );
  sqlite3_bind_int64(  stmt, 3,  to_unix_seconds(event.end)                         );
  sqlite3_bind_text(   stmt, 4,  event.name.c_str(),          -1, SQLITE_TRANSIENT  );
  sqlite3_bind_text(   stmt, 5,  event.description.c_str(),   -1, SQLITE_TRANSIENT  );
  sqlite3_bind_int(    stmt, 6,  event.is_checkable                                 );
  sqlite3_bind_int(    stmt, 7,  event.is_checked                                   );
  sqlite3_bind_int(    stmt, 8, +event.status                                       );

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    last_error_ = sqlite3_errmsg(_db);
    sqlite3_finalize(stmt);
    return false;
  }

  if (event.id == 0) {
    event.id = sqlite3_last_insert_rowid(_db);
  }

  sqlite3_finalize(stmt);
  return true;
}

bool SqliteDb::insert_action(Action &action, const uint64_t &event_id) {
  last_error_.clear();
  if (!_db) {
    last_error_ = "db is not open";
    return false;
  }

  const char *q = "INSERT INTO actions "
                  "(id, event_id, status, type, command, exit_code, output) "
                  "VALUES "
                  "(:id, :event_id, :status, :type, :command, :exit_code, :output) "
                  "ON CONFLICT(id) DO UPDATE SET "
                  "event_id=excluded.event_id, "
                  "status=excluded.status, "
                  "command=excluded.command, "
                  "exit_code=excluded.exit_code, "
                  "output=excluded.output";

  sqlite3_stmt *stmt = nullptr;
  int rc = sqlite3_prepare_v2(_db, q, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    last_error_ = sqlite3_errmsg(_db);
    return false;
  }

	sqlite3_bind_int64(stmt, sqlite3_bind_parameter_index(stmt, ":event_id"), event_id);
  action.save_to_db(stmt);

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    last_error_ = sqlite3_errmsg(_db);
    sqlite3_finalize(stmt);
    return false;
  }

  if (action.id == 0) {
    action.id = sqlite3_last_insert_rowid(_db);
  }

  sqlite3_finalize(stmt);
  return true;
}

std::vector<std::shared_ptr<Event>> SqliteDb::fetch_events() {
	std::vector<std::shared_ptr<Event>> events;
	fetch_events(events);
	return events;
}

bool SqliteDb::fetch_events(std::vector<std::shared_ptr<Event>> &events) {
  events.clear();

  last_error_.clear();
  if (!_db) {
    last_error_ = "db is not open";
    return false;
  }

  const char *q = "SELECT "

									"id, "
									"start_unix, "
									"end_unix, "
									"name, "
									"description, "
									"is_checkable, "
									"is_checked, "
									"status "

									"FROM events;";

  sqlite3_stmt *stmt;

  int rc = sqlite3_prepare_v2(_db, q, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    last_error_ = sqlite3_errmsg(_db);
    return false;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    Event e;

    e.id = sqlite3_column_int(stmt, 0);
    e.start = from_unix_seconds(sqlite3_column_int(stmt, 1));
    e.end = from_unix_seconds(sqlite3_column_int(stmt, 2));
    e.name = reinterpret_cast<const char *>((sqlite3_column_text(stmt, 3)));
    e.description = reinterpret_cast<const char *>((sqlite3_column_text(stmt, 4)));
    e.is_checkable = sqlite3_column_int(stmt, 5);
    e.is_checked = sqlite3_column_int(stmt, 6);
    e.status = static_cast<Event::Status>(sqlite3_column_int(stmt, 6));

    events.push_back(std::make_shared<Event>(e));
  }

  sqlite3_finalize(stmt);

  return true;
}

std::vector<std::shared_ptr<Action>> SqliteDb::fetch_actions(uint64_t event_id) {
	std::vector<std::shared_ptr<Action>> actions;
	fetch_actions(event_id, actions);
	return actions;
}

bool SqliteDb::fetch_actions(const uint64_t event_id, std::vector<std::shared_ptr<Action>> &actions) {
  actions.clear();

  last_error_.clear();
  if (!_db) {
    last_error_ = "db is not open";
    return false;
  }

  const char *q = "SELECT "

									"id, "
									"type, "
									"status, "
									"command, "
									"exit_code, "
									"output "

									"FROM actions "
									"WHERE event_id = ?";

  sqlite3_stmt *stmt;

  int rc = sqlite3_prepare_v2(_db, q, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    last_error_ = sqlite3_errmsg(_db);
    return false;
  }

  sqlite3_bind_int64(stmt, 1, event_id);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    ActionType type = static_cast<ActionType>(sqlite3_column_int(stmt, 1));

    std::shared_ptr<Action> a;

    if (type == ActionType::Command) {
      a = std::make_shared<ActionCommand>();
    } else {
      continue;
    }

    a->load_from_db(stmt);

    actions.push_back(a);
  }

  sqlite3_finalize(stmt);

	return true;
}

bool auto_insert_event(SqliteDb &db, Event &event) {
  if (!db.insert_event(event)) { return false; }

  for (auto &action : event.actions) {
    if (!db.insert_action(*action, event.id)) { return false; }
  }

  return true;
}

bool auto_insert_events(SqliteDb &db, std::vector<std::shared_ptr<Event>> &events) {
  for (auto &event : events) {
    if (!auto_insert_event(db, *event)) { return false; }
  }

  return true;
}

std::vector<std::shared_ptr<Event>> auto_fetch_events(SqliteDb &db) {
	std::vector<std::shared_ptr<Event>> events;
	auto_fetch_events(db, events);
	return events;
}

bool auto_fetch_events(SqliteDb &db, std::vector<std::shared_ptr<Event>> &events) {
  events.clear();

  db.fetch_events(events);
  if (!db.last_error().empty()) { return false; }

  for (auto &event : events) {
    db.fetch_actions(event->id, event->actions);
    if (!db.last_error().empty()) { return false; }
  }

	return true;
}
