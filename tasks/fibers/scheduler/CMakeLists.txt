begin_task()

# Library
add_task_library(exe)

# Playground
add_playground(play)

# Workloads
add_task_dir_target(workloads workloads)

# Tests

# Coroutine
add_task_test_dir(tests/coroutine/standalone coroutine_tests)

# Compute Thread Pool
add_task_test_dir(tests/executors/tp/compute/unit tp_compute_unit_tests)
add_task_test_dir(tests/executors/tp/compute/stress tp_compute_stress_tests)

# Fibers over executors
add_task_test_dir(tests/fibers/executors/tp fibers_over_executors_tests)

# Fast Thread Pool
add_task_test_dir(tests/executors/tp/fast/unit tp_fast_unit_tests)
add_task_test_dir(tests/executors/tp/fast/stress tp_fast_stress_tests)

# Fibers
add_task_test_dir(tests/fibers/core/ fibers_core_tests)
add_task_test_dir(tests/fibers/racy fibers_racy_tests)

# Sync
add_task_test_dir(tests/fibers/mutex/unit fibers_mutex_unit_tests)
add_task_test_dir(tests/fibers/mutex/stress fibers_mutex_stress_tests)

add_task_test_dir(tests/fibers/wait_group/unit fibers_wait_group_unit_tests)
add_task_test_dir(tests/fibers/wait_group/stress fibers_wait_group_stress_tests)

add_task_test_dir(tests/fibers/condvar/unit fibers_condvar_unit_tests)
add_task_test_dir(tests/fibers/condvar/stress fibers_condvar_stress_tests)

end_task()
