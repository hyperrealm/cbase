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

#include <stdarg.h>
#include <string.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* File scope variables */

static c_bool_t (*__C_mem_errfunc)(void) = NULL;

static void (*__C_mem_alloc_hook)(const void *, const void *, size_t) = NULL;

/* Functions */

void C_mem_default_alloc_hook(const void *p_old, const void *p_new, size_t len)
{
  C_debug_printf("default alloc hook\n");

  if(p_old && ! p_new && ! len)
    C_log_info("MEMORY: Free                  @ %p", p_old);
  else if(p_new && ! p_old)
    C_log_info("MEMORY: Alloc   %7d bytes @ %p", len, p_new);
  else if(p_old && p_new)
    C_log_info("MEMORY: Realloc %7d bytes @ %p from %p", len, p_new,
               p_old);
}

/*
 */

void *C_mem_manage(void *p, size_t n, c_bool_t clearf)
{
  void *r = NULL;

  for(;;)
  {
    if((r = realloc(p, n)))
    {
      if(__C_mem_alloc_hook)
        __C_mem_alloc_hook(p, r, n);

      if(clearf)
        memset(r, 0, n);

      break;
    }
    else
    {
      if(__C_mem_errfunc)
      {
        if(! __C_mem_errfunc())
          break;
      }
      else
        break;
    }
  }

  return(r);
}

/*
 */

void C_mem_set_errorfunc(c_bool_t (*func)(void))
{

  __C_mem_errfunc = func;
}

/*
 */

void C_mem_set_alloc_hook(void (*func)(const void *p_old, const void *p_new,
                                       size_t len))
{

  __C_mem_alloc_hook = func;
}

/*
 */

void *C_mem_free(void *p)
{

  if(p)
    free(p);

  if(__C_mem_alloc_hook)
    __C_mem_alloc_hook(p, NULL, 0);

  return(NULL);
}

/*
 */

void C_mem_free_vec(char **v)
{
  char **p;

  if(!v)
    return;

  for(p = v; *p; ++p)
    C_free(*p);
  C_free(v);
}

/*
 */

uint_t C_mem_va_free(uint_t n, ...)
{
  int i, r = 0;
  va_list vp;
  void *p;

  va_start(vp, n);

  for(i = n; i--;)
    if((p = va_arg(vp, void *)))
      C_free(p), ++r;

  va_end(vp);
  return(r);
}

/*
 */

size_t C_mem_defrag(void *p, size_t elemsz, size_t len,
                    c_bool_t (*isempty)(void *elem))
{
  uint_t mlen = 0, hlen = 0, i = 0;
  void *h = NULL, *m = NULL, *q;

  for(q = p; i < len; ++i, q += elemsz)
  {
    if(isempty(q))
    {
      if(mlen)
      {
        memmove(h, m, (size_t)(mlen * elemsz));
        h += (mlen * elemsz);
        mlen = 0;
      }
      if(!(hlen++))
        h = q;
    }
    else
    {
      if(!hlen)
        continue;
      else if(!(mlen++))
        m = q;
    }
  }

  return((size_t)(len - hlen));
}

/*
 */

c_buffer_t *C_buffer_create(size_t bufsz)
{
  c_buffer_t *buf = C_new(c_buffer_t);

  buf->bufsz = bufsz;
  buf->buf = C_newb(bufsz);
  return(buf);
}

/*
 */

c_buffer_t *C_buffer_resize(c_buffer_t *buf, size_t newsz)
{

  if(newsz <= 0)
    return(NULL);

  buf->buf = C_realloc(buf->buf, newsz, char);
  buf->bufsz = newsz;

  return(buf);
}

/*
 */

void C_buffer_clear(c_buffer_t *buf)
{

  if(! buf)
    return;

  buf->datalen = 0;
  memset((void *)buf->buf, 0, buf->bufsz);
}

/*
 */

void C_buffer_destroy(c_buffer_t *buf)
{

  if(!buf)
    return;

  C_free(buf->buf);
  C_free(buf);
}

/* end of source file */
