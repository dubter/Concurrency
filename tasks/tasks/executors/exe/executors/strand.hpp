#pragma once

#include <exe/executors/executor.hpp>

namespace exe::executors {

// Strand (serial executor, asynchronous mutex) executes
// tasks (critical sections) non-concurrently and in FIFO order
// via underlying executor


class Strand : public IExecutor {
 public:
  explicit Strand(IExecutor& underlying);

  // Non-copyable
  Strand(const Strand&) = delete;
  Strand& operator=(const Strand&) = delete;

  // Non-movable
  Strand(Strand&&) = delete;
  Strand& operator=(Strand&&) = delete;

  // IExecutor
  void Submit(Task cs) override;

 private:
  // ???
};

}  // namespace exe::executors
