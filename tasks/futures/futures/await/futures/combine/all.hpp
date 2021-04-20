#pragma once

#include <await/futures/core/promise.hpp>
#include <await/futures/combine/detail/combine.hpp>

#include <wheels/support/vector.hpp>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// All

namespace detail {

template <typename T>
class AllCombinator {};

}  // namespace detail

// std::vector<Future<T>> -> Future<std::vector<T>>
// All values | first error

template <typename T>
Future<std::vector<T>> All(std::vector<Future<T>> /*inputs*/) {
  return Future<std::vector<T>>::Invalid();  // Not implemented
}

template <typename T, typename... Fs>
Future<std::vector<T>> All(Future<T>&& first, Fs&&... rest) {
  return All(wheels::ToVector(std::move(first), std::forward<Fs>(rest)...));
}

}  // namespace await::futures
