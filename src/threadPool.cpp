#include "threadPool.h"

ThreadPool::ThreadPool(size_t threadsCount) {
	for (size_t i = 0; i < threadsCount; ++i) {
		threads.emplace_back([this]() {
				while (true) {
				std::function<void()> task;
				{
				std::unique_lock<std::mutex> lock(this->mutex);

				this->cv.wait(lock, [this] { 
						return this->stop || !this->tasks.empty(); 
						});

				if (this->stop) {
				return;
				}

				task = std::move(this->tasks.front());
				this->tasks.pop();
				}

				task(); 
				}
				});
	}
}

void ThreadPool::enqueue(std::function<void()> task) {
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (stop) return;
		tasks.push(std::move(task));
	}
	cv.notify_one();
}

ThreadPool::~ThreadPool() {
	{
		std::lock_guard<std::mutex> lock(mutex);
		stop = true;
	}
	cv.notify_all();

	threads.clear(); 
}
