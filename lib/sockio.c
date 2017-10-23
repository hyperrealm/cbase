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

#include <string.h>

/* Local headers */

#include "netcommon.h"
#include "cbase/defs.h"
#include "cbase/net.h"
#include "cbase/cerrno.h"
#include "cbase/system.h"

/* Macros */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/* Functions */

int C_socket_send(c_socket_t *s, const char *buf, size_t bufsz, c_bool_t oobf)
{
  int b;

  if(!s || !buf)
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }

  if(s->state != C_NET_CONNECTED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(-1);
  }

  switch(s->type)
  {
    case C_NET_TCP:
    {
      int flags = MSG_NOSIGNAL | (oobf ? MSG_OOB : 0),
        bsofar = 0,
        bleft = (int)bufsz;
      char *p = (char *)buf;

      do
      {
      SEND1:
        b = send(s->sd, p, bleft, flags);

        if(b == 0)
        {
          C_error_set_errno(C_ELOSTCONN);
          return(-bsofar);
        }

        else if(b < 0)
        {
          switch(errno)
          {
            case EWOULDBLOCK:
              C_error_set_errno(C_EBLOCKED);
              return(-bsofar);

            case EINTR:
              goto SEND1;

            default:
              C_error_set_errno(C_ESEND);
              return(-bsofar);
          }
        }

        else bleft -= b, bsofar += b, p += b;
      }
      while(bleft);

      return(bsofar);
    }

    case C_NET_UDP:
    {
    SENDTO1:
      b = sendto(s->sd, buf, (int)bufsz, 0,
                 ((s->state == C_NET_CONNECTED)
                  ? NULL : (struct sockaddr *)&(s->raddr)),
                 ((s->state == C_NET_CONNECTED)
                  ? 0 : (int)sizeof(struct sockaddr_in)));

      if(b == 0)
      {
        C_error_set_errno(C_ELOSTCONN);
        return(0);
      }

      else if(b < 0)
      {
        switch(errno)
        {
          case EMSGSIZE:
            C_error_set_errno(C_EMSG2BIG);
            return(-1);

          case EWOULDBLOCK:
            C_error_set_errno(C_EBLOCKED);
            return(0);

          case EINTR:
            goto SENDTO1;

          default:
            C_error_set_errno(C_ESEND);
            return(-1);
        }
      }

      else return(b);
    }

    default:
      C_error_set_errno(C_EBADTYPE);
      return(-1);
  }
}

/*
 */

int C_socket_recv(c_socket_t *s, char *buf, size_t bufsz, c_bool_t oobf)
{
  int b = 0;

  if(!s || !buf || !bufsz)
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }

  if(s->state != C_NET_CONNECTED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(-1);
  }

  switch(s->type)
  {
    case C_NET_TCP:
    {
      int flags = MSG_NOSIGNAL | (oobf ? MSG_OOB : 0),
        bsofar = 0,
        bleft = (int)bufsz;
      char *p = buf;

      do
      {
      RECV1:
        b = recv(s->sd, p, bleft, flags);

        if(b == 0)
        {
          C_error_set_errno(C_ELOSTCONN);
          return(-bsofar);
        }

        else if(b < 0)
        {
          switch(errno)
          {
            case EWOULDBLOCK:
              C_error_set_errno(C_EBLOCKED);
              return(-bsofar);

            case EINTR:
              goto RECV1;

            default:
              C_error_set_errno(C_ERECV);
              return(-bsofar);
          }
        }

        else bleft -= b, bsofar += b, p += b;
      }
      while(bleft);

      return(bsofar);
    }

    case C_NET_UDP:
    {
      socklen_t sz = (socklen_t)sizeof(struct sockaddr_in);

    RECVFROM1:
      b = recvfrom(s->sd, buf, (int)bufsz, 0,
                   ((s->state == C_NET_CONNECTED)
                    ? NULL : (struct sockaddr *)&(s->raddr)),
                   ((s->state == C_NET_CONNECTED)
                    ? NULL : &sz));


      if(b == 0)
      {
        C_error_set_errno(C_ELOSTCONN);
        return(0);
      }

      else if(b < 0)
      {
        switch(errno)
        {
          case EMSGSIZE:
            C_error_set_errno(C_EMSG2BIG);
            return(-1);

          case EWOULDBLOCK:
            C_error_set_errno(C_EBLOCKED);
            return(0);

          case EINTR:
            goto RECVFROM1;

          default:
            C_error_set_errno(C_ERECV);
            return(-1);
        }
      }

      else return(b);
    }

    default:
      C_error_set_errno(C_EBADTYPE);
      return(-1);
  }
}

/*
 */

