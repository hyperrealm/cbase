/* ----------------------------------------------------------------------------
   cbase - A C Foundation Library
   Copyright (C) 1994-2025 Mark A Lindner

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

#ifndef __cbase_data_h
#define __cbase_data_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cbase/defs.h>

/* ----------------------------------------------------------------------------
 * linked lists
 * ----------------------------------------------------------------------------
 */

  typedef struct c_link_t
  {
    void *data;
    struct c_link_t *next;
    struct c_link_t *prev;
  } c_link_t;

#define C_link_next(L) ((L)->next)
#define C_link_prev(L) ((L)->prev)
#define C_link_data(L) ((L)->data)

  typedef struct c_linklist_t
  {
    c_link_t *head;
    c_link_t *tail;
    c_link_t *p;
    size_t size;
    void (*destructor)(void *);
  } c_linklist_t;

#define C_linklist_head(L) ((L)->head)
#define C_linklist_tail(L) ((L)->tail)
#define C_linklist_size(L) ((L)->size)

  extern c_linklist_t *C_linklist_create(void);
  extern void C_linklist_destroy(c_linklist_t *l);

  extern c_bool_t C_linklist_set_destructor(c_linklist_t *l,
                                            void (*destructor)(void *));

  extern c_bool_t C_linklist_store(c_linklist_t *l, const void *data);

  extern c_bool_t C_linklist_append(c_linklist_t *l, const void *data);
  extern c_bool_t C_linklist_prepend(c_linklist_t *l, const void *data);

  extern void *C_linklist_restore(c_linklist_t *l);

  extern c_bool_t C_linklist_search(c_linklist_t *l, const void *data);

  extern c_bool_t C_linklist_move(c_linklist_t *l, int where);
  extern c_bool_t C_linklist_delete(c_linklist_t *l);

  extern c_bool_t C_linklist_store_r(c_linklist_t *l, const void *data,
                                     c_link_t **p);

  extern void *C_linklist_restore_r(c_linklist_t *l, c_link_t **p);

  extern c_bool_t C_linklist_search_r(c_linklist_t *l, const void *data,
                                      c_link_t **p);

  extern c_bool_t C_linklist_delete_r(c_linklist_t *l, c_link_t **p);
  extern c_bool_t C_linklist_move_r(c_linklist_t *l, int where, c_link_t **p);


#define C_LINKLIST_HEAD 0
#define C_LINKLIST_TAIL 1
#define C_LINKLIST_NEXT 2
#define C_LINKLIST_PREV 3
#define C_LINKLIST_END  4

#define C_linklist_move_head(L) C_linklist_move((L), C_LINKLIST_HEAD)
#define C_linklist_move_tail(L) C_linklist_move((L), C_LINKLIST_TAIL)
#define C_linklist_move_next(L) C_linklist_move((L), C_LINKLIST_NEXT)
#define C_linklist_move_prev(L) C_linklist_move((L), C_LINKLIST_PREV)
#define C_linklist_move_end(L)  C_linklist_move((L), C_LINKLIST_END)

#define C_linklist_ishead(L) (((L)->p == (L)->head) ? TRUE : FALSE)
#define C_linklist_istail(L) (((L)->p == (L)->tail) ? TRUE : FALSE)
#define C_linklist_isend(L) (((L)->p == NULL) ? TRUE : FALSE)

#define C_linklist_length(L) ((L)->size)

#define C_linklist_move_head_r(L, P)            \
  C_linklist_move_r((L), C_LINKLIST_HEAD, (P))
#define C_linklist_move_tail_r(L, P)            \
  C_linklist_move_r((L), C_LINKLIST_TAIL, (P))
#define C_linklist_move_next_r(L, P)            \
  C_linklist_move_r((L), C_LINKLIST_NEXT, (P))
#define C_linklist_move_prev_r(L, P)            \
  C_linklist_move_r((L), C_LINKLIST_PREV, (P))
#define C_linklist_move_end_r(L, P)             \
  C_linklist_move_r((L), C_LINKLIST_END, (P))

#define C_linklist_ishead_r(L, P)               \
  ((*(P) == (L)->head) ? TRUE : FALSE)
#define C_linklist_istail_r(L, P)               \
  ((*(P) == (L)->tail) ? TRUE : FALSE)
#define C_linklist_isend_r(L, P)                \
  (*(P) == NULL ? TRUE : FALSE)

/* ----------------------------------------------------------------------------
 * stacks
 * ----------------------------------------------------------------------------
 */

  typedef c_linklist_t c_stack_t;

#define C_stack_create() C_linklist_create()
#define C_stack_destroy(S) C_linklist_destroy(S)

#define C_stack_set_destructor(S, D)            \
  C_linklist_set_destructor((S), (D))

  extern void *C_linklist_pop(c_linklist_t *l);
  extern void *C_linklist_peek(c_linklist_t *l);

#define C_stack_push(S, D) C_linklist_prepend((S), (D))
#define C_stack_pop(S) C_linklist_pop(S)
#define C_stack_peek(S) C_linklist_peek(S)

#define C_stack_depth(S) C_linklist_length(S)

/* ----------------------------------------------------------------------------
 * queues
 * ----------------------------------------------------------------------------
 */

  typedef c_linklist_t c_queue_t;

#define C_queue_create() C_linklist_create()
#define C_queue_destroy(Q) C_linklist_destroy(Q)

#define C_queue_set_destructor(Q, D)            \
  C_linklist_set_destructor((Q), (D))

#define C_queue_enqueue(Q, D) C_linklist_append((Q), (D))

#define C_queue_dequeue(Q) C_linklist_pop(Q)

#define C_queue_length(Q) C_linklist_length(Q)

/* ----------------------------------------------------------------------------
 * dynamic arrays
 * ----------------------------------------------------------------------------
 */

  typedef struct c_darray_t
  {
    void *mem;
    uint_t resize;
    size_t elemsz;
    uint_t size;
    uint_t isize;
    uint_t bot;
    uint_t del_count;
    char *free_list;
    uint_t iresize;
  } c_darray_t;

#define C_darray_size(A) ((A)->bot - (A)->del_count)
#define C_darray_last(A) ((A)->bot)

#define C_DARRAY_MAX_RESIZE 100

  extern c_darray_t *C_darray_create(uint_t resize, size_t elemsz);
  extern void C_darray_destroy(c_darray_t *a);

  extern void *C_darray_store(c_darray_t *a, const void *data, uint_t *index);
  extern void *C_darray_restore(c_darray_t *a, uint_t index);
  extern c_bool_t C_darray_delete(c_darray_t *a, uint_t index);

  extern c_darray_t *C_darray_load(const char *path);
  extern c_bool_t C_darray_save(c_darray_t *a, const char *path);
  extern c_darray_t *C_darray_defragment(c_darray_t *a);
  extern c_bool_t C_darray_iterate(c_darray_t *a,
                                   c_bool_t (*iter)(void *elem, uint_t index,
                                                    void *hook),
                                   uint_t index, void *hook);

/* ----------------------------------------------------------------------------
 * dynamic strings
 * ----------------------------------------------------------------------------
 */

  typedef struct c_dstring_t
  {
    char *mem;
    off_t p;
    off_t len;
    uint_t blk;
    uint_t blocksz;
  } c_dstring_t;

#define C_dstring_length(D) ((D)->len)

#define C_DSTRING_SEEK_ABS 0
#define C_DSTRING_SEEK_REL 1
#define C_DSTRING_SEEK_END 2

#define C_DSTRING_MIN_BLOCKSZ 80
#define C_DSTRING_LOAD_BLOCKSZ 4096

  extern c_dstring_t *C_dstring_create(uint_t blocksz);
  extern char *C_dstring_destroy(c_dstring_t *d);

  extern c_bool_t C_dstring_putc(c_dstring_t *d, char c);
  extern c_bool_t C_dstring_puts(c_dstring_t *d, const char *s);
  extern c_bool_t C_dstring_puts_len(c_dstring_t *d, const char *s, size_t len);
  extern char C_dstring_getc(c_dstring_t *d);
  extern char *C_dstring_gets(c_dstring_t *d, char *s, size_t len, char termin);

  extern c_bool_t C_dstring_seek(c_dstring_t *d, off_t where, int whence);
  extern c_bool_t C_dstring_trunc(c_dstring_t *d, off_t length);
  extern c_dstring_t *C_dstring_load(const char *path, uint_t blocksz);
  extern c_bool_t C_dstring_save(c_dstring_t *d, const char *path);

#define C_dstring_ungetc(D) C_dstring_seek((D), -1L, C_DSTRING_SEEK_REL)
#define C_dstring_rewind(D) C_dstring_seek((D),  0L, C_DSTRING_SEEK_ABS)
#define C_dstring_append(D) C_dstring_seek((D),  0L, C_DSTRING_SEEK_END)

/* ----------------------------------------------------------------------------
 * hash tables
 * ----------------------------------------------------------------------------
 */

  typedef struct c_tag_t
  {
    char *key;
    void *data;
  } c_tag_t;

#define C_tag_key(T) ((T)->key)
#define C_tag_data(T) ((T)->data)

  typedef struct c_hashtable_t
  {
    uint_t buckets;
    c_linklist_t **table;
    size_t size;
    void (*destructor)(void *);
  } c_hashtable_t;

#define C_hashtable_size(H) ((H)->size)

  extern c_hashtable_t *C_hashtable_create(uint_t buckets);
  extern void C_hashtable_destroy(c_hashtable_t *h);

  extern c_bool_t C_hashtable_set_destructor(c_hashtable_t *h,
                                             void (*destructor)(void *));

  extern c_bool_t C_hashtable_set_hashfunc(uint_t (*func)(const char *s,
                                                          uint_t modulo));

  extern c_bool_t C_hashtable_store(c_hashtable_t *h, const char *key,
                                    const void *data);
  extern void *C_hashtable_restore(c_hashtable_t *h, const char *key);
  extern c_bool_t C_hashtable_delete(c_hashtable_t *h, const char *key);

  extern char **C_hashtable_keys(c_hashtable_t *h, size_t *len);

/* ----------------------------------------------------------------------------
 * b-trees
 * ----------------------------------------------------------------------------
 */

  typedef unsigned long long c_id_t;

  typedef struct c_datum_t
  {
    c_id_t key;
    void *value;
  } c_datum_t;

#define C_datum_key(D)                          \
  (D)->key

#define C_datum_value(D)                        \
  (D)->value

  typedef struct c_btree_node_t
  {
    uint_t count;
    c_datum_t *keys;
    struct c_btree_node_t **children;
  } c_btree_node_t;

  typedef struct c_btree_t
  {
    c_btree_node_t *root;
    uint_t order;
    uint_t nkeys;
    void (*destructor)(void *);
  } c_btree_t;

  extern c_btree_t *C_btree_create(uint_t order);
  extern void C_btree_destroy(c_btree_t *tree);

  extern c_bool_t C_btree_set_destructor(c_btree_t *tree,
                                         void (*destructor)(void *));

  extern c_bool_t C_btree_store(c_btree_t *tree, c_id_t key, const void *data);
  extern void *C_btree_restore(c_btree_t *tree, c_id_t key);

  extern c_bool_t C_btree_delete(c_btree_t *tree, c_id_t key);

  extern c_bool_t C_btree_iterate(c_btree_t *btree,
                                  c_bool_t (*consumer)(void *elem, void *hook),
                                  void *hook);

#define C_btree_order(T)                        \
  ((T)->order)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_data_h */

/* end of library header */
