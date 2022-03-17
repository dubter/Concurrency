begin_task()

task_link_libraries(asio)

# Task libs
add_task_library(tinyfibers)
add_task_library(echo)

# Tests support lib
add_task_library(tests/tests_support tests_support)

# Tests

# TinyFibers
add_task_test_dir(tests/tinyfibers/core core_tests)
add_task_test_dir(tests/tinyfibers/sleep sleep_tests)
add_task_test_dir(tests/tinyfibers/net net_tests)

# Echo
add_task_test_dir(tests/echo echo_tests)

end_task()
