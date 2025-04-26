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

#include <string.h>
#include <stdarg.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Functions */

c_strbuffer_t *C_strbuffer_create(size_t bufsz)
{
  c_strbuffer_t *sb;

  if(bufsz < 1)
    return(FALSE);

  sb = C_new(c_strbuffer_t);

  sb->bufsz = bufsz;
  sb->left = sb->bufsz - 1;
  sb->buf = sb->pos = C_newstr(sb->bufsz);
  *(sb->pos) = NUL;

  return(sb);
}

/*
 */

c_bool_t C_strbuffer_destroy(c_strbuffer_t *sb)
{
  if(!sb)
    return(FALSE);

  C_free(sb->buf);
  C_free(sb);

  return(TRUE);
}

/*
 */

c_bool_t C_strbuffer_clear(c_strbuffer_t *sb)
{
  if(! sb)
    return(FALSE);

  sb->pos = sb->buf;
  sb->left = sb->bufsz - 1;
  *(sb->pos) = NUL;

  return(TRUE);
}

/*
 */

c_bool_t C_strbuffer_strcpy(c_strbuffer_t *sb, const char *s)
{

  if(!sb || !s)
    return(FALSE);

  if(C_strbuffer_clear(sb))
    return(C_strbuffer_strcat(sb, s));

  return(FALSE);
}

/*
 */

c_bool_t C_strbuffer_strcat(c_strbuffer_t *sb, const char *s)
{
  c_bool_t ok = TRUE;
  size_t l;

  if(!sb || !s)
    return(FALSE);

  l = strlen(s);

  if(l > sb->left)
  {
    l = sb->left;
    ok = FALSE;
  }

  strncpy(sb->pos, s, l);
  sb->pos += l;
  sb->left -= l;
  *(sb->pos) = NUL;

  return(ok);
}

/*
 */

c_bool_t C_strbuffer_sprintf(c_strbuffer_t *sb, const char *s, ...)
{
  va_list vp;
  int l;
  c_bool_t ok = TRUE;

  if(!sb || !s)
    return(FALSE);

  va_start(vp, s);
  l = vsnprintf(sb->pos, sb->left, s, vp);
  va_end(vp);

  if(l > sb->left)
  {
    l = sb->left;
    ok = FALSE;
  }

  sb->pos += l;
  sb->left -= l;
  *(sb->pos) = NUL;

  return(ok);
}

/*
 */

c_bool_t C_strbuffer_putc(c_strbuffer_t *sb, char c)
{
  if(!sb || (c == NUL))
    return(FALSE);

  if(sb->left < 1)
    return(FALSE);

  *(sb->pos) = c;

  --sb->left;
  ++sb->pos;

  return(TRUE);
}

/*
 */

size_t C_strbuffer_strlen(c_strbuffer_t *sb)
{
  if(!sb)
    return(0);

  return(strlen(sb->buf));
}

/* end of source file */
