#include "../include/action.h"

#include <cstdio>

void Action::exec() {
	status = Status::RUNNING;

	FILE* pipe = popen((command + " 2>/dev/null").c_str(), "r");

	output.clear();
	char buffer[128];
	while (fgets(buffer, sizeof(buffer), pipe)) output += buffer;

	exit_code = WEXITSTATUS(pclose(pipe));

  status = Status::PAST;
}

int operator+(Action::Status s) {
	return static_cast<int>(s);
}
