#pragma once

#include <exe/coroutine/routine.hpp>
#include <exe/tp/thread_pool.hpp>

namespace exe::fibers {

using Routine = coroutine::Routine;

using Scheduler = tp::ThreadPool;

// Considered harmful

void Go(Routine routine);

void Go(Scheduler& scheduler, Routine routine);

namespace self {

void Yield();

}  // namespace self

}  // namespace exe::fibers
