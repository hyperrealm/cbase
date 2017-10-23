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

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/ipc.h"

/* File scope functions */

static off_t __C_memfile_round_size(off_t size)
{
#if defined _SC_PAGESIZE
  long pagesize = sysconf(_SC_PAGESIZE);
#elif defined HAVE_GETPAGESIZE
  long pagesize = getpagesize();
#else
#error "No means to determine system page size."
#endif

  return((off_t)(size + pagesize - (size % pagesize)));
}

/* Functions */

c_memfile_t *C_memfile_open(const char *file, c_bool_t readonly)
{
  c_memfile_t *f;
  struct stat stbuf;

  if(!file)
    return(NULL);

  if(stat(file, &stbuf))
    return(NULL);

  f = C_new(c_memfile_t);
  if((f->fd = open(file, O_RDWR)) < 0)
    return(C_free(f));

  f->length = stbuf.st_size;
  f->base = mmap(NULL, f->length,
                 (readonly ? PROT_READ : (PROT_READ | PROT_WRITE)), MAP_SHARED,
                 f->fd, 0);

  if(f->base == MAP_FAILED)
  {
    close(f->fd);
    return(C_free(f));
  }

  return(f);
}

/*
 */

c_bool_t C_memfile_close(c_memfile_t *mf)
{
  if(!mf)
    return(FALSE);

  if(msync(mf->base, mf->length, MS_SYNC))
    return(FALSE);

  if(munmap(mf->base, mf->length))
    return(FALSE);

  if(close(mf->fd))
    return(FALSE);

  C_free(mf);

  return(TRUE);
}

/*
 */

c_bool_t C_memfile_resize(c_memfile_t *mf, off_t length)
{
  off_t newsize;

  if(!mf || (length < 0))
    return(FALSE);

  newsize = __C_memfile_round_size(length);

  if(ftruncate(mf->fd, newsize))
    return(FALSE);

  if(newsize > mf->length)
    memset(mf->base + mf->length, 0, (newsize - mf->length));

  mf->length = newsize;

  return(TRUE);
}

/*
 */

c_bool_t C_memfile_sync(c_memfile_t *mf, c_bool_t async)
{
  if(!mf)
    return(FALSE);

  return(! msync(mf->base, mf->length, (async ? MS_ASYNC : MS_SYNC)));
}

/* end of source file */
