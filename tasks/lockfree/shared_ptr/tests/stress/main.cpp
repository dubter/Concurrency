#include <wheels/test/test_framework.hpp>

#include <twist/rt/fault/adversary/lockfree.hpp>

////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv) {
  twist::rt::fault::SetAdversary(twist::rt::fault::CreateLockFreeAdversary());

  auto all_tests = wheels::test::ListAllTests();
  wheels::test::RunTestsMain(all_tests, argc, argv);

  return 0;
}
