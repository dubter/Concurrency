#include <mtf/fibers/api.hpp>

#include <mtf/coroutine/impl.hpp>
#include <mtf/fibers/stacks.hpp>

namespace mtf::fibers {

using coroutine::impl::Coroutine;

////////////////////////////////////////////////////////////////////////////////

class Fiber {
 public:
  Fiber(Routine /*routine*/, Scheduler& /*scheduler*/) {
  }

 private:
  // Similar to those in the TinyFibers scheduler

  void Schedule() {
    // Not implemented
  }

  void Dispatch() {
    // Not implemented
  }

  void Destroy() {
    // Not implemented
  }

 private:
  // ???
};

////////////////////////////////////////////////////////////////////////////////

void Spawn(Routine /*routine*/, Scheduler& /*scheduler*/) {
  // Not implemented
}

void Spawn(Routine /*routine*/) {
  // Not implemented
}

void Yield() {
  // Not implemented
}

}  // namespace mtf::fibers
