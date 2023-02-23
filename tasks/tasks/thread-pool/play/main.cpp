#include <tp/thread_pool.hpp>
#include <tp/current.hpp>

#include <fmt/core.h>

int main() {
  tp::ThreadPool pool{/*workers=*/4};

  pool.Submit([] {
    fmt::println("Running in thread pool");
  });

  pool.WaitIdle();
  pool.Stop();

  return 0;
}
