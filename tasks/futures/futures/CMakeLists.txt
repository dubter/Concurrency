begin_task()

# Library
add_task_library(await)

# Unit tests
add_task_test_dir(tests/unit/executors executors_unit_tests)
add_task_test_dir(tests/unit/futures futures_unit_tests)

# Stress tests

end_task()
