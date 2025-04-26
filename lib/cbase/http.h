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

#ifndef __cbase_http_h
#define __cbase_http_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cbase/defs.h>
#include <cbase/system.h>
#include <cbase/data.h>
#include <cbase/net.h>

/* ----------------------------------------------------------------------------
 * HTTP server functions
 * ----------------------------------------------------------------------------
 */

  typedef void (*c_httpsrv_handler_t)(c_socket_t * /* socket */,
                                      const char * /* uri */,
                                      c_hashtable_t * /* params */);

  typedef struct c_httpsrv_t
  {
    c_socket_t socket;
    int max_workers;
    int num_workers;
    int timeout;
    c_hashtable_t *handlers;
    c_httpsrv_handler_t dfl_handler;
  } c_httpsrv_t;

  typedef struct c_http_param_t
  {
    char *value;
    c_linklist_t *values;
  } c_http_param_t;

  extern c_httpsrv_t *C_httpsrv_create(in_port_t port, int max_workers,
                                       int timeout);
  extern void C_httpsrv_destroy(c_httpsrv_t *srv);

  extern void C_httpsrv_set_default_handler(c_httpsrv_t *srv,
                                            c_httpsrv_handler_t handler);

  extern void C_httpsrv_add_handler(c_httpsrv_t *srv, const char *uri,
                                    c_httpsrv_handler_t handler);
  extern void C_httpsrv_remove_handler(c_httpsrv_t *srv, const char *uri);

  extern void C_httpsrv_send_headers(c_socket_t *s, const char *mime_type,
                                     long length);

  extern c_bool_t C_httpsrv_send_status(c_socket_t *s, int status,
                                        c_bool_t errmsg);

  extern c_bool_t C_httpsrv_accept(c_httpsrv_t *srv);

  extern c_http_param_t *C_http_param_get(c_hashtable_t *params,
                                          const char *key);

#define C_http_param_isarray(P)                 \
  ((P)->values != NULL)

#define C_http_param_value(P)                   \
  ((P)->value)

#define C_http_param_values(P)                  \
  ((P)->values)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_http_h */

/* end of library header */
