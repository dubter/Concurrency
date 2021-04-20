#include <await/executors/priority.hpp>

#include <queue>

namespace await::executors {

IPriorityExecutorPtr MakePriorityExecutor(IExecutorPtr /*executor*/) {
  return nullptr;  // Not implemented
}

}  // namespace await::executors
