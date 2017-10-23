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

#include <limits.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Functions */

c_timer_t *C_timer_create(void)
{
  c_timer_t *timer = C_new(c_timer_t);

  time(&(timer->created)); /* set creation time on timer */

  timer->running = FALSE;

  return(timer);
}

/*
 */

void C_timer_destroy(c_timer_t *timer)
{
  C_free(timer);
}

/*
 */

void C_timer_start(c_timer_t *timer)
{
  if(! timer->running)
  {
    timer->usr_time = timer->sys_time = timer->real_time = 0;
    times(&(timer->t1));
    gettimeofday(&(timer->tv1), NULL);

    timer->running = TRUE;
  }
}

/*
 */

void C_timer_resume(c_timer_t *timer)
{
  if(! timer->running)
  {
    times(&(timer->t1));
    gettimeofday(&(timer->tv1), NULL);

    timer->running = TRUE;
  }
}

/*
 */

void C_timer_stop(c_timer_t *timer)
{
  long clk_tck = sysconf(_SC_CLK_TCK);

  if(timer->running)
  {
    times(&(timer->t2));
    gettimeofday(&(timer->tv2), NULL);

    timer->usr = (timer->t2.tms_utime - timer->t1.tms_utime);
    timer->sys = (timer->t2.tms_stime - timer->t1.tms_stime);
    timer->real = ((timer->tv2.tv_sec * 1000 + timer->tv2.tv_usec)
                   - (timer->tv1.tv_sec * 1000 + timer->tv1.tv_usec));

    timer->usr_time += ((float)(timer->usr) / (float)clk_tck);
    timer->sys_time += ((float)(timer->sys) / (float)clk_tck);
    timer->real_time += timer->real;

    timer->running = FALSE;
  }
}

/*
 */

void C_timer_reset(c_timer_t *timer)
{
  timer->running = FALSE;
  timer->usr_time = timer->sys_time = timer->real_time = 0;
}

/* end of source file */
