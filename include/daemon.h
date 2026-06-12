#pragma once

#include "dbManager.h"
#include "event.h"
#include "action.h"
#include <cstdint>
#include <thread>
#include <vector>

class Daemon {
public:
  Daemon(SqliteDb &db);
  ~Daemon();

  void load_db();
	bool start_daemon();
	bool stop_daemon();

  std::vector<std::shared_ptr<Event>> events;

	struct SleepStep { int max_timer; int ms; };
	std::vector<SleepStep> steps {
		{ 10,   10 },
		{ 18,   50 },
		{ 23,  100 },
		{ 33, 1000 },
		{ 34, 5000 }
	};

private:
  // std::vector<Event> events;
  SqliteDb _db;
	
	bool _is_running = false;
	std::thread daemon_proc;
	std::vector<std::thread> action_procs;

	void exec_action_in_thread(Action &action, uint64_t event_id);
  void daemon();
};
