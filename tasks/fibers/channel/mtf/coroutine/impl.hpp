#pragma once

#include <mtf/coroutine/routine.hpp>

#include <wheels/memory/view.hpp>

#include <exception>

namespace mtf::coroutine::impl {

// Stackful asymmetric coroutine

class Coroutine {
 public:
  Coroutine(Routine routine, wheels::MutableMemView stack);

  // Non-copyable
  Coroutine(const Coroutine&) = delete;
  Coroutine& operator=(const Coroutine&) = delete;

  void Resume();

  // Suspends current coroutine
  static void Suspend();

  bool IsCompleted() const;

 private:
  [[noreturn]] static void Trampoline();

 private:
  // ???
};

}  // namespace mtf::coroutine::impl
