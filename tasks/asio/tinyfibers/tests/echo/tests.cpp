#include <wheels/test/test_framework.hpp>

#include <echo/server.hpp>

#include <tinyfibers/api.hpp>
#include <tinyfibers/net/socket.hpp>
#include <tinyfibers/test/test.hpp>

#include <tests_support/generator.hpp>
#include <tests_support/buffer.hpp>
#include <tests_support/port.hpp>
#include <tests_support/read.hpp>
#include <tests_support/tcp_client.hpp>

#include <thread>

using namespace tinyfibers;
using namespace tinyfibers::net;

using namespace tests::support;

//////////////////////////////////////////////////////////////////////

namespace {

uint16_t EchoServerPort() {
  return tests::support::StaticFreePort();
}

void LaunchEchoServer() {
  // Serve forever in background thread
  std::thread([]() {
    RunScheduler([]() {
      echo::ServeForever(EchoServerPort());
    });
  }).detach();
}

SimpleTcpClient MakeEchoClient() {
  return {"localhost", EchoServerPort()};
}

}  // namespace

//////////////////////////////////////////////////////////////////////

TEST_SUITE(EchoServer) {
  TINY_FIBERS_TEST(HelloWorld) {
    static const std::string kMessage = "Hello, World!";

    Socket socket = Socket::ConnectToLocal(EchoServerPort());

    socket.Write(asio::buffer(kMessage)).ExpectOk();
    std::string response = Read(socket, kMessage.length());
    ASSERT_EQ(response, kMessage);

    socket.Write(asio::buffer("!")).ExpectOk();
    std::string terminator = Read(socket, 1);

    socket.ShutdownWrite().ExpectOk();

    ASSERT_EQ(terminator, "!");
  }

  TINY_FIBERS_TEST(ShutdownWrite) {
    static const std::string kMessage = "Test shutdown";

    auto client = MakeEchoClient();

    client.Write(kMessage).ExpectOk();
    client.ShutdownWrite().ExpectOk();

    ASSERT_EQ(client.ReadAll().ValueOrThrow(), kMessage);
  }

  TINY_FIBERS_TEST(TwoClients) {
    auto first = MakeEchoClient();
    auto second = MakeEchoClient();

    first.Write("Hi, Jake!").ExpectOk();
    second.Write("Hi, Finn!").ExpectOk();

    ASSERT_EQ(first.Read(9).ValueOrThrow(), "Hi, Jake!");
    ASSERT_EQ(second.Read(9).ValueOrThrow(), "Hi, Finn!");
  }
}

int main(int argc, const char* argv[]) {
  std::cout << "Echo server port: " << EchoServerPort() << std::endl;

  LaunchEchoServer();

  wheels::test::RunTestsMain(wheels::test::ListAllTests(), argc, argv);
  return EXIT_SUCCESS;
}
