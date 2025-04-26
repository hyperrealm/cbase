
lib_LTLIBRARIES = libcbase.la libcbase_mt.la

# Versioning rules ( C:R:A )
# 1. Start with version 0:0:0.
# 2. If the library source code has changed at all, incremement R.
# 3. If any interfaces have been added, removed, or changed, increment C and
#    set R to 0.
# 4. If any interfaces have been added, increment A.
# 5. If any interfaces have been removed, set A to 0.
# For more info see page 27 of the GNU Libtool Manual.

VERINFO = -version-info 9:0:0

libcbase_la_LDFLAGS = $(VERINFO)
libcbase_mt_la_LDFLAGS = $(VERINFO)

libsrc = bitstring.c btree.c byteord.c darray.c debug.c dlobject.c dstring.c \
	error.c except.c exec.c file.c filedesc.c hashtab.c hex.c \
	io.c linklist.c log.c memfile.c memory.c netinfo.c pty.c random.c \
	sched.c sem.c shmem.c signals.c sockctl.c sockio.c strings.c \
	strbuf.c system.c time.c timer.c tty.c vector.c version.c \
	netcommon.h getXXbyYY_r.c getXXbyYY_r.h mempool.c

libinc = cbase/cbase.h cbase/data.h cbase/defs.h cbase/cerrno.h \
	cbase/except.h cbase/ipc.h cbase/net.h cbase/sched.h \
	cbase/system.h cbase/util.h cbase/version.h

libcbase_la_SOURCES =  $(libsrc)

libcbase_mt_la_SOURCES = $(libsrc)

libcbase_la_LIBADD =

libcflags = -Wall -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -I.

libcbase_la_CFLAGS = $(libcflags)

libcbase_mt_la_CFLAGS = $(libcflags) -DTHREADED_LIBRARY
cbaseincludedir = $(includedir)/cbase

cbaseinclude_HEADERS = $(libinc)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libcbase.pc libcbase_mt.pc
