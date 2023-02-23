#pragma once

#include <exe/executors/executor.hpp>
#include <exe/futures/core/future.hpp>

namespace exe::futures {

// Usage:
// auto f = futures::Execute(pool, []() -> int {
//   return 42;  // <-- Computation runs in provided executor
// });

template <typename F>
auto Execute(executors::IExecutor& /*where*/, F /*func*/) {
  using T = std::invoke_result_t<F>;

  return Future<T>::Invalid();  // Not implemented
}

}  // namespace exe::futures
