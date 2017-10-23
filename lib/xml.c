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

#ifdef HAVE_LIBEXPAT

/* System headers */

#include <ctype.h>
#include <string.h>

#include <expat.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/data.h"
#include "cbase/system.h"
#include "cbase/util.h"
#include "cbase/xml.h"

/* Structures */

struct c_xml_parser_context
{
  c_xml_document_t *doc;
  c_xml_element_t *current;
  c_xml_element_t *root;
  c_dstring_t *chardata;
};

/* File Scope Functions */

static void __C_xml_param_destructor(void *value)
{
  C_free(value);
}

/* Functions */

c_xml_element_t *C_xml_element_create(const char *name)
{
  c_xml_element_t *e = C_new(c_xml_element_t);
  e->name = C_string_dup(name);

  return(e);
}

/*
 */

static c_bool_t __C_xml_element_destroy_tree(c_xml_element_t *elem,
                                             c_bool_t recursive)
{
  void *c;

  if(!elem)
    return(FALSE);

  /* remove children */

  if(elem->children && recursive)
  {
    for(C_linklist_move_head(elem->children);
        (c = C_linklist_restore(elem->children)) != NULL;
        C_linklist_move_next(elem->children))
    {
      __C_xml_element_destroy_tree((c_xml_element_t *)c, TRUE);
    }

    C_linklist_destroy(elem->children);
  }

  /* remove parameters */

  C_xml_element_delete_params(elem);

  /* delete the element itself */

  C_free(elem->name);
  C_free(elem->content);
  C_free(elem);

  return(TRUE);
}

/*
 */

c_bool_t C_xml_element_destroy(c_xml_element_t *elem)
{
  return(__C_xml_element_destroy_tree(elem, FALSE));
}

/*
 */

c_bool_t C_xml_element_destroy_recursive(c_xml_element_t *elem)
{
  return(__C_xml_element_destroy_tree(elem, TRUE));
}

/*
 */

c_xml_document_t *C_xml_document_create(const char *encoding)
{
  c_xml_document_t *d = C_new(c_xml_document_t);

  d->encoding = C_string_dup(encoding);

  return(d);
}

/*
 */

c_bool_t C_xml_document_destroy(c_xml_document_t *doc)
{
  if(!doc)
    return(FALSE);

  if(doc->root)
  {
    if(! __C_xml_element_destroy_tree(doc->root, TRUE))
      return(FALSE);
  }

  C_free(doc->encoding);
  C_free(doc);

  return(TRUE);
}

/*
 */

c_bool_t C_xml_document_set_root(c_xml_document_t *doc, c_xml_element_t *root)
{
  if(!doc)
    return(FALSE);

  doc->root = root;

  return(TRUE);
}

/*
 */

c_xml_element_t *C_xml_document_get_root(c_xml_document_t *doc)
{
  if(!doc)
    return(NULL);

  return(doc->root);
}

/*
 */

c_bool_t C_xml_element_set_content(c_xml_element_t *elem, const char *content)
{
  if(!elem)
    return(FALSE);

  if(elem->children)
    if(C_linklist_size(elem->children) > 0)
      return(FALSE);

  if(elem->content)
    C_free(elem->content);

  elem->content = (content ? C_string_dup(content) : NULL);
  return(TRUE);
}

/*
 */

const char *C_xml_element_get_content(c_xml_element_t *elem)
{
  if(!elem)
    return(NULL);

  return(elem->content);
}

/*
 */

c_bool_t C_xml_element_set_param(c_xml_element_t *elem, const char *param,
                                 const char *value)
{
  if(!elem || !param || !value)
    return(FALSE);

  if(!elem->params)
  {
    elem->params = C_hashtable_create(26);
    C_hashtable_set_destructor(elem->params, __C_xml_param_destructor);
  }

  C_hashtable_store(elem->params, param, C_string_dup(value));

  return(TRUE);
}

/*
 */

const char *C_xml_element_get_param(c_xml_element_t *elem, const char *param)
{
  if(!elem || !param)
    return(NULL);

  if(!(elem->params))
    return(NULL);

  return((const char *)C_hashtable_restore(elem->params, param));
}

/*
 */

c_bool_t C_xml_element_delete_param(c_xml_element_t *elem, const char *param)
{
  if(!elem || !param)
    return(FALSE);

  if(!(elem->params))
    return(FALSE);

  return(C_hashtable_delete(elem->params, param));
}

/*
 */

c_bool_t C_xml_element_delete_params(c_xml_element_t *elem)
{
  if(!elem)
    return(FALSE);

  if(!(elem->params))
    return(TRUE);

  C_hashtable_destroy(elem->params);
  elem->params = NULL;

  return(TRUE);
}

/*
 */

