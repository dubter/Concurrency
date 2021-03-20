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
        GIT_TAG 3aefaf69e694434149d678633052614f429b1fbc
)
FetchContent_MakeAvailable(wheels)

# --------------------------------------------------------------------

message(STATUS "FetchContent: context")

FetchContent_Declare(
        context
        GIT_REPOSITORY https://gitlab.com/Lipovsky/context.git
        GIT_TAG 35b73b15738614d91f950a98f1c074ea2a48ee57
)
FetchContent_MakeAvailable(context)

# --------------------------------------------------------------------

message(STATUS "FetchContent: twist")

FetchContent_Declare(
        twist
        GIT_REPOSITORY https://gitlab.com/Lipovsky/twist.git
        GIT_TAG d303efca6abbf770cf05fb0a9f46196f686f35d8
)
FetchContent_MakeAvailable(twist)

# --------------------------------------------------------------------

message(STATUS "FetchContent: tinyfibers")

FetchContent_Declare(
        tinyfibers
        GIT_REPOSITORY https://gitlab.com/Lipovsky/tinyfibers.git
        GIT_TAG e28ec9fe419a79f630fb1538b36afa26de88f8bd
)
FetchContent_MakeAvailable(tinyfibers)

# --------------------------------------------------------------------

message(STATUS "FetchContent: asio")

FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-18-1
)
FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
