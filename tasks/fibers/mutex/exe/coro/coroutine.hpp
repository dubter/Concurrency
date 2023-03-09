#include <exe/coro/routine.hpp>

#include <sure/context.hpp>
#include <sure/stack.hpp>

namespace exe::coro {

class Coroutine {
 public:
  explicit Coroutine(Routine);

  void Resume();

  // Suspend current coroutine
  static void Suspend();

  bool IsCompleted() const;

 private:
  // ???
};

}  // namespace exe::coro
