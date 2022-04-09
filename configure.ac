# Process this file with autoconf to produce a configure script.
AC_INIT(STOICAL, 0.1.8, wantingwaiting@users.sf.net)
AC_CONFIG_SRCDIR([src/dict.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

dnl Build options
AC_ARG_ENABLE(threads, [  --disable-threads       Disable threads ],
[enable_threads=no],
[enable_threads=yes] )
AC_ARG_ENABLE(readline, [  --disable-readline      Don't use GNU Readline ],
[enable_readline=no],
[enable_readline=yes] )
AC_ARG_ENABLE(safe, [  --disable-safe          Don't check boundaries ],
[enable_safe=no],
[
	enable_safe=yes
	AC_DEFINE(SAFE, 1, [Perform bounds checking on stacks and arrays] )
] )
AC_ARG_ENABLE(debug, [  --enable-debug          Print verbose debugging info ],
[
	enable_debug=yes
	AC_DEFINE(DEBUG, 1, [Print verbose debugging info])
	dnl Force profiling and safety 
	enable_safe=yes
	enable_profile=yes
],
[enable_debug=no] )
AC_ARG_ENABLE(profile, [  --enable-profile        Track invocation, times, memory and etc..],
[
	enable_profile=yes
	AC_DEFINE(PROFILE, 1, [Track invocation, times, memory and etc..])
],
[enable_profile=no] )

# Checks for libraries.
if test $enable_threads = yes; then
	AC_CHECK_LIB([pthread], [pthread_create])
fi
if test $enable_readline = yes; then
	AC_CHECK_LIB([readline], [readline])
	AC_CHECK_LIB([termcap], [tputs])
fi
AC_CHECK_LIB([m], [fabsf])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h termios.h unistd.h sys/sendfile.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])

AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(void *, 4) dnl sizeof (void *)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(float, 4)
AC_CHECK_SIZEOF(double, 4])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gethostbyname memchr memmove mkdir regcomp rmdir setenv socket strtol pthread_delay_np sendfile pselect])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/Makefile
		 doc/stoical.1])
AC_OUTPUT

dnl sumarize configuration 
AC_MSG_RESULT()
AC_MSG_RESULT(----{ $PACKAGE_STRING)
AC_MSG_RESULT(CFLAGS	: ${CFLAGS})
AC_MSG_RESULT(LIBS	: ${LIBS})
AC_MSG_RESULT(prefix	: ${prefix})
AC_MSG_RESULT()
AC_MSG_RESULT(threads	: ${enable_threads})
AC_MSG_RESULT(debug	: ${enable_debug})
AC_MSG_RESULT(safe	: ${enable_safe})
AC_MSG_RESULT(profile	: ${enable_profile})
AC_MSG_RESULT(}-----)

