#include <cstdlib>
#include <iostream>
#include <memory>

#include "../include/event.h"
#include "../include/action.h"
#include "../include/dbManager.h"
#include "../include/daemon.h"

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
	// auto a = std::make_shared<ActionCommand>();
	// a->command = "notify-send 'Hello World!'";
	// e->actions.push_back(a);
	
	// Event e;
	// e.name = "test";
	// e.description = "test event";
	//
	// events.push_back(std::make_shared<Event>(e));

	// auto a = events[0]->actions[0];
	// a->exec();

	// --- write everything ---
	// auto_insert_events(db, events);
	// if_err();

	// [ TEST DAEMON ]
	Daemon d(db);
	d.load_db();
	d.daemon();
}
