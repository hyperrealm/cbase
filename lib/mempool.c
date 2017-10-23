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


/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"

/* Functions */

c_mempool_t *C_mempool_create(size_t size)
{
  c_mempool_t *pool = C_new(c_mempool_t);

  pool->size = size;
  pool->base = (void *)C_newb(size);
  if(! pool->base)
  {
    C_free(pool);
    return(NULL);
  }

  pool->pos = 0;

  return(pool);
}

/*
 */

void C_mempool_destroy(c_mempool_t *pool)
{
  if(! pool)
    return;

  C_free(pool->base);
  C_free(pool);
}

/*
 */

void *C_mempool_alloc(c_mempool_t *pool, size_t size)
{
  void *p;
  size_t rsz;
  int r;

  if(!pool || (size < 1))
    return(NULL);

  p = pool->base + pool->pos;
  rsz = size;
  r = (size % sizeof(void *));
  if(r != 0)
    rsz += (sizeof(void *) - r);

  if(C_mempool_avail(pool) < rsz)
    return(NULL);

  pool->pos += rsz;

  return(p);
}

/*
 */

size_t C_mempool_avail(c_mempool_t *pool)
{
  if(! pool)
    return(0);

  return(pool->size - pool->pos);
}

/* end of source file */
