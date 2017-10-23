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
#ifdef THREADED_LIBRARY
#include <pthread.h>
#endif /* THREADED_LIBRARY */

/* Local headers */

#include "cbase/http.h"

/* File scope variables */

static const int __C_httpsrv_status_codes[] = { 200, 400, 403, 404, 408,
                                                500, 501, 503, 0 };

static const char *__C_httpsrv_status_msgs[] =
{
  /* 200 */ "OK",
  /* 400 */ "Bad Request",
  /* 403 */ "Forbidden",
  /* 404 */ "Not Found",
  /* 408 */ "Request Timed Out",
  /* 500 */ "Internal Server Error",
  /* 501 */ "Not Implemented",
  /* 503 */ "Server Busy",
  NULL
};

static const char *__C_httpsrv_default_mime = "binary/octet-stream";

/* Functions */

static void __C_httpsrv_param_destructor(void *arg)
{
  c_http_param_t *param = (c_http_param_t *)arg;

  C_free(param->value); /* ok if NULL */

  if(param->values)
    C_linklist_destroy(param->values);

  C_free(param);
}

/*
 */

static void __C_httpsrv_destructor(void *arg)
{
  C_free(arg);
}

/*
 * Since a URL encoded string is longer than the original string, we
 * can URL decode the string in-place, instead of using a separate
 * output buffer.
 */

static c_bool_t __C_httpsrv_urldecode(char *text)
{
  char *r, *w;
  int c, left = strlen(text);

  for(r = text, w = text; *r; ++r, --left)
  {
    if(*r == '+')
      *(w++) = ' ';
    else if(*r == '%')
    {
      if(left < 2)
        return(FALSE);

      c = C_hex_frombyte(++r);
      ++r;
      *(w++) = (char)c;
      left -= 2;
    }
    else
      *(w++) = *r;
  }

  *w = NUL;
  return(TRUE);
}

/*
 */

static c_bool_t __C_httpsrv_parse_query(char *query, c_hashtable_t *params)
{
  char *r, *key = query, *value = NULL;
  c_bool_t inkey = TRUE;
  c_http_param_t *param;

  for(r = query; *r; ++r)
  {
    if(inkey && (*r == '='))
    {
      inkey = FALSE;
      *r = NUL;
      if(! __C_httpsrv_urldecode(key))
        return(FALSE);
      value = r + 1;
    }
    else if(!inkey && ((*r == '&') || (*(r + 1) == NUL)))
    {
      inkey = TRUE;
      if(*r == '&')
        *r = NUL;
      if(! __C_httpsrv_urldecode(value))
        return(FALSE);

      param = (c_http_param_t *)C_hashtable_restore(params, key);
      if(! param)
      {
        param = C_new(c_http_param_t);
        C_hashtable_store(params, key, param);
      }

      if(! param->value && ! param->values)
        param->value = C_string_dup(value);
      else
      {
        if(! param->values)
        {
          param->values = C_linklist_create();
          C_linklist_set_destructor(param->values, __C_httpsrv_destructor);

          if(param->value) /* if existing single param, move into list */
          {
            C_linklist_store(param->values, param->value);
            param->value = NULL;
          }
        }

        C_linklist_store(param->values, C_string_dup(value));
      }

      key = r + 1;
    }
  }

  return(TRUE);
}

/*
 */

c_httpsrv_t *C_httpsrv_create(in_port_t port, int max_workers, int timeout)
{
  c_httpsrv_t *srv = C_new(c_httpsrv_t);

  srv->max_workers = max_workers;
  srv->timeout = timeout;

  C_socket_create_s(&(srv->socket), C_NET_TCP);

  if(! C_socket_listen(&(srv->socket), port))
    return(C_free(srv));

  srv->handlers = C_hashtable_create(10);

  return(srv);
}

/*
 */

void C_httpsrv_destroy(c_httpsrv_t *srv)
{
  if(! srv)
    return;

  C_socket_shutdown(&(srv->socket), C_NET_SHUTALL);
  C_socket_destroy_s(&(srv->socket));

  C_hashtable_destroy(srv->handlers);

  C_free(srv);
}

/*
 */

void C_httpsrv_add_handler(c_httpsrv_t *srv, const char *uri,
                           c_httpsrv_handler_t handler)
{
  C_hashtable_store(srv->handlers, uri, handler);
}

/*
 */

void C_httpsrv_remove_handler(c_httpsrv_t *srv, const char *uri)
{
  C_hashtable_delete(srv->handlers, uri);
}

/*
 */

void C_httpsrv_set_default_handler(c_httpsrv_t *srv,
                                   c_httpsrv_handler_t handler)
{
  srv->dfl_handler = handler;
}

/*
 */

