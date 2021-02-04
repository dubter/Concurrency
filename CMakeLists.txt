cmake_minimum_required(VERSION 3.14)

project(concurrency_course)

include(cmake/Ccache.cmake)
include(cmake/Helpers.cmake)
include(cmake/CheckCompiler.cmake)
include(cmake/CompileOptions.cmake)
include(cmake/Sanitize.cmake)
include(cmake/Platform.cmake)

add_subdirectory(library)

include(cmake/Task.cmake)
add_subdirectory(tasks)
