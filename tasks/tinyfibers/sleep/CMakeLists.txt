begin_task()

task_link_libraries(asio)

add_task_library(tinyfibers)

# Tests
add_task_test_dir(tests/core core_tests)
add_task_test_dir(tests/sleep sleep_tests)

end_task()
