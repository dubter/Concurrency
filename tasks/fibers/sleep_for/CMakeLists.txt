begin_task()

# Dependencies
task_link_libraries(asio)

# Library
add_task_library(exe)

# Playground
add_task_playground(play)

# Tests

# Coroutine
add_task_test_dir(tests/coro coro_tests)

# Fibers
add_task_test(fibers_yield_tests tests/fibers/yield/main.cpp)
add_task_test(fibers_sleep_for_tests_1 tests/fibers/sleep_for_1/main.cpp)
add_task_test(fibers_sleep_for_tests_2 tests/fibers/sleep_for_2/main.cpp)

end_task()
