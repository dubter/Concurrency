#pragma once

#include <exe/futures/core/future.hpp>
#include <exe/futures/combine/detail/combine.hpp>
#include <exe/futures/util/just.hpp>

#include <wheels/support/vector.hpp>

namespace exe::futures {

namespace detail {

template <typename T>
class AllCombinator {
 public:
  explicit AllCombinator(size_t /*inputs*/) {
  }

  auto MakeFuture() {
    return Future<std::vector<T>>::Invalid();
  }
};

}  // namespace detail

// All combinator
// All values or first error
// std::vector<Future<T>> -> Future<std::vector<T>>

template <typename T>
Future<std::vector<T>> All(std::vector<Future<T>> inputs) {
  if (inputs.empty()) {
    return JustValue<std::vector<T>>({});
  }
  return detail::Combine<detail::AllCombinator<T>>(std::move(inputs));
}

// Usage:
// auto all = futures::All(std::move(f1), std::move(f2));

template <typename T, typename... Fs>
Future<std::vector<T>> All(Future<T>&& first, Fs&&... rest) {
  return All(wheels::ToVector(std::move(first), std::forward<Fs>(rest)...));
}

}  // namespace exe::futures
