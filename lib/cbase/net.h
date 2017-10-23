/* ----------------------------------------------------------------------------
   cbase - A C Foundation Library
   Copyright (C) 1994-2014  Mark A Lindner

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

#ifndef __cbase_net_h
#define __cbase_net_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cbase/defs.h>

/* ----------------------------------------------------------------------------
 * sockets
 * ----------------------------------------------------------------------------
 */

  typedef struct c_socket_t
  {
    int sd;
    FILE *sfp;
    struct sockaddr_in laddr;
    struct sockaddr_in raddr;
    unsigned short flags;
    unsigned short state;
    unsigned short type;
    unsigned short _pad;
    int timeout;
    int conn_timeout;
    void *hook;
  } c_socket_t;

/* ----------------------------------------------------------------------------
 * socket control functions
 * ----------------------------------------------------------------------------
 */

  extern c_socket_t *C_socket_create(int type);
  extern c_bool_t C_socket_create_s(c_socket_t *s, int type);
  extern c_bool_t C_socket_destroy(c_socket_t *s);
  extern c_bool_t C_socket_destroy_s(c_socket_t *s);
  extern c_bool_t C_socket_listen(c_socket_t *s, in_port_t port);
  extern c_socket_t *C_socket_accept(c_socket_t *ms);
  extern c_bool_t C_socket_accept_s(c_socket_t *s, c_socket_t *ms);

  extern c_bool_t C_socket_connect(c_socket_t *s, const char *host,
                                   in_port_t port);
  extern c_bool_t C_socket_shutdown(c_socket_t *s, uint_t how);

  extern c_bool_t C_socket_fopen(c_socket_t *s, int buffering);
  extern c_bool_t C_socket_fclose(c_socket_t *s);
  extern c_socket_t *C_socket_reopen(int sd);
  extern c_bool_t C_socket_reopen_s(c_socket_t *s, int sd);

  extern c_bool_t C_socket_get_peeraddr(c_socket_t *s, char *buf,
                                        size_t bufsz);

  extern c_bool_t C_socket_set_option(c_socket_t *s, uint_t option,
                                      c_bool_t flag, uint_t value);
  extern c_bool_t C_socket_get_option(c_socket_t *s, uint_t option,
                                      c_bool_t *flag, uint_t *value);

/* buffering modes */

#define C_NET_BUFFERING_NONE _IONBF
#define C_NET_BUFFERING_LINE _IOLBF
#define C_NET_BUFFERING_FULL _IOFBF

/* socket shutdown states */

#define C_NET_SHUTRD 1
#define C_NET_SHUTWR 2
#define C_NET_SHUTALL 3

/* socket options */

#define C_NET_OPT_BLOCK 1
#define C_NET_OPT_LINGER 2
#define C_NET_OPT_REUSEADDR 3
#define C_NET_OPT_OOBINLINE 4
#define C_NET_OPT_KEEPALIVE 5
#define C_NET_OPT_RECVBUF 6
#define C_NET_OPT_SENDBUF 7

/* socket types */

#define C_NET_MAXTYPE 2
#define C_NET_NTYPES 3
#define C_NET_TCP 0
#define C_NET_UDP 1
#define C_NET_UNKNOWN 2
#define C_NET_OTHER C_NET_UNKNOWN

/* constants */

#define C_NET_BACKLOG 5

/* flags and masks */

#define C_NET_MSHUT 0x01
#define C_NET_MUNBLOCK 0x02
#define C_NET_OSHUT 0
#define C_NET_OSHUTRD 0
#define C_NET_OSHUTWR 1
#define C_NET_OUNBLOCK 2

/* macros */

#define C_socket_set_iobufsz(S, I, O)           \
  C_socket_iobufsz((S), (I), (O), TRUE)
#define C_socket_get_iobufsz(S, I, O)           \
  C_socket_iobufsz((S), (I), (O), FALSE)
#define C_socket_block(S)                               \
  C_socket_set_option((S), C_NET_OPT_BLOCK, TRUE, 0)
