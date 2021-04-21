#pragma once

namespace mtf::fibers {

class Fiber;

// Lightweight non-owning handle to the fiber object

class FiberHandle {
 public:
  explicit FiberHandle(Fiber* f) : f_(f) {
  }

  FiberHandle() : FiberHandle(nullptr) {
  }

  static FiberHandle Invalid() {
    return FiberHandle(nullptr);
  }

  bool IsValid() const {
    return f_ != nullptr;
  }

  void Resume();

 private:
  Fiber* f_;
};

}  // namespace mtf::fibers
