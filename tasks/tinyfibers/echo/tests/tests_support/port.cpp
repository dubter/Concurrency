#include <tests_support/port.hpp>

#include <random>

#include <asio.hpp>

namespace tests::support {

using asio::ip::tcp;

uint16_t FindFreePort() {
  asio::io_context io_context;

  std::random_device rd;
  std::mt19937 twister(rd());
  std::uniform_int_distribution random_port(30'000, 60'000);

  while (true) {
    uint16_t port = random_port(twister);

    asio::ip::tcp::acceptor acceptor(io_context);

    std::error_code error;

    acceptor.open(tcp::v4(), error);
    acceptor.bind({tcp::v4(), port}, error);
    if (error) {
      continue;
    }

    return port;
  }
}

uint16_t StaticFreePort() {
  static uint16_t free_port = FindFreePort();
  return free_port;
}

}  // namespace tests::support
