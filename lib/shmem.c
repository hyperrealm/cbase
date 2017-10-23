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

#ifdef linux
#define _XOPEN_SOURCE 500
#endif

/* System headers */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#ifdef linux
#include <pthread.h> /* Linux references a pthread struct in semaphore.h  */
#endif

/* Local headers */

#include "cbase/defs.h"
#include "cbase/ipc.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Functions */

static size_t __C_shmem_round_size(size_t size)
{
#if defined _SC_PAGESIZE
  long pagesize = sysconf(_SC_PAGESIZE);
#elif defined HAVE_GETPAGESIZE
  long pagesize = getpagesize();
#else
#error "No means to determine system page size."
#endif

  return((size_t)(size + pagesize - (size % pagesize)));
}

/*
 */

c_shmem_t *C_shmem_create(const char *name, size_t size, mode_t mode)
{
  c_shmem_t *m;
  int fd;
  c_bool_t init = TRUE;
  size_t rsize;

  if((fd = shm_open(name, (O_CREAT | O_RDWR | O_EXCL), mode)) < 0)
  {
    if(errno == EEXIST)
    {
      init = FALSE;

      if((fd = shm_open(name, (O_CREAT | O_RDWR), mode)) < 0)
        return(NULL);
    }
    else
      return(NULL);
  }

  rsize = __C_shmem_round_size(size);

  if(init)
  {
    if(ftruncate(fd, (off_t)rsize) != 0)
      return(NULL);
  }

  m = C_new(c_shmem_t);
  m->fd = fd;
  m->size = rsize;
  m->name = C_string_dup(name);
  m->base = mmap(NULL, m->size, (PROT_READ | PROT_WRITE), MAP_SHARED, m->fd,
                 (off_t)0);


  if(init)
    memset(m->base, 0, m->size);

  return(m);
}

/*
 */

void C_shmem_destroy(c_shmem_t *mem)
{
  if(!mem)
    return;

  munmap(mem->base, mem->size);
  shm_unlink(mem->name);
  C_free(mem->name);
  C_free(mem);
}

/*
 */

c_bool_t C_shmem_resize(c_shmem_t *mem, size_t size)
{
  size_t oldsize, newsize;

  if(!mem || (size <= 0))
    return(FALSE);

  oldsize = mem->size;
  newsize = __C_shmem_round_size(size);

  if(ftruncate(mem->fd, newsize))
    return(FALSE);

  mem->size = newsize;
  if(newsize > oldsize)
    memset(mem->base + oldsize, 0, (newsize - oldsize));

  return(TRUE);
}

/* end of source file */
