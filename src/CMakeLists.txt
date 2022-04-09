add_executable(stoical debug.c dict.c hash.c kernel.c mem.c signal.c string.c term.c)
target_compile_definitions(
    stoical PRIVATE
    -DVERSION=${STOICAL_ESCAPED_VERSION_STRING}
    -DLIBROOT=${STOICAL_ESCAPED_LIBROOT_STRING}
)
