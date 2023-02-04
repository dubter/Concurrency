#include <futures/promise.hpp>

#include <fmt/core.h>

#include <thread>

int main() {
  // Contract
  stdlike::Promise<std::string> p;
  auto f = p.MakeFuture();

  // Producer
  std::thread producer([p = std::move(p)]() mutable {
    p.SetValue("Hi");
  });

  // Consumer
  auto value = f.Get();
  fmt::println("Value = {}", value);

  producer.join();

  return 0;
}
