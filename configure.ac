# Process this file with autoconf to produce a configure script.
AC_INIT(cbase, 1.3.7, hyperrealm@gmail.com)
AC_CONFIG_AUX_DIR([aux-build])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

AC_CHECK_LIB(crypt, crypt)
AC_CHECK_LIB(dl, dlopen)
AC_CHECK_LIB(pthread, pthread_create)
AC_CHECK_LIB(rt, sem_open)
AC_CHECK_LIB(rt, shm_open)
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(nsl, gethostbyaddr)
AC_CHECK_LIB(util, openpty)

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h netinet/in.h stdlib.h string.h sys/file.h sys/ioctl.h sys/time.h termios.h unistd.h stdint.h crypt.h stropts.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_CHECK_MEMBERS([struct msghdr.msg_accrights, struct msghdr.msg_control],,,
[#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif])

dnl Checks for POSIX socket types

AH_TEMPLATE([HAVE_TYPE_SOCKLEN_T], [Define if sys/socket.h or unistd.h defines socklen_t])

AC_MSG_CHECKING(for socklen_t in sys/socket.h or unistd.h)
AC_TRY_LINK([#include <sys/socket.h>
#include <unistd.h>],
[socklen_t t;],
AC_DEFINE(HAVE_TYPE_SOCKLEN_T)
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))

AH_TEMPLATE([HAVE_TYPE_IN_ADDR_T], [Define if netinet/in.h or sys/types.h defines in_addr_t])

AC_MSG_CHECKING(for in_addr_t in netinet/in.h or sys/types.h)
AC_TRY_LINK([#include <sys/types.h>
#include <netinet/in.h>],
[in_addr_t t;],
AC_DEFINE(HAVE_TYPE_IN_ADDR_T)
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))

AH_TEMPLATE([HAVE_TYPE_IN_PORT_T], [Define if netinet/in.h or sys/types.h defines in_port_t])

AC_MSG_CHECKING(for in_port_t in netinet/in.h or sys/types.h)
AC_TRY_LINK([#include <sys/types.h>
#include <netinet/in.h>],
[in_port_t t;],
AC_DEFINE(HAVE_TYPE_IN_PORT_T)
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))

dnl Checks for constant CMSG_SPACE

AC_MSG_CHECKING([for constant CMSG_SPACE])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
  #if HAVE_SYS_SOCKET_H
  #include <sys/socket.h>
  #endif

  int a[CMSG_SPACE(int)];
]])], [
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_CONSTANT_CMSG_SPACE, 1, [Define to 1 if CMSG_SPACE is constant])
], [
  AC_MSG_RESULT(no)
])

dnl Checks for gethostbyname_r & friends

AC_SEARCH_LIBS(gethostbyname_r, [socket nsl])
AC_SEARCH_LIBS(getservbyname_r, [socket nsl])

AH_TEMPLATE([HAVE_GETxxxxBYyyyy_R_POSIX], [Define if POSIX-style getXXXXbyYYYY_r functions are available.])

AC_MSG_CHECKING([for POSIX-style gethostbyXXXX_r and getservbyXXXX_r])
AC_TRY_LINK([
#define _POSIX_PTHREAD_SEMANTICS
#include <netdb.h>
#include <stddef.h>
],
[
  struct hostent *he;
  struct servent *se;

  he = gethostbyname_r((const char *)NULL, (struct hostent *)NULL,
	(char *)NULL, (int)0, (struct hostent **)NULL, (int *)NULL);
  he = gethostbyaddr_r((const char *)NULL, (int)0, (int)0,
	(struct hostent *)NULL, (char *)NULL, (int)0, (struct hostent **)NULL,
	(int *)NULL);

  se = getservbyname_r((const char *)NULL, (const char *)NULL,
	(struct servent *)NULL, (char *)NULL, (int)0, (struct servent **)NULL);
  se = getservbyport_r((int)0, (const char *)NULL,
	(struct servent *)NULL, (char *)NULL, (int)0, (struct servent **)NULL);
],
AC_DEFINE(HAVE_GETxxxxBYyyyy_R_POSIX)
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no)
)

AH_TEMPLATE([HAVE_GETxxxxBYyyyy_R_SUN], [Define if Solaris-style getXXXXbyYYYY_r functions are available.])

AC_MSG_CHECKING([for Solaris-style gethostbyXXXX_r and getservbyXXXX_r])
AC_TRY_LINK([
#define _POSIX_PTHREAD_SEMANTICS
#include <netdb.h>
#include <stddef.h>
],
[ struct hostent *he;
  struct servent *se;

  he = gethostbyname_r((const char *)NULL, (struct hostent *)NULL,
	(char *)NULL, (int)0, (int *)NULL);
  he = gethostbyaddr_r((const char *)NULL, (int)0, (int)0,
	(struct hostent *)NULL, (char *)NULL, (int)0, (int *)NULL);

  se = getservbyname_r((const char *)NULL, (const char *)NULL,
	(struct servent *)NULL, (char *)NULL, (int)0);
  se = getservbyport_r((int)0, (const char *)NULL,
	(struct servent *)NULL, (char *)NULL, (int)0);
],
AC_DEFINE(HAVE_GETxxxxBYyyyy_R_SUN)
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no)
)

dnl Checks for library functions.

AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 flockfile ftruncate getcwd inet_ntoa localtime_r memmove memset mkdir munmap pathconf select socket strchr strerror strpbrk uname getgrnam_r sranddev])

dnl Check for expat XML parser library

AC_CHECK_HEADER(expat.h,have_expat_h=yes,have_expat_h=no)
AC_CHECK_LIB(expat, XML_ParserCreate,
[
LIBS="-lexpat $LIBS"
have_expat_lib=yes
],)

AH_TEMPLATE([HAVE_LIBEXPAT], [Define if the expat XML parser library is available])

if test "x$have_expat_h" = xyes && test "x$have_expat_lib" = xyes; then
AC_DEFINE(HAVE_LIBEXPAT)
else
cat <<EOF

*******************************************************************
You do not have the expat library, so the XML functions will not be
compiled.
You may obtain this library at:
http://www.jclark.com/xml/expat.html
*******************************************************************

EOF
fi

dnl AC_CONFIG_FILES([])
AC_OUTPUT([Makefile lib/Makefile lib/libcbase.pc lib/libcbase_mt.pc
	doc/Makefile])

