begin_task()

# Library
add_task_library(await)

# Unit tests
add_task_test_dir(tests/unit/executors executors_unit_tests)

# Stress tests
add_task_test_dir(tests/stress/executors executors_stress_tests)

end_task()
