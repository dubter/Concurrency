#pragma once

#include <exe/executors/task.hpp>

#include <exe/executors/ftp/metrics.hpp>
#include <exe/executors/ftp/queues/work_stealing_queue.hpp>

#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/thread.hpp>

#include <cstdlib>
#include <optional>
#include <random>
#include <span>

namespace exe::executors::ftp {

class FastThreadPool;

class Worker {
 private:
  static const size_t kLocalQueueCapacity = 256;

 public:
  Worker(FastThreadPool& host, size_t index);

  void Start();
  void Join();

  // Submit task
  void PushToLocalQueue(TaskBase* task);
  void PushToLifoSlot(TaskBase* task);

  // Steal from this worker
  size_t StealTasks(std::span<TaskBase*> out_buffer);

  // Wake parked worker
  void Wake();

  static Worker* Current();

  WorkerMetrics Metrics() const {
    return metrics_;
  }

  FastThreadPool& Host() const {
    return host_;
  }

 private:
  void OffloadTasksToGlobalQueue(TaskBase* overflow);

  TaskBase* TryStealTasks(size_t series);
  TaskBase* GrabTasksFromGlobalQueue();
  TaskBase* TryPickNextTask();
  TaskBase* TryPickTaskBeforePark();

  // Blocking
  TaskBase* PickNextTask();

  void Work();

 private:
  FastThreadPool& host_;
  const size_t index_;

  // Worker thread
  std::optional<twist::stdlike::thread> thread_;

  // Local queue
  WorkStealingQueue<kLocalQueueCapacity> local_tasks_;

  // For work stealing
  std::mt19937_64 twister_;

  // LIFO slot
  TaskBase* lifo_slot_;

  // Parking lot
  twist::stdlike::atomic<uint32_t> wakeups_{0};

  WorkerMetrics metrics_;
};

}  // namespace exe::executors::ftp
