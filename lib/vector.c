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

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Functions */

c_vector_t *C_vector_start(uint_t resize_rate)
{
  c_vector_t *v = C_new(c_vector_t);

  v->len = v->c = 0;
  v->vs = v->v = C_newa(resize_rate, char *);
  v->blk = 1;
  v->blksz = resize_rate;

  return(v);
}

/*
 */

c_bool_t C_vector_store(c_vector_t *v, const char *s)
{

  if(!v || !s)
    return(FALSE);

  *(v->v) = (char *)s;
  (v->len)++;
  if(++(v->c) == v->blksz)
  {
    v->c = 0;
    v->vs = C_realloc(v->vs, v->blksz * ++(v->blk), char *);
    v->v = v->vs + ((v->blk - 1) * v->blksz);
  }
  else
    ++(v->v);

  return(TRUE);
}

/*
 */

char **C_vector_end(c_vector_t *v, size_t *len)
{
  char **r;

  if(!v)
    return(NULL);

  if(len)
    *len = (size_t)v->len;

  r = C_realloc(v->vs, ++(v->len), char *);
  *(r + v->len - 1) = NULL;
  C_free(v);

  return(r);
}

/*
 */

c_bool_t C_vector_contains(c_vector_t *v, const char *s)
{
  char **p;
  uint_t len;

  if(!v || !s)
    return(FALSE);

  for(p = v->vs, len = v->len; len--; ++p)
  {
    if(! strcmp(*p, s))
      return(TRUE);
  }

  return(FALSE);
}

/*
 */

void C_vector_abort(c_vector_t *v)
{

  if(v)
  {
    C_free_vec(v->vs);
    C_free(v);
  }
}

/* end of source file */
