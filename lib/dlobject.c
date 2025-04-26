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

#include <dlfcn.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Functions */

c_dlobject_t *C_dlobject_create(const char *path)
{
  c_dlobject_t *obj;

  if(! path)
    return(FALSE);

  if(! *path)
    return(FALSE);

  obj = C_new(c_dlobject_t);
  obj->path = C_string_dup(path);
  obj->loaded = FALSE;

  return(obj);
}

/*
 */

c_bool_t C_dlobject_load(c_dlobject_t *obj, c_bool_t lazy)
{
  if(! obj)
    return(FALSE);

  obj->error = NULL;

  if(C_dlobject_isloaded(obj))
    return(FALSE);

  if(! (obj->handle = dlopen(obj->path, (lazy ? RTLD_LAZY : RTLD_NOW))))
  {
    obj->error = (char *)dlerror();
    return(FALSE);
  }

  obj->loaded = TRUE;

  return(TRUE);
}

/*
 */

c_bool_t C_dlobject_unload(c_dlobject_t *obj)
{
  if(! obj)
    return(FALSE);

  obj->error = NULL;

  if(! C_dlobject_isloaded(obj))
    return(FALSE);

  if(dlclose(obj->handle) != 0)
  {
    obj->error = (char *)dlerror();
    return(FALSE);
  }

  obj->loaded = FALSE;

  return(TRUE);
}

/*
 */

c_bool_t C_dlobject_destroy(c_dlobject_t *obj)
{
  if(! obj)
    return(FALSE);

  if(C_dlobject_isloaded(obj))
    return(FALSE);

  C_free(obj->path);
  C_free(obj);

  return(TRUE);
}

/*
 */

void *C_dlobject_lookup(c_dlobject_t *obj, const char *symbol)
{
  void *p;

  if(! obj || ! symbol)
    return(FALSE);

  obj->error = NULL;

  if(! C_dlobject_isloaded(obj))
    return(FALSE);

  if(! *symbol)
    return(FALSE);

  if(!(p = dlsym(obj->handle, symbol)))
  {
    obj->error = (char *)dlerror();
    return(NULL);
  }

  return(p);
}

/* end of source file */
