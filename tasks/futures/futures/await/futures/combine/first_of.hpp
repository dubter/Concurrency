#pragma once

#include <await/futures/core/promise.hpp>
#include <await/futures/combine/detail/combine.hpp>

#include <wheels/support/vector.hpp>

namespace await::futures {

//////////////////////////////////////////////////////////////////////

// FirstOf

namespace detail {

template <typename T>
class FirstOfCombinator {};

}  // namespace detail

// std::vector<Future<T>> -> Future<T>
// First value | last error
// Returns invalid future on empty input

template <typename T>
Future<T> FirstOf(std::vector<Future<T>> /*inputs*/) {
  return Future<T>::Invalid();
}

template <typename T, typename... Fs>
auto FirstOf(Future<T>&& first, Fs&&... rest) {
  return FirstOf(wheels::ToVector(std::move(first), std::forward<Fs>(rest)...));
}

}  // namespace await::futures
