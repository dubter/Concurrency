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
        GIT_TAG master
)
FetchContent_MakeAvailable(wheels)

# --------------------------------------------------------------------

message(STATUS "FetchContent: context")

FetchContent_Declare(
        context
        GIT_REPOSITORY https://gitlab.com/Lipovsky/context.git
        GIT_TAG 28cd099628152d8fa66300e2b77c7b94a4fe743d
)
FetchContent_MakeAvailable(context)

# --------------------------------------------------------------------

message(STATUS "FetchContent: twist")

FetchContent_Declare(
        twist
        GIT_REPOSITORY https://gitlab.com/Lipovsky/twist.git
        GIT_TAG 15d6e76166dd0847af25662e2932f1ca9574886d
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