#define C_socket_unblock(S)                             \
  C_socket_set_option((S), C_NET_OPT_BLOCK, FALSE, 0)
#define C_socket_get_fd(S) ((S)->sd)
#define C_socket_get_fp(S) ((S)->sfp)
#define C_socket_isblocked(S)                           \
  (((S)->flags & C_NET_MUNBLOCK) ? FALSE : TRUE)
#define C_socket_get_type(S) ((S)->type)
#define C_socket_get_ipaddr(S)                          \
  (in_addr_t)(nothl((S)->laddr.sin_addr.s_addr))
#define C_socket_get_peeripaddr(S)                      \
  (in_addr_t)(ntohl((S)->raddr.sin_addr.s_addr))

#define C_socket_set_timeout(S, T)              \
  (S)->timeout = (T)
#define C_socket_get_timeout(S)                 \
  ((S)->timeout)

#define C_socket_set_conn_timeout(S, T)         \
  (S)->conn_timeout = (T)
#define C_socket_get_conn_timeout(S)            \
  ((S)->conn_timeout)

#define C_socket_set_userdata(S, D)             \
  (S)->hook = (D)
#define C_socket_get_userdata(S)                \
  ((S)->hook)

/* ----------------------------------------------------------------------------
 * socket multicast functions
 * ----------------------------------------------------------------------------
 */

  extern c_bool_t C_socket_mcast_join(c_socket_t *s, const char *addr);
  extern c_bool_t C_socket_mcast_leave(c_socket_t *s, const char *addr);

  extern c_bool_t C_socket_mcast_set_ttl(c_socket_t *s, c_byte_t ttl);
  extern c_bool_t C_socket_mcast_set_loop(c_socket_t *s, c_bool_t loop);

/* ----------------------------------------------------------------------------
 * socket I/O functions
 * ----------------------------------------------------------------------------
 */

  extern int C_socket_send(c_socket_t *s, const char *buf, size_t bufsz,
                           c_bool_t oobf);
  extern int C_socket_recv(c_socket_t *s, char *buf, size_t bufsz,
                           c_bool_t boof);

  extern int C_socket_sendto(c_socket_t *s, const char *buf, size_t bufsz,
                             const char *addr, in_port_t port);
  extern int C_socket_recvfrom(c_socket_t *s, char *buf, size_t bufsz,
                               char *addr, size_t addrsz);

  extern int C_socket_sendreply(c_socket_t *s, const char *buf, size_t bufsz);
  extern int C_socket_recvreply(c_socket_t *s, char *buf, size_t bufsz);

  extern int C_socket_sendline(c_socket_t *s, const char *buf);
  extern int C_socket_recvline(c_socket_t *s, char *buf, size_t bufsz);

#define C_NET_DFL_TIMEOUT       30 /* 30 sec */
#define C_NET_DFL_CONN_TIMEOUT  -1 /* infinite */

/* these interfaces are deprecated */
#define C_socket_writeline(S, B, T, X, Y)       \
  C_socket_sendline((S), (B))
#define C_socket_readline(S, B, Z, T, X, Y)     \
  C_socket_recvline((S), (B), (Z))
#define C_socket_wl(S, B, T)                    \
  C_socket_sendline((S), (B))
#define C_socket_rl(S, B, Z, T)                 \
  C_socket_recvline((S), (B), (Z))
/* end of deprecated interfaces */

/* ----------------------------------------------------------------------------
 * network information functions
 * ----------------------------------------------------------------------------
 */

  extern in_port_t C_net_get_svcport(const char *name, uint_t *type);
  extern c_bool_t C_net_get_svcname(in_port_t port, uint_t *type, char *buf,
                                    size_t bufsz);
  extern c_bool_t C_net_resolve(const char *ipaddr, char *buf, size_t bufsz);
  extern c_bool_t C_net_resolve_local(char *addr, char *ipaddr, size_t bufsz,
                                      in_addr_t *ip);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_net_h */

/* end of library header */

