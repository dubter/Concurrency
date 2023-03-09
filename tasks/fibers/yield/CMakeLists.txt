begin_task()

# Library
add_task_library(exe)

# Playground
add_task_playground(play)

# Tests

# Coroutine
add_task_test_dir(tests/coroutine coroutine_tests)

# Thread Pool
add_task_test_dir(tests/thread_pool/unit thread_pool_unit_tests)
add_task_test_dir(tests/thread_pool/stress thread_pool_stress_tests)

# Fibers
add_task_test_dir(tests/fibers/core/ fibers_core_tests)
add_task_test_dir(tests/fibers/racy fibers_racy_tests)

end_task()
