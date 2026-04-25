#include "event.h"
#include "dbManager.h"
#include <iostream>

int main() {
	SqliteDb db("db.db");

	std::cout << db.create_tables() << '\n';
	std::cout << "last error: <" << db.last_error() << ">" << '\n';

	// Task t;
	// t.command = "notify-send asd";

	// Event e;
	// e.name = "asd";
	// e.description = "descriptionnnnn";
	// e.tasks.push_back(t);
	// e.exec();

	// db.insert_event(e);
	// std::cout << "event id: " << e.id << '\n';
	
	std::vector<Event> events = db.fetch_events();
	for (auto e : events) {
		std::cout << "name: " << e.name << '\n';
		std::cout << "desc: " << e.description << '\n';
	}

	std::cout << "last error: <" << db.last_error() << ">" << '\n';
}
