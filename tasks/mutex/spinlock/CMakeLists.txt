enable_language(ASM)

begin_task()
set_task_sources(atomics.hpp atomics.S spinlock.hpp)
add_task_test(unit_tests tests/unit.cpp)
add_task_test(stress_tests tests/stress.cpp)
end_task()
