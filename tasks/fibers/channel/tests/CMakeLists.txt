add_executable(fibers_tests coroutine.cpp thread_pool.cpp fibers.cpp stacks.cpp mutex.cpp channels.cpp condvar.cpp all.cpp)
target_link_libraries(fibers_tests fibers)
