#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
private:
	std::vector<std::jthread> threads;
	std::queue<std::function<void()>> tasks;

	std::mutex mutex;
	std::condition_variable cv;
	bool stop = false;

public:
	ThreadPool(size_t threadsCount = std::thread::hardware_concurrency()) {
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

	void enqueue(std::function<void()> task) {
		{
			std::lock_guard<std::mutex> lock(mutex);
			if (stop) return;
			tasks.push(std::move(task));
		}
		cv.notify_one();
	}

	~ThreadPool() {
		{
			std::lock_guard<std::mutex> lock(mutex);
			stop = true;
		}
		cv.notify_all();

		threads.clear(); 
	}
};
