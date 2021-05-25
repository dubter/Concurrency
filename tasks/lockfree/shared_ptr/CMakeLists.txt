begin_task()

set_task_sources(
        atomic_stamped_ptr.hpp
        atomic_shared_ptr.hpp
        lock_free_stack.hpp)

# Unit tests
add_task_test_dir(tests/unit unit_tests)

# Stress tests
add_task_test_dir(tests/stress stress_tests)

end_task()
