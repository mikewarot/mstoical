# TODO: consider lowering this version requirement to some lower v3.x
cmake_minimum_required(VERSION 3.17)

project(STOICAL VERSION 0.1.9 LANGUAGES C)
# this code looks quite old, let's not set a C standard version for now

# BEGIN macros
# escape version string for ease of setting string as macro
set(
    STOICAL_VERSION_STRING
    "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
)
set(STOICAL_ESCAPED_VERSION_STRING "\"${STOICAL_VERSION_STRING}\"")
# XXX: Hardcoded libroot path for now. TODO: use CMake to set it programmatically
set(STOICAL_ESCAPED_LIBROOT_STRING "\"/usr/local/lib/stoical\"")
# END macros

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckTypeSize)

# BEGIN configure variables for config.h
set(DEBUG ON) # TODO: set from CMAKE_BUILD_TYPE == Debug
# header file checks
# XXX: HAVE_LIBPTHREAD
# XXX: HAVE_LIBREADLINE
# XXX: HAVE_LIBTERMCAP
check_symbol_exists(regcomp regex.h HAVE_REGCOMP)
check_include_files(sys/sendfile.h HAVE_SYS_SENDFILE_H)
# package stats
set(PACKAGE_BUGREPORT "https://github.com/saxbophone")
set(PACKAGE_NAME "STOICAL")
set(PACKAGE_STRING "STOICAL ${STOICAL_VERSION_STRING}")
set(PACKAGE_VERSION ${STOICAL_VERSION_STRING})
# debug stuff
set(PROFILE ON)
set(SAFE ON)
# type statistics checks
check_type_size("double" SIZEOF_DOUBLE)
check_type_size("float" SIZEOF_FLOAT)
check_type_size("int" SIZEOF_INT)
check_type_size("long" SIZEOF_LONG)
check_type_size("short" SIZEOF_SHORT)
check_type_size("void*" SIZEOF_VOID_P)
# END configure variables

configure_file(config.h.in config.h @ONLY)

# make sure the processed version of config.h is in the include path
# TODO: consider doing this non-globally
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
