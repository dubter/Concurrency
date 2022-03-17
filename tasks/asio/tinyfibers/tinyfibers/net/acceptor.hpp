#pragma once

#include <tinyfibers/net/socket.hpp>

#include <asio/ip/tcp.hpp>

#include <wheels/support/result.hpp>

namespace tinyfibers::net {

class Acceptor {
 public:
  Acceptor();

  // Non-copyable
  Acceptor(const Acceptor&) = delete;
  Acceptor& operator=(const Acceptor&) = delete;

  wheels::Status BindTo(uint16_t port);
  // Returns port number
  wheels::Result<uint16_t> BindToAvailablePort();

  uint16_t GetPort() const;

  wheels::Status Listen(uint32_t backlog = 128);

  wheels::Result<Socket> Accept();

 private:
  // Use asio::ip::tcp::acceptor
};

}  // namespace tinyfibers::net
