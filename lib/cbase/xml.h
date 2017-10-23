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

#ifndef __cbase_xml_h
#define __cbase_xml_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include <cbase/defs.h>
#include <cbase/data.h>

/* ----------------------------------------------------------------------------
 * XML functions
 * ----------------------------------------------------------------------------
 */

  typedef struct c_xml_element_t
  {
    char *name;
    char *content;
    c_hashtable_t *params;
    c_linklist_t *children;
    void *hook;
    struct c_xml_element_t *parent;
  } c_xml_element_t;

  typedef struct c_xml_document_t
  {
    char *encoding;
    c_xml_element_t *root;
  } c_xml_document_t;

  extern c_xml_document_t *C_xml_document_create(const char *encoding);
  extern c_bool_t C_xml_document_destroy(c_xml_document_t *doc);

  extern c_xml_element_t *C_xml_element_create(const char *name);
  extern c_bool_t C_xml_element_destroy(c_xml_element_t *elem);
  extern c_bool_t C_xml_element_destroy_recursive(c_xml_element_t *elem);

  extern c_bool_t C_xml_document_set_root(c_xml_document_t *doc,
                                          c_xml_element_t *root);
  extern c_xml_element_t *C_xml_document_get_root(c_xml_document_t *doc);

  extern c_bool_t C_xml_element_set_content(c_xml_element_t *elem,
                                            const char *content);
  extern const char *C_xml_element_get_content(c_xml_element_t *elem);

  extern c_bool_t C_xml_element_set_param(c_xml_element_t *elem,
                                          const char *param,
                                          const char *value);
  extern const char *C_xml_element_get_param(c_xml_element_t *elem,
                                             const char *param);

  extern c_bool_t C_xml_element_delete_param(c_xml_element_t *elem,
                                             const char *param);
  extern c_bool_t C_xml_element_delete_params(c_xml_element_t *elem);

  extern c_xml_element_t **C_xml_element_get_children(c_xml_element_t *parent);
  extern c_xml_element_t **C_xml_element_get_children_named(
    c_xml_element_t *parent, const char *name);
  extern c_xml_element_t *C_xml_element_get_first_child(
    c_xml_element_t *parent);
  extern c_xml_element_t *C_xml_element_get_first_child_named(
    c_xml_element_t *parent, const char *name);

  extern c_bool_t C_xml_element_add_child(c_xml_element_t *parent,
                                          c_xml_element_t *elem);

  extern c_bool_t C_xml_element_remove_child(c_xml_element_t *parent,
                                             c_xml_element_t *elem);
  extern c_bool_t C_xml_element_remove_children(c_xml_element_t *parent);
  extern c_bool_t C_xml_element_remove_children_named(c_xml_element_t *parent,
                                                      const char *name);

  extern c_bool_t C_xml_document_write(c_xml_document_t *doc, FILE *fp);
  extern c_bool_t C_xml_document_read(c_xml_document_t *doc, FILE *fp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_xml_h */

/* end of library header */
