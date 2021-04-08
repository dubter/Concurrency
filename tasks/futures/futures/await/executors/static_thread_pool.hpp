#pragma once

#include <await/executors/thread_pool.hpp>

namespace await::executors {

// Fixed-size pool of threads + unbounded blocking queue
IThreadPoolPtr MakeStaticThreadPool(size_t threads, const std::string& name);

}  // namespace await::executors
