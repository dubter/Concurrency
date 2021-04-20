#pragma once

#include <await/executors/executor.hpp>
#include <await/futures/core/promise.hpp>

#include <wheels/support/result.hpp>

namespace await::futures {

// Execute callable object `target` via executor `e`
// and return future
//
// Usage:
// auto tp = MakeStaticThreadPool(4, "tp");
// auto future = AsyncVia(tp, []() { return 42; });;

template <typename F>
auto AsyncVia(executors::IExecutorPtr, F&& target) {
  using T = decltype(target());

  return Future<T>::Invalid();  // Not implemented
  // Use MakeContract, wheels::make_result::Invoke
}

}  // namespace await::futures
