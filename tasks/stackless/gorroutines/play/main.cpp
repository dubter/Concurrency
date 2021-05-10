#include <gorr/runtime/thread_pool.hpp>
#include <gorr/runtime/yield.hpp>
#include <gorr/runtime/mutex.hpp>
#include <gorr/runtime/join.hpp>

#include <iostream>

gorr::StaticThreadPool scheduler{/*threads=*/4};

gorr::JoinHandle GorRoutine() {
  co_await scheduler.Schedule();
  std::cout << "Hello, world!" << std::endl;
  co_return;
};

int main() {
  gorr::Detach(GorRoutine());

  scheduler.Join();
  return 0;
}
