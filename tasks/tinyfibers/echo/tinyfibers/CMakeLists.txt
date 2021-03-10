# --------------------------------------------------------------------

get_filename_component(LIB_INCLUDE_PATH ".." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${LIB_PATH}/*.hpp)

add_library(${LIB_TARGET} STATIC ${LIB_CXX_SOURCES} ${LIB_HEADERS})
target_include_directories(${LIB_TARGET} PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(${LIB_TARGET} PUBLIC asio pthread wheels context)

# --------------------------------------------------------------------

if(TINY_FIBERS_DEVELOPER)

# Linters

if(CLANG_FORMAT_TOOL)
    add_clang_format_target(
            ${LIB_TARGET}_clang_format
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${LIB_HEADERS} ${LIB_CXX_SOURCES})
endif()

if(CLANG_TIDY_TOOL)
    add_clang_tidy_target(
            ${LIB_TARGET}_clang_tidy
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${LIB_INCLUDE_PATH}
            ${LIB_HEADERS} ${LIB_CXX_SOURCES})
endif()

endif()