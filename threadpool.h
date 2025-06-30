#pragma once
#include <thread>
#include <functional>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <mutex>
// #include <lock

class ThreadPool {
public:
  ThreadPool(size_t pool_nums);
  void enqueue(std::function<void()> work);
  void shutdown();

private:
  void worker();
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> work_queues_;
  std::condition_variable cv_;
  std::atomic_bool stop_;
  std::mutex queue_mutex_;
};

ThreadPool::ThreadPool(size_t pool_nums) : stop_(false) {
  for (size_t i = 0; i < pool_nums; i ++) {
    workers_.emplace_back([this] { this->worker();});
  }
}

void ThreadPool::enqueue(std::function<void ()> work) {
  {
    std::scoped_lock<std::mutex> lock(queue_mutex_);
    work_queues_.emplace(work);
  }
  cv_.notify_one();
}

void ThreadPool::worker() {
  while (true) {
      std::function<void()> work;
      {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        cv_.wait(lock, [this] { return stop_ || !work_queues_.empty(); });
        if (stop_ && work_queues_.empty()) {
          return;
        }
        work = std::move(work_queues_.front());
        work_queues_.pop();
      }
      work();
  }
}