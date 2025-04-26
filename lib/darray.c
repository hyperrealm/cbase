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

/* Macros */

#define C_DARRAY_NBBY 8
#define C_DARRAY_USED_MASK(B) ~(1 << (B))
#define C_DARRAY_FREE_MASK(B) (1 << (B))

/* Functions */

c_darray_t *C_darray_create(uint_t resize, size_t elemsz)
{
  c_darray_t *a;
  char *b;
  uint_t i;

  if(resize < 1 || resize > C_DARRAY_MAX_RESIZE || !elemsz)
    return(NULL);

  a = C_new(c_darray_t);
  a->resize = resize * C_DARRAY_NBBY;
  a->iresize = resize;
  a->elemsz = elemsz;
  a->mem = (void *)C_malloc(a->resize * a->elemsz, char);
  a->size = a->resize;
  a->isize = a->iresize;
  a->bot = a->del_count = 0;
  a->free_list = C_malloc(a->iresize, char);
  for(i = a->iresize, b = a->free_list; i--; b++)
    *b = ~0;

  return(a);
}

/*
 */

void C_darray_destroy(c_darray_t *a)
{

  if(!a)
    return;

  C_free(a->free_list);
  C_free(a->mem);
  C_free(a);
}

/*
 */

void *C_darray_store(c_darray_t *a, const void *data, uint_t *index)
{
  uint_t e, bit, byte, o;
  char b, *bp;
  div_t dv;

  if(!a)
    return(NULL);

  if(!a->del_count)
  {
    if(a->bot == a->size)
    {
      if((a->size / C_DARRAY_NBBY) == a->isize)
      {
        o = a->isize;
        a->free_list = C_realloc(a->free_list, (a->isize += a->iresize), char);
        memset((void *)(a->free_list + o), ~0, a->iresize);
      }
      a->mem = (void *)C_realloc(a->mem, (a->size += a->resize) * a->elemsz,
                                 char);
    }
    dv = div((e = (a->bot)++), C_DARRAY_NBBY);
    byte = dv.quot, bit = dv.rem;
  }
  else
  {
    a->del_count--;
    for(byte = 0, bp = a->free_list; !*bp; byte++, bp++);
    b = *bp;
    for(bit = 0; bit < C_DARRAY_NBBY; bit++)
    {
      if(b & 1)
        break;
      b >>= 1;
    }
    e = C_DARRAY_NBBY * byte + bit;
  }
  if(index) *index = e;
  *(a->free_list + byte) &= C_DARRAY_USED_MASK(bit);

  return(memcpy((void *)(a->mem + (e * a->elemsz)), data, a->elemsz));
}

/*
 */

void *C_darray_restore(c_darray_t *a, uint_t index)
{
  div_t dv;

  if(!a)
    return(NULL);
  if(index >= a->bot)
    return(NULL);

  dv = div(index, C_DARRAY_NBBY);

  return((*(a->free_list + dv.quot) & C_DARRAY_FREE_MASK(dv.rem))
         ? NULL : (void *)(a->mem + (index * a->elemsz)));
}

/*
 */

c_bool_t C_darray_delete(c_darray_t *a, uint_t index)
{
  div_t dv;
  char *b;

  if(!a)
    return(FALSE);
  if(index >= a->bot)
    return(FALSE);
  dv = div(index, C_DARRAY_NBBY);
  b = (a->free_list + dv.quot);
  if(*b & C_DARRAY_FREE_MASK(dv.rem))
    return(FALSE);

  *b |= C_DARRAY_FREE_MASK(dv.rem);
  a->del_count++;

  return(TRUE);
}

/*
 */

c_darray_t *C_darray_load(const char *path)
{
  FILE *fp;
  c_darray_t *a;

  if(!path)
    return(NULL);
  if(!*path)
    return(NULL);
  if(!(fp = fopen(path, "r")))
    return(NULL);

  a = C_new(c_darray_t);
  if(fread((void *)a, sizeof(c_darray_t), (size_t)1, fp) != 1)
  {
    C_free(a);
    fclose(fp);
    return(NULL);
  }

  a->free_list = C_newstr(a->isize);
  if(fread((void *)a->free_list, sizeof(char), a->isize, fp) != a->isize)
  {
    fclose(fp);
    C_free(a->free_list);
    C_free(a);
    return(NULL);
  }

  a->mem = (void *)C_malloc(a->size * a->elemsz, char);
  if(fread(a->mem, a->elemsz, a->size, fp) != a->size)
  {
    fclose(fp);
    C_free(a->free_list);
    C_free(a->mem);
    C_free(a);
    return(NULL);
  }

  fclose(fp);
  return(a);
}

/*
 */

c_bool_t C_darray_save(c_darray_t *a, const char *path)
{
  FILE *fp;

  if(!a || !path)
    return(FALSE);
  if(!*path)
    return(FALSE);
  if(!(fp = fopen(path, "w")))
    return(FALSE);

  if(fwrite((void *)a, sizeof(c_darray_t), (size_t)1, fp) != 1)
  {
    fclose(fp);
    return(FALSE);
  }

  if(fwrite((void *)a->free_list, sizeof(char), a->isize, fp) != a->isize)
  {
    fclose(fp);
    return(FALSE);
  }

  if(fwrite(a->mem, a->elemsz, a->size, fp) != a->size)
  {
    fclose(fp);
    return(FALSE);
  }

  fclose(fp);
  return(TRUE);
}

/*
 */

c_darray_t *C_darray_defragment(c_darray_t *a)
{
  uint_t i;
  c_darray_t *n;
  void *e;

  if(!a) return(NULL);
  if(!a->del_count) return(a);

  n = C_darray_create(a->resize, a->elemsz);
  for(i = 0; i < a->bot; i++)
    if((e = C_darray_restore(a, i)))
      C_darray_store(n, e, NULL);
  C_darray_destroy(a);

  return(n);
}

/*
 */

c_bool_t C_darray_iterate(c_darray_t *a,
                          c_bool_t (*iter)(void *elem, uint_t index,
                                           void *hook),
                          uint_t index, void *hook)
{
  char *byp, by, bil;
  div_t dv;
  uint_t i, left;
  void *e;

  if(!a || !iter)
    return(FALSE);
  if(a->bot < index)
    return(TRUE);
  dv = div(index, C_DARRAY_NBBY);

  for(e = a->mem + (a->elemsz * index),
        byp = a->free_list + dv.quot,
        i = 0,
        left = a->bot - index,
        bil = C_DARRAY_NBBY - dv.rem,
        by = *byp >> dv.rem;
      left--;
      i++, e += a->elemsz)
  {
    if(!(by & 1))
      if(!iter(e, i, hook))
        return(FALSE);

    if(!--bil)
      bil = C_DARRAY_NBBY, by = *(byp++);
  }

  return(TRUE);
}

/* end of source file */
