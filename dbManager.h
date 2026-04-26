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

	// is db available
  bool ok() const;
  const std::string &last_error() const;

	// create default tbls
  bool create_tables();
  bool insert_event(Event &event);
	bool insert_task(Task &task, uint64_t &event_id);
	std::vector<Event> fetch_events();
	// get tasks and they events id
	std::vector<std::pair<uint64_t, Task>> fetch_tasks();
	// get tasks by event_id
	std::vector<Task> fetch_tasks(uint64_t event_id);

private:
  sqlite3 *db_ = nullptr;
  std::string last_error_;

	// exec non returning/param sql
  bool exec(const std::string &sql);
};

