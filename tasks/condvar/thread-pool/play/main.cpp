#include <tp/thread_pool.hpp>

#include <iostream>

int main() {
  tp::ThreadPool pool{/*workers=*/4};

  pool.Submit([]() {
    std::cout << "Hi from pool" << std::endl;
  });

  pool.WaitIdle();
  pool.Stop();

  return 0;
}
