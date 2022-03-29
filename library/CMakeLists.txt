include(FetchContent)

# --------------------------------------------------------------------

# Offline mode (uncomment next line to enable)
# set(FETCHCONTENT_FULLY_DISCONNECTED ON)

# set(FETCHCONTENT_QUIET OFF)

# --------------------------------------------------------------------

# Libraries

# --------------------------------------------------------------------

message(STATUS "FetchContent: wheels")

FetchContent_Declare(
        wheels
        GIT_REPOSITORY https://gitlab.com/Lipovsky/wheels.git
        GIT_TAG 43693e7ef6ebc9edda3d68e38550c813275e8a8c
)
FetchContent_MakeAvailable(wheels)

# --------------------------------------------------------------------

message(STATUS "FetchContent: context")

FetchContent_Declare(
        context
        GIT_REPOSITORY https://gitlab.com/Lipovsky/context.git
        GIT_TAG bd921018925ec84642601e5a6b6b111dfaf8d4e4
)
FetchContent_MakeAvailable(context)

# --------------------------------------------------------------------

message(STATUS "FetchContent: twist")

FetchContent_Declare(
        twist
        GIT_REPOSITORY https://gitlab.com/Lipovsky/twist.git
        GIT_TAG 186a1fa4e8733e8466a2f58f10e7b6ec6762d5ff
)
FetchContent_MakeAvailable(twist)

# --------------------------------------------------------------------

message(STATUS "FetchContent: tinyfibers")

FetchContent_Declare(
        tinyfibers
        GIT_REPOSITORY https://gitlab.com/Lipovsky/tinyfibers.git
        GIT_TAG 125ff950c06a29830b771231dc74738a751c1da8
)
FetchContent_MakeAvailable(tinyfibers)

# --------------------------------------------------------------------

message(STATUS "FetchContent: asio")

FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-22-1
)
FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
