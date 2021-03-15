#include <wheels/test/test_framework.hpp>

#include <tinyfibers/test/test.hpp>

#include <tinyfibers/api.hpp>
#include <tinyfibers/net/acceptor.hpp>
#include <tinyfibers/net/socket.hpp>

#include <tests_support/generator.hpp>
#include <tests_support/buffer.hpp>
#include <tests_support/read.hpp>

using namespace tinyfibers;
using namespace tinyfibers::net;

using namespace tests::support;

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Sockets) {
  TINY_FIBERS_TEST(ChooseAvailablePort) {
    Acceptor acceptor_1;
    acceptor_1.BindTo(0).ExpectOk();

    Acceptor acceptor_2;
    acceptor_2.BindTo(0).ExpectOk();

    std::cout << "Available ports: " << acceptor_1.GetPort() << ", "
              << acceptor_2.GetPort() << std::endl;

    ASSERT_NE(acceptor_1.GetPort(), acceptor_2.GetPort());
  }

  TINY_FIBERS_TEST(AddressAlreadyInUse) {
    Acceptor acceptor_1;
    acceptor_1.BindToAvailablePort().ThrowIfError();

    Acceptor acceptor_2;
    auto status = acceptor_2.BindTo(acceptor_1.GetPort());
    ASSERT_FALSE(status.IsOk());
  }

  TINY_FIBERS_TEST(FailToConnect) {
    return;  // Under investigation

    Acceptor acceptor;
    uint16_t port = acceptor.BindToAvailablePort();

    std::cout << "Bind acceptor to port " << port << std::endl;

    auto connect = [port]() {
      auto socket = Socket::ConnectToLocal(port);
      ASSERT_TRUE(socket.HasError());

      std::cout << "Cannot connect ot port " << port << ": "
                << socket.GetErrorCode().message() << std::endl;
    };

    Spawn(connect).Join();

    // Socket client_socket = acceptor.Accept();
  }

  TINY_FIBERS_TEST(Accept) {
    Acceptor acceptor;
    uint16_t port = acceptor.BindToAvailablePort();
    acceptor.Listen().ExpectOk();

    auto connect = [port]() {
      Socket socket = Socket::ConnectToLocal(port);
    };

    Spawn(connect).Detach();

    Socket client_socket = acceptor.Accept();
  }

  TINY_FIBERS_TEST(Connect) {
    Acceptor acceptor;
    uint16_t port = acceptor.BindToAvailablePort();
    acceptor.Listen().ExpectOk();

    auto accept = [&acceptor]() {
      Socket socket = acceptor.Accept();
    };

    Spawn(accept).Detach();

    Socket socket = Socket::ConnectToLocal(port);
  }

  TINY_FIBERS_TEST(Hello) {
    static const std::string kHelloMessage = "Hello, World!";

    Acceptor acceptor;
    uint16_t port = acceptor.BindToAvailablePort();
    acceptor.Listen().ExpectOk();

    auto client = [port]() {
      Socket socket = Socket::ConnectToLocal(port);
      socket.Write(asio::buffer(kHelloMessage)).ExpectOk();
    };

    Spawn(client).Detach();

    Socket socket = acceptor.Accept();
    std::string message = Read(socket, kHelloMessage.length());

    std::cout << "Message sent: '" << kHelloMessage << "'" << std::endl;
    std::cout << "Message received: " << message << "'" << std::endl;

    ASSERT_EQ(message, kHelloMessage);
  }

  TINY_FIBERS_TEST(ReadAll) {
    static const std::string kHelloMessage = "Hello, World!";

    Acceptor acceptor;
    uint16_t port = acceptor.BindToAvailablePort();
    acceptor.Listen().ExpectOk();

    auto client = [port]() {
      Socket socket = Socket::ConnectToLocal(port);
      socket.Write(asio::buffer(kHelloMessage)).ExpectOk();
      socket.ShutdownWrite().ExpectOk();
    };

    Spawn(client).Detach();

    Socket socket = acceptor.Accept();

    std::string message = ReadAll(socket);
    ASSERT_EQ(message, kHelloMessage);
  }

  TINY_FIBERS_TEST(Shutdown) {
    static const std::string kHelloMessage = "Hi, Neo!";
    static const std::string kReplyMessage = "Hi, Morpheus!";

    Acceptor acceptor;
    uint16_t port = acceptor.BindToAvailablePort();
    acceptor.Listen().ExpectOk();

    auto client = [port]() {
      Socket socket = Socket::ConnectToLocal(port);
      socket.Write(asio::buffer(kHelloMessage)).ExpectOk();

      socket.ShutdownWrite().ExpectOk();

      std::string reply = ReadAll(socket);
      ASSERT_EQ(reply, kReplyMessage);
    };

    Spawn(client).Detach();

    Socket client_socket = acceptor.Accept();
    std::string message = ReadAll(client_socket);
    ASSERT_EQ(message, kHelloMessage);

    client_socket.Write(asio::buffer(kReplyMessage)).ExpectOk();
    client_socket.ShutdownWrite().ExpectOk();
  }

  TINY_FIBERS_TEST(SocketStreamReads) {
    static const size_t kStreamLength = 16 * 1024 * 1024;
    static const size_t kWriteBufSize = 4113;
    static const size_t kReadBufSize = 3517;

    Acceptor acceptor;
    uint16_t port = acceptor.BindToAvailablePort();
    acceptor.Listen().ExpectOk();

    DataGenerator generator(kStreamLength);
    GrowingBuffer sent;
    GrowingBuffer received;

    auto sender = [port, &generator, &sent]() {
      Socket socket = Socket::ConnectToLocal(port);

      std::vector<char> write_buf(kWriteBufSize);

      while (generator.HasMore()) {
        size_t buf_size = generator.NextChunk(write_buf.data(), kWriteBufSize);
        socket.Write(asio::buffer(write_buf, buf_size)).ExpectOk();
        sent.Append(write_buf.data(), buf_size);
      }
      socket.ShutdownWrite().ExpectOk();
    };

    Spawn(sender).Detach();

    Socket socket = acceptor.Accept();

    std::vector<char> read_buf(kReadBufSize);

    while (true) {
      size_t bytes_read = socket.ReadSome(asio::buffer(read_buf, kReadBufSize));
      if (bytes_read > 0) {
        received.Append(read_buf.data(), bytes_read);
      } else {
        break;
      }
    }

    ASSERT_EQ(sent.ToString(), received.ToString());
  }

  TINY_FIBERS_TEST(SocketHugeRead) {
    static const std::string kHelloMessage = "Hello, World!";
    static const size_t kStreamLength = 8 * 1024 * 1024;
    static const size_t kChunkSize = 4096;

    Acceptor acceptor;
    uint16_t port = acceptor.BindToAvailablePort();
    acceptor.Listen().ExpectOk();

    DataGenerator generator(kStreamLength);
    GrowingBuffer sent;

    auto sender = [port, &generator, &sent]() {
      Socket socket = Socket::ConnectToLocal(port);

      std::vector<char> chunk(kChunkSize);

      while (generator.HasMore()) {
        size_t chunk_size = generator.NextChunk(chunk.data(), kChunkSize);
        socket.Write(asio::buffer(chunk, chunk_size)).ExpectOk();
        sent.Append(chunk.data(), chunk_size);
      }
    };

    Spawn(sender).Detach();

    Socket socket = acceptor.Accept();

    std::string read_buf;
    read_buf.resize(kStreamLength);

    size_t bytes_read = socket.Read(asio::buffer(read_buf));

    ASSERT_EQ(bytes_read, kStreamLength);
    ASSERT_EQ(read_buf, sent.ToString());
  }

  TINY_FIBERS_TEST(HttpBin) {
    Socket socket = Socket::ConnectTo("httpbin.org", 80);

    static const std::string_view kCRLF = "\r\n";

    std::string request = wheels::StringBuilder()
                          << "GET /status/200 HTTP/1.0" << kCRLF
                          << "Host: httpbin.org" << kCRLF << kCRLF;

    socket.Write(asio::buffer(request)).ExpectOk();
    socket.ShutdownWrite().ExpectOk();

    std::string response = ReadAll(socket);

    std::cout << "Response from httpbin: " << response << std::endl;
  }
}

RUN_ALL_TESTS()