c_xml_element_t **C_xml_element_get_children(c_xml_element_t *elem)
{
  c_vector_t *vec;
  void *c;
  c_xml_element_t *celem;

  if(!elem)
    return(NULL);

  if(!(elem->children))
    return(NULL);

  vec = C_vector_start(20);

  for(C_linklist_move_head(elem->children);
      (c = C_linklist_restore(elem->children)) != NULL;
      C_linklist_move_next(elem->children))
  {
    celem = (c_xml_element_t *)c;
    C_vector_store(vec, (char *)celem);
  }

  return((c_xml_element_t **)C_vector_end(vec, NULL));
}

/*
 */

c_xml_element_t *C_xml_element_get_first_child(c_xml_element_t *elem)
{
  c_xml_element_t *celem;

  C_linklist_move_head(elem->children);
  celem = C_linklist_restore(elem->children);

  return(celem);
}

/*
 */

c_xml_element_t **C_xml_element_get_children_named(c_xml_element_t *elem,
                                                   const char *name)
{
  c_vector_t *vec;
  void *c;
  c_xml_element_t *celem;

  if(!elem || !name)
    return(NULL);

  if(!(elem->children))
    return(NULL);

  vec = C_vector_start(20);

  for(C_linklist_move_head(elem->children);
      (c = C_linklist_restore(elem->children)) != NULL;
      C_linklist_move_next(elem->children))
  {
    celem = (c_xml_element_t *)c;

    if(!strcmp(celem->name, name))
      C_vector_store(vec, (char *)celem);
  }

  return((c_xml_element_t **)C_vector_end(vec, NULL));
}

/*
 */

c_xml_element_t *C_xml_element_get_first_child_named(c_xml_element_t *elem,
                                                     const char *name)
{
  void *c;
  c_xml_element_t *celem;

  if(!elem || !name)
    return(NULL);

  if(!(elem->children))
    return(NULL);

  for(C_linklist_move_head(elem->children);
      (c = C_linklist_restore(elem->children)) != NULL;
      C_linklist_move_next(elem->children))
  {
    celem = (c_xml_element_t *)c;

    if(!strcmp(celem->name, name))
      return celem;
  }

  return(NULL);
}

/*
 */

c_bool_t C_xml_element_add_child(c_xml_element_t *parent,
                                 c_xml_element_t *elem)
{
  if(!parent || !elem)
    return(FALSE);

  if(parent->content)
    return(FALSE);

  if(parent == elem)
    return(FALSE);

  if(!(parent->children))
    parent->children = C_linklist_create();

  if(C_linklist_search(parent->children, elem))
    return(FALSE);

  C_linklist_store(parent->children, elem);

  elem->parent = parent;

  return(TRUE);
}

/*
 */

c_bool_t C_xml_element_remove_child(c_xml_element_t *parent,
                                    c_xml_element_t *elem)
{
  if(!parent || !elem)
    return(FALSE);

  if(!(parent->children))
    return(FALSE);

  if(parent == elem)
    return(FALSE);

  if(!C_linklist_search(parent->children, elem))
    return(FALSE);

  C_linklist_delete(parent->children);
  elem->parent = NULL;

  return(__C_xml_element_destroy_tree(elem, TRUE));
}

/*
 */

c_bool_t C_xml_element_remove_children(c_xml_element_t *parent)
{
  void *c;
  c_xml_element_t *celem;

  if(!parent)
    return(FALSE);

  if(!(parent->children))
    return(TRUE);

  /* loop over children and destroy them */

  for(C_linklist_move_head(parent->children);
      (c = C_linklist_restore(parent->children)) != NULL;
      C_linklist_move_next(parent->children))
  {
    celem = (c_xml_element_t *)c;

    if(! __C_xml_element_destroy_tree(celem, TRUE))
      return(FALSE);
  }

  C_linklist_destroy(parent->children);

  parent->children = NULL;

  return(TRUE);
}

/*
 */

c_bool_t C_xml_element_remove_children_named(c_xml_element_t *parent,
                                             const char *name)
{
  void *c;
  c_xml_element_t *celem;

  if(!parent)
    return(FALSE);

  if(!(parent->children))
    return(TRUE);

  /* loop over children and destroy the ones with matching names */

  for(C_linklist_move_head(parent->children);
      (c = C_linklist_restore(parent->children)) != NULL;)
  {
    celem = (c_xml_element_t *)c;

    if(! strcmp(celem->name, name))
    {
      C_linklist_delete(parent->children);

      if(! __C_xml_element_destroy_tree(celem, TRUE))
        return(FALSE);
    }
    else
      C_linklist_move_next(parent->children);
  }

  return(FALSE);
}

/*
 */

