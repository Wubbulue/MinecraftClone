#include "ThreadPool.h"


ThreadPool& ThreadPool::shared_instance() {
	static ThreadPool pool(std::thread::hardware_concurrency());
	return pool;
}
