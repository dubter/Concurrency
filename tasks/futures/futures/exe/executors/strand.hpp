#pragma once

#include <exe/executors/executor.hpp>

namespace exe::executors {

// Strand (serial executor, asynchronous mutex)
// Executes (via underlying executor) tasks
// non-concurrently and in FIFO order

class Strand : public IExecutor {
 public:
  explicit Strand(IExecutor& underlying);

  // IExecutor
  void Execute(Task task) override;

 private:
  // ???
};

}  // namespace exe::executors
