#pragma once

#include <vector>

namespace await::futures::detail {

// Generic algorithm

template <template <typename> class Combinator, typename T>
auto Combine(std::vector<Future<T>> /*inputs*/) {
  return;  // Not implemented
}

}  // namespace await::futures::detail
