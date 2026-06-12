#include "../include/daemon.h"
#include "../include/dbManager.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <thread>

Daemon::Daemon(SqliteDb &db) : _db(db) {}

void Daemon::daemon() {
	load_db();

	int timer = 0;

	while (true) {
		auto current_time = std::chrono::system_clock::now();

		for (auto &event : events) {
			printf("check event %s(%lu, %i) : \"%s\"\n", event->name.c_str(), event->id, +event->status, event->description.c_str());

			// рассматриваем не прошедшие event
			if (event->status == Event::Status::PAST) {
				continue; 
			}

			// пропускаем все будущие event и те которым уже поздно выполняться
			bool time_to_start = (current_time >= event->start && (current_time - event->start) <= std::chrono::seconds(10));
			bool is_running = (event->status == Event::Status::RUNNING);

			if (!time_to_start && !is_running) {
				break; 
			}

			// основная логика
			if (event->status == Event::Status::STOPPED) {
				printf("execute event\n");
			}

			// выполняем action внутри текущего активного event
			for (auto &action : event->actions) {
				action->print_debug();

				event->status = Event::Status::RUNNING;

				if (action->status == Action::Status::PAST) { continue; }
				if (action->status == Action::Status::STOPPED) {
					printf("execute action\n");
					exec_action_in_thread(*action, event->id);
					_db.insert_event(*event);
				}

				timer = 0;

				break;
			}

			// проверяем завершение всех action в событии
			if (!event->actions.empty() && event->actions.back()->status == Action::Status::PAST) {
				printf("mark event as past\n");
				event->status = Event::Status::PAST;
				auto_insert_event(_db, *event);
			}		
		}

		if (timer < steps[std::size(steps) - 1].max_timer) { ++timer; }

		int sleep_ms = steps[std::size(steps) - 1].ms;

		for (const auto& step : steps) {
			if (timer <= step.max_timer) {
				sleep_ms = step.ms;
				break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
	}
}

void Daemon::load_db() {
	auto_fetch_events(_db, events);

	//     FOR TESTING
	if (!events.empty()) {
		events.front()->start = std::chrono::system_clock::now() + std::chrono::seconds(10);
	}
	// END FOR TESTING

	std::sort(events.begin(), events.end());
}

void Daemon::exec_action_in_thread(Action &action, const uint64_t event_id) {
	std::thread th(
			[&action, event_id, this]() {
			action.status = Action::Status::RUNNING;
			action.exec();
			action.status = Action::Status::PAST;
			_db.insert_action(action, event_id);
			});
	th.detach();
}
