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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifdef THREADED_LIBRARY
#include <pthread.h>
#endif

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"

/* File scope variables */

static FILE * __C_log_stream = NULL;

static c_bool_t __C_log_console = TRUE, __C_log_termattr = TRUE;

/* Functions */

void C_log_set_console(c_bool_t flag)
{
  __C_log_console = flag;
}

/*
 */

void C_log_set_stream(FILE *stream)
{
  if(stream != stderr)
    __C_log_stream = stream;
}

/*
 */

void C_log_message_x(int level, const char *format, ...)
{
  char buf[32];
  size_t l;
  c_bool_t nl = FALSE;
  c_bool_t tty = (isatty(fileno(stderr)) && __C_log_termattr);
  va_list vp;

  if(! format)
    return;

  l = strlen(format);
  if(l > 0)
    nl = (*(format + --l) == '\n');

  C_time_format(0, buf, sizeof(buf), "[%x %X] ");

  /* log to console */

  if(__C_log_console)
  {
    va_start(vp, format);

    fputs(buf, stderr);

    if(__C_log_termattr && tty)
    {
      fputs(C_TERMATTR_BOLD, stderr);

      switch(level)
      {
        case C_LOG_WARNING:
          fputs(C_TERMATTR_FG_MAGENTA, stderr);
          break;

        case C_LOG_ERROR:
          fputs(C_TERMATTR_FG_RED, stderr);
          break;

        case C_LOG_INFO:
        default:
          break;
      }
    }

    vfprintf(stderr, format, vp);
    if(! nl)
      fputc('\n', stderr);

    if(__C_log_termattr && tty)
    {
      fputs(C_TERMATTR_FG_DEFAULT, stderr);
      fputs(C_TERMATTR_NORMAL, stderr);
    }

    fflush(stderr);

    va_end(vp);
  }

  /* log to file */

  if(__C_log_stream)
  {
    va_start(vp, format);

    fputs(buf, __C_log_stream);
    vfprintf(__C_log_stream, format, vp);
    if(! nl)
      fputc('\n', __C_log_stream);

    fflush(__C_log_stream);

    va_end(vp);
  }
}

/*
 */

void C_log_set_termattr(c_bool_t flag)
{
  __C_log_termattr = flag;
}

/* end of source file */
