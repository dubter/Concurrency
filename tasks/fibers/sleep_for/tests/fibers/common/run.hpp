#pragma once

#include <exe/fibers/core/api.hpp>

#include <asio.hpp>

#include <atomic>
#include <vector>
#include <thread>

template <typename F>
size_t RunScheduler(size_t threads, F init) {
  // I/O scheduler
  asio::io_context scheduler;

  // Spawn initial fiber

  bool done = false;

  exe::fibers::Go(scheduler, [init, &done]() {
    init();
    done = true;
  });

  // Run event loop

  std::vector<std::thread> runners;

  std::atomic<size_t> total_handlers{0};

  for (size_t i = 0; i < threads; ++i) {
    runners.emplace_back([&scheduler, &total_handlers]() {
      size_t count = scheduler.run();
      total_handlers.fetch_add(count);
    });
  }
  // Join runners
  for (auto& t : runners) {
    t.join();
  }

  ASSERT_TRUE(done);

  return total_handlers.load();
}
