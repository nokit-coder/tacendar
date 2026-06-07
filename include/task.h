#pragma once

#include <cstdint>
#include <string>
#include <thread>


struct Task {
	enum class Status {
		STOPPED,
		RUNNING,
		PAST,
	};

	uint64_t id = 0; // 0 -> undef
	std::string command;
	Status status = Status::STOPPED;
	uint16_t exit_code = 0;
	std::string output;

	void exec();
};

int operator+(Task::Status s);