static void __C_xml_write_element(c_xml_element_t *elem, FILE *fp, int depth)
{
  int i;
  char **keys, **key, *v;
  void *c;
  c_xml_element_t *celem;
  c_bool_t endTag = FALSE;

  /* write out this tag */

  for(i = depth; i--; fputc(' ', fp), fputc(' ', fp));
  fputc('<', fp);
  fputs(elem->name, fp);

  if(elem->params)
  {
    keys = C_hashtable_keys(elem->params, NULL);

    for(key = keys; *key; key++)
    {
      v = (char *)C_hashtable_restore(elem->params, *key);
      fprintf(fp, " %s=\"%s\"", *key, v);
    }

    C_free_vec(keys);
  }

  /* we either have children or content, but not both */

  if(elem->content || elem->children)
  {
    fputc('>', fp);
    endTag = TRUE;
  }

  /* output content */

  if(elem->content)
    fputs(elem->content, fp);

  /* recurse on children */

  if(elem->children)
  {
    fputc('\n', fp);

    for(C_linklist_move_head(elem->children);
        (c = C_linklist_restore(elem->children)) != NULL;
        C_linklist_move_next(elem->children))
    {
      celem = (c_xml_element_t *)c;
      __C_xml_write_element(celem, fp, depth + 1);
    }
  }

  if(endTag)
  {
    if(elem->children)
      for(i = depth; i--; fputc(' ', fp), fputc(' ', fp));

    fputs("</", fp);
    fputs(elem->name, fp);
  }
  else
    fputc('/', fp);

  fputc('>', fp);
  fputc('\n', fp);
}

/*
 */

c_bool_t C_xml_document_write(c_xml_document_t *doc, FILE *fp)
{
  if(! doc || !fp)
    return(FALSE);

  fputs("<?xml version=\"1.0\" encoding=\"", fp);
  fputs(doc->encoding, fp);
  fputs("\"?>\n", fp);

  __C_xml_write_element(doc->root, fp, 0);

  return(TRUE);
}

/*
 */

static void __C_xml_handler_chardata(void *userData, const XML_Char *s,
                                     int len)
{
  struct c_xml_parser_context *ctx = (struct c_xml_parser_context *)userData;

  if(!(ctx->chardata))
    ctx->chardata = C_dstring_create(80);

  C_dstring_puts_len(ctx->chardata, s, len);
}

/*
 */

static void __C_xml_handler_elemstart(void *userData, const XML_Char *name,
                                      const XML_Char **attr)
{
  struct c_xml_parser_context *ctx = (struct c_xml_parser_context *)userData;
  c_xml_element_t *elem;
  int i;

  elem = C_xml_element_create(name);
  if(ctx->current)
    C_xml_element_add_child(ctx->current, elem);

  for(i = 0; attr[i]; i += 2)
    C_xml_element_set_param(elem, attr[i], attr[i + 1]);

  if(! ctx->root)
    ctx->root = elem;

  ctx->current = elem; /* push */
}

static void __C_xml_handler_elemend(void *userData, const XML_Char *name)
{
  struct c_xml_parser_context *ctx = (struct c_xml_parser_context *)userData;
  char *s, *s0, *s1;

  if(ctx->chardata)
  {
    s = C_dstring_destroy(ctx->chardata);
    ctx->chardata = NULL;

    /* now trim the whitespace */

    for(s0 = s; *s0; s0++)
    {
      if(! isspace((int)*s0))
        break;
    }

    for(s1 = s + strlen(s) - 1; s1 > s0; s1--)
    {
      if(! isspace((int)*s1))
      {
        *(++s1) = NUL;
        break;
      }
    }

    if(*s0)
      C_xml_element_set_content(ctx->current, s0);

    C_free(s);
  }

  ctx->current = ctx->current->parent; /* pop */
}

/*
 */

static void __C_xml_handler_comment(void *userData, const XML_Char *data)
{
  /*
    struct c_xml_parser_context *ctx = (struct c_xml_parser_context *)userData;
  */

  /* not implemented */
}

/*
 */

c_bool_t C_xml_document_read(c_xml_document_t *doc, FILE *fp)
{
  XML_Parser parser;
  struct c_xml_parser_context ctx;
  char buf[1024];

  if(! doc || !fp)
    return(FALSE);

  parser = XML_ParserCreate(doc->encoding);

  XML_SetElementHandler(parser, __C_xml_handler_elemstart,
                        __C_xml_handler_elemend);
  XML_SetCharacterDataHandler(parser, __C_xml_handler_chardata);
  XML_SetCommentHandler(parser, __C_xml_handler_comment);

  ctx.doc = doc;
  ctx.current = NULL;
  ctx.root = NULL;
  ctx.chardata = NULL;

  XML_SetUserData(parser, &ctx);

  for(;;)
  {
    int len;
    c_bool_t done;

    len = fread(buf, 1, sizeof(buf), fp);
    if(ferror(fp))
    {
      fprintf(stderr, "Read error.\n");
      return(FALSE);
    }

    done = feof(fp);

    if(! XML_Parse(parser, buf, len, done))
    {
      fprintf(stderr, "Parser error at line %d:\n%s\n",
              (int)XML_GetCurrentLineNumber(parser),
              XML_ErrorString(XML_GetErrorCode(parser)));
      return(FALSE);
    }

    if(done)
      break;
  }

  XML_ParserFree(parser);

  C_xml_document_set_root(doc, ctx.root);

  return(TRUE);
}

#endif /* HAVE_LIBEXPAT */

/* end of source file */
