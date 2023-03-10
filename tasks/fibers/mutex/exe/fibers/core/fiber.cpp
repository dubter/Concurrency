#include <exe/fibers/core/fiber.hpp>

#include <twist/ed/local/ptr.hpp>

namespace exe::fibers {

void Fiber::Suspend(/*???*/) {
  // Not implemented
}

void Fiber::Schedule() {
  // Not implemented
}

void Fiber::Resume() {
  // Not implemented
}

void Fiber::Run() {
  // Not implemented
}

Fiber& Fiber::Self() {
  std::abort();  // Not implemented
}

}  // namespace exe::fibers
