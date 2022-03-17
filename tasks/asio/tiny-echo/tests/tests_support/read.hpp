#pragma once

#include <wheels/support/result.hpp>
#include <tinyfibers/net/socket.hpp>

#include <string>

namespace tests::support {

using tinyfibers::net::Socket;

// Read exactly `bytes` bytes from socket
wheels::Result<std::string> Read(Socket& socket, size_t bytes);

// Read all bytes from socket
wheels::Result<std::string> ReadAll(Socket& socket);

}  // namespace tests::support
