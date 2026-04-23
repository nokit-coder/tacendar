#include <cstdint>
#include <string>

enum TaskStatus {
	STOPPED,
	RUNNING,
};

struct Task {
	std::string command;
	TaskStatus status;  // TODO: сделать надо(в будущее)
	uint32_t exit_code; // TODO: сделать надо(в будущее)
	std::string output; // TODO: сделать надо(в будущее)
};
