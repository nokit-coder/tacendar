#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>

#include "event.h"

void Event::exec() const {
  for (const auto &task : tasks) {
    std::system(task.command.c_str());
  }
}

int64_t to_unix_seconds(std::chrono::system_clock::time_point tp) {
  return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

std::chrono::system_clock::time_point from_unix_seconds(int64_t unix_seconds) {
  return std::chrono::system_clock::time_point{std::chrono::seconds{unix_seconds}};
}
