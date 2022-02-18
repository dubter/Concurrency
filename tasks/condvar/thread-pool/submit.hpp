#pragma once

namespace tp {

// Usage:
// Submit(tp, []() {
//   std::cout << "Hi from pool!" << std::endl;
// });

template <typename F>
void Submit(StaticThreadPool& tp, F&& f) {
  tp.Submit(std::forward<F>(f));
}

}  // namespace tp