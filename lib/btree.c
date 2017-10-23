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

#include <string.h>
#include <sys/types.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"
#include "cbase/data.h"

/* Macros */

#define __C_BTREE_OK 0
#define __C_BTREE_OVERFLOW 1
#define __C_BTREE_DUPLICATE 2
#define __C_BTREE_UNDERFLOW 3
#define __C_BTREE_NOTFOUND 4

/* Functions */

c_btree_t *C_btree_create(uint_t order)
{
  c_btree_t *t;

  if(order < 2)
    return(NULL);

  t = C_new(c_btree_t);

  t->root = NULL;
  t->order = order;
  t->nkeys = t->order * 2;

  return(t);
}

/*
 */

static c_btree_node_t *__C_btree_create_node(c_btree_t *tree)
{
  c_btree_node_t *node = C_new(c_btree_node_t);

  node->keys = C_newa(tree->nkeys, c_datum_t);
  node->children = C_newa((tree->nkeys + 1), c_btree_node_t *);

  return(node);
}

/*
 */

static void __C_btree_destroy_node(c_btree_t *tree, c_btree_node_t *node)
{
  C_free(node->keys);
  C_free(node->children);
  C_free(node);
}

/*
 */

static void __C_btree_destroy_recursive(c_btree_t *tree, c_btree_node_t *node)
{
  int j;

  for(j = 0; j < node->count; ++j)
    __C_btree_destroy_recursive(tree, node->children[j]);

  __C_btree_destroy_node(tree, node);
}

/*
 */

void C_btree_destroy(c_btree_t *tree)
{
  if(! tree)
    return;

  __C_btree_destroy_recursive(tree, tree->root);

  C_free(tree);
}

/*
 */

c_bool_t C_btree_set_destructor(c_btree_t *tree, void (*destructor)(void *))
{
  if(!tree)
    return(FALSE);

  tree->destructor = destructor;

  return(TRUE);
}

/*
 */

static uint_t __C_btree_bsearch(c_id_t x, c_datum_t *a, uint_t len)
{
  uint_t mid, left = 0, right = len - 1;

  if(x <= a[left].key)
    return(0);

  if(x > a[right].key)
    return(len);

  while((right - left) > 1)
  {
    mid = (right + left) / 2;
    if(x <= a[mid].key)
      right = mid;
    else
      left = mid;
  }

  return(right);
}

/*
 */

static int __C_btree_insert_node(c_btree_t *tree, c_datum_t key,
                                 c_btree_node_t *node, c_datum_t *rkey,
                                 c_btree_node_t **rnode)
{
  c_btree_node_t *newnode, *_node;
  c_datum_t newkey, _key;
  int i, j, s;

  /* We're at a leaf, and can't go any deeper. This node will need to be
     split. */

  if(! node)
  {
    *rkey = key;
    *rnode = NULL;
    return(__C_BTREE_OVERFLOW);
  }

  /* Figure out which subtree of this node should accept the new key,
     and try to store it in that subtree */

  i = __C_btree_bsearch(key.key, node->keys, node->count);
  if((i < node->count) && (node->keys[i].key == key.key))
    return(__C_BTREE_DUPLICATE);

  s = __C_btree_insert_node(tree, key, node->children[i], &newkey,
                            &newnode);

  if(s != __C_BTREE_OVERFLOW)
    return(s);

  /* The subtree could not accept the new key, so let's try to store it in
     this node, if there is room. */

  if(node->count < tree->nkeys)
  {
    i = __C_btree_bsearch(newkey.key, node->keys, node->count);

    /* shift keys & pointers to right for insertion */

    for(j = node->count; j > i; --j)
    {
      node->keys[j] = node->keys[j - 1];
      node->children[j + 1] = node->children[j];
    }

    /* insert new key */

    node->keys[i] = newkey;
    node->children[i + 1] = newnode;
    ++node->count;
    return(__C_BTREE_OK);
  }

  /* There was no room for the key in this node, so split the node. */

  if(i == tree->nkeys)
  {
    _key = newkey;
    _node = newnode;
  }
  else
  {
    _key = node->keys[tree->nkeys - 1];
    _node = node->children[tree->nkeys];

    for(j = (tree->nkeys - 1); j > i; --j)
    {
      node->keys[j] = node->keys[j - 1];
      node->children[j + 1] = node->children[j];
    }

    node->keys[i] = newkey;
    node->children[i + 1] = newnode;
  }

  /* move right half of keys & child pointers into new node, and float
     middle item (excess) up to level above */

  *rkey = node->keys[tree->order];
  node->count = tree->order;
  *rnode = __C_btree_create_node(tree);
  (*rnode)->count = tree->order;

  for(j = 0; j < tree->order - 1; ++j)
  {
    (*rnode)->keys[j] = node->keys[j + tree->order + 1];
    (*rnode)->children[j] = node->children[j + tree->order + 1];
  }

  (*rnode)->keys[tree->order - 1] = _key;
  (*rnode)->children[tree->order - 1] = node->children[tree->nkeys];
  (*rnode)->children[tree->order] = _node;

  return(__C_BTREE_OVERFLOW);
}

/*
 */

c_bool_t C_btree_store(c_btree_t *tree, c_id_t key, const void *data)
{
  c_btree_node_t *newnode, *n;
  c_datum_t newkey, ikey;
  int s;

  if(!tree || (key == 0LL))
    return(FALSE);

  ikey.key = key;
  ikey.value = (void *)data;

  s = __C_btree_insert_node(tree, ikey, tree->root, &newkey, &newnode);
  if(s == __C_BTREE_DUPLICATE)
    return(FALSE);

  if(s == __C_BTREE_OVERFLOW)
  {
    n = __C_btree_create_node(tree);
    n->count = 1;
    n->keys[0] = newkey;
    n->children[0] = tree->root;
    n->children[1] = newnode;

    tree->root = n;
  }

  return(TRUE);
}

/*
 */

static int __C_btree_delete_node(c_btree_t *tree, c_id_t key,
                                 c_btree_node_t *node)
{
  c_bool_t borrowleft;
  int i, j, nq, s;
  c_btree_node_t *left, *right, *q, *q1;
  c_datum_t tmp, *_key = NULL;

  if(! node)
    return(__C_BTREE_NOTFOUND);

  /* Search for the key in this node. If found, and if the number of items
   * remaining is at least M (or for the root, at least 1), then delete it,
   * and we're done. Otherwise, we have underflow.
   */

  i = __C_btree_bsearch(key, node->keys, node->count);
  if(! node->children[0]) /* node is a leaf */
  {
    /* is it actually in this node? */

    if((i == node->count) || (node->keys[i].key > key))
      return(__C_BTREE_NOTFOUND);

    /* shift remaining elements over */

    for(j = i + 1; j < node->count; ++j)
    {
      node->keys[j - 1] = node->keys[j];
      node->children[j] = node->children[j + 1];
    }

    /* free datum here */

    tree->destructor(node->keys[i].value);

    --node->count;

    if(node->count >= ((node == tree->root) ? 1 : tree->order))
      return(__C_BTREE_OK);
    else
      return(__C_BTREE_UNDERFLOW);
  }

  /* Node is not a leaf. If the key is found in this node, then start at
   * left child and follow rightmost branches until we find a leaf.
   */

  /* *t is an interior node (not a leaf): */

  _key = &(node->keys[i]);
  left = node->children[i];

  if((i < node->count) && (node->keys[i].key == key))
  {
    /* key found in interior node. Go to left child
     * p[i], then follow a path all the way to a leaf, using rightmost
     * branches.
     */

    q = left; /* node->children[i]; */
    nq = q->count;

    while((q1 = q->children[nq]) != NULL)
    {
      q = q1;
      nq = q->count;
    }

    /* exchange k[i] with the rightmost item in leaf. This item is the largest
     * key that is less than the key we are deleting. */

    tmp = q->keys[nq - 1];
    q->keys[nq - 1] = node->keys[i];
    node->keys[i] = tmp;
  }

  /* The key now resides in a leaf node, and we can delete it easily.
   */

  s = __C_btree_delete_node(tree, key, left);
  if(s != __C_BTREE_UNDERFLOW)
    return(s);

  /* Deletion resulted in an underflow. Try to borrow a node from the left.
   * and if necessary, merge
   */

  /* If there is no right sibling OR if there is a left sibling and the
   * left sibling has more than M keys and the right sibling has M keys,
   * the we borrow from the left.
   */

  borrowleft = ((i == node->count)
                || ((i > 0) && (node->children[i + 1]->count == tree->order)
                    && (node->children[i - 1]->count > tree->order)));

  if(borrowleft) /* p[i] is rightmost pointer in *p */
  {
    _key = &(node->keys[i - 1]); /* the item we are borrowing */

    left = node->children[i - 1]; /* left sibling */
    right = node->children[i]; /* this node */
  }
  else
    right = node->children[i + 1]; /* right sibling */

  if(borrowleft) /* borrowing from left sibling */
  {
    /* shift everything over to the right one position in this node */

    right->children[right->count + 1] = right->children[right->count];

    for(j = right->count; j > 0; --j)
    {
      right->keys[j] = right->keys[j - 1];
      right->children[j] = right->children[j - 1];
    }

    ++right->count;

    /* leftmost item in this node becomes rightmost item from left sibling */

    right->keys[0] = *_key;
    right->children[0] = left->children[left->count];

    *_key = left->keys[left->count - 1];

    --left->count;

    if(left->count >= tree->order)
      return(__C_BTREE_OK); /* enough children in left sibling; no merge
                               necessary */
  }

  else if(right->count > tree->order) /* borrowing from right */
  {
    /* rightmost item in this node becomes leftmost item from right sibling */

    left->keys[tree->order - 1] = *_key;
    left->children[tree->order] = right->children[0];
    *_key = right->keys[0];

    /* shift everything to the left one position in the right sibling */

    ++left->count;
    --right->count;

    for(j = 0; j < right->count; ++j)
    {
      right->keys[j] = right->keys[j + 1];
      right->children[j] = right->children[j + 1];
    }

    right->children[right->count] = right->children[right->count + 1];
    return(__C_BTREE_OK);
  }

  /* merge */

  left->keys[tree->order - 1] = *_key;
  left->children[tree->order] = right->children[0];

  for(j = 0; j < tree->order; ++j)
  {
    left->keys[tree->order + j] = right->keys[j];
    left->children[tree->order + j + 1] = right->children[j + 1];
  }

  left->count = tree->nkeys;

  __C_btree_destroy_node(tree, right);

  /* shift the items on the right of the deleted item to the left */

  for(j = i + 1; j < node->count; ++j)
  {
    node->keys[j - 1] = node->keys[j];
    node->children[j] = node->children[j + 1];
  }

  --node->count;

  /* do we have underflow in the parent now? */

  if(node->count < ((node == tree->root) ? 1 : tree->order))
    return(__C_BTREE_UNDERFLOW);

  return(__C_BTREE_OK);
}