int C_socket_sendto(c_socket_t *s, const char *buf, size_t bufsz,
                    const char *addr, in_port_t port)
{
  int b;

  if(!s || !buf || !bufsz)
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }

  if(s->state != C_NET_CREATED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(-1);
  }

  if(s->type != C_NET_UDP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(-1);
  }

  if(addr)
  {
    if(! *addr)
    {
      C_error_set_errno(C_EINVAL);
      return(-1);
    }

    if(!__C_socket_addr2sock(&(s->raddr), addr))
    {
      C_error_set_errno(C_EADDRINFO);
      return(-1);
    }

    s->raddr.sin_port = htons(port);
  }

SENDTO2:
  b = sendto(s->sd, buf, (int)bufsz, 0, (struct sockaddr *)&(s->raddr),
             (int)sizeof(struct sockaddr_in));

  if(b == 0)
  {
    C_error_set_errno(C_ELOSTCONN);
    return(-1);
  }

  else if(b < 0)
  {
    switch(errno)
    {
      case EMSGSIZE:
        C_error_set_errno(C_EMSG2BIG);
        return(-1);

      case EWOULDBLOCK:
        C_error_set_errno(C_EBLOCKED);
        return(0);

      case EINTR:
        goto SENDTO2;

      default:
        C_error_set_errno(C_ESENDTO);
        return(-1);
    }
  }

  else return(b);
}

/*
 */

int C_socket_sendreply(c_socket_t *s, const char *buf, size_t bufsz)
{
  int b;

  if(!s || !buf || !bufsz)
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }

  if(s->state != C_NET_CREATED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(-1);
  }

  if(s->type != C_NET_UDP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(-1);
  }

SENDTO3:
  b = sendto(s->sd, buf, (int)bufsz, 0, (struct sockaddr *)&(s->raddr),
             (int)sizeof(struct sockaddr_in));

  if(b == 0)
  {
    C_error_set_errno(C_ELOSTCONN);
    return(-1);
  }

  else if(b < 0)
  {
    switch(errno)
    {
      case EMSGSIZE:
        C_error_set_errno(C_EMSG2BIG);
        return(-1);

      case EWOULDBLOCK:
        C_error_set_errno(C_EBLOCKED);
        return(0);

      case EINTR:
        goto SENDTO3;

      default:
        C_error_set_errno(C_ESENDTO);
        return(-1);
    }
  }

  else return(b);
}

/*
 */

int C_socket_recvfrom(c_socket_t *s, char *buf, size_t bufsz, char *addr,
                      size_t addrsz)
{
  int b;
  socklen_t sz = (socklen_t)sizeof(struct sockaddr_in);

  if(!s || !buf || !bufsz)
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }

  if(s->state != C_NET_CREATED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(-1);
  }

  if(s->type != C_NET_UDP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(-1);
  }

RECVFROM2:
  b = recvfrom(s->sd, buf, (int)bufsz, 0, (struct sockaddr *)&(s->raddr), &sz);

  if(b == 0)
  {
    C_error_set_errno(C_ELOSTCONN);
    return(-1);
  }

  else if(b < 0)
  {
    switch(errno)
    {
      case EMSGSIZE:
        C_error_set_errno(C_EMSG2BIG);
        return(-1);

      case EWOULDBLOCK:
        C_error_set_errno(C_EBLOCKED);
        return(0);

      case EINTR:
        goto RECVFROM2;

      default:
        C_error_set_errno(C_ERECVFROM);
        return(-1);
    }
  }
  else
  {
    if(addr)
    {
      if(!addrsz)
      {
        C_error_set_errno(C_EINVAL);
        return(-1);
      }

      __C_socket_sock2addr(&(s->raddr), addr, addrsz);
    }

    return(b);
  }
}

/*
 */

int C_socket_recvreply(c_socket_t *s, char *buf, size_t bufsz)
{
  int b;
  socklen_t sz = (socklen_t)sizeof(struct sockaddr_in);

  if(!s || !buf || !bufsz)
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }

  if(s->state != C_NET_CREATED)
  {
    C_error_set_errno(C_EBADSTATE);
    return(-1);
  }

  if(s->type != C_NET_UDP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(-1);
  }

RECVFROM3:
  b = recvfrom(s->sd, buf, (int)bufsz, 0, (struct sockaddr *)&(s->raddr), &sz);

  if(b == 0)
  {
    C_error_set_errno(C_ELOSTCONN);
    return(-1);
  }

  else if(b < 0)
  {
    switch(errno)
    {
      case EMSGSIZE:
        C_error_set_errno(C_EMSG2BIG);
        return(-1);

      case EWOULDBLOCK:
        C_error_set_errno(C_EBLOCKED);
        return(0);

      case EINTR:
        goto RECVFROM3;

      default:
        C_error_set_errno(C_ERECVFROM);
        return(-1);
    }
  }

  else return(b);
}

/*
 */

