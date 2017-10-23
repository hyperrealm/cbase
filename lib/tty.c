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

#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/ipc.h"
#include "cbase/cerrno.h"
#include "cbase/system.h"

/* File scope variables */

static struct termios __C_tty_mode;
static struct termios __C_tty_buf;
static c_bool_t __C_tty_rawf = FALSE;
static c_bool_t __C_tty_storef = FALSE;

/* Functions */

c_bool_t C_tty_raw(int fd)
{
  struct termios t;

  if(!isatty(fd))
  {
    C_error_set_errno(C_ENOTTY);
    return(FALSE);
  }

  if(tcgetattr(fd, &t))
  {
    C_error_set_errno(C_ETCATTR);
    return(FALSE);
  }

  __C_tty_mode = t; /* save tty mode */

  t.c_iflag = 0;
  t.c_oflag &= ~OPOST;
  t.c_lflag &= ~(ECHO | ICANON | ISIG);
  t.c_cflag &= ~(CSIZE | PARENB);
  t.c_cflag |= CS8;
  t.c_cc[VMIN] = t.c_cc[VTIME] = 1;

  if(tcsetattr(fd, TCSANOW, &t))
  {
    C_error_set_errno(C_ETCATTR);
    return(FALSE);
  }

  __C_tty_rawf = TRUE;

  return(TRUE);
}

/*
 */

c_bool_t C_tty_unraw(int fd)
{

  if(!isatty(fd))
  {
    C_error_set_errno(C_ENOTTY);
    return(FALSE);
  }

  if(!__C_tty_rawf)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(!tcsetattr(fd, TCSANOW, &(__C_tty_mode)))
  {
    C_error_set_errno(C_ETCATTR);
    return(FALSE);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_tty_store(int fd)
{

  if(!isatty(fd))
  {
    C_error_set_errno(C_ENOTTY);
    return(FALSE);
  }

  if(!tcgetattr(fd, &(__C_tty_buf)))
  {
    C_error_set_errno(C_ETCATTR);
    return(FALSE);
  }

  __C_tty_storef = TRUE;
  return(TRUE);
}

/*
 */

c_bool_t C_tty_restore(int fd)
{

  if(!isatty(fd))
  {
    C_error_set_errno(C_ENOTTY);
    return(FALSE);
  }

  if(!__C_tty_storef)
  {
    C_error_set_errno(C_EINVAL);
    return(FALSE);
  }

  if(!tcsetattr(fd, TCSANOW, &(__C_tty_buf)))
  {
    C_error_set_errno(C_ETCATTR);
    return(FALSE);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_tty_sane(int fd)
{
  int i;

  if(!isatty(fd))
  {
    C_error_set_errno(C_ENOTTY);
    return(FALSE);
  }

  memset((void *)&__C_tty_buf, 0, sizeof(struct termios));

  __C_tty_buf.c_iflag = ICRNL | IXON | IXOFF;
  __C_tty_buf.c_oflag = OPOST;
  __C_tty_buf.c_cflag = CREAD | HUPCL;
  __C_tty_buf.c_lflag = ECHO | ECHOE | ECHOK | ICANON | ISIG | IEXTEN;

  for(i = 0; i < NCCS; i++)
    __C_tty_buf.c_cc[i] = -1;

  __C_tty_buf.c_cc[VEOF]   =     3;
  __C_tty_buf.c_cc[VEOL]   =    28;
  __C_tty_buf.c_cc[VERASE] =     8;
  __C_tty_buf.c_cc[VINTR]  =    21;
  __C_tty_buf.c_cc[VKILL]  =     4;
  __C_tty_buf.c_cc[VQUIT]  =   255;
  __C_tty_buf.c_cc[VSUSP]  =   255;
  __C_tty_buf.c_cc[VSTART] =    17;
  __C_tty_buf.c_cc[VSTOP]  =    19;

  if(!tcsetattr(fd, TCSANOW, &(__C_tty_buf)))
  {
    C_error_set_errno(C_ETCATTR);
    return(FALSE);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_tty_getsize(uint_t *columns, uint_t *rows)
{
  if(! isatty(STDOUT_FILENO))
    return(FALSE);

#if defined(TIOCGWINSZ)

  {
    struct winsize ws;

    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
    {
      if(columns)
        *columns = ws.ws_col;
      if(rows)
        *rows = ws.ws_row;
      return(TRUE);
    }
    else
      return(FALSE);
  }

#elif defined(TIOCGSIZE)

  {
    struct ttysize ts;

    if(ioctl(STDOUT_FILENO, TIOCGSIZE, &ts) == 0)
    {
      if(columns)
        *columns = ts.ts_cols;
      if(rows)
        *rows = ts.ts_lines;
      return(TRUE);
    }
    else
      return(FALSE);
  }

#else

#warning No way to determine terminal size.

  return(FALSE);

#endif
}

/* end of source file */
