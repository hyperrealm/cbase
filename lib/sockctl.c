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

/* Feature test switches */

#include "config.h"

/* System headers */

#include <fcntl.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>

/* Local headers */

#include "netcommon.h"
#include "cbase/defs.h"
#include "cbase/net.h"
#include "cbase/cerrno.h"
#include "cbase/system.h"
#include "getXXbyYY_r.h"

/* File scope variables */

const int __C_net_socktypes[C_NET_NTYPES] =
  { SOCK_STREAM, SOCK_DGRAM, -1 };

const char *__C_net_protocols[C_NET_NTYPES] =
  { "tcp", "udp", NULL };

/* External functions */

c_bool_t __C_socket_addr2sock(struct sockaddr_in *sa, const char *addr)
{
  struct hostent he, *rhe;
  c_buffer_t *rbuf = __C_net_get_buffer();
  int herr;

  if(!sa || !addr)
    return(FALSE);

  if(isdigit((int)*addr))
  {
    if((sa->sin_addr.s_addr = inet_addr(addr)) == INADDR_NONE)
      return(FALSE);
    sa->sin_family = AF_INET;
  }
  else
  {
  GETHOSTBYNAME:
    if(C_gethostbyname_r(addr, &he, rbuf->buf, rbuf->bufsz, &rhe, &herr) < 0)
    {
      if(herr == ERANGE)
      {
        C_buffer_resize(rbuf, rbuf->bufsz + C_NET_BUFSZ);
        goto GETHOSTBYNAME;
      }
      else
      {
        C_error_set_errno(C_EADDRINFO);
        return(FALSE);
      }
    }

    memcpy((void *)&(sa->sin_addr), (void *)he.h_addr, (size_t)he.h_length);
    sa->sin_family = he.h_addrtype;
  }

  return(TRUE);
}

/*
 */

c_bool_t __C_socket_sock2addr(struct sockaddr_in *sa, char *addr,
                              size_t addrsz)
{
  struct hostent he, *rhe;
  c_buffer_t *rbuf = __C_net_get_buffer();
  int herr;

  if(!sa || !addr || !addrsz)
    return(FALSE);

GETHOSTBYADDR:
  if(C_gethostbyaddr_r((char *)&(sa->sin_addr.s_addr), sizeof(in_addr_t),
                       AF_INET, &he, rbuf->buf, rbuf->bufsz, &rhe, &herr) < 0)
  {
    if(herr == ERANGE)
    {
      C_buffer_resize(rbuf, rbuf->bufsz + C_NET_BUFSZ);
      goto GETHOSTBYADDR;
    }
    else
    {
      C_error_set_errno(C_EADDRINFO);
      return(FALSE);
    }
  }

  strncpy(addr, he.h_name, --addrsz);
  *(addr + addrsz) = NUL;

  return(TRUE);
}

/*
 */

static c_bool_t __C_socket_create(c_socket_t *s, int type)
{
  int sd;

  if((type < 0) || (type >= C_NET_MAXTYPE))
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if((sd = socket(AF_INET, __C_net_socktypes[type], 0)) < 0)
  {
    C_error_set_errno(C_ESOCKET);
    return(FALSE);
  }

  s->sd = sd;
  s->sfp = NULL;
  s->flags = 0;
  s->state = C_NET_CREATED;
  s->type = type;
  s->timeout = C_NET_DFL_TIMEOUT;
  s->conn_timeout = C_NET_DFL_CONN_TIMEOUT;

  return(TRUE);
}

/*
 */

c_bool_t C_socket_create_s(c_socket_t *s, int type)
{
  if(!s)
    return(FALSE);

  C_zero(s, c_socket_t);

  return(__C_socket_create(s, type));
}

/*
 */

c_socket_t * C_socket_create(int type)
{
  c_socket_t *s = C_new(c_socket_t);

  if(! __C_socket_create(s, type))
    return(C_free(s));

  return(s);
}

/*
 */

c_bool_t C_socket_listen(c_socket_t *s, in_port_t port)
{
  struct hostent he, *rhe;
  c_buffer_t *rbuf = __C_net_get_buffer();
  int herr, x = 1;
  char *myname;

  if(!s)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->state != C_NET_CREATED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(FALSE);
  }

  myname = C_system_get_hostname();

GETHOSTBYNAME2:
  if(C_gethostbyname_r(myname, &he, rbuf->buf, rbuf->bufsz, &rhe, &herr) < 0)
  {
    if(herr == ERANGE)
    {
      C_buffer_resize(rbuf, rbuf->bufsz + C_NET_BUFSZ);
      goto GETHOSTBYNAME2;
    }
    else
    {
      C_error_set_errno(C_EADDRINFO);
      return(FALSE);
    }
  }

  s->laddr.sin_addr.s_addr = htonl(INADDR_ANY);
  s->laddr.sin_family = he.h_addrtype;
  s->laddr.sin_port = htons(port);

  if(setsockopt(s->sd, SOL_SOCKET, SO_REUSEADDR, (void *)&x, sizeof(int)) < 0)
  {
    C_error_set_errno(C_ESOCKINFO);
    return(FALSE);
  }

  if(bind(s->sd, (struct sockaddr *)&(s->laddr), sizeof(struct sockaddr_in))
     < 0)
  {
    C_error_set_errno(C_EBIND);
    return(FALSE);
  }

  if(s->type == C_NET_TCP)
  {
    if(listen(s->sd, C_NET_BACKLOG) < 0)
    {
      C_error_set_errno(C_ELISTEN);
      return(FALSE);
    }
    s->state = C_NET_LISTENING;
  }

  return(TRUE);
}

/*
 */

static c_bool_t __C_socket_accept(c_socket_t *s, c_socket_t *ms)
{
  socklen_t sz = (socklen_t)sizeof(struct sockaddr_in);

  if(!ms)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(ms->state != C_NET_LISTENING)
  {
    C_error_set_errno(C_EBADSTATE);
    return(FALSE);
  }

  if(ms->type != C_NET_TCP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(FALSE);
  }

ACCEPT:
  if((s->sd = accept(ms->sd, (struct sockaddr *)&(s->raddr), &sz)) < 0)
  {
    switch(errno)
    {
      case EWOULDBLOCK:
        C_error_set_errno(C_EBLOCKED);
        return(FALSE);

      case EINTR:
      case ECONNABORTED:
        goto ACCEPT;

      default:
        C_error_set_errno(C_EACCEPT);
        return(FALSE);
    }
  }

  s->type = ms->type;
  s->state = C_NET_CONNECTED;
  s->flags = ms->flags;
  s->sfp = NULL;

  s->flags &= ~(C_NET_MUNBLOCK);

  memcpy((void *)&(s->laddr), (void *)&(ms->laddr),
         sizeof(struct sockaddr_in));

  return(TRUE);
}

/*
 */

c_socket_t *C_socket_accept(c_socket_t *ms)
{
  c_socket_t *s = C_new(c_socket_t);

  if(!__C_socket_accept(s, ms))
    return(C_free(s));

  return(s);
}

/*
 */

c_bool_t C_socket_accept_s(c_socket_t *s, c_socket_t *ms)
{

  if(!ms || !s)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  C_zero(s, c_socket_t);

  return(__C_socket_accept(s, ms));
}

/*
 */

c_bool_t C_socket_connect(c_socket_t *s, const char *host, in_port_t port)
{
  int flags = 0, err = 0;
  socklen_t sz = (socklen_t)sizeof(struct sockaddr_in);
  fd_set rfd, wfd;
  c_bool_t ok = FALSE;

  if(!s || !host)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(! *host)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->state != C_NET_CREATED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(FALSE);
  }

  if(!__C_socket_addr2sock(&(s->raddr), host))
  {
    C_error_set_errno(C_EADDRINFO);
    return(FALSE);
  }

  s->raddr.sin_port = htons(port);

  FD_ZERO(&rfd);
  FD_SET(s->sd, &rfd);
  wfd = rfd;

  if(s->conn_timeout > 0)
  {
    flags = fcntl(s->sd, F_GETFL, 0);
    fcntl(s->sd, F_SETFL, flags | O_NONBLOCK);
  }

CONNECT:
  if(connect(s->sd, (struct sockaddr *)&(s->raddr), sz) < 0)
  {
    switch(errno)
    {
      case ECONNREFUSED:
        C_error_set_errno(C_ENOCONN);
        break;

      case EINTR:
        goto CONNECT;

      case EINPROGRESS:
      {
        fd_set rset, wset;
        int r;
        struct timeval tv;

        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_SET(s->sd, &rset);
        FD_SET(s->sd, &wset);
        tv.tv_sec = s->conn_timeout;
        tv.tv_usec = 0;

        r = select(FD_SETSIZE, &rset, &wset, NULL, &tv);

        if(r == 0)
          C_error_set_errno(C_ETIMEOUT);
        else
        {
          if(FD_ISSET(s->sd, &rset) || FD_ISSET(s->sd, &wset))
          {
            socklen_t len = sizeof(err);
            if(getsockopt(s->sd, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
              C_error_set_errno(C_ECONNECT);

            ok = (err == 0);
          }
          else
            C_error_set_errno(C_ETIMEOUT);
        }

        break;
      }

      default:
        C_error_set_errno(C_ECONNECT);
    }
  }
  else
    ok = TRUE;

  if(s->conn_timeout > 0)
    fcntl(s->sd, F_SETFL, flags); /* restore flags */

  if(ok)
  {
    s->state = C_NET_CONNECTED;
    sz = sizeof(struct sockaddr_in);
    getsockname(s->sd, (struct sockaddr *)&(s->laddr), &sz);
    return(TRUE);
  }
  else
    return(FALSE);
}

/*
 */

c_bool_t C_socket_shutdown(c_socket_t *s, uint_t how)
{

  if(!s || (how < C_NET_SHUTRD) || (how > C_NET_SHUTALL))
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->state != C_NET_CONNECTED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(FALSE);
  }

  s->state = C_NET_SHUTDOWN;
  s->flags |= (how << C_NET_OSHUT);
  shutdown(s->sd, --how);

  return(TRUE);
}

/*
 */

static c_bool_t __C_socket_destroy(c_socket_t *s)
{

  if(!s)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(!((s->state == C_NET_CREATED) || ((s->state == C_NET_SHUTDOWN)
                                       && (s->flags & C_NET_MSHUT))))
  {
    C_error_set_errno(C_EBADSTATE);
    return(FALSE);
  }

  if(s->sfp)
    fclose(s->sfp);

  close(s->sd);

  return(TRUE);
}

/*
 */

c_bool_t C_socket_destroy(c_socket_t *s)
{
  c_bool_t ok;

  ok = __C_socket_destroy(s);
  if(ok)
    C_free(s);

  return(ok);
}

/*
 */

c_bool_t C_socket_destroy_s(c_socket_t *s)
{

  return(__C_socket_destroy(s));
}

/*
 */

c_bool_t C_socket_fopen(c_socket_t *s, int buffering)
{

  if(!s)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->state != C_NET_CONNECTED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(FALSE);
  }

  if(s->type != C_NET_TCP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(FALSE);
  }

  if(!(s->sfp = fdopen(s->sd, "r+")))
  {
    C_error_set_errno(C_EFDOPEN);
    return(FALSE);
  }

  if(setvbuf(s->sfp, NULL, buffering, 0))
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_socket_get_peeraddr(c_socket_t *s, char *buf, size_t bufsz)
{

  if(!s || !buf || !bufsz)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->state != C_NET_CONNECTED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(FALSE);
  }

  if(!__C_socket_sock2addr(&(s->raddr), buf, bufsz))
  {
    C_error_set_errno(C_EADDRINFO);
    return(FALSE);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_socket_fclose(c_socket_t *s)
{

  if(!s)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->sfp)
  {
    fclose(s->sfp);
    s->sfp = NULL;
  }

  return(TRUE);
}

/*
 */

static c_bool_t __C_socket_reopen(c_socket_t *s, int sd)
{
  int flags, i, type;
  socklen_t sz;

  if(sd < 0)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if((flags = fcntl(sd, F_GETFL, 0)) == -1)
  {
    C_error_set_errno(C_EFCNTL);
    return(FALSE);
  }

  /* get socket type */

  sz = (socklen_t)sizeof(int);
  if(getsockopt(sd, SOL_SOCKET, SO_TYPE, (char *)&type, &sz))
  {
    C_error_set_errno(C_ESOCKINFO);
    return(FALSE);
  }

  for(i = 0; i < C_NET_MAXTYPE; ++i)
    if(type == __C_net_socktypes[i]) break;
  if(i == C_NET_MAXTYPE)
  {
    C_error_set_errno(C_EBADTYPE);
    return(FALSE);
  }

  s->sd = sd;
  s->type = i;
  s->state = C_NET_CONNECTED;

  /* get socket's local address */

  sz = sizeof(struct sockaddr_in);
  if(getsockname(s->sd, (struct sockaddr *)&(s->laddr), &sz) < 0)
  {
    C_error_set_errno(C_ESOCKINFO);
    return(FALSE);
  }

  /* get socket's remote address */

  sz = sizeof(struct sockaddr_in);
  if(getpeername(sd, (struct sockaddr *)&(s->raddr), &sz) < 0)
  {
    switch(errno)
    {
      case ENOTCONN:
        s->state = C_NET_CREATED;
        break;

      default:
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
    }
  }

  /* get flags */

  if(flags & O_NONBLOCK)
    s->flags |= C_NET_MUNBLOCK;

  return(TRUE);
}

