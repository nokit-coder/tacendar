#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <sqlite3.h>

#include "action.h"
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
	bool insert_action(Action &action, uint64_t &event_id);

	std::vector<std::shared_ptr<Event>> fetch_events();
	bool fetch_events(std::vector<std::shared_ptr<Event>> &events);
	// fetch actions by event_id
	std::vector<std::shared_ptr<Action>> fetch_actions(uint64_t event_id);
	bool fetch_actions(uint64_t event_id, std::vector<std::shared_ptr<Action>> &action);

private:
  sqlite3 *_db = nullptr;
  std::string last_error_;

	// exec non returning/param sql
  bool exec(const char sql[]);
  bool exec(const std::string &sql);
};

bool auto_insert_event(SqliteDb &db, Event &event);
bool auto_insert_events(SqliteDb &db, std::vector<std::shared_ptr<Event>> &events);

std::vector<std::shared_ptr<Event>> auto_fetch_events(SqliteDb &db);
bool auto_fetch_events(SqliteDb &db, std::vector<std::shared_ptr<Event>> &events);
