#include <chrono>
#include <cstdlib>
#include <string>

#include "../include/event.h"

int operator+(Event::Status s) {
	return static_cast<int>(s);
}

bool operator<(const Event &e1, const Event &e2) {
	return e1.start < e2.start;
}

int64_t to_unix_seconds(std::chrono::system_clock::time_point tp) {
  return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

std::chrono::system_clock::time_point from_unix_seconds(int64_t unix_seconds) {
  return std::chrono::system_clock::time_point{std::chrono::seconds{unix_seconds}};
}
