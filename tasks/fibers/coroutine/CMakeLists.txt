begin_task()

# Library
add_task_library(exe)

# Playground
add_playground(play)

# Tests
add_task_test_dir(tests/coroutine/standalone coroutine_tests)
add_task_test_dir(tests/coroutine/processors processors_tests)

add_task_test_dir(tests/thread_pool/unit thread_pool_unit_tests)
add_task_test_dir(tests/thread_pool/stress thread_pool_stress_tests)

add_task_test_dir(tests/fibers/core fibers_core_tests)

end_task()
