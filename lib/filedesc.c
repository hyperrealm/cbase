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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/ipc.h"

/* Macros */

/* Structures & Unions */

#ifdef HAVE_STRUCT_MSGHDR_MSG_CONTROL

#ifdef HAVE_CONSTANT_CMSG_SPACE
#define CBASE_CMSG_SPACE CMSG_SPACE
#else
#define CBASE_CMSG_SPACE(L) (sizeof(struct cmsghdr) + (L) + 16)
#endif /* HAVE_CONSTANT_CMSG_SPACE */

union __c_cmsg_un
{
  struct cmsghdr header;
  char control[CBASE_CMSG_SPACE(sizeof(int))];
};
#endif /* HAVE_STRUCT_MSGHDR_MSG_CONTROL */

/* Functions */

c_bool_t C_fd_send(int sd, int fd)
{
  char *t = "x";
  struct iovec vec;
  struct msghdr msg;

#ifdef HAVE_STRUCT_MSGHDR_MSG_CONTROL
  struct cmsghdr *cmsg;
  union __c_cmsg_un control_un;

  msg.msg_control = control_un.control;
  msg.msg_controllen = sizeof(control_un.control);

  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_len = CMSG_LEN(sizeof(int));
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  memcpy(CMSG_DATA(cmsg), &fd, sizeof(int));
#endif /* HAVE_STRUCT_MSGHDR_MSG_CONTROL */

#ifdef HAVE_STRUCT_MSGHDR_MSG_ACCRIGHTS
  msg.msg_accrights = (caddr_t)&fd;
  msg.msg_accrightslen = sizeof(int);
#endif /* HAVE_STRUCT_MSGHDR_MSG_ACCRIGHTS */

  vec.iov_base = t;
  vec.iov_len = 1;
  msg.msg_iov = &vec;
  msg.msg_iovlen = 1;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;

  return(sendmsg(sd, &msg, 0) != -1);
}

/*
 */

c_bool_t C_fd_recv(int sd, int *fd)
{
  struct iovec vec;
  struct msghdr msg;
  ssize_t n;
  char t;
#ifdef HAVE_STRUCT_MSGHDR_MSG_ACCRIGHTS
  int nfd;
#endif
#ifdef HAVE_STRUCT_MSGHDR_MSG_CONTROL
  struct cmsghdr *cmsg;
  union __c_cmsg_un control_un;

  msg.msg_control = control_un.control;
  msg.msg_controllen = sizeof(control_un.control);
#endif /* HAVE_STRUCT_MSGHDR_MSG_CONTROL */

#ifdef HAVE_STRUCT_MSGHDR_MSG_ACCRIGHTS
  msg.msg_accrights = (caddr_t)&nfd;
  msg.msg_accrightslen = sizeof(int);
#endif /* HAVE_STRUCT_MSGHDR_MSG_ACCRIGHTS */

  vec.iov_base = &t;
  vec.iov_len = 1;
  msg.msg_iov = &vec;
  msg.msg_iovlen = 1;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;

  if((n = recvmsg(sd, &msg, 0)) != 1)
    return(FALSE); /* error, or corrupt message? */

#ifdef HAVE_STRUCT_MSGHDR_MSG_CONTROL
  if(((cmsg = CMSG_FIRSTHDR(&msg)) != NULL)
     && (cmsg->cmsg_len == CMSG_LEN(sizeof(int))))
  {
    if(cmsg->cmsg_level != SOL_SOCKET)
      return(FALSE);
    if(cmsg->cmsg_type != SCM_RIGHTS)
      return(FALSE);

    memcpy(fd, CMSG_DATA(cmsg), sizeof(int));
  }
  else
    return(FALSE); /* descriptor was not passed */
#endif /* HAVE_STRUCT_MSGHDR_MSG_CONTROL */

#ifdef HAVE_STRUCT_MSGHDR_MSG_ACCRIGHTS
  if(msg.msg_accrightslen != sizeof(int))
    return(FALSE); /* descriptor was not passed */

  *fd = nfd;
#endif /* HAVE_STRUCT_MSGHDR_MSG_ACCRIGHTS */

  return(TRUE);
}

/* end of source file */
