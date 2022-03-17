#include <tinyfibers/net/acceptor.hpp>

#include <tinyfibers/core/scheduler.hpp>

#include <wheels/result/make.hpp>

using wheels::Result;
using wheels::Status;

using wheels::make_result::Fail;
using wheels::make_result::JustStatus;
using wheels::make_result::Ok;
using wheels::make_result::PropagateError;
using wheels::make_result::ToStatus;

namespace tinyfibers::net {

Acceptor::Acceptor() {
  // Not implemented
}

Status Acceptor::BindTo(uint16_t /*port*/) {
  std::abort();  // Not implemented
}

Result<uint16_t> Acceptor::BindToAvailablePort() {
  std::abort();  // Not implemented
}

Status Acceptor::Listen(uint32_t /*backlog*/) {
  std::abort();  // Not implemented
}

Result<Socket> Acceptor::Accept() {
  std::abort();  // Not implemented
}

uint16_t Acceptor::GetPort() const {
  return 0;  // Not implemented
}

}  // namespace tinyfibers::net
