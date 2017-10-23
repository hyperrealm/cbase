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
#include <termios.h>
#include <stdarg.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"

/* Macros */

#define C_IO_BLOCKSZ 40

/* Functions */

int C_io_getchar(uint_t delay)
{
  struct termios t_old, t_new;
  int c;

  if(tcgetattr(STDIN_FILENO, &t_old))
    return(-1);
  t_new = t_old;

  t_new.c_iflag &= ~(IXON | IXOFF);
  t_new.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  t_new.c_cc[VMIN] = 0;
  t_new.c_cc[VTIME] = (delay * 10);

  if(tcsetattr(STDIN_FILENO, TCSANOW, &t_new))
    return(-1);
  if((c = getchar()) == EOF)
    clearerr(stdin);

  tcsetattr(STDIN_FILENO, TCSANOW, &t_old);

  return(c);
}

/*
 */

int C_io_gets(FILE *fp, char *buf, size_t bufsz, char termin)
{
  char *p = buf;
  int len = 0, ch;

  if(!buf || !bufsz)
    return(EOF);

  for(; (ch = fgetc(fp));)
  {
    if((ch == EOF) || (ch == termin))
      break;
    *(p++) = ch;
    if(++len == bufsz)
      break;
  }

  *p = NUL;
  if(!len && ch == EOF)
    return(EOF);

  return(len);
}

/*
 */

int C_io_getpasswd(const char *prompt, char *buf, size_t bufsz)
{
  struct termios t_old, t_new;
  int r;

  if(!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
    return(-1);

  tcgetattr(STDIN_FILENO, &t_old);
  t_new = t_old;

  t_new.c_lflag &= ~ECHO;

  if(tcsetattr(STDIN_FILENO, TCSANOW, &t_new) < 0)
    return(-1);

  fputs(prompt, stdout);
  fflush(stdout);

  r = C_io_gets(stdin, buf, bufsz, '\n');
  fputc('\n', stdout);

  if(tcsetattr(STDIN_FILENO, TCSANOW, &t_old) < 0)
    return(-1);

  return(r);
}

/*
 */

char *C_io_getline(FILE *fp, char termin, int *len)
{
  int c = 0, blk = 1, ch, l = 0;
  char *s, *p;

  if(!fp)
    return(NULL);

  s = C_newstr(C_IO_BLOCKSZ);

  for(p = s; (ch = fgetc(fp));)
  {
    if((ch == EOF) || (ch == termin))
      break;
    *p = (char)ch, l++;
    if(++c == C_IO_BLOCKSZ)
    {
      c = 0;
      s = C_realloc(s, (C_IO_BLOCKSZ * ++blk), char);
      p = s + ((blk - 1) * C_IO_BLOCKSZ);
    }
    else p++;
  }

  if(len)
    *len = l;

  if(!l && (ch == EOF))
  {
    C_free(s);
    return(NULL);
  }

  *p = NUL;

  return(s = C_realloc(s, ++l, char));
}

/*
 */

char *C_io_getline_buf(FILE *fp, char termin, c_buffer_t *buf)
{
  int ch, l = 0;
  char *p;

  if(!fp || !buf)
    return(NULL);

  if(C_buffer_size(buf) < C_IO_BLOCKSZ)
    C_buffer_resize(buf, C_IO_BLOCKSZ);

  for(p = C_buffer_data(buf); (ch = fgetc(fp));)
  {
    if((ch == EOF) || (ch == termin))
      break;

    *p = (char)ch;
    if(++l == C_buffer_size(buf))
    {
      C_buffer_resize(buf, l + C_IO_BLOCKSZ);
      p = C_buffer_data(buf) + l;
    }
    else
      p++;
  }

  C_buffer_datalen(buf) = l;

  if(!l && (ch == EOF))
    return(NULL);

  *p = NUL;

  return(C_buffer_data(buf));
}

/*
 */

int C_io_fprintf(FILE *stream, const char *format, ...)
{
  va_list vp;
  int r;

#ifdef THREADED_LIBRARY
#ifdef HAVE_FLOCKFILE
  flockfile(stream);
#endif /* HAVE_FLOCKFILE */
#endif /* THREADED_LIBRARY */
  va_start(vp, format);
  r = vfprintf(stream, format, vp);
  va_end(vp);

#ifdef THREADED_LIBRARY
#ifdef HAVE_FLOCKFILE
  funlockfile(stream);
#endif /* HAVE_FLOCKFILE */
#endif /* THREADED_LIBRARY */

  return(r);
}

/* end of source file */
