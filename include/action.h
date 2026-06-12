#pragma once

#include <cstdint>
#include <string>
#include <sqlite3.h>

enum class ActionType {
	Base,
	Command,
	Notify, // future
};

struct Action {
	enum class Status {
		STOPPED,
		RUNNING,
		PAST,
	};

	uint64_t id = 0; // 0 -> undef
	ActionType type;
	Status status = Status::STOPPED;

	Action(const ActionType &t) : type(t) {}
	virtual ~Action() = default;

	virtual void exec() = 0;
	virtual void save_to_db(sqlite3_stmt* stmt) const;
	virtual void load_from_db(sqlite3_stmt* stmt);
	virtual void print_debug();
};

int operator+(Action::Status s);
int operator+(ActionType t);

struct ActionCommand : Action {
	ActionCommand();

	virtual void exec();
	void save_to_db(sqlite3_stmt* stmt) const;
	void load_from_db(sqlite3_stmt* stmt);
	void print_debug();

	std::string command;
	uint16_t exit_code = 0;
	std::string output;
};
