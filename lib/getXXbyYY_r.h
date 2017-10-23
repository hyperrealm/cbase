/* ----------------------------------------------------------------------------
   cbase - A C Foundation Library
   Copyright (C) 1994-2010  Mark A Lindner

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

#ifndef __cbase_getXXbyYY_r_h
#define __cbase_getXXbyYY_r_h

#include "config.h"

#include <netdb.h>

#ifdef HAVE_GETxxxxBYyyyy_R_POSIX

#define C_gethostbyname_r gethostbyname_r
#define C_gethostbyaddr_r gethostbyaddr_r
#define C_getservbyname_r getservbyname_r
#define C_getservbyport_r getservbyport_r

#else /* ! HAVE_GETxxxxBYyyyy_R_POSIX */

extern int C_gethostbyname_r(const char *name, struct hostent *ret,
                             char *buffer, int buflen, struct hostent **result,
                             int *h_errnop);

extern int C_gethostbyaddr_r(const char *addr, int length, int type,
                             struct hostent *ret, char *buffer, int buflen,
                             struct hostent **result, int *h_errnop);

extern int C_getservbyname_r(const char *name, const char *proto,
                             struct servent *ret, char *buffer, int buflen,
                             struct servent **result);

extern int C_getservbyport_r(int port, const char *proto, struct servent *ret,
                             char *buffer, int buflen,
                             struct servent **result);

#endif /* HAVE_GETxxxxBYyyyy_R_POSIX */

#endif /* __cbase_getXXbyYY_r_h */

/* end of header file */
