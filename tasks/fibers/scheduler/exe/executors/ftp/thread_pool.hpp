#pragma once

#include <exe/executors/executor.hpp>

#include <exe/executors/ftp/queues/global_queue.hpp>
#include <exe/executors/ftp/worker.hpp>
#include <exe/executors/ftp/coordinator.hpp>
#include <exe/executors/ftp/metrics.hpp>

// random_device
#include <twist/stdlike/random.hpp>

#include <deque>

namespace exe::executors::ftp {

// Scalable work-stealing scheduler for short-lived tasks

class FastThreadPool : public IExecutor {
  friend class Worker;

 public:
  explicit FastThreadPool(size_t threads);
  ~FastThreadPool();

  // Non-copyable
  FastThreadPool(const FastThreadPool&) = delete;
  FastThreadPool& operator=(const FastThreadPool&) = delete;

  // IExecutor
  void Execute(TaskBase* task) override;

  void WaitIdle();

  void Stop();

  // After Stop
  PoolMetrics Metrics() const;

  static FastThreadPool* Current();

 private:
  std::deque<Worker> workers_;
  Coordinator coordinator_;
  GlobalQueue global_tasks_;
  // ???
};

}  // namespace exe::executors::ftp
