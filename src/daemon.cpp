#include "../include/daemon.h"
#include "../include/dbManager.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <thread>

Daemon::Daemon(SqliteDb &db) : _db(db) {}

void Daemon::daemon() {
	load_db();

	int timer = 0;

	while (true) {
		auto current_time = std::chrono::system_clock::now();

		for (auto &event : events) {
			printf("check event %s(%i, %i) : \"%s\"\n", event.name.c_str(), event.id, +event.status, event.description.c_str());

			// рассматриваем не прошедшие события
			if (event.status == Event::Status::PAST) {
				continue; 
			}

			// пропускаем все будущие события
			bool time_to_start = (current_time >= event.start && (current_time - event.start) <= std::chrono::seconds(10));
			bool is_running = (event.status == Event::Status::RUNNING);

			if (!time_to_start && !is_running) {
				break; 
			}

			// основная логика
			if (event.status == Event::Status::STOPPED) {
				printf("execute event\n");
				event.exec();
				timer = 0;
			}

			// выполняем задачи внутри текущего активного события
			for (auto &action : event.actions) {
				printf("check actions %lu(%i) : \"%s\" : %i : \"%s\"\n", action.id, action.status, action.command.c_str(), action.exit_code, action.output.c_str());

				if (action.status == Action::Status::PAST) { continue; }
				if (action.status == Action::Status::STOPPED) {
					printf("execute action\n");
					exec_action_in_thread(action);
					event.status = Event::Status::RUNNING;
					timer = 0;
				}

				break;
			}

			// проверяем завершение всех задач в событии
			if (!event.actions.empty() && event.actions.back().status == Action::Status::PAST) {
				printf("mark event as past\n");
				event.status = Event::Status::PAST;
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
	events = auto_fetch_events(_db);

	//     FOR TESTING
	if (!events.empty()) {
		events.front().start = std::chrono::system_clock::now() + std::chrono::seconds(10);
	}
	// END FOR TESTING

	std::sort(events.begin(), events.end());
}

void exec_action_in_thread(Action &action) {
	std::thread th(&Action::exec, &action);
	th.detach();
}
