#include "../include/task.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>

void Task::exec() {
	status = RUNNING;

	FILE* pipe = popen(command.c_str(), "r");

	char buffer[128];
	while (fgets(buffer, sizeof(buffer), pipe)) output += buffer;

	exit_code = WEXITSTATUS(pclose(pipe));

  status = STOPPED;
  std::cout << exit_code << " << out\n";
}

std::thread exec_task_thread(Task &t) {
  std::thread th(&Task::exec, &t);
	th.detach();
  return th;
}
