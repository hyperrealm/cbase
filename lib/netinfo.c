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
#ifdef THREADED_LIBRARY
#include <pthread.h>
#endif /* THREADED_LIBRARY */

/* Local headers */

#include "netcommon.h"
#include "cbase/defs.h"
#include "cbase/net.h"
#include "cbase/cerrno.h"
#include "cbase/system.h"
#include "getXXbyYY_r.h"

/* File scope variables */

#ifdef THREADED_LIBRARY

static pthread_once_t __C_net_once = PTHREAD_ONCE_INIT;
static pthread_key_t __C_net_key;

/* File scope functions */

static void __C_net_destructor(void *arg)
{
  C_buffer_destroy((c_buffer_t *)arg);
}

/*
 */

static void __C_net_init_once(void)
{
  pthread_key_create(&__C_net_key, __C_net_destructor);
}

#endif /* THREADED_LIBRARY */

/* External functions */

#ifdef THREADED_LIBRARY

c_buffer_t *__C_net_get_buffer(void)
{
  c_buffer_t *buf = NULL;

  pthread_once(&__C_net_once, __C_net_init_once);

  buf = (c_buffer_t *)pthread_getspecific(__C_net_key);
  if(!buf)
  {
    /* didn't exist, so create it */

    buf = C_buffer_create(C_NET_BUFSZ);
    pthread_setspecific(__C_net_key, (void *)buf);
  }

  return(buf);
}

#else

c_buffer_t *__C_net_get_buffer(void)
{
  static c_buffer_t *buf = NULL;

  if(!buf)
    buf = C_buffer_create(C_NET_BUFSZ);

  return(buf);
}

#endif /* THREADED_LIBRARY */

/*
 */

in_port_t C_net_get_svcport(const char *name, uint_t *type)
{
  struct servent se, *rse;
  int i, r;
  c_buffer_t *rbuf = __C_net_get_buffer();

  if(!name || !type)
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }
  if(!*name || (*type > C_NET_MAXTYPE))
  {
    C_error_set_errno(C_EINVAL);
    return(-1);
  }

GETSERVBYNAME:
  if(C_getservbyname_r(name, __C_net_protocols[*type], &se, rbuf->buf,
                       rbuf->bufsz, &rse) < 0)
  {
    if(errno == ERANGE)
    {
      C_buffer_resize(rbuf, rbuf->bufsz + C_NET_BUFSZ);
      goto GETSERVBYNAME;
    }
    else
    {
      C_error_set_errno(C_ESVCINFO);
      return(-1);
    }
  }

  if(*type == C_NET_UNKNOWN)
    for(i = 0; i < C_NET_MAXTYPE - 1; i++)
      if(!strcmp(se.s_proto, __C_net_protocols[i]))
      {
        *type = i;
        break;
      }

  r = (int)ntohl(se.s_port);

  return((in_port_t)r);
}

/*
 */

c_bool_t C_net_get_svcname(in_port_t port, uint_t *type, char *buf,
                           size_t bufsz)
{
  struct servent se, *rse;
  int i;
  c_buffer_t *rbuf = __C_net_get_buffer();

  if(!type || !buf || !bufsz)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(*type > C_NET_MAXTYPE)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

GETSERVBYPORT:
  if(C_getservbyport_r((int)port, __C_net_protocols[*type], &se, rbuf->buf,
                       rbuf->bufsz, &rse) < 0)
  {
    if(errno == ERANGE)
    {
      C_buffer_resize(rbuf, rbuf->bufsz + C_NET_BUFSZ);
      goto GETSERVBYPORT;
    }
    else
    {
      C_error_set_errno(C_ESVCINFO);
      return(FALSE);
    }
  }

  if(*type == C_NET_UNKNOWN)
    for(i = 0; i < C_NET_MAXTYPE - 1; i++)
      if(!strcmp(se.s_proto, __C_net_protocols[i]))
      {
        *type = i;
        break;
      }

  strncpy(buf, se.s_name, --bufsz);
  *(buf + bufsz) = NUL;
  return(TRUE);
}

/*
 */

c_bool_t C_net_resolve(const char *ipaddr, char *buf, size_t bufsz)
{
  in_addr_t addr;
  struct hostent he, *rhe;
  c_buffer_t *rbuf = __C_net_get_buffer();
  int herr;

  if(!ipaddr || !buf || !bufsz)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }
  if(!*ipaddr)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if((addr = inet_addr(ipaddr)) == -1)
  {
    C_error_set_errno(C_EADDRINFO);
    return(FALSE);
  }

GETHOSTBYADDR:
  if(C_gethostbyaddr_r((char *)&addr, sizeof(in_addr_t), AF_INET, &he,
                       rbuf->buf, rbuf->bufsz, &rhe, &herr) < 0)
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

  strncpy(buf, he.h_name, --bufsz);
  *(buf + bufsz) = NUL;
  return(TRUE);
}

/*
 */

c_bool_t C_net_resolve_local(char *addr, char *ipaddr, size_t bufsz,
                             in_addr_t *ip)
{
  struct hostent he, *rhe;
  struct in_addr in;
  char *p;
  c_buffer_t *rbuf = __C_net_get_buffer();
  int herr;

  if((!addr && !ipaddr && !ip) || (bufsz == 0))
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(!(p = C_system_get_hostname()))
  {
    C_error_set_errno(C_EADDRINFO);
    return(FALSE);
  }

GETHOSTBYNAME:
  if(C_gethostbyname_r(p, &he, rbuf->buf, rbuf->bufsz, &rhe, &herr) < 0)
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

  if(addr)
  {
    strncpy(addr, he.h_name, --bufsz);
    *(addr + bufsz) = NUL;
  }

  memcpy((void *)&(in.s_addr), (void *)*(he.h_addr_list), sizeof(in.s_addr));

  if(ipaddr)
  {
    strncpy(ipaddr, inet_ntoa(in), bufsz);
    *(ipaddr + bufsz) = NUL;
  }

  if(ip)
    *ip = ntohl(in.s_addr);

  return(TRUE);
}

/* end of source file */
