#pragma once

#include <tp/queue.hpp>
#include <tp/task.hpp>

#include <twist/ed/stdlike/thread.hpp>

namespace tp {

// Fixed-size pool of worker threads

class ThreadPool {
 public:
  explicit ThreadPool(size_t threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Non-movable
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  // Schedules task for execution in one of the worker threads
  void Submit(Task);

  // Waits until outstanding work count has reached zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  void Stop();

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

 private:
  // Worker threads, task queue, etc
};

}  // namespace tp
