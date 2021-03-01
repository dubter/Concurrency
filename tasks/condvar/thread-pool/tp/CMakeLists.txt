# Thread pool library(tp)

get_filename_component(LIB_INCLUDE_PATH ".." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

add_library(tp STATIC
        static_thread_pool.hpp
        static_thread_pool.cpp
        helpers.hpp
        helpers.cpp
        blocking_queue.hpp
        task.hpp
        thread_label.hpp
        thread_label.cpp)

target_include_directories(tp PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(tp PUBLIC wheels twist)
