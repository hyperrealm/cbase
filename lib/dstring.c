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

/* Local headers */

#include "cbase/defs.h"
#include "cbase/data.h"
#include "cbase/system.h"

/* Functions */

c_dstring_t *C_dstring_create(uint_t blocksz)
{
  c_dstring_t *d;

  if(blocksz < C_DSTRING_MIN_BLOCKSZ)
    return(NULL);

  d = C_new(c_dstring_t);
  d->mem = C_newstr(blocksz);
  d->p = d->len = 0;
  d->blk = 1;
  d->blocksz = blocksz;

  return(d);
}

/*
 */

char *C_dstring_destroy(c_dstring_t *d)
{
  char *r;

  if(!d)
    return(NULL);

  r = d->mem;
  *(r + d->len) = NUL;
  r = C_realloc(r, d->len + 1, char);
  C_free(d);

  return(r);
}

/*
 */

static char *__C_dstring_resize(c_dstring_t *d, off_t rlen)
{
  uint_t r;

  if(!d)
    return(NULL);

  r = d->p;
  if(((d->p += rlen) >= (d->blocksz * d->blk)) || !rlen)
  {
    d->blk = (d->p / d->blocksz) + 1;
    d->mem = C_realloc(d->mem, d->blk * d->blocksz, char);
  }
  if(!rlen || d->p > d->len)
    d->len = d->p;

  return(rlen ? (char *)(d->mem + r) : NULL);
}

/*
 */

c_bool_t C_dstring_putc(c_dstring_t *d, char c)
{
  char *r;

  if(!d || !c)
    return(FALSE);

  r = __C_dstring_resize(d, 1);
  *r = c;

  return(TRUE);
}

/*
 */

c_bool_t C_dstring_puts(c_dstring_t *d, const char *s)
{
  return(C_dstring_puts_len(d, s, strlen(s)));
}

/*
 */

c_bool_t C_dstring_puts_len(c_dstring_t *d, const char *s, size_t len)
{
  char *r;

  if(!d || !s)
    return(FALSE);
  if(!*s)
    return(FALSE);
  if(len <= 0)
    return(FALSE);

  r = __C_dstring_resize(d, (uint_t)len);
  memcpy((void *)r, (void *)s, len);

  return(TRUE);
}

/*
 */

char C_dstring_getc(c_dstring_t *d)
{

  if(!d)
    return(NUL);
  if(d->p >= d->len)
    return(NUL);

  return(*(d->mem + d->p++));
}

/*
 */

char *C_dstring_gets(c_dstring_t *d, char *s, size_t len, char termin)
{
  char *p, *t = s;
  size_t l = len;
  uint_t i;

  if(!d || !s || !len)
    return(NULL);
  if(d->p == d->len)
    return(NULL);

  for(p = d->mem + d->p, i = d->len - d->p;
      i && l && (*p != termin);
      --i, --l)
  {
    *(t++) = *(p++);
  }
  *t = NUL;
  d->p = p - d->mem;

  return(s);
}

/*
 */

c_bool_t C_dstring_seek(c_dstring_t *d, off_t where, int whence)
{
  off_t o;

  if(!d)
    return(FALSE);

  switch(whence)
  {
    case C_DSTRING_SEEK_REL:
      o = d->p + where;
      break;

    case C_DSTRING_SEEK_ABS:
      o = where;
      break;

    case C_DSTRING_SEEK_END:
      o = d->len - where;
      break;

    default:
      return(FALSE);
  }

  if(o < 0 || o > d->len)
    return(FALSE);
  d->p = o;

  return(TRUE);
}

/*
 */

c_bool_t C_dstring_trunc(c_dstring_t *d, off_t size)
{

  if(!d)
    return(FALSE);
  if(size > d->len)
    return(FALSE);

  d->p = size;
  __C_dstring_resize(d, 0);

  return(TRUE);
}

/*
 */

c_dstring_t *C_dstring_load(const char *path, uint_t blocksz)
{
  c_dstring_t *d;
  FILE *fp;
  struct stat st;
  uint_t i;
  char *p;
  size_t b;

  if(blocksz < C_DSTRING_MIN_BLOCKSZ || !path)
    return(NULL);
  if(!*path)
    return(NULL);
  if(stat(path, &st))
    return(NULL);
  if(!(fp = fopen(path, "r")))
    return(NULL);

  d = C_new(c_dstring_t);
  d->len = i = st.st_size;
  d->p = 0;
  d->blocksz = blocksz;
  d->blk = (d->len / d->blocksz) + 1;
  d->mem = p = C_newstr(d->blk * d->blocksz);
  while((b = fread(p, C_DSTRING_LOAD_BLOCKSZ, sizeof(char), fp))
        == C_DSTRING_LOAD_BLOCKSZ)
    i -= C_DSTRING_LOAD_BLOCKSZ, p += C_DSTRING_LOAD_BLOCKSZ;
  fclose(fp);

  if(b != i)
  {
    C_free(d->mem);
    C_free(d);
    return(NULL);
  }

  return(d);
}

/*
 */

c_bool_t C_dstring_save(c_dstring_t *d, const char *path)
{
  FILE *fp;
  uint_t i;
  char *p;
  size_t b;

  if(!d || !path)
    return(FALSE);
  if(!*path)
    return(FALSE);
  if(!(fp = fopen(path, "w")))
    return(FALSE);

  i = d->len, p = d->mem;
  while((b = fwrite(p, C_DSTRING_LOAD_BLOCKSZ, sizeof(char), fp))
        == C_DSTRING_LOAD_BLOCKSZ)
    i -= C_DSTRING_LOAD_BLOCKSZ, p += C_DSTRING_LOAD_BLOCKSZ;
  fclose(fp);

  return(b != i ? FALSE : TRUE);
}

/* end of source file */
