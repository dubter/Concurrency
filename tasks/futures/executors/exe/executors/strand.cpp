#include <exe/executors/strand.hpp>

namespace exe::executors {

Strand::Strand(IExecutor& /*underlying*/) {
}

void Strand::Execute(Task /*task*/) {
  // Not implemented
}

}  // namespace exe::executors
