#include <tinyfibers/net/socket.hpp>

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

Result<Socket> Socket::ConnectTo(const std::string& /*host*/,
                                 uint16_t /*port*/) {
  std::abort();  // Not implemented
}

Result<Socket> Socket::ConnectToLocal(uint16_t /*port*/) {
  std::abort();  // Not implemented
}

Result<size_t> Socket::ReadSome(MutableBuffer /*buffer*/) {
  std::abort();  // Not implemented
}

Result<size_t> Socket::Read(MutableBuffer /*buffer*/) {
  std::abort();  // Not implemented
}

Status Socket::Write(ConstBuffer /*buffer*/) {
  std::abort();  // Not implemented
}

Status Socket::ShutdownWrite() {
  std::abort();  // Not implemented
}

Socket::Socket(asio::ip::tcp::socket&& impl) : socket_(std::move(impl)) {
}

}  // namespace tinyfibers::net