static int __C_socket_write(int sd, const char *buf, size_t bufsz,
                            int timeout)
{
  char *p = (char *)buf;
  int bsofar = 0, bleft = (int)bufsz, b, e;
  struct timeval tv;
  fd_set fdset;

  FD_ZERO(&fdset);
  FD_SET(sd, &fdset);

  for(;;)
  {
  SELECT:
    tv.tv_usec = 0, tv.tv_sec = timeout;
    e = select(FD_SETSIZE, NULL, &fdset, NULL, &tv);

    if(e == 0)
    {
      C_error_set_errno(C_ETIMEOUT);
      return(-bsofar);
    }

    else if(e < 0)
    {
      switch(errno)
      {
        case EINTR:
          goto SELECT;

        default:
          C_error_set_errno(C_ESELECT);
          return(-bsofar);
      }
    }

  SEND3:
    b = send(sd, p, bleft, MSG_NOSIGNAL);

    if(b == 0)
    {
      C_error_set_errno(C_ELOSTCONN);
      return(-bsofar);
    }

    else if(b < 0)
    {
      switch(errno)
      {
        case EWOULDBLOCK:
          C_error_set_errno(C_ETIMEOUT);
          return(-bsofar);

        case EINTR:
          goto SEND3;

        default:
          C_error_set_errno(C_ESEND);
          return(-bsofar);
      }
    }
    else
    {
      p += b, bsofar += b;
      if(!(bleft -= b))
        break;
    }
  }

  return(bsofar);
}

/*
 */

static int __C_socket_read(int sd, char *buf, size_t bufsz, char termin,
                           int timeout)
{
  char *p = buf, *q;
  int bsofar = 0, bleft = (int)(--bufsz), i, b, e;
  struct timeval tv;
  fd_set fdset;

  FD_ZERO(&fdset);
  FD_SET(sd, &fdset);

  for(;;)
  {

  SELECT2:
    tv.tv_usec = 0, tv.tv_sec = timeout;
    e = select(FD_SETSIZE, &fdset, NULL, NULL, &tv);

    if(e == 0)
    {
      C_error_set_errno(C_ETIMEOUT);
      return(-bsofar);
    }

    else if(e < 0)
    {
      switch(errno)
      {
        case EINTR:
          goto SELECT2;

        default:
          C_error_set_errno(C_ESELECT);
          return(-bsofar);
      }
    }

  RECV3:
    b = recv(sd, p, bleft, MSG_PEEK | MSG_NOSIGNAL);

    if(b == 0)
    {
      C_error_set_errno(C_ELOSTCONN);
      return(-bsofar);
    }

    else if(b < 0)
    {
      switch(errno)
      {
        case EWOULDBLOCK:
          C_error_set_errno(C_ETIMEOUT);
          return(-bsofar);

        case EINTR:
          goto RECV3;

        default:
          C_error_set_errno(C_ERECV);
          return(-bsofar);
      }
    }

    else
    {
      /* try to find terminator */

      for(q = p, i = 0; i < b; ++q, ++i)
        if(*q == termin)
        {
          b = bleft = i + 1;
          break;
        }

    RECV4:
      if((i = recv(sd, p, b, MSG_NOSIGNAL)) != b)
      {
        if((i < 0) && (errno == EINTR))
          goto RECV4;

        C_error_set_errno(C_ERECV);
        return(-bsofar);
      }

      bsofar += b;
      if(!(bleft -= b)) break;
      p += b;
    }
  }
  *(buf + bsofar) = NUL;

  return(bsofar);
}

/*
 */

int C_socket_sendline(c_socket_t *s, const char *buf)
{
  int b = 0, c;

  if(!s || !buf)
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }

  if(!((s->state == C_NET_CONNECTED)
       && !(s->flags & C_NET_MUNBLOCK)
       && !C_bit_isset(s->flags, C_NET_OSHUTWR)))
  {
    C_error_set_errno(C_EBADSTATE);
    return(-1);
  }

  if(s->type != C_NET_TCP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(-1);
  }

  if(*buf)
    if((b = __C_socket_write(s->sd, buf, strlen(buf), s->timeout)) <= 0)
      return(b);

  if((c = __C_socket_write(s->sd, CRLF, 2, s->timeout)) <= 0)
    return(-c + b);
  else b += c;

  return(b);
}

/*
 */

int C_socket_recvline(c_socket_t *s, char *buf, size_t bufsz)
{
  int b;

  if(!s || !buf || !bufsz)
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }

  if(!((s->state == C_NET_CONNECTED)
       && !(s->flags & C_NET_MUNBLOCK)
       && !C_bit_isset(s->flags, C_NET_OSHUTRD)))
  {
    C_error_set_errno(C_EBADSTATE);
    return(-1);
  }

  if(s->type != C_NET_TCP)
  {
    C_error_set_errno(C_EBADTYPE);
    return(-1);
  }

  b = __C_socket_read(s->sd, buf, bufsz, '\n', s->timeout);

  /* chop off EOL characters */

  if(b > 0)
  {
    char *p;

    for(p = buf + b - 1; p >= buf; --p)
    {
      if(strchr(CRLF, *p))
        *p = NUL;
    }
  }

  return(b);
}

/* end of source file */
