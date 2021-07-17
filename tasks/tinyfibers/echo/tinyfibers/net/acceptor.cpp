#include <tinyfibers/net/acceptor.hpp>

#include <tinyfibers/runtime/scheduler.hpp>
#include <tinyfibers/runtime/parking_lot.hpp>

using wheels::Result;
using wheels::Status;

using wheels::make_result::Fail;
using wheels::make_result::JustStatus;
using wheels::make_result::NotSupported;
using wheels::make_result::Ok;
using wheels::make_result::PropagateError;
using wheels::make_result::ToStatus;

namespace tinyfibers::net {

Acceptor::Acceptor() {
  // Not implemented
}

Status Acceptor::BindTo(uint16_t /*port*/) {
  return NotSupported();  // Your code goes here
}

Result<uint16_t> Acceptor::BindToAvailablePort() {
  return NotSupported();  // Your code goes here
}

Status Acceptor::Listen(uint32_t /*backlog*/) {
  return NotSupported();  // Your code goes here
}

Result<Socket> Acceptor::Accept() {
  return NotSupported();  // Your code goes here
}

uint16_t Acceptor::GetPort() const {
  return 0;  // Not implemented
}

}  // namespace tinyfibers::net
