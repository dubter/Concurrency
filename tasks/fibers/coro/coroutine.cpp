#include "coroutine.hpp"

#include <twist/ed/local/ptr.hpp>
#include <wheels/core/assert.hpp>

static twist::ed::ThreadLocalPtr<Coroutine> current_thread = nullptr;

Coroutine::Coroutine(Routine routine)
    : is_completed_(false),
      routine_(std::move(routine)),
      stack_(sure::Stack::AllocateBytes(amount_bytes_)),
      exception_ptr_(nullptr) {
  callee_ctx_.Setup(stack_.MutView(), this);
}

void Coroutine::Run() noexcept {
  if (current_thread == nullptr) {
    return;
  }
  try {
    current_thread->routine_();
  } catch (...) {
    current_thread->exception_ptr_ = std::current_exception();
  }

  current_thread->is_completed_ = true;
  current_thread->callee_ctx_.ExitTo(current_thread->caller_ctx_);
}

void Coroutine::Resume() {
  if (is_completed_) {
    return;
  }
  Coroutine* parent_coro = current_thread;
  current_thread = this;
  caller_ctx_.SwitchTo(callee_ctx_);
  current_thread = parent_coro;
  if (exception_ptr_ != nullptr) {
    std::rethrow_exception(exception_ptr_);
  }
}

void Coroutine::Suspend() {
  current_thread->callee_ctx_.SwitchTo(current_thread->caller_ctx_);
}

bool Coroutine::IsCompleted() const {
  return is_completed_;
}