c_bool_t C_httpsrv_send_status(c_socket_t *s, int status, c_bool_t errmsg)
{
  int n = C_lengthof(__C_httpsrv_status_msgs);
  int i;
  char buf[128];
  const char *p = NULL;

  for(i = 0; i < n; ++i)
  {
    if(status == __C_httpsrv_status_codes[i])
    {
      p = __C_httpsrv_status_msgs[i];
      break;
    }
  }

  if(! p)
    return(FALSE);

  snprintf(buf, sizeof(buf), "HTTP/1.0 %d %s", status, p);
  C_socket_sendline(s, buf);

  if((status != 200) && errmsg)
  {
    C_socket_sendline(s, "");

    snprintf(buf, sizeof(buf), "<html><h1>%d - %s</h1></html>", status, p);
    C_socket_sendline(s, buf);
  }

  return(TRUE);
}

/*
 */

static void *__C_httpsrv_worker(void *arg)
{
  c_httpsrv_handler_t handler = NULL;
  c_hashtable_t *params = NULL;
  c_socket_t *s = (c_socket_t *)arg;
  c_httpsrv_t *srv = (c_httpsrv_t *)C_socket_get_userdata(s);
  char uri[256], buf[128], **vec, verb[8], proto[16], *p;
  size_t len;
  c_bool_t first = TRUE;

  /* read request headers */

  while(C_socket_recvline(s, buf, sizeof(buf)) > 0)
  {
    if(*buf == NUL)
      break;

    if(first)
    {
      vec = C_string_split(buf, " ", &len);
      if(len != 3)
      {
        C_httpsrv_send_status(s, 400, TRUE);
        goto CLEANUP;
      }

      first = FALSE;

      strncpy(verb, vec[0], sizeof(verb));
      strncpy(uri, vec[1], sizeof(uri));
      strncpy(proto, vec[2], sizeof(proto));

      C_free_vec(vec);
    }
  }

  /* TO-DO: we currently do not keep track of worker count */

  if(srv->num_workers == srv->max_workers)
  {
    C_httpsrv_send_status(s, 503, TRUE);
    goto CLEANUP;
  }

  /* only GET is supported at this time */

  if(strcmp(verb, "GET"))
  {
    C_httpsrv_send_status(s, 501, TRUE);
    goto CLEANUP;
  }

  p = strchr(uri, '?');
  if(p)
    *(p++) = NUL;

  if(! __C_httpsrv_urldecode(uri))
  {
    C_httpsrv_send_status(s, 400, TRUE);
    goto CLEANUP;
  }

  handler = (c_httpsrv_handler_t)C_hashtable_restore(srv->handlers, uri);

  if(! handler)
    handler = srv->dfl_handler;

  if(! handler)
  {
    C_httpsrv_send_status(s, 404, TRUE);
    goto CLEANUP;
  }

  /* parse query string */

  if(p)
  {
    params = C_hashtable_create(10);
    C_hashtable_set_destructor(params, __C_httpsrv_param_destructor);

    if(! __C_httpsrv_parse_query(p, params))
    {
      C_httpsrv_send_status(s, 400, TRUE);
      goto CLEANUP;
    }
  }

  /* call handler */

  handler(s, uri, params);

  /* cleanup */

CLEANUP:

  if(params)
    C_hashtable_destroy(params);

  C_socket_shutdown(s, C_NET_SHUTALL);
  C_socket_destroy(s);

  return(NULL);
}

/*
 */

c_bool_t C_httpsrv_accept(c_httpsrv_t *srv)
{
  c_socket_t *s;
#ifdef THREADED_LIBRARY
  pthread_t thread;
  pthread_attr_t attr;
#endif /* THREADED_LIBRARY */

  s = C_socket_accept(&(srv->socket));
  C_socket_block(s);
  C_socket_set_userdata(s, srv);
  C_socket_set_timeout(s, srv->timeout);

#ifdef THREADED_LIBRARY
  /* fork a thread to do the work */

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &attr, __C_httpsrv_worker, (void *)s);
#else /* ! THREADED_LIBRARY */
  __C_httpsrv_worker(s);
#endif /* THREADED_LIBRARY */

  return(TRUE);
}

/*
 */

void C_httpsrv_send_headers(c_socket_t *s, const char *mime_type, long length)
{
  char buf[128];

  C_socket_sendline(s, "Connection: close");
  C_socket_sendline(s, "Serverk: CFL httpsrv");

  snprintf(buf, sizeof(buf), "Content-Type: %s", (mime_type ? mime_type
                                                  : __C_httpsrv_default_mime));
  C_socket_sendline(s, buf);

  if(length >= 0)
  {
    snprintf(buf, sizeof(buf), "Content-Length: %ld", length);
    C_socket_sendline(s, buf);
  }

  C_socket_sendline(s, "");
}

/*
 */

c_http_param_t *C_http_param_get(c_hashtable_t *params, const char *key)
{

  if(! params)
    return(NULL);

  return((c_http_param_t *)C_hashtable_restore(params, key));
}

/* end of source file */
