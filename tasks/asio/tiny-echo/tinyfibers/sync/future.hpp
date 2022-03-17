#pragma once

#include <tinyfibers/runtime/parking_lot.hpp>

#include <wheels/result/result.hpp>

#include <optional>

namespace tinyfibers {

template <typename T>
class FutureLite {
 public:
  wheels::Result<T> Get() {
    std::abort();  // Not implemented
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
