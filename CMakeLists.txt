# TODO: consider lowering this version requirement to some lower v3.x
cmake_minimum_required(VERSION 3.17)

project(STOICAL VERSION 0.1.9 LANGUAGES C)
# from looking at the code, this project was probably written in ANSI C, with GNU extensions and some C99 features
set(CMAKE_C_STANDARD 99)

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

# add custom dependencies directory
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

# dependency checks
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(Readline)
find_package(Termcap)

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckTypeSize)

# BEGIN configure variables for config.h
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(DEBUG ON)
endif()
set(HAVE_LIBPTHREAD ${Threads_FOUND})
set(HAVE_LIBREADLINE ${Readline_FOUND})
set(HAVE_LIBTERMCAP ${Termcap_FOUND})
# header file checks
check_symbol_exists(regcomp regex.h HAVE_REGCOMP)
check_include_files(sys/sendfile.h HAVE_SYS_SENDFILE_H)
# package stats
set(PACKAGE_BUGREPORT "https://github.com/saxbophone")
set(PACKAGE_NAME "STOICAL")
set(PACKAGE_STRING "STOICAL ${STOICAL_VERSION_STRING}")
set(PACKAGE_TARNAME "stoical")
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

add_subdirectory(src)

# make sure the processed version of config.h is in the include path
target_include_directories(stoical PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

include(CTest)
enable_testing()
# XXX: interactive mode, no easy way to test currently
# add_test(NAME stoical COMMAND stoical)
add_test(NAME stoical_help COMMAND stoical -h)
# XXX: -v option is documented in the help text but isn't implemented!
# add_test(NAME stoical_version COMMAND stoical -v)
# XXX: documented as "Exectue the instructions 'code'" --what could this mean?
# add_test(NAME stoical_execute COMMAND stoical -e)
add_test(NAME stoical_library_path COMMAND stoical -l "${CMAKE_SOURCE_DIR}/lib" -f "${CMAKE_SOURCE_DIR}/examples/threads")
add_test(NAME stoical_run_file COMMAND stoical -f "${CMAKE_SOURCE_DIR}/examples/threads")
