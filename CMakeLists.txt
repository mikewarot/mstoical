# TODO: consider lowering this version requirement to some lower v3.x
cmake_minimum_required(VERSION 3.17)

project(STOICAL VERSION 0.1.1 LANGUAGES C)
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
check_include_files(arpa/inet.h HAVE_ARPA_INET_H)
check_symbol_exists(DIR dirent.h HAVE_DIRENT_H)
# XXX: HAVE_DOPRNT?
check_include_files(fcntl.h HAVE_FCNTL_H)
check_symbol_exists(gethostbyname netdb.h HAVE_GETHOSTBYNAME)
check_include_files(inttypes.h HAVE_INTTYPES_H)
# XXX: who doesn't have libm these days?
set(HAVE_LIBM ON)
# XXX: HAVE_LIBPTHREAD
# XXX: HAVE_LIBREADLINE
# XXX: HAVE_LIBTERMCAP
# XXX: who doesn't have malloc() these days?
set(HAVE_MALLOC ON)
set(HAVE_MEMCHR ON)
set(HAVE_MEMMOVE ON)
check_include_files(memory.h HAVE_MEMORY_H)
check_symbol_exists(mkdir sys/stat.h HAVE_MKDIR)
check_symbol_exists(DIR ndir.h HAVE_NDIR_H)
check_include_files(netdb.h HAVE_NETDB_H)
check_include_files(netinet/in.h HAVE_NETINET_IN_H)
# XXX: HAVE_PTHREAD_DELAY_NP
check_symbol_exists(regcomp regex.h HAVE_REGCOMP)
check_symbol_exists(rmdir unistd.h HAVE_RMDIR)
check_symbol_exists(sendfile sys/sendfile.h HAVE_SENDFILE)
check_symbol_exists(setenv stdlib.h HAVE_SETENV)
check_symbol_exists(socket sys/socket.h HAVE_SOCKET)
# XXX: assume this is an old bug we no longer have
set(HAVE_STAT_EMPTY_STRING_BUG OFF)
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
