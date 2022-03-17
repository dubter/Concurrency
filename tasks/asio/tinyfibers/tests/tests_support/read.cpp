#include <tests_support/read.hpp>
#include <tests_support/buffer.hpp>

namespace tests::support {

using wheels::Result;
using wheels::make_result::Ok;
using wheels::make_result::PropagateError;

Result<std::string> Read(Socket& socket, size_t bytes) {
  std::string data;
  data.resize(bytes);

  auto bytes_read = socket.Read({data.data(), data.size()});

  if (bytes_read.HasError()) {
    return PropagateError(bytes_read);
  }

  data.resize(*bytes_read);
  return Ok(data);
}

Result<std::string> ReadAll(Socket& socket) {
  GrowingBuffer read;

  static const size_t kBufSize = 1237;
  std::vector<char> buf(kBufSize);

  while (true) {
    auto bytes_read = socket.Read(asio::buffer(buf));
    if (bytes_read.HasError()) {
      return PropagateError(bytes_read);
    }
    if (*bytes_read > 0) {
      read.Append(buf.data(), *bytes_read);
    }
    if (*bytes_read < kBufSize) {
      return Ok(read.ToString());
    }
  }
}

}  // namespace tests::support