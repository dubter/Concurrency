#include <exe/fibers/core/api.hpp>

#include <fmt/core.h>

using namespace exe;
using namespace std::chrono_literals;

int main() {
  asio::io_context scheduler;

  fibers::Go(scheduler, []() {
    for (size_t i = 0; i < 10; ++i) {
      fmt::println("Step {}", i);
      fibers::self::SleepFor(256ms);
    }
  });

  scheduler.run();

  return 0;
}
