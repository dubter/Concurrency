#include <exe/executors/thread_pool.hpp>

#include <exe/tasks/run/fire.hpp>
#include <exe/tasks/run/teleport.hpp>
#include <exe/tasks/run/yield.hpp>

#include <exe/tasks/sync/mutex.hpp>
#include <exe/tasks/sync/wait_group.hpp>

#include <wheels/core/defer.hpp>

#include <iostream>

using namespace exe;

int main() {
  executors::ThreadPool scheduler{4};

  auto main = [&]() -> tasks::Task<> {
    co_await tasks::TeleportTo(scheduler);

    tasks::Mutex mutex;
    size_t cs = 0;

    tasks::WaitGroup wg;

    auto contender = [&]() -> tasks::Task<> {
      co_await tasks::TeleportTo(scheduler);

      wheels::Defer defer([&wg]() {
        wg.Done();
      });

      for (size_t j = 0; j < 100'000; ++j) {
        auto lock = co_await mutex.ScopedLock();
        // Critical section
        ++cs;
      }
    };

    wg.Add(17);
    for (size_t i = 0; i < 17; ++i) {
      tasks::FireAndForget(contender());
    }

    co_await wg.Wait();

    std::cout << "# critical sections = " << cs << std::endl;
  };

  tasks::FireAndForget(main());

  scheduler.WaitIdle();

  scheduler.Stop();

  return 0;
}
