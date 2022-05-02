#pragma once

#include <exe/futures/core/future.hpp>

namespace exe::futures {

namespace detail {

// Your code goes here

}  // namespace detail

// ~ std::future::get
// Blocking
template <typename T>
wheels::Result<T> GetResult(Future<T> /*future*/) {
  return Future<T>::Invalid();  // Not implemented
}

// Blocking
template <typename T>
T GetValue(Future<T> future) {
  return GetResult(std::move(future)).ValueOrThrow();
}

}  // namespace exe::futures
