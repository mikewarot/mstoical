add_executable(stoical debug.c dict.c hash.c kernel.c mem.c signal.c string.c term.c)
target_compile_definitions(
    stoical PRIVATE
    -DVERSION=${STOICAL_ESCAPED_VERSION_STRING}
    -DLIBROOT=${STOICAL_ESCAPED_LIBROOT_STRING}
)

if(Threads_FOUND)
    target_link_libraries(stoical PRIVATE Threads::Threads)
endif()

if(Readline_FOUND)
    target_link_libraries(stoical PRIVATE Readline)
endif()

if(Termcap_FOUND)
    target_link_libraries(stoical PRIVATE Termcap)
endif()
