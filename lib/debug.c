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

static c_bool_t __C_debug_trace = TRUE;

static FILE *__C_debug_stream = NULL;

static c_bool_t __C_debug_termattr = TRUE;

/* Functions */

void C_debug_set_trace(c_bool_t flag)
{
  __C_debug_trace = flag;
}

/*
 */

void C_debug_set_stream(FILE *stream)
{
  __C_debug_stream = stream;
}

/*
 */

void C_debug_set_termattr(c_bool_t flag)
{
  __C_debug_termattr = flag;
}

/*
 */

void C_debug_printf_x(const char *file, int line, int severity,
                      const char *format, ...)
{
  va_list vp;
  FILE *stream = (__C_debug_stream ? __C_debug_stream : stderr);
  size_t l;
  c_bool_t nl = FALSE;
  c_bool_t tty = (isatty(fileno(stream)) && __C_debug_termattr);

  /* If we don't have flockfile() and funlockfile(), then we can't guarantee
   * exclusive access to the stream. A workaround for systems that do not have
   * these functions would be non-trivial, and probably not worth the effort.
   */

  if(! format)
    return;

  l = strlen(format);
  if(l > 0)
    nl = (*(format + --l) == '\n');

#ifdef THREADED_LIBRARY
#ifdef HAVE_FLOCKFILE
  flockfile(stderr);
#endif /* HAVE_FLOCKFILE */
#endif /* THREADED_LIBRARY */

  if(tty)
  {
    fprintf(stream, C_TERMATTR_BOLD);
    if(severity > C_DEBUG_INFO)
      fprintf(stream, C_TERMATTR_FG_RED);
  }

  if(__C_debug_trace)
  {
#ifdef THREADED_LIBRARY
    fprintf(stream, "[%lX] ", (long)pthread_self());
#endif
    fprintf(stream, "%s(%d): ", file, line);
  }

  va_start(vp, format);
  vfprintf(stream, format, vp);
  va_end(vp);

  if(tty)
  {
    fputs(C_TERMATTR_FG_DEFAULT, stream);
    fputs(C_TERMATTR_NORMAL, stream);
  }

  if(! nl)
    fputc('\n', stream);

  fflush(stream);
#ifdef THREADED_LIBRARY
#ifdef HAVE_FLOCKFILE
  funlockfile(stderr);
#endif /* HAVE_FLOCKFILE */
#endif /* THREADED_LIBRARY */
}

/*
 */

c_bool_t C_debug_doassert(char *file, int line, char *expression)
{
  C_debug_printf_x(file, line, C_DEBUG_ERROR, "Assertion failed: %s\n",
                   expression);
  abort();

  return(TRUE);
}

/* end of source file */
