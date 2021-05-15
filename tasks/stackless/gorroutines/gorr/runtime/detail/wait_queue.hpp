#pragma once

#include <experimental/coroutine>

#include <wheels/support/intrusive_list.hpp>

namespace gorr {

namespace detail {

struct WaitNode : public wheels::IntrusiveListNode<WaitNode> {
  void Resume() {
    coro.resume();
  }

  std::experimental::coroutine_handle<> coro;
};

using WaitQueue = wheels::IntrusiveList<WaitNode>;

}  // namespace detail

}  // namespace gorr
