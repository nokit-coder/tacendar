#include "daemon.h"
#include "action.h"
#include "dbManager.h"
#include "event.h"

#include <condition_variable>
#include <chrono>
#include <cstdint>
#include <cstdio>
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
	cv.notify_one();
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
				for (auto event : events) {
					std::println("check running event {}({}, {}) : \"{}\"", event->name, event->id, +event->status, event->description); // DEBUG
					std::vector<std::shared_ptr<Action>> actions;
					_db.fetch_actions(event->id, actions);
					for (auto action : actions) {
						if (action->status == Action::Status::STOPPED) {
							action->status = Action::Status::RUNNING;
							if (!_db.insert_action(*action, event->id)) {
								std::println("fuck... failed to update action: {}", _db.last_error());
								continue;
							}
							exec_action_in_thread(action, event->id);
						}
					}

					if (actions.back()->status == Action::Status::PAST || actions.empty()) {
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
			std::println(std::cerr, "failed to fetch next event: {}", _db.last_error());
			continue;
		}

		// ожидание
		if (event != nullptr) {
			std::println("wait start"); // DEBUG
			cv.wait_until(lock, event->start);
			std::println("wait end"); // DEBUG
		} else {
			// бесконечный сон
			std::println("sleep start"); // DEBUG
			cv.wait(lock);
			std::println("sleep end"); // DEBUG
			continue;
		}

		if (!_is_running) { break; }
		if (_db_changed)  { _db_changed = false; std::println("db changed"); continue; }


		// выполнение event
	 	std::println("check event {}({}, {}) : \"{}\"", event->name, event->id, +event->status, event->description); // DEBUG

		for (auto &action : event->actions) {
			action->print_debug(); // DEBUG

			if (action->status == Action::Status::PAST) { continue; }
			if (action->status == Action::Status::STOPPED) {
				printf("execute action\n"); // DEBUG
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
					action->status = Action::Status::STOPPED;
					std::println("fuck... failed to update action: {}", _db.last_error());
					continue;
				}
				exec_action_in_thread(action, event->id);
			}

			// if (event->status == Event::Status::RUNNING)
			// event->status = Event::Status::RUNNING;

			// break;
		}

		if (event->actions.empty() || event->actions.back()->status == Action::Status::PAST) {
			std::println("mark event as past"); // DEBUG
			event->status = Event::Status::PAST;
			if (_db.insert_event(*event)) {
				std::println("fuck... failed to update event: {}", _db.last_error());
			}
		}

		// std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}
}

void Daemon::watchdog() {
	// while (_is_running) {
	// 	std::vector<std::shared_ptr<Event>> db_events;
	// 	auto_fetch_events(_db, db_events);
	//
	// 	{
	// 		// анализ diff с памятью
	// 		std::lock_guard<std::mutex> lock(_mutex);
	//
	// 		_events.clear();
	//
	// 		for (auto &db_event : db_events) {
	// 			if (db_event->status == Event::Status::PAST) { continue; }
	//
	// 			_events.push_back(std::make_shared<Event>(*db_event));
	// 		}
	//
	// 		_db_changed = true;
	// 		cv.notify_one();
	//
	// 	}
	//
	// 	std::this_thread::sleep_for(std::chrono::seconds(5));
	// }
}

void Daemon::load_db() {
	// auto_fetch_events(_db, _events);
	//
	// //     FOR TESTING
	// if (!_events.empty()) {
	// 	_events.front()->start = std::chrono::system_clock::now() + std::chrono::seconds(5);
	// 	_events.front()->end = std::chrono::system_clock::now() + std::chrono::seconds(10);
	// 	_db.insert_event(*_events.front());
	// }
	// // END FOR TESTING
	//
	// std::sort(_events.begin(), _events.end());
}

void Daemon::exec_action_in_thread(std::shared_ptr<Action> action, const uint64_t event_id) {
	action_procs.enqueue([this, action, event_id]() {
		std::println("executing action in thread");

		action->exec();

		std::println("done executing action in thread");

		{
			std::lock_guard<std::mutex> lock(this->_mutex);

			action->status = Action::Status::PAST;

			if (!this->_db.insert_action(*action, event_id)) {
				std::println("fuck... failed to save action: {}", this->_db.last_error());
			}

			this->_db_changed = true;
		}

		this->cv.notify_all();
	});
}
