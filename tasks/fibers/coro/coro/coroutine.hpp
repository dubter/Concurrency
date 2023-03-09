#include <sure/context.hpp>
#include <sure/stack.hpp>

#include <coro/routine.hpp>

namespace coro {

class Coroutine {
 public:
  explicit Coroutine(Routine routine);

  void Resume();

  // Suspend running coroutine
  static void Suspend();

  bool IsCompleted() const;

 private:
  // ???
};

}  // namespace coro
