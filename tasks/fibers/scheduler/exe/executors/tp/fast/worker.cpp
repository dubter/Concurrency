#include <exe/executors/tp/fast/worker.hpp>
#include <exe/executors/tp/fast/thread_pool.hpp>

#include <twist/ed/lang/thread_local.hpp>

namespace exe::executors::tp::fast {

Worker::Worker(ThreadPool& host, size_t index)
    : host_(host), index_(index) {
}

void Worker::Start() {
  thread_.emplace([this]() {
    Work();
  });
}

void Worker::Join() {
  thread_->join();
}

void Worker::PushToLocalQueue(TaskBase* /*task*/) {
  // Not implemented
}

void Worker::PushToLifoSlot(TaskBase* /*task*/) {
  // Not implemented
}

TaskBase* Worker::PickNextTask() {
  // [Periodically] Global queue
  // 0) LIFO slot
  // 1) Local queue
  // 2) Global queue
  // 3) Work stealing
  // 4) Park worker

  return nullptr;  // Not implemented
}

void Worker::Work() {
  // Your code goes here
  while (TaskBase* next = PickNextTask()) {
    next->Run();
  }
}

}  // namespace exe::executors::tp::fast
