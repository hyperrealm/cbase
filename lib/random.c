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

#ifdef THREADED_LIBRARY
#include <pthread.h>
#endif /* THREADED_LIBRARY */

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"

/* File scope variables */

#ifdef THREADED_LIBRARY

static pthread_once_t __C_random_once = PTHREAD_ONCE_INIT;
static pthread_key_t __C_random_key;

#endif /* THREADED_LIBRARY */

/* File scope functions */

#ifdef THREADED_LIBRARY

static void __C_random_destructor(void *arg)
{
  C_free(arg);
}

/*
 */

static void __C_random_init_once(void)
{

  pthread_key_create(&__C_random_key, __C_random_destructor);
}

#endif /* THREADED_LIBRARY */

/* External functions */

void C_random_seed(void)
{
#ifdef THREADED_LIBRARY
  /* no-op */
#else

#ifdef HAVE_SRANDDEV
  sranddev();
#else
  srand((uint_t)time(NULL) + (uint_t)getpid());
#endif

#endif /* THREADED_LIBRARY */
}

/*
 */

uint_t C_random(uint_t range)
{
  uint_t r;

#ifdef THREADED_LIBRARY
  uint_t *seed;

  pthread_once(&__C_random_once, __C_random_init_once);

  seed = (uint_t *)pthread_getspecific(__C_random_key);
  if(! seed)
  {
    seed = C_new(uint_t);
    *seed = (time(NULL) + (long)pthread_self());
    pthread_setspecific(__C_random_key, (void *)seed);
  }

  r = (uint_t)rand_r(seed);
#else
  r = (uint_t)rand();
#endif /* THREADED_LIBRARY */

  return((int)((float)range * r / RAND_MAX));
}

/* end of source file */
