#include <wheels/test/test_framework.hpp>

#include <twist/fault/adversary/lockfree.hpp>

////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv) {
  twist::fault::SetAdversary(twist::fault::CreateLockFreeAdversary());

  auto all_tests = wheels::test::ListAllTests();
  wheels::test::RunTestsMain(all_tests, argc, argv);

  return 0;
}
