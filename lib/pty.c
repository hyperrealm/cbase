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

#define _XOPEN_SOURCE 500

/* System headers */

#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>
#if defined HAVE_STROPTS_H
#include <stropts.h>
#elif defined HAVE_OPENPTY
#include <util.h>
#else
#include <grp.h>
#endif
#include <termios.h>
#include <stdlib.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/ipc.h"
#include "cbase/system.h"
#include "cbase/cerrno.h"

/* Macros */

#ifdef HAVE_STROPTS_H
#define C_TERM_MASTER "/dev/ptmx"
#endif

/* Functions */

#if !(defined HAVE_STROPTS_H || defined HAVE_OPENPTY)
static int __C_pty_open_master_BSD(char *pts_name);
static int __C_pty_open_slave_BSD(int fdm, char *pts_name);
#endif

/*
 */

c_pty_t *C_pty_create(void)
{
  int mfd, sfd;
  c_pty_t *pty;
  char pts_name[11] = "", *pts = NULL;

#if defined HAVE_STROPTS_H

  c_bool_t ok = FALSE;

  /* open master */

  if((mfd = open(C_TERM_MASTER, O_RDWR)) >= 0)
    if(grantpt(mfd) >= 0)
      if(unlockpt(mfd) >= 0)
        ok = TRUE;

  if(! ok)
  {
    C_error_set_errno(C_EGETPTY);
    return(NULL);
  }

  /* open slave */

  pts = (char *)ptsname(mfd);
  if(pts == NULL)
  {
    C_error_set_errno(C_EGETPTY);
    return(NULL);
  }

  if((sfd = open(pts, O_RDWR)) < 0)
  {
    C_error_set_errno(C_EOPEN);
    return(NULL);
  }

  ok = FALSE;

  if(ioctl(sfd, I_PUSH, "ptem") >= 0)
    if(ioctl(sfd, I_PUSH, "ldterm") >= 0)
      ok = TRUE;

  if(! ok)
  {
    C_error_set_errno(C_EIOCTL);
    return(NULL);
  }

#elif defined HAVE_OPENPTY

  if(openpty(&mfd, &sfd, pts_name, NULL, NULL) < 0)
  {
    C_error_set_errno(C_EGETPTY);
    return(NULL);
  }

  pts = pts_name;

#else /* fall back on the old crude BSD way */

  if((mfd = __C_pty_open_master_BSD(pts_name)) < 0)
  {
    C_error_set_errno(C_EOPEN);
    return(NULL);
  }

  if((sfd = __C_pty_open_slave_BSD(mfd, pts_name)) < 0)
  {
    C_error_set_errno(C_EOPEN);
    return(NULL);
  }

  pts = pts_name;

#endif

  pty = C_new(c_pty_t);
  pty->master_fd = mfd;
  pty->slave_fd = sfd;
  strcpy(pty->pts_name, pts_name);

  return(pty);
}

/*
 */

c_bool_t C_pty_destroy(c_pty_t *pty)
{
  if(! pty)
    return(FALSE);

  close(pty->master_fd);
  close(pty->slave_fd);
  C_free(pty);

  return(TRUE);
}

#if !(defined HAVE_STROPTS_H || defined HAVE_OPENPTY)

/* The following code comes from A.P.U.E. */

static int __C_pty_open_master_BSD(char *pts_name)
{
  int fdm;
  char *ptr1, *ptr2;

  strcpy(pts_name, "/dev/ptyXY");
  /* array index: 0123456789 (for references in following code) */
  for(ptr1 = "pqrstuvwxyzPQRST"; *ptr1 != 0; ptr1++)
  {
    pts_name[8] = *ptr1;
    for (ptr2 = "0123456789abcdef"; *ptr2 != 0; ptr2++)
    {
      pts_name[9] = *ptr2;

      /* try to open master */
      if((fdm = open(pts_name, O_RDWR)) < 0)
      {
        if(errno == ENOENT) /* different from EIO */
          return(-1); /* out of pty devices */
        else
          continue; /* try next pty device */
      }

      pts_name[5] = 't'; /* change "pty" to "tty" */
      return(fdm); /* got it, return fd of master */
    }
  }
  return(-1); /* out of pty devices */
}

/*
 */

static int __C_pty_open_slave_BSD(int fdm, char *pts_name)
{
  struct group *grptr;
  int gid, fds;

  if((grptr = getgrnam("tty")) != NULL)
    gid = grptr->gr_gid;
  else
    gid = -1; /* group tty is not in the group file */

  /* following two functions don't work unless we're root */
  chown(pts_name, getuid(), gid);
  chmod(pts_name, S_IRUSR | S_IWUSR | S_IWGRP);

  if((fds = open(pts_name, O_RDWR)) < 0)
  {
    close(fdm);
    return(-1);
  }

  return(fds);
}

#endif

/* end of source file */
