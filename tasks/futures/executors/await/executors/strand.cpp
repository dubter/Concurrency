#include <await/executors/strand.hpp>
#include <await/executors/helpers.hpp>

namespace await::executors {

IExecutorPtr MakeStrand(IExecutorPtr /*executor*/) {
  return nullptr;  // Not implemented
}

}  // namespace await::executors
