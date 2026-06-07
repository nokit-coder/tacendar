#pragma once

#include "dbManager.h"
#include "event.h"
#include "task.h"
#include <thread>
#include <vector>

class Daemon {
public:
  Daemon(SqliteDb &db);

  void load_db();
	void watchdog();

  void daemon(); // временно на стадии тестирования

  std::vector<Event> events;

	struct SleepStep { int max_timer; int ms; };
	static constexpr SleepStep steps[] = {
		{ 10,   10 },
		{ 18,   50 },
		{ 23,  100 },
		{ 33, 1000 },
		{ 34,  5000 }
	};

private:
  // std::vector<Event> events;
  SqliteDb _db;
	std::thread daemon_proc;
	std::thread task_procs;

  // void daemon();
};

void exec_task_in_thread(Task &task);
