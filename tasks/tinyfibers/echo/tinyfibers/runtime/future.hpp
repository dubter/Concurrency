#pragma once

#include <tinyfibers/runtime/parking_lot.hpp>

#include <wheels/support/result.hpp>

#include <optional>

namespace tinyfibers {

template <typename T>
class Future {
 public:
  // Blocks until the future is fulfilled
  wheels::Result<T> Get() {
    return wheels::make_result::NotSupported();
  }

  void SetValue(T value) {
    Set(wheels::make_result::Ok(std::move(value)));
  }

  void SetError(std::error_code error) {
    Set(wheels::make_result::Fail(error));
  }

 private:
  void Set(wheels::Result<T>&& /*result*/) {
    // Not implemented
  }

 private:
  // ???
};

}  // namespace tinyfibers
