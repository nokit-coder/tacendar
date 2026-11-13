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
	ThreadPool(size_t threadsCount = std::thread::hardware_concurrency());

	void enqueue(std::function<void()> task);

	~ThreadPool();
};
