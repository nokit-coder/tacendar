#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <sqlite3.h>

#include "event.h"

class SqliteDb {
public:
  explicit SqliteDb(const std::string &filename);
  ~SqliteDb();

  bool open(const std::string &filename);

  bool ok() const;
  const std::string &last_error() const;

  bool create_tables();
  bool insert_event(const Event &e, int64_t *out_event_id = nullptr);
  std::vector<Event> fetch_events();

  // bool seed_events_if_empty();

private:
  sqlite3 *db_ = nullptr;
  std::string last_error_;

  bool exec(const std::string &sql);
};

