#pragma once

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "action.h"
#include "dbManager.h"
#include "event.h"

#include "taskManager.cpp"

class Daemon {
public:
  Daemon(SqliteDb &db);
  ~Daemon();

  void load_db();
  bool start_daemon();
  bool stop_daemon();
  void watchdog();

  std::condition_variable cv;

private:
  SqliteDb _db;

  bool _is_running = false;
  bool _db_changed = false;
  std::jthread daemon_proc;
  std::jthread watchdog_proc;
	ThreadPool action_procs;

  std::mutex _mutex;

  void daemon();
  std::shared_ptr<Event> find_event_by_id(uint64_t event_id);
  void exec_action_in_thread(std::shared_ptr<Action> action, const uint64_t event_id);
};
