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
        GIT_TAG 1be2c9a6011179d01d5366aa39745e8ba04719b0
)
FetchContent_MakeAvailable(wheels)

# --------------------------------------------------------------------

message(STATUS "FetchContent: twist")

FetchContent_Declare(
        twist
        GIT_REPOSITORY https://gitlab.com/Lipovsky/twist.git
        GIT_TAG 4741e59079438d6a60bde59a4f56bd47dee26687
)
FetchContent_MakeAvailable(twist)

# --------------------------------------------------------------------

message(STATUS "FetchContent: tinyfibers")

FetchContent_Declare(
        tinyfibers
        GIT_REPOSITORY https://gitlab.com/Lipovsky/tinyfibers.git
        GIT_TAG ef636e6b440b9add342b59589cf003e6d2d51352
)
FetchContent_MakeAvailable(tinyfibers)
