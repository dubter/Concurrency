#pragma once

namespace exe::fibers {

class Fiber;

// Lightweight non-owning handle to a _suspended_ fiber object

class FiberHandle {
  friend class Fiber;
 public:
  FiberHandle() : FiberHandle(nullptr) {
  }

  static FiberHandle Invalid() {
    return FiberHandle(nullptr);
  }

  bool IsValid() const {
    return fiber_ != nullptr;
  }

  // Resume asynchronously in associated scheduler
  void Resume();

  // Resume synchronously in the current thread
  void ResumeHere();

 private:
  explicit FiberHandle(Fiber* fiber) : fiber_(fiber) {
  }

  Fiber* Release();

 private:
  Fiber* fiber_;
};

}  // namespace exe::fibers
