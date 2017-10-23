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
#include <semaphore.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/ipc.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Functions */

c_sem_t *C_sem_create(const char *name, mode_t mode, uint_t value)
{
  c_sem_t *sem;
  sem_t *s;

  if((value < 1) || (value > C_SEM_MAX_VALUE))
    return(NULL);

  if((s = sem_open(name, (O_CREAT | O_RDWR), mode, value))
     == (sem_t *)SEM_FAILED)
    return(NULL);

  sem = C_new(c_sem_t);
  sem->sem = s;
  sem->name = C_string_dup(name);
  sem->initial_value = value;

  return(sem);
}

/*
 */

void C_sem_destroy(c_sem_t *sem)
{
  if(!sem)
    return;

  sem_close(sem->sem);
  sem_unlink(sem->name);

  C_free(sem->name);
  C_free(sem);
}

/*
 */

c_bool_t C_sem_wait(c_sem_t *sem)
{
  if(!sem || !(sem->sem))
    return(FALSE);

  return(sem_wait(sem->sem) == 0);
}

/*
 */

c_bool_t C_sem_trywait(c_sem_t *sem)
{
  if(!sem || !(sem->sem))
    return(FALSE);

  return(sem_trywait(sem->sem) == 0);
}

/*
 */

c_bool_t C_sem_post(c_sem_t *sem)
{
  if(!sem || !(sem->sem))
    return(FALSE);

  return(sem_post(sem->sem) == 0);
}

/*
 */

int C_sem_value(c_sem_t *sem)
{
  int val, r;

  if(!sem || !(sem->sem))
    return(0);

  r = sem_getvalue(sem->sem, &val);

  return((errno == 0) ? val : -1);
}

/* end of source file */
