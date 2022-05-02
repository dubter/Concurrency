#ifndef FUTURE_IMPL
#error Do not include this file directly
#endif

#include <exe/futures/core/detail/traits.hpp>

namespace exe::futures {

//////////////////////////////////////////////////////////////////////

// Static constructors

template <typename T>
Future<T> Future<T>::Invalid() {
  return Future<T>{nullptr};
}

//////////////////////////////////////////////////////////////////////

// Executors

template <typename T>
Future<T> Future<T>::Via(executors::IExecutor& e) && {
  auto state = ReleaseState();
  state->SetExecutor(&e);
  return Future{std::move(state)};
}

template <typename T>
executors::IExecutor& Future<T>::GetExecutor() const {
  return AccessState().GetExecutor();
}

template <typename T>
void Future<T>::Subscribe(Callback<T> callback) && {
  return ReleaseState()->SetCallback(std::move(callback));
}

//////////////////////////////////////////////////////////////////////

// Synchronous Then

template <typename T>
template <typename F> requires SyncContinuation<F, T>
auto Future<T>::Then(F /*continuation*/) && {
  using U = std::invoke_result_t<F, T>;

  return Future<U>::Invalid();  // Not implemented
}

//////////////////////////////////////////////////////////////////////

// Asynchronous Then

template <typename T>
template <typename F> requires AsyncContinuation<F, T>
auto Future<T>::Then(F /*continuation*/) && {
  using U = typename detail::Flatten<std::invoke_result_t<F, T>>::ValueType;

  return Future<U>::Invalid();  // Not implemented
}

//////////////////////////////////////////////////////////////////////

// Recover

template <typename T>
template <typename F> requires ErrorHandler<F, T>
    Future<T> Future<T>::Recover(F /*error_handler*/) && {
  return Future<T>::Invalid();  // Not implemented
}

}  // namespace exe::futures
