begin_task()

task_link_libraries(asio)

# Task libs
add_task_library(tinyfibers)
add_task_library(echo)

# Tests support lib
add_task_library(tests/tests_support tests_support)

# Tests
add_task_test_dir(tests/core core_tests)
add_task_test_dir(tests/sleep sleep_tests)
add_task_test_dir(tests/net net_tests)
add_task_test_dir(tests/echo echo_tests)

end_task()
