#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "task.h"

class Event {
public:
  std::chrono::system_clock::time_point start{};
  std::chrono::system_clock::time_point end{};
  std::string name;
  std::string description;
  bool is_checkable = false;
  bool is_checked = false;
  std::vector<Task> tasks;

  void exec() const;
};

int64_t to_unix_seconds(std::chrono::system_clock::time_point tp);
std::chrono::system_clock::time_point from_unix_seconds(int64_t unix_seconds);

