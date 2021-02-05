#include "table.hpp"

#include <wheels/test/test_framework.hpp>

#include <twist/test/inject_fault.hpp>

namespace dining {

void Plate::Access() {
  ASSERT_FALSE_M(accessed_.exchange(true, std::memory_order_relaxed),
                 "Mutual exclusion violated");
  twist::test::InjectFault();
  ASSERT_TRUE_M(accessed_.exchange(false, std::memory_order_relaxed),
                "Mutual exclusion violated");
}

}  // namespace dining
