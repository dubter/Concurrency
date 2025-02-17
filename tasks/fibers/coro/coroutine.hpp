#pragma once

#include <sure/context.hpp>
#include <sure/stack.hpp>
#include <function2/function2.hpp>

#include <exception>

// Simple stackful coroutine
class Coroutine : sure::ITrampoline {
 public:
  using Routine = fu2::unique_function<void()>;

  explicit Coroutine(Routine routine);

  void Resume();

  // Suspend running coroutine
  static void Suspend();

  bool IsCompleted() const;

 private:
  void Run() noexcept override;

 private:
  const size_t amount_bytes_ = 65536;

  bool is_completed_;
  Routine routine_;
  sure::Stack stack_;

  sure::ExecutionContext callee_ctx_;
  sure::ExecutionContext caller_ctx_;
  std::exception_ptr exception_ptr_;
};
