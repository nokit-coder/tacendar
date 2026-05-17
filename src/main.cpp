#include "../include/event.h"
#include "../include/task.h"
#include "../include/dbManager.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <thread>

int main() {
	SqliteDb db("db.db");

	// std::thread a();

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
	
	// std::vector<Event> events = db.fetch_events();
	// if (!db.last_error().empty()) {
	// 	std::cout << "last error: <" << db.last_error() << ">" << '\n';
	// 	return 0;
	// }
	//
	// std::cout << "-----" << '\n';
	// for (auto& e : events) {
	// 	std::cout << "id: " << e.id << '\n';
	// 	std::cout << "name: " << e.name << '\n';
	// 	std::cout << "desc: " << e.description << '\n';
	// 	e.tasks = db.fetch_tasks(e.id);
	// 	for (auto& t : e.tasks) {
	// 		std::cout << "id: " << t.id << '\n';
	// 		std::cout << "command: " << t.command << '\n';
	// 		t.exec();
	// 	}
	// 		std::cout << "-----" << '\n';
	// }
	
	// --- load everything ---

	std::vector<Event> events = auto_fetch_events(db);
	if (!db.last_error().empty()) {
		std::cout << "last error: <" << db.last_error() << ">" << '\n';
		return 0;
	}

	// --- print info ---

	std::cout << "-----" << '\n';
	for (auto& e : events) {
		std::cout << "id: " << e.id << '\n';
		std::cout << "name: " << e.name << '\n';
		std::cout << "desc: " << e.description << '\n';
		for (auto& t : e.tasks) {
			std::cout << "id: " << t.id << '\n';
			std::cout << "command: " << t.command << '\n';
			exec_task_thread(t);
		}
		std::cout << "-----" << '\n';
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(5));

	// --- wait for all task stop ---
	bool f = true;
	while (f) {
		f = false;
		for (auto &e : events) {
			for (auto &t : e.tasks) {
				if (t.status == TaskStatus::RUNNING) {
					f = true; 
					std::printf("running fignya '%s'\n", t.command.c_str());
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	// --- write everything ---
	auto_insert_events(db, events);

	if (!db.last_error().empty()) {
		std::cout << "last error: <" << db.last_error() << ">" << '\n';
		return 0;
	}
}
