#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <print>

#include "event.h"
#include "action.h"
#include "dbManager.h"
#include "daemon.h"

SqliteDb db("db.db");

void if_err() {
	if (!db.last_error().empty()) {
		std::cout << "last error: <" << db.last_error() << ">" << '\n';
		exit(0);
	}
}

int main() {
	std::cout << db.create_tables() << '\n';
	if_err();

	// --- load everything ---

	// std::vector<std::shared_ptr<Event>> events;
	// auto_fetch_events(db, events);
	// if_err();

	// --- print info ---

	// std::cout << "----------" << '\n';
	// for (auto& e : events) {
	// 	std::cout << "id: " << e->id << '\n';
	// 	std::cout << "name: " << e->name << '\n';
	// 	std::cout << "desc: " << e->description << '\n';
	// 	for (auto& t : e->actions) {
	// 		std::cout << "id: " << t->id << '\n';
	// 		std::cout << "status: " << +t->status << '\n';
	// 	}
	// 	std::cout << "----------" << '\n';
	// }

	// auto e = events[0];
	Event e;
	auto a = std::make_shared<ActionCommand>();
	a->command = "notify-send 'Hello World!'";
	e.actions.push_back(a);
	// auto_insert_event(db, e);
	
	// Event e;
	// e.name = "test";
	// e.description = "test event";
	//
	// events.push_back(std::make_shared<Event>(e));

	// auto a = events[0]->actions[0];
	// a->status = Action::Status::STOPPED;

	// --- write everything ---
	// auto_insert_events(db, events);
	// if_err();

	// [ TEST DAEMON ]
	Daemon d(db);
	// std::this_thread::sleep_for(std::chrono::seconds(1));
	d.start_daemon();

	std::this_thread::sleep_for(std::chrono::seconds(2));

	std::println("e1");
	e.start = std::chrono::system_clock::now() + std::chrono::seconds(4);
	e.end = std::chrono::system_clock::now() + std::chrono::seconds(9);
	auto_insert_event(db, e);

	std::println("notify");
	d.notify();
	
	// std::this_thread::sleep_for(std::chrono::seconds(10));
	//
	// std::println("e2");
	// e.id = 0;
	// e.start = std::chrono::system_clock::now() + std::chrono::seconds(5);
	// e.end = std::chrono::system_clock::now() + std::chrono::seconds(10);
	// auto_insert_event(db, e);
	//
	// std::this_thread::sleep_for(std::chrono::seconds(10));
	//
	// std::println("e3");
	// e.start = std::chrono::system_clock::now() + std::chrono::seconds(5);
	// e.end = std::chrono::system_clock::now() + std::chrono::seconds(10);
	// auto_insert_event(db, e);

	std::this_thread::sleep_for(std::chrono::seconds(10));
}
