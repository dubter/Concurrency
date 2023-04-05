#include <exe/fibers/sched/yield.hpp>

namespace exe::fibers {

void Yield() {
  Fiber::Self()->Yield();
}

}  // namespace exe::fibers
