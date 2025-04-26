/* ----------------------------------------------------------------------------
   cbase - A C Foundation Library
   Copyright (C) 1994-2025  Mark A Lindner

   This file is part of cbase.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

/* Feature test switches */

#include "config.h"

/* System headers */

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef THREADED_LIBRARY
#include <pthread.h>
#endif /* THREADED_LIBRARY */

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/cerrno.h"

/* File scope variables */

static char *__C_error_progname = "<program>";

#ifdef THREADED_LIBRARY

static pthread_once_t __C_error_once = PTHREAD_ONCE_INIT;
static pthread_key_t __C_error_key;

#else /* THREADED_LIBRARY */

static int __c_errno = 0; /* global cbase error number */

#endif /* THREADED_LIBRARY */

static const int __C_error_nerr = 34;

static const char *__C_error_errlist[] =
  { /*  0 */ "OK",
    /*  1 */ "invalid argument(s)",
    /*  2 */ "socket() call failed",
    /*  3 */ "call not valid in this state",
    /*  4 */ "unable to resolve network address",
    /*  5 */ "bind() call failed",
    /*  6 */ "operation would block",
    /*  7 */ "accept() call failed",
    /*  8 */ "wrong socket type for this operation",
    /*  9 */ "listen() call failed",
    /* 10 */ "unable to obtain socket info",
    /* 11 */ "fcntl() call failed",
    /* 12 */ "connection to peer lost",
    /* 13 */ "fdopen() call failed",
    /* 14 */ "connect() call failed",
    /* 15 */ "no connection available",
    /* 16 */ "send() call failed",
    /* 17 */ "recv() call failed",
    /* 18 */ "UDP message too big",
    /* 19 */ "sendto() call failed",
    /* 20 */ "recvfrom() call failed",
    /* 21 */ "connection or I/O timed out",
    /* 22 */ "unable to obtain service info",
    /* 23 */ "unable to fork",
    /* 24 */ "table full",
    /* 25 */ "select() call failed",
    /* 26 */ "ioctl() call failed",
    /* 27 */ "unable to get/set TTY attributes",
    /* 28 */ "descriptor does not refer to a TTY",
    /* 29 */ "getpty() call failed",
    /* 30 */ "open() call failed",
    /* 31 */ "general pseudoterminal error",
    /* 32 */ "execv() call failed",
    /* 33 */ "function/feature not implemented"
  };

/* Functions */

void C_error_init(const char *progname)
{
  if(progname)
    __C_error_progname = (char *)progname;
}

/*
 */

void C_error_printf(const char *fmt, ...)
{
  va_list vp;

#ifdef THREADED_LIBRARY
#ifdef HAVE_FLOCKFILE
  flockfile(stderr);
#endif /* HAVE_FLOCKFILE */
#endif /* THREADED_LIBRARY */

  fprintf(stderr, "%s: ", __C_error_progname);
  va_start(vp, fmt);
  vfprintf(stderr, fmt, vp);
  va_end(vp);
  fflush(stderr);

#ifdef THREADED_LIBRARY
#ifdef HAVE_FLOCKFILE
  funlockfile(stderr);
#endif /* HAVE_FLOCKFILE */
#endif /* THREADED_LIBRARY */
}

/*
 */

void C_error_usage(const char *usage)
{

  fprintf(stderr, "%s: Usage: %s %s\n", __C_error_progname,
          __C_error_progname, usage);
  fflush(stderr);
}

/*
 */

void C_error_syserr(void)
{

#ifdef THREADED_LIBRARY
#ifdef HAVE_FLOCKFILE
  flockfile(stderr);
#endif /* HAVE_FLOCKFILE */
#endif /* THREADED_LIBRARY */

  fprintf(stderr, "%s: %s\n", __C_error_progname, strerror(errno));
  fflush(stderr);

#ifdef THREADED_LIBRARY
#ifdef HAVE_FLOCKFILE
  funlockfile(stderr);
#endif /* HAVE_FLOCKFILE */
#endif /* THREADED_LIBRARY */
}

/*
 */

const char *C_error_string(void)
{
  if((c_errno < 0) || (c_errno >= __C_error_nerr))
    return(NULL);

  return(__C_error_errlist[c_errno]);
}

/*
 */

#ifdef THREADED_LIBRARY

static void __C_error_destructor(void *datum)
{
  C_free(datum);
}

/*
 */

static void __C_error_init_once(void)
{
  pthread_key_create(&__C_error_key, __C_error_destructor);
}

#endif /* THREADED_LIBRARY */

/*
 */

int C_error_get_errno(void)
{
#ifdef THREADED_LIBRARY

  int *e;

  pthread_once(&__C_error_once, __C_error_init_once);

  e = (int *)pthread_getspecific(__C_error_key);
  if(!e)
  {
    /* didn't exist, so create it */

    e = C_new(int);
    *e = C_EOK;
    pthread_setspecific(__C_error_key, (void *)e);
  }

  return(*e);

#else
  return(__c_errno);
#endif /* THREADED_LIBRARY */
}

/*
 */

void C_error_set_errno(int err)
{
#ifdef THREADED_LIBRARY
  int *e;

  pthread_once(&__C_error_once, __C_error_init_once);

  e = (int *)pthread_getspecific(__C_error_key);
  if(!e)
  {
    /* didn't exist, so create it */

    e = C_new(int);
    pthread_setspecific(__C_error_key, (void *)e);
  }

  *e = err;

#else /* THREADED_LIBRARY */
  __c_errno = err;

#endif /* THREADED_LIBRARY */
}

/* end of source file */
