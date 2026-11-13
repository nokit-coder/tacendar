#include "daemon.h"
#include "action.h"
#include "dbManager.h"
#include "event.h"

#include <condition_variable>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <print>
#include <vector>

Daemon::Daemon(SqliteDb &db) : _db(db) {}

Daemon::~Daemon() {
	stop_daemon();
}

void Daemon::notify() {
	_cv.notify_all();
}

bool Daemon::start_daemon() {
	if (_is_running == true) { return false; }
	std::println(std::cout, "starting daemon");
	_is_running = true;
	daemon_proc = std::jthread(&Daemon::daemon, this);
	watchdog_proc = std::jthread(&Daemon::watchdog, this);
	return true;
}

bool Daemon::stop_daemon() {
	if (_is_running == false) { return false; }
	std::println(std::cout, "stopping daemon");
	_is_running = false;
	_cv.notify_one();
	if (daemon_proc.joinable()) { daemon_proc.join(); }
	if (watchdog_proc.joinable()) { watchdog_proc.join(); }
	return true;
}

void Daemon::daemon() {
	while (_is_running) {
		// если база проблема с action и база не изменилась скипаем
		// поиск ближайшего event

		// ожидание
		// если прерывание
		// - база изменилась
		// - завершение демона
		// - action завершился
		
		// обработка event

		if (!_is_running) { break; }
		if (_db_changed) { _db_changed = false; std::println("db changed"); }

		auto now = std::chrono::system_clock::now();

		std::unique_lock<std::mutex> lock(_mutex);

		// обработка всех running event
		{
			std::vector<std::shared_ptr<Event>> events;
			if (_db.fetch_running_events(events)) {
				for (const auto& event : events) {
					std::println("check running event {}({}, {}) : \"{}\"", event->name, event->id, +event->status, event->description); // DEBUG
					_db.fetch_actions(event->id, event->actions);
					for (const auto& action : event->actions) {
						if (action->status == Action::Status::STOPPED) {
							action->status = Action::Status::RUNNING;
							if (!_db.insert_action(*action, event->id)) {
								std::println("fuck... failed to update action: {}", _db.last_error());
								continue;
							}
							exec_action_in_thread(action, event->id);
						}
					}

					if (event->actions.empty() || event->actions.back()->status == Action::Status::PAST) {
						std::println("mark event as past"); // DEBUG
						event->status = Event::Status::PAST;
						if (!_db.insert_event(*event)) {
							std::println("fuck... failed to update event: {}", _db.last_error());
						}
					}
				}
			}
		}

		// поиск ближайшего event дял обработки
		std::shared_ptr<Event> event;
		if (!_db.fetch_next_event(event, to_unix_seconds(now))) {
			std::println("failed to fetch next event: {}", _db.last_error());
		}

		if (event != nullptr) {
			_db.fetch_actions(event->id, event->actions);
		}

		// ожидание
		if (event != nullptr) {
			std::println("wait start"); // DEBUG
			_cv.wait_until(lock, event->start);
			std::println("wait end"); // DEBUG
		} else {
			// бесконечный сон
			std::println("sleep start"); // DEBUG
			_cv.wait(lock);
			std::println("sleep end"); // DEBUG
			continue;
		}

		if (!_is_running) { break; }
		if (_db_changed)  { continue; }

		if (event == nullptr) { continue; }

		// выполнение event
	 	std::println("check event {}({}, {}) : \"{}\"", event->name, event->id, +event->status, event->description); // DEBUG

		for (const auto &action : event->actions) {
			action->print_debug(); // DEBUG

			if (action->status != Action::Status::STOPPED) { continue; }
			std::println("execute action\n"); // DEBUG

			// сохраняем в бд
			if (event->status != Event::Status::RUNNING) {
				event->status = Event::Status::RUNNING;
				if (!_db.insert_event(*event)) {
					std::println("fuck... failed to update event: {}", _db.last_error());
					continue;
				}
			}

			action->status = Action::Status::RUNNING;
			if (!_db.insert_action(*action, event->id)) {
				std::println("fuck... failed to update action: {}", _db.last_error());
				continue;
			}
			exec_action_in_thread(action, event->id);

			// break;
		}
	}
}

void Daemon::watchdog() {
}

void Daemon::exec_action_in_thread(std::shared_ptr<Action> action, const uint64_t event_id) {
	action_procs.enqueue([this, action, event_id]() {
		std::println("executing action in thread"); // DEBUG

		action->exec();

		std::println("done executing action in thread"); // DEBUG

		{
			std::lock_guard<std::mutex> lock(this->_mutex);

			action->status = Action::Status::PAST;

			if (!this->_db.insert_action(*action, event_id)) {
				std::println("fuck... failed to save action: {}", this->_db.last_error());
			}

			this->_db_changed = true;
		}

		this->_cv.notify_all();
	});
}
