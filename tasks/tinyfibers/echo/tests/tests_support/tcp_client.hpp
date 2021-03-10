#pragma once

#include <tinyfibers/net/socket.hpp>

#include <string>
#include <cstdint>

namespace tests::support {

using tinyfibers::net::Socket;

class SimpleTcpClient {
 public:
  SimpleTcpClient(std::string host, uint16_t port)
      : socket_(Socket::ConnectTo(host, port)) {
  }

  wheels::Result<void> Write(const std::string& data) {
    return socket_.Write(asio::buffer(data));
  }

  wheels::Result<std::string> Read(size_t bytes) {
    return ::tests::support::Read(socket_, bytes);
  }

  wheels::Result<std::string> ReadAll() {
    return ::tests::support::ReadAll(socket_);
  }

  wheels::Result<void> ShutdownWrite() {
    return socket_.ShutdownWrite();
  }

 private:
  Socket socket_;
};

}  // namespace tests::support
