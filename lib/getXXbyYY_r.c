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
   License along with this library; if not,  see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

/* gethostbyname_r, gethostbyaddr_r, getservbyname_r, and
 * getservbyport_r are particularly non-portable. As of this writing,
 * they are available on Solaris and Linux, but not on OS X. The Linux
 * versions have the POSIX-style parameter list while the Solaris
 * versions are slightly different (they have one less parameter). To
 * make things even more interesting, on Linux these functions live in
 * glibc while on Solaris the gethostby*_r functions are in the `nsl'
 * library and the getservby*_r functions are in the `socket' library
 * (even though they are all declared in `netdb.h').
 *
 * To simplify the code that uses these functions, we declare our own
 * versions of them. If the host system has the POSIX-style functions,
 * we simply #define our functions to be aliases for the native
 * functions. If the host system has the Solaris-style functions
 * (presumably because the host system *is* Solaris) then our
 * functions are simple wrappers around the native functions to
 * provide a POSIX-style interface. Finally, if native functions are
 * not available at all, we emulate them by calling their
 * non-reentrant counterparts and doing a deep-copy of the results
 * into the user-specified buffers. In the multi-threaded version of
 * the library, we additionally wrap these calls in mutex locks to
 * provide thread-safety.
 */

/* Feature test switches */

#include "config.h"

/* System headers */

#include <string.h>
#include <errno.h>
#ifdef THREADED_LIBRARY
#include <pthread.h>
#endif

/* Local headers */

#include "getXXbyYY_r.h"

/* File scope variables */

#if !(defined(HAVE_GETxxxxBYyyyy_R_POSIX) || defined(HAVE_GETxxxxBYyyyy_R_SUN))

#ifdef THREADED_LIBRARY
static pthread_mutex_t __C_getXXbyYY_r_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#endif

/* File scope functions */

#if !(defined(HAVE_GETxxxxBYyyyy_R_POSIX) || defined(HAVE_GETxxxxBYyyyy_R_SUN))

static int __C_copy_array(char **src, char *buf, size_t bufsz)
{
  char *p = buf, **q, **w;
  int alen, left = bufsz, pos = 0, off, pad, hl;

  /* compute the length of the pointer array first */

  for(alen = 0, q = src; *q; ++alen, ++q);
  off = sizeof(char *) * ++alen;

  if(left < off)
    return(0);

  /* copy strings after the end of the space reserved for the array,
     while storing pointers to the copied strings in successive slots in
     that reserved space */

  left -= off;
  pos += off;
  p += off;

  for(w = &buf, q = src; *q; ++q, ++w)
  {
    hl = strlen(*q) + 1;

    if(left < hl)
      return(0);

    strncpy(p, *q, hl);
    *w = p;
    p += hl;
    left -= hl;
    off += hl;
  }
  *w = NULL;

  /* skip enough bytes to word-align next write */

  pad = sizeof(void *) - (off % sizeof(void *));

  if(left < pad)
    return(0);

  off += pad;

  return(off);
}

/*
 */

static struct servent * __C_copy_struct_servent(struct servent *rse,
                                                struct servent *result,
                                                char *buffer, int buflen)
{
  char *p = buffer;
  int left = buflen, hl, len = 0, r;

  /* copy the servent structure */

  memcpy((void *)result, (void *)rse, sizeof(struct servent));

  /* copy the canonical name */

  hl = strlen(rse->s_name) + 1;
  if(left < hl)
    return(NULL);

  strncpy(p, rse->s_name, hl);
  len += hl;
  p += hl;
  left -= hl;

  /* copy the array of aliases */

  r = __C_copy_array(rse->s_aliases, p, left);
  if(r == 0)
    return(NULL);

  len += r;
  p += r;
  left -= r;

  /* copy the protocol name */

  hl = strlen(rse->s_proto) + 1;
  if(left < hl)
    return(NULL);

  strncpy(p, rse->s_proto, hl);

  return(result);
}

/*
 */

static struct hostent * __C_copy_struct_hostent(struct hostent *rhe,
                                                struct hostent *result,
                                                char *buffer, int buflen)
{
  char *p = buffer;
  int left = buflen, hl, len = 0, r;

  /* copy the hostent structure */

  memcpy((void *)result, (void *)rhe, sizeof(struct hostent));

  /* copy the canonical name */

  hl = strlen(rhe->h_name) + 1;
  if(left < hl)
    return(NULL);

  strncpy(p, rhe->h_name, hl);
  len += hl;
  p += hl;
  left -= hl;

  /* copy the array of aliases */

  r = __C_copy_array(rhe->h_aliases, p, left);
  if(r == 0)
    return(NULL);

  len += r;
  p += r;
  left -= r;

  /* copy the array of addresses */

  r = __C_copy_array(rhe->h_addr_list, p, left);
  if(r == 0)
    return(NULL);

  return(result);
}

