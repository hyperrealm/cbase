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

#ifndef __cbase_netcommon_h
#define __cbase_netcommon_h

#include "config.h"

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <inttypes.h>

#include "cbase/util.h"

#ifndef HAVE_TYPE_IN_ADDR_T
typedef uint32_t in_addr_t;
#endif

#ifndef HAVE_TYPE_IN_PORT_T
typedef uint16_t in_port_t;
#endif

#ifndef HAVE_TYPE_SOCKLEN_T
typedef size_t socklen_t;
#endif

#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

#define C_NET_BUFSZ 8192

#define C_NET_CREATED 0
#define C_NET_LISTENING 1
#define C_NET_ACCEPTING 2
#define C_NET_CONNECTED 3
#define C_NET_SHUTDOWN 4

#define C_NET_NTYPES 3

extern const int __C_net_socktypes[C_NET_NTYPES];
extern const char *__C_net_protocols[C_NET_NTYPES];

extern c_bool_t __C_socket_addr2sock(struct sockaddr_in *sa, const char *addr);
extern c_bool_t __C_socket_sock2addr(struct sockaddr_in *sa, char *addr,
                                     size_t addrsz);

extern c_buffer_t *__C_net_get_buffer(void);

#endif /* __cbase_netcommon_h */

/* end of common header */
