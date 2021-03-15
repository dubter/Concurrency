#include <tinyfibers/net/socket.hpp>

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

Result<Socket> Socket::ConnectTo(const std::string& /*host*/,
                                 uint16_t /*port*/) {
  return NotSupported();  // Your code goes here
}

Result<Socket> Socket::ConnectToLocal(uint16_t /*port*/) {
  return NotSupported();  // Your code goes here
}

Result<size_t> Socket::ReadSome(MutableBuffer /*buffer*/) {
  return NotSupported();  // Your code goes here
}

Result<size_t> Socket::Read(MutableBuffer /*buffer*/) {
  return NotSupported();  // Your code goes here
}

Status Socket::Write(ConstBuffer /*buffer*/) {
  return NotSupported();  // Your code goes here
}

Status Socket::ShutdownWrite() {
  return NotSupported();  // Your code goes here
}

Socket::Socket(asio::ip::tcp::socket&& impl) : socket_(std::move(impl)) {
}

}  // namespace tinyfibers::net
