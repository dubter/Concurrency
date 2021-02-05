cmake_minimum_required(VERSION 3.5)

begin_task()
set_task_sources(guarded.hpp)
add_task_test(unit_tests unit_tests.cpp)
add_task_test(stress_tests stress_tests.cpp)
end_task()
