#include <exe/fibers/sched/go.hpp>

namespace exe::fibers {

void Go(Scheduler& scheduler, Routine routine) {
  (new Fiber(scheduler, std::move(routine)))->Schedule();
}

void Go(Routine routine) {
  (new Fiber(Fiber::GetScheduler(), std::move(routine)))->Schedule();
}

}  // namespace exe::fibers
