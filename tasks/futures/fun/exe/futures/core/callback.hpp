#pragma once

#include <wheels/result/result.hpp>

#include <function2/function2.hpp>

namespace exe::futures {

// Intrusive?
// Asynchronous callback
template <typename T>
using Callback = fu2::unique_function<void(wheels::Result<T>)>;

}  // namespace exe::futures
