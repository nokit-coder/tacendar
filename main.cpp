#include "event.h"

int main() {
	Task t;
	t.command = "notify-send asd";
	Event e;
	e.tasks.push_back(t);
	e.exec();
}
