#pragma once

#include <exe/executors/executor.hpp>

#include <cstdlib>

namespace exe::executors {

// Fixed-size pool of worker threads

class ThreadPool : public IExecutor {
 public:
  explicit ThreadPool(size_t threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // IExecutor
  void Execute(TaskBase* task) override;

  void WaitIdle();

  void Stop();

  static ThreadPool* Current();

 private:
  // Worker threads, task queue, etc
};

}  // namespace exe::executors