/*
 */

c_bool_t C_btree_delete(c_btree_t *tree, c_id_t key)
{
  int s;
  c_btree_node_t *newroot;

  if(!tree || (key == 0))
    return(FALSE);

  s = __C_btree_delete_node(tree, key, tree->root);
  if(s == __C_BTREE_NOTFOUND)
    return(FALSE);

  if(s == __C_BTREE_UNDERFLOW)
  {
    /* underflow, so shrink tree */

    newroot = tree->root->children[0];
    __C_btree_destroy_node(tree, tree->root);
    tree->root = newroot;
  }

  return(TRUE);
}

/*
 */

void * C_btree_restore(c_btree_t *tree, c_id_t key)
{
  c_btree_node_t *node;
  int i;

  if(!tree || !key)
    return(FALSE);

  node = tree->root;

  while(node)
  {
    i = __C_btree_bsearch(key, node->keys, node->count);
    if((i < node->count) && (node->keys[i].key == key))
    {
      /* found it! */
      return(node->keys[i].value);
    }

    node = node->children[i];
  }

  return(NULL);
}

/*
 */

static c_bool_t __C_btree_iterate(c_btree_node_t *node,
                                  c_bool_t (*consumer)(void *elem, void *hook),
                                  void *hook)
{
  int i;

  /* go through all elements in this node */

  for(i = 0; i < node->count; ++i)
  {
    c_datum_t datum = node->keys[i];

    if(! consumer(datum.value, hook))
      return(FALSE);
  }

  /* now recurse on children */

  for(i = 0; i <= node->count; ++i)
  {
    if(! __C_btree_iterate(node->children[i], consumer, hook))
      return(FALSE);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_btree_iterate(c_btree_t *btree,
                         c_bool_t (*consumer)(void *elem, void *hook),
                         void *hook)
{

  return(__C_btree_iterate(btree->root, consumer, hook));
}

/* end of source file */
