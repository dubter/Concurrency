#include <coro/coroutine.hpp>

#include <fmt/core.h>

using namespace coro;

int main() {

  Coroutine coro([] {
    Coroutine::Suspend();
  });


  return 0;
}
