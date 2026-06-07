#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <sqlite3.h>

#include "event.h"

class SqliteDb {
public:
  SqliteDb(const std::string &filename);
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
	void fetch_events(std::vector<Event> &events);
	// get tasks and they events id
	std::vector<std::pair<uint64_t, Task>> fetch_tasks(); // idk what pupros of th
	// get tasks by event_id
	std::vector<Task> fetch_tasks(uint64_t event_id);
	void fetch_tasks(uint64_t event_id, std::vector<Task> &tasks);

private:
  sqlite3 *_db = nullptr;
  std::string last_error_;

	// exec non returning/param sql
  bool exec(const char sql[]);
  bool exec(const std::string &sql);
};

bool auto_insert_event(SqliteDb &db, Event &event);
bool auto_insert_events(SqliteDb &db, std::vector<Event> &events);

std::vector<Event> auto_fetch_events(SqliteDb &db);
void auto_fetch_events(SqliteDb &db, std::vector<Event> &events);
