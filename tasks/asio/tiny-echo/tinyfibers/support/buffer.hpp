#pragma once

#include <asio/buffer.hpp>

namespace tinyfibers {

using MutableBuffer = asio::mutable_buffer;
using ConstBuffer = asio::const_buffer;

}  // namespace tinyfibers
