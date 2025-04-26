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

/* Local headers */

#include "cbase/defs.h"
#include "cbase/data.h"
#include "cbase/system.h"

/* Functions */

static c_link_t *__C_linklist_unlink_r(c_linklist_t *l, c_link_t **p)
{
  c_link_t *r;

  r = *p;
  if(l->head == r)
    l->head = r->next;
  if(l->tail == r)
    l->tail = r->prev;
  if((*p)->prev)
    (*p)->prev->next = (*p)->next;
  if((*p)->next)
    (*p)->next->prev = (*p)->prev;
  (*p) = (*p)->next;
  --l->size;

  return(r);
}

/*
 */

c_linklist_t *C_linklist_create(void)
{
  c_linklist_t *l = C_new(c_linklist_t);

  l->head = l->tail = l->p = NULL;
  l->size = 0L;

  return(l);
}

/*
 */

void C_linklist_destroy(c_linklist_t *l)
{
  c_link_t *p, *q;

  if(!l)
    return;

  for(p = l->head; p;)
  {
    q = p;
    p = p->next;

    if(l->destructor)
      l->destructor(q->data);

    C_free(q);
  }
  C_free(l);
}

/*
 */

c_bool_t C_linklist_set_destructor(c_linklist_t *l,
                                   void (*destructor)(void *data))
{
  if(!l)
    return(FALSE);

  l->destructor = destructor;
  return(TRUE);
}

/*
 */

c_bool_t C_linklist_store(c_linklist_t *l, const void *data)
{
  if(!l)
    return(FALSE);

  return(C_linklist_store_r(l, data, &(l->p)));
}

/*
 */

c_bool_t C_linklist_store_r(c_linklist_t *l, const void *data, c_link_t **p)
{
  c_link_t *q;

  if(!l || !data)
    return(FALSE);

  q = C_new(c_link_t);
  q->data = (void *)data;

  if(*p == l->head) /* new head? */
  {
    q->next = l->head;
    q->prev = NULL;
    if(l->head)
      l->head->prev = q;
    l->head = q;
    if(!l->tail)
      l->tail = q;
  }
  else if(!(*p)) /* new tail? */
  {
    q->next = NULL;
    q->prev = l->tail;
    if(l->tail)
      l->tail->next = q;
    l->tail = q;
  }
  else
  {
    q->next = *p;
    q->prev = (*p)->prev;
    if((*p)->prev)
      (*p)->prev->next = q;
    (*p)->prev = q;
  }
  *p = q;
  ++l->size;

  return(TRUE);
}

/*
 */

void *C_linklist_restore(c_linklist_t *l)
{
  if(!l)
    return(NULL);

  return(C_linklist_restore_r(l, &(l->p)));
}

/*
 */

void *C_linklist_restore_r(c_linklist_t *l, c_link_t **p)
{

  if(!l)
    return(NULL);

  return(*p ? (*p)->data : NULL);
}

/*
 */

c_bool_t C_linklist_search(c_linklist_t *l, const void *data)
{
  if(!l)
    return(FALSE);

  return(C_linklist_search_r(l, data, &(l->p)));
}

/*
 */

c_bool_t C_linklist_search_r(c_linklist_t *l, const void *data, c_link_t **p)
{
  void *d;

  for(C_linklist_move_head_r(l, p);
      (d = C_linklist_restore_r(l, p)) != NULL;
      C_linklist_move_next_r(l, p))
  {
    if(d == data)
      return(TRUE);
  }

  return(FALSE);
}

/*
 */

c_bool_t C_linklist_prepend(c_linklist_t *l, const void *data)
{
  c_link_t *p;

  if(!l || !data)
    return(FALSE);

  C_linklist_move_head_r(l, &p);
  return(C_linklist_store_r(l, data, &p));
}

/*
 */

void *C_linklist_pop(c_linklist_t *l)
{
  void *r;
  c_link_t *p, *q;

  if(!l)
    return(NULL);
  if(!l->size)
    return(NULL);

  C_linklist_move_head_r(l, &p);
  q = __C_linklist_unlink_r(l, &p);
  r = q->data;
  C_free(q);

  return(r);
}

/*
 */

void *C_linklist_peek(c_linklist_t *l)
{
  c_link_t *p;

  if(!l)
    return(NULL);
  if(!l->size)
    return(NULL);

  C_linklist_move_head_r(l, &p);
  return(C_linklist_restore_r(l, &p));
}

/*
 */

c_bool_t C_linklist_append(c_linklist_t *l, const void *data)
{
  c_link_t *p;

  if(!l || !data)
    return(FALSE);

  C_linklist_move_end_r(l, &p);
  return(C_linklist_store_r(l, data, &p));
}

/*
 */

c_bool_t C_linklist_delete(c_linklist_t *l)
{
  if(!l)
    return(FALSE);
  if(!l->p)
    return(FALSE);

  return(C_linklist_delete_r(l, &(l->p)));
}

/*
 */

c_bool_t C_linklist_delete_r(c_linklist_t *l, c_link_t **p)
{
  c_link_t *r;

  if(!l)
    return(FALSE);
  if(!*p)
    return(FALSE);

  r = __C_linklist_unlink_r(l, p);

  if(l->destructor)
    l->destructor(r->data);

  C_free(r);

  return(TRUE);
}

/*
 */

c_bool_t C_linklist_move(c_linklist_t *l, int where)
{
  if(!l)
    return(FALSE);

  return(C_linklist_move_r(l, where, &(l->p)));
}

/*
 */

c_bool_t C_linklist_move_r(c_linklist_t *l, int where, c_link_t **p)
{

  if(!l)
    return(FALSE);

  switch(where)
  {
    case C_LINKLIST_HEAD:
      *p = l->head;
      return(TRUE);

    case C_LINKLIST_TAIL:
      *p = l->tail;
      return(TRUE);

    case C_LINKLIST_NEXT:
      if(*p)
      {
        *p = (*p)->next;
        return(TRUE);
      }
      break;

    case C_LINKLIST_PREV:
      if(*p)
        if((*p)->prev)
        {
          *p = (*p)->prev;
          return(TRUE);
        }
      break;

    case C_LINKLIST_END:
      *p = NULL;
      return(TRUE);
      break;
  }

  return(FALSE);
}

/* end of source file */
