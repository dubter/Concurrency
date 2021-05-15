begin_task()

add_task_library(gorr)

add_task_test_dir(tests)

add_task_test_dir(tests/unit unit_tests)
add_task_test_dir(tests/stress stress_tests)

add_playground(play)

end_task()
