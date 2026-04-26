#pragma once

#include <cstdint>
#include <string>

enum TaskStatus {
	STOPPED,
	RUNNING,
};

struct Task {
	uint64_t id = 0; // 0 -> undef
	std::string command;
	TaskStatus status = STOPPED; // TODO: сделать надо(в будущее)
	uint16_t exit_code = 0; // TODO: сделать надо(в будущее)
	std::string output; // TODO: сделать надо(в будущее)
};
