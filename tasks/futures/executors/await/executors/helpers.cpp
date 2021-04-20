#include <await/executors/helpers.hpp>

namespace await::executors {

void ExecuteHere(Task& task) {
  try {
    task();
  } catch (...) {
    // Intentionally ignore exceptions
  }
}

}  // namespace await::executors
