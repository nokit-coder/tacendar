#include <iostream>

#include "../include/event.h"
#include "../include/action.h"
#include "../include/dbManager.h"
#include "../include/daemon.h"

int main() {
	SqliteDb db("db.db");

	std::cout << db.create_tables() << '\n';
	std::cout << "last error: <" << db.last_error() << ">" << '\n';

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
		for (auto& t : e.actions) {
			std::cout << "id: " << t.id << '\n';
			std::cout << "command: " << t.command << '\n';
		}
		std::cout << "-----" << '\n';
	}

	// --- write everything ---
	/*
	auto_insert_events(db, events);

	if (!db.last_error().empty()) {
		std::cout << "last error: <" << db.last_error() << ">" << '\n';
		return 0;
	}
	*/

	// [ TEST DAEMON ]
	Daemon d(db);
	d.load_db();
	d.daemon();
}