#endif

/*
 */

#if defined(HAVE_GETxxxxBYyyyy_R_SUN)

int C_gethostbyname_r(const char *name, struct hostent *ret, char *buffer,
                      int buflen, struct hostent **result, int *h_errnop)
{
  struct hostent *r = gethostbyname_r(name, ret, buffer, buflen, h_errnop);

  if(result != NULL)
    *result = r;

  return(r ? 0 : -1);
}

/*
 */

int C_gethostbyaddr_r(const char *addr, int length, int type, struct hostent *ret,
                      char *buffer, int buflen, struct hostent **result,
                      int *h_errnop)
{
  struct hostent *r = gethostbyaddr_r(addr, length, type, ret, buffer, buflen,
                                      h_errnop);

  if(result != NULL)
    *result = r;

  return(r ? 0 : -1);
}

/*
 */

int C_getservbyname_r(const char *name, const char *proto, struct servent *ret,
                      char *buffer, int buflen, struct servent **result)
{
  struct servent *r = getservbyname_r(name, proto, ret, buffer, buflen);

  if(result != NULL)
    *result = r;

  return(r ? 0 : -1);
}

/*
 */

int C_getservbyport_r(int port, const char *proto, struct servent *ret,
                      char *buffer, int buflen, struct servent **result)
{
  struct servent *r = getservbyport_r(port, proto, ret, buffer, buflen);

  if(result != NULL)
    *result = r;

  return(r ? 0 : -1);
}

#elif !defined(HAVE_GETxxxxBYyyyy_R_POSIX)

/*
 */

int C_gethostbyname_r(const char *name, struct hostent *ret, char *buffer,
                      int buflen, struct hostent **result, int *h_errnop)
{
  struct hostent *rhe;

#ifdef THREADED_LIBRARY
  pthread_mutex_lock(&__C_getXXbyYY_r_mutex);
#endif

  rhe = gethostbyname(name);
  if(! rhe)
  {
    *h_errnop = h_errno;
    ret = NULL;
  }
  else if(! __C_copy_struct_hostent(rhe, ret, buffer, buflen))
  {
    errno = ERANGE;
    ret = NULL;
  }

  if(result)
    *result = ret;

#ifdef THREADED_LIBRARY
  pthread_mutex_unlock(&__C_getXXbyYY_r_mutex);
#endif

  return(ret ? 0 : -1);
}

/*
 */

int C_gethostbyaddr_r(const char *addr, int length, int type,
                      struct hostent *ret, char *buffer, int buflen,
                      struct hostent **result, int *h_errnop)
{
  struct hostent *rhe;

#ifdef THREADED_LIBRARY
  pthread_mutex_lock(&__C_getXXbyYY_r_mutex);
#endif

  rhe = gethostbyaddr(addr, length, type);
  if(! rhe)
  {
    *h_errnop = h_errno;
    ret = NULL;
  }
  else if(! __C_copy_struct_hostent(rhe, ret, buffer, buflen))
  {
    errno = ERANGE;
    ret = NULL;
  }

  if(result)
    *result = ret;

#ifdef THREADED_LIBRARY
  pthread_mutex_unlock(&__C_getXXbyYY_r_mutex);
#endif

  return(ret ? 0 : -1);
}

/*
 */

int C_getservbyname_r(const char *name, const char *proto, struct servent *ret,
                      char *buffer, int buflen, struct servent **result)
{
  struct servent *rse;

#ifdef THREADED_LIBRARY
  pthread_mutex_lock(&__C_getXXbyYY_r_mutex);
#endif

  rse = getservbyname(name, proto);
  if(! rse)
    ret = NULL;
  else if(! __C_copy_struct_servent(rse, ret, buffer, buflen))
  {
    errno = ERANGE;
    ret = NULL;
  }

  if(result)
    *result = ret;

#ifdef THREADED_LIBRARY
  pthread_mutex_unlock(&__C_getXXbyYY_r_mutex);
#endif

  return(ret ? 0 : -1);
}

/*
 */

int C_getservbyport_r(int port, const char *proto, struct servent *ret,
                      char *buffer, int buflen, struct servent **result)
{
  struct servent *rse;

#ifdef THREADED_LIBRARY
  pthread_mutex_lock(&__C_getXXbyYY_r_mutex);
#endif

  rse = getservbyport(port, proto);
  if(! rse)
    ret = NULL;
  else if(! __C_copy_struct_servent(rse, ret, buffer, buflen))
  {
    errno = ERANGE;
    ret = NULL;
  }

  if(result)
    *result = ret;

#ifdef THREADED_LIBRARY
  pthread_mutex_unlock(&__C_getXXbyYY_r_mutex);
#endif

  return(ret ? 0 : -1);
}

#endif

/* end of source file */
