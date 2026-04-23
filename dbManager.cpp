#include <sqlite3.h>

#include "dbManager.h"

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