/*
 */

c_socket_t *C_socket_reopen(int sd)
{
  c_socket_t *s = C_new(c_socket_t);

  if(!__C_socket_reopen(s, sd))
    return(C_free(s));

  return(s);
}

/*
 */

c_bool_t C_socket_reopen_s(c_socket_t *s, int sd)
{

  if(!s)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  C_zero(s, c_socket_t);

  return(__C_socket_reopen(s, sd));
}

/*
 */

c_bool_t C_socket_set_option(c_socket_t *s, uint_t option, c_bool_t flag,
                             uint_t value)
{

  if(! s)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  switch(option)
  {
    /* block */

    case C_NET_OPT_BLOCK:
    {
      int flags;

      if(s->state == C_NET_SHUTDOWN)
      {
        C_error_set_errno(C_EBADSTATE);
        return(FALSE);
      }

      if((flags = fcntl(s->sd, F_GETFL, 0)) == -1)
      {
        C_error_set_errno(C_EFCNTL);
        return(FALSE);
      }

      flag ? (flags &= ~O_NONBLOCK) : (flags |= O_NONBLOCK);
      flag ? (s->flags &= ~C_NET_MUNBLOCK) : (s->flags |= C_NET_MUNBLOCK);
      if(fcntl(s->sd, F_SETFL, flags) == -1)
      {
        C_error_set_errno(C_EFCNTL);
        return(FALSE);
      }

      return(TRUE);
    }

    /* linger */

    case C_NET_OPT_LINGER:
    {
      struct linger l;

      l.l_onoff = (flag ? 1 : 0);
      l.l_linger = value;

      if(setsockopt(s->sd, SOL_SOCKET, SO_LINGER, (char *)&l,
                    sizeof(struct linger)) != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }
      return(TRUE);
    }

    /* reuse address */

    case C_NET_OPT_REUSEADDR:
    {
      int v = flag;

      if(setsockopt(s->sd, SOL_SOCKET, SO_REUSEADDR, (char *)&v, sizeof(int))
         != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      return(TRUE);
    }

    /* oob data inline */

    case C_NET_OPT_OOBINLINE:
    {
      int v = flag;

      if(setsockopt(s->sd, SOL_SOCKET, SO_OOBINLINE, (char *)&v, sizeof(int))
         != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      return(TRUE);
    }

    /* keepalive */

    case C_NET_OPT_KEEPALIVE:
    {
      int v = flag;

      if(setsockopt(s->sd, SOL_SOCKET, SO_KEEPALIVE, (char *)&v, sizeof(int))
         != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      return(TRUE);
    }

    /* recv buffer */

    case C_NET_OPT_RECVBUF:
    {
      int v = (int)value;

      if(setsockopt(s->sd, SOL_SOCKET, SO_RCVBUF, (char *)&v, sizeof(int))
         != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      return(TRUE);
    }

    /* send buffer */

    case C_NET_OPT_SENDBUF:
    {
      int v = (int)value;

      if(setsockopt(s->sd, SOL_SOCKET, SO_SNDBUF, (char *)&v, sizeof(int))
         != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      return(TRUE);
    }

    /* unknown */

    default:
      C_error_set_errno(C_EINVAL);
      return(FALSE);
  }
}

/*
 */

c_bool_t C_socket_get_option(c_socket_t *s, uint_t option, c_bool_t *flag,
                             uint_t *value)
{
  if(!s || !flag || !value)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  switch(option)
  {
    /* block */

    case C_NET_OPT_BLOCK:
    {
      return(!(s->flags & C_NET_MUNBLOCK));
    }

    /* linger */

    case C_NET_OPT_LINGER:
    {
      socklen_t sz = (socklen_t)sizeof(struct linger);
      struct linger l;

      if(getsockopt(s->sd, SOL_SOCKET, SO_LINGER, (char *)&l, &sz) != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      *flag = l.l_onoff ? TRUE : FALSE;
      *value = (uint_t)l.l_linger;

      return(TRUE);
    }

    /* reuse address */

    case C_NET_OPT_REUSEADDR:
    {
      socklen_t sz = (socklen_t)sizeof(int);
      int v;

      if(getsockopt(s->sd, SOL_SOCKET, SO_REUSEADDR, (char *)&v, &sz) != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      *flag = v ? TRUE : FALSE;

      return(TRUE);
    }

    /* oob data inline */

    case C_NET_OPT_OOBINLINE:
    {
      socklen_t sz = (socklen_t)sizeof(int);
      int v;

      if(getsockopt(s->sd, SOL_SOCKET, SO_OOBINLINE, (char *)&v, &sz) != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      *flag = v ? TRUE : FALSE;

      return(TRUE);
    }

    /* keepalive */

    case C_NET_OPT_KEEPALIVE:
    {
      socklen_t sz = (socklen_t)sizeof(int);
      int v;

      if(getsockopt(s->sd, SOL_SOCKET, SO_KEEPALIVE, (char *)&v, &sz) != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      *flag = v ? TRUE : FALSE;

      return(TRUE);
    }

    /* recv buffer */

    case C_NET_OPT_RECVBUF:
    {
      socklen_t sz = (socklen_t)sizeof(int);
      int v;

      if(getsockopt(s->sd, SOL_SOCKET, SO_RCVBUF, (char *)&v, &sz) != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      *value = (uint_t)v;

      return(TRUE);
    }

    /* send buffer */

    case C_NET_OPT_SENDBUF:
    {
      socklen_t sz = (socklen_t)sizeof(int);
      int v;

      if(getsockopt(s->sd, SOL_SOCKET, SO_SNDBUF, (char *)&v, &sz) != 0)
      {
        C_error_set_errno(C_ESOCKINFO);
        return(FALSE);
      }

      *value = (uint_t)v;

      return(TRUE);
    }

    /* unknown */

    default:
      C_error_set_errno(C_EINVAL);
      return(FALSE);
  }
}

/*
 */

c_bool_t C_socket_mcast_join(c_socket_t *s, const char *addr)
{
  struct ip_mreq mreq;
  struct sockaddr_in saddr;

  if(! s || !addr)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(!*addr)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->type != C_NET_UDP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(FALSE);
  }

  if(! __C_socket_addr2sock(&saddr, addr))
  {
    C_error_set_errno(C_EADDRINFO);
    return(FALSE);
  }

  memcpy(&mreq.imr_multiaddr, &(saddr.sin_addr), sizeof(struct in_addr));
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);

  if(setsockopt(s->sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
  {
    C_error_set_errno(C_ESOCKINFO);
    return(FALSE);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_socket_mcast_leave(c_socket_t *s, const char *addr)
{
  struct ip_mreq mreq;
  struct sockaddr_in saddr;

  if(! s || !addr)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(!*addr)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->type != C_NET_UDP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(FALSE);
  }

  if(! __C_socket_addr2sock(&saddr, addr))
  {
    C_error_set_errno(C_EADDRINFO);
    return(FALSE);
  }

  memcpy(&mreq.imr_multiaddr, &(saddr.sin_addr), sizeof(struct in_addr));
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);

  if(setsockopt(s->sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq))
     < 0)
  {
    C_error_set_errno(C_ESOCKINFO);
    return(FALSE);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_socket_mcast_set_ttl(c_socket_t *s, c_byte_t ttl)
{
  if(!s)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->type != C_NET_UDP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(FALSE);
  }

  if(setsockopt(s->sd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
  {
    C_error_set_errno(C_ESOCKINFO);
    return(FALSE);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_socket_mcast_set_loop(c_socket_t *s, c_bool_t loop)
{
  u_char flag = (u_char)loop;

  if(!s)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(s->type != C_NET_UDP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(FALSE);
  }

  if(setsockopt(s->sd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag)) < 0)
  {
    C_error_set_errno(C_ESOCKINFO);
    return(FALSE);
  }

  return(TRUE);
}

/* end of source file */
