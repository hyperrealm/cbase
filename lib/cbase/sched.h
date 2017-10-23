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

#ifndef __cbase_sched_h
#define __cbase_sched_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <signal.h>
#include <time.h>

#include <cbase/defs.h>
#include <cbase/data.h>
#include <cbase/util.h>

/* ----------------------------------------------------------------------------
 * real-time scheduler
 * ----------------------------------------------------------------------------
 */

#define C_SCHED_MASK_COUNT 5

  typedef struct c_sched_t
  {
#if defined(__SVR4) && defined(__sun)
    timer_t timer;
    struct sigevent evp;
#endif
    void *hook;
    void (*handler)(void);
    c_linklist_t *events;
  } c_sched_t;

  typedef struct c_schedevt_t
  {
    c_bitstring_t *mask[C_SCHED_MASK_COUNT];
    c_bool_t once;
    void (*handler)(struct c_schedevt_t *, time_t);
    void (*destructor)(struct c_schedevt_t *);
    void *hook;
    c_bool_t active;
    uint_t id;
  } c_schedevt_t;

  extern c_bool_t C_sched_init(void);
  extern c_bool_t C_sched_shutdown(void);
  extern void C_sched_poll(void);

  extern c_schedevt_t *C_sched_event_create(const char *timespec,
                                            c_bool_t once, void *hook,
                                            void (*handler)(c_schedevt_t *,
                                                            time_t),
                                            void (*destructor)(c_schedevt_t *),
                                            uint_t id);

  extern c_bool_t C_sched_event_destroy(c_schedevt_t *e);

  extern c_bool_t C_sched_event_activate(c_schedevt_t *e);
  extern c_bool_t C_sched_event_deactivate(c_schedevt_t *e);
  extern c_schedevt_t *C_sched_event_find(uint_t id);

#define C_sched_event_data(D)                   \
  (D)->hook

#define C_sched_event_id(D)                     \
  (D)->id

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_sched_h */

/* end of library header */

