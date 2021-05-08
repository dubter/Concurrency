#pragma once

#include <task/task.hpp>

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

namespace task {

namespace detail {

}  // namespace detail

template <typename T>
T Await(Task<T> /*task*/) {
  std::abort();  // Not implemented
}

}  // namespace task
