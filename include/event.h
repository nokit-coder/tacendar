#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "task.h"

class Event {
public:
	enum class Status {
		STOPPED,
		RUNNING,
		PAST,
	};

  uint64_t id = 0; // 0 -> undef
  std::chrono::system_clock::time_point start{}; // event start time/date
  std::chrono::system_clock::time_point end{}; // event end time/date
  std::string name;
  std::string description;
  bool is_checkable = false; // is check box available
  bool is_checked = false; // is check box activen
	Status status = Status::STOPPED;
  std::vector<Task> tasks;

  void exec() const;
};

int operator+(Event::Status s);

bool operator<(const Event &e1, const Event &e2);

int64_t to_unix_seconds(std::chrono::system_clock::time_point tp);
std::chrono::system_clock::time_point from_unix_seconds(int64_t unix_seconds);

