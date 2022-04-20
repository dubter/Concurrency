#include <exe/executors/thread_pool.hpp>
#include <exe/executors/strand.hpp>
#include <exe/executors/execute.hpp>

#include <iostream>

using namespace exe::executors;

int main() {
  ThreadPool pool{4};
  Strand strand{pool};

  size_t cs = 0;

  for (size_t i = 0; i < 12345; ++i) {
    Execute(strand, [&cs]() {
      ++cs;  // <-- Critical section
    });
  }

  pool.WaitIdle();

  std::cout << "# critical sections: " << cs << std::endl;

  pool.Stop();
  return 0;
}
