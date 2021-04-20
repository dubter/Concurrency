#pragma once

#include <await/futures/core/callback.hpp>

#include <await/executors/executor.hpp>
#include <await/executors/helpers.hpp>
#include <await/executors/inline.hpp>

#include <wheels/support/function.hpp>
#include <wheels/support/result.hpp>

#include <twist/stdlike/atomic.hpp>

#include <optional>

namespace await::futures {

namespace detail {

//////////////////////////////////////////////////////////////////////

// State shared between Promise and Future

template <typename T>
class SharedState {
 public:
  SharedState() {
  }

  void SetResult(wheels::Result<T>&& result) {
    result_.emplace(std::move(result));  // Not implemented
  }

  bool HasResult() const {
    return false;  // Not implemented
  }

  // Do we need it here?
  wheels::Result<T> GetResult() {
    return GetReadyResult();  // Not implemented
  }

  // Precondition: f.IsReady() == true
  wheels::Result<T> GetReadyResult() {
    return std::move(*result_);  // Not implemented
  }

 private:
  std::optional<wheels::Result<T>> result_;
};

//////////////////////////////////////////////////////////////////////

template <typename T>
using StateRef = std::shared_ptr<SharedState<T>>;

template <typename T>
inline StateRef<T> MakeSharedState() {
  return std::make_shared<SharedState<T>>();
}

//////////////////////////////////////////////////////////////////////

// Common base for Promise and Future

template <typename T>
class HoldState {
 protected:
  HoldState(StateRef<T> state) : state_(std::move(state)) {
  }

  // Movable
  HoldState(HoldState&& that) = default;
  HoldState& operator=(HoldState&& that) = default;

  // Non-copyable
  HoldState(const HoldState& that) = delete;
  HoldState& operator=(const HoldState& that) = delete;

  StateRef<T> ReleaseState() {
    CheckState();
    return std::move(state_);
  }

  bool HasState() const {
    return (bool)state_;
  }

  void CheckState() const {
    WHEELS_VERIFY(HasState(), "No shared state or shared state released");
  }

 protected:
  StateRef<T> state_;
};

}  // namespace detail

}  // namespace await::futures
