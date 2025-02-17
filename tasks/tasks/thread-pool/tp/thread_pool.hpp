#pragma once

#include <tp/queue.hpp>
#include <tp/task.hpp>
#include <tp/wait_group.hpp>
#include <list>
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

  // Launches worker threads
  void Start();

  // Schedules task for execution in one of the worker threads
  void Submit(Task);

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

  // Waits until outstanding work count reaches zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  void Stop();

 private:
  void Work();

 private:
  size_t num_threads_{twist::ed::stdlike::thread::hardware_concurrency()};
  detail::WaitGroup wait_group_;
  UnboundedBlockingQueue<Task> blocking_queue_;
  std::list<twist::ed::stdlike::thread> workers_;
};

}  // namespace tp
