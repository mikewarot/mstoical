# TODO: consider lowering this version requirement to some lower v3.x
cmake_minimum_required(VERSION 3.17)

project(STOICAL VERSION 0.1.1 LANGUAGES C)
# this code looks quite old, let's not set a C standard version for now

# escape version string for ease of setting string as macro
set(
    STOICAL_VERSION_STRING
    "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
)
set(STOICAL_ESCAPED_VERSION_STRING "\"${STOICAL_VERSION_STRING}\"")
# XXX: Hardcoded libroot path for now. TODO: use CMake to set it programmatically
set(STOICAL_ESCAPED_LIBROOT_STRING "\"/usr/local/lib/stoical\"")

add_subdirectory(src)
