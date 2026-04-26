#include "event.h"
#include "dbManager.h"
#include <iostream>

int main() {
	SqliteDb db("db.db");

	std::cout << db.create_tables() << '\n';
	std::cout << "last error: <" << db.last_error() << ">" << '\n';

	// Task t;
	// t.command = "notify-send asd";
	//
	// Event e;
	// e.name = "asd";
	// e.description = "descriptionnnnn";
	// e.tasks.push_back(t);
	// e.exec();
	//
	// db.insert_event(e);
	// std::cout << "event id: " << e.id << '\n';
	//
	// for (auto i : e.tasks) {
	// 	db.insert_task(i, e.id);
	// 	std::cout << "event id: " << e.id << '\n';
	// }
	
	std::vector<Event> events = db.fetch_events();
	if (!db.last_error().empty()) {
		std::cout << "last error: <" << db.last_error() << ">" << '\n';
		return 0;
	}

	std::cout << "-----" << '\n';
	for (auto& e : events) {
		std::cout << "id: " << e.id << '\n';
		std::cout << "name: " << e.name << '\n';
		std::cout << "desc: " << e.description << '\n';
		e.tasks = db.fetch_tasks(e.id);
		for (auto& t : e.tasks) {
			std::cout << "id: " << t.id << '\n';
			std::cout << "command: " << t.command << '\n';
		}
			std::cout << "-----" << '\n';
	}
	// auto tasks = db.fetch_tasks();
	// for (auto& t : tasks) {
	// 	std::cout << "event_id: " << t.first << '\n';
	// 	std::cout << "command: " << t.second.command << '\n';
	// }

	std::cout << "last error: <" << db.last_error() << ">" << '\n';
}
