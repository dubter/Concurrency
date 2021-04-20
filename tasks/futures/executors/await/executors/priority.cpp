#include <await/executors/priority.hpp>

#include <queue>

namespace await::executors {

class PriorityExecutor : public IPriorityExecutor {
  // Not implemented
};

IPriorityExecutorPtr MakePriorityExecutor(IExecutorPtr /*executor*/) {
  return nullptr;  // Not implemented
}

}  // namespace await::executors
