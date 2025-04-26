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
#include "cbase/util.h"

/* File scope variables */

static uint_t (*__C_hashtable_hashfunc)(const char *s, uint_t modulo)
  = C_string_hash;

/* Functions */

c_bool_t C_hashtable_set_hashfunc(uint_t (*func)(const char *s, uint_t modulo))
{
  c_bool_t r = FALSE;

  if(func)
    __C_hashtable_hashfunc = func, r = TRUE;

  return(r);
}

/*
 */

c_hashtable_t *C_hashtable_create(uint_t buckets)
{
  c_hashtable_t *h;

  if(!buckets)
    return(NULL);

  h = C_new(c_hashtable_t);
  h->buckets = buckets;
  h->size = 0;
  h->table = C_calloc(buckets, c_linklist_t *);

  return(h);
}

/*
 */

void C_hashtable_destroy(c_hashtable_t *h)
{
  uint_t i;
  c_linklist_t **p;
  c_tag_t *tag;

  if(!h)
    return;

  /* delete each linked list */

  for(i = h->buckets, p = h->table; i--; ++p)
  {
    if(*p)
    {
      /* delete each tag (& its key) in the linked list */

      for(C_linklist_move_head(*p);
          (tag = (c_tag_t *)C_linklist_restore(*p)) != NULL;
          C_linklist_move_next(*p))
      {
        C_free(tag->key);

        if(h->destructor)
          h->destructor(tag->data);

        C_free(tag);
      }

      C_linklist_destroy(*p);
    }
  }

  /* delete the backbone and the container structure */

  C_free(h->table);
  C_free(h);
}

/*
 */

c_bool_t C_hashtable_set_destructor(c_hashtable_t *h,
                                    void (*destructor)(void *data))
{
  if(!h)
    return(FALSE);

  h->destructor = destructor;

  return(TRUE);
}

/*
 */

c_bool_t C_hashtable_store(c_hashtable_t *h, const char *key, const void *data)
{
  c_tag_t *tag;
  c_linklist_t **l;

  if(!h || !key || !data)
    return(FALSE);
  if(!*key)
    return(FALSE);

  tag = C_new(c_tag_t);
  tag->key = C_string_dup(key);
  tag->data = (char *)data;

  l = &(h->table[__C_hashtable_hashfunc(key, h->buckets)]);

  if(!(*l))
    *l = C_linklist_create();

  C_linklist_prepend(*l, (void *)tag);
  ++h->size;

  return(TRUE);
}

/*
 */

void *C_hashtable_restore(c_hashtable_t *h, const char *key)
{
  c_tag_t *tag;
  c_linklist_t *l;

  if(!h || !key)
    return(FALSE);
  if(! *key)
    return(FALSE);

  if((l = h->table[__C_hashtable_hashfunc(key, h->buckets)]))
  {
    for(C_linklist_move_head(l); !C_linklist_isend(l);
        C_linklist_move_next(l))
    {
      tag = (c_tag_t *)C_linklist_restore(l);
      if(!strcmp(tag->key, key))
        return(tag->data);
    }
  }

  return(NULL);
}

/*
 */

c_bool_t C_hashtable_delete(c_hashtable_t *h, const char *key)
{
  c_tag_t *tag;
  c_linklist_t *l;

  if(! *key)
    return(FALSE);

  if((l = h->table[__C_hashtable_hashfunc(key, h->buckets)]))
  {
    for(C_linklist_move_head(l); !C_linklist_isend(l);
        C_linklist_move_next(l))
    {
      tag = (c_tag_t *)C_linklist_restore(l);
      if(!strcmp(tag->key, key))
      {
        C_linklist_delete(l);
        --h->size;

        C_free(tag->key);

        if(h->destructor)
          h->destructor(tag->data);

        C_free(tag);

        return(TRUE);
      }
    }
  }

  return(FALSE);
}

/*
 */

char **C_hashtable_keys(c_hashtable_t *h, size_t *len)
{
  c_vector_t *vec;
  c_linklist_t **p;
  c_tag_t *t;
  char **v;
  uint_t i;

  if(!h)
    return(NULL);

  vec = C_vector_start(h->buckets);

  for(i = h->buckets, p = h->table; i--; ++p)
  {
    for(C_linklist_move_head(*p);
        (t = (c_tag_t *)C_linklist_restore(*p)) != NULL;
        C_linklist_move_next(*p))
    {
      C_vector_store(vec, C_string_dup(t->key));
    }
  }

  v = C_vector_end(vec, len);

  return(v);
}

/* end of source file */
