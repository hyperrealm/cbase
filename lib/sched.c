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

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/sched.h"

/* Macros */

#define ONE_MINUTE 60 /* seconds in a minute */

#define C_SCHED_INTERVAL ONE_MINUTE

#define C_SCHED_MINUTE_MASK 0
#define C_SCHED_HOUR_MASK 1
#define C_SCHED_DATE_MASK 2
#define C_SCHED_MONTH_MASK 3
#define C_SCHED_WEEKDAY_MASK 4

/* File scope variables */

#if defined(__SVR4) && defined(__sun)

static struct sigaction __C_sched_old_sigaction;

#endif

static void (*__C_sched_event_handler)(c_schedevt_t *);
static c_sched_t *__C_sched_scheduler;

static const int __C_sched_range_max[] = { 59, 23, 31, 12, 6 };
static const int __C_sched_range_min[] = { 0, 0, 1, 1, 0 };

#ifdef THREADED_LIBRARY
static pthread_t __C_sched_wait_thread;
#endif /* THREADED_LIBRARY */

/* File scope functions */

static c_bool_t __C_sched_event_deactivate(c_sched_t *s, c_schedevt_t *e)
{
  c_link_t *lp;

  if(!s || !e)
    return(FALSE);

  if(! C_linklist_search_r(s->events, (void *)e, &lp))
    return(FALSE); /* not found */

  if(! C_linklist_delete_r(s->events, &lp))
    return(FALSE); /* delete failed (?) */

  if(e->destructor)
    e->destructor(e);

  return(TRUE);
}

/*
 */

static c_bool_t __C_sched_event_activate(c_sched_t *s, c_schedevt_t *e)
{
  c_link_t *lp = NULL;

  if(!s || !e)
    return(FALSE);

  if(C_linklist_search_r(s->events, (void *)e, &lp))
    return(FALSE); /* found it already in there */

  C_linklist_move_tail_r(s->events, &lp);

  if(! C_linklist_store_r(s->events, (void *)e, &lp))
    return(FALSE); /* store failed (?) */

  return(TRUE);
}

/*
 */

static void __C_sched_default_handler(c_schedevt_t *evt)
{
  time_t t = time(NULL);
  struct tm now;
  uint_t tval[C_SCHED_MASK_COUNT];
  c_link_t *lp;

  localtime_r(&t, &now);

  tval[C_SCHED_MINUTE_MASK] = (uint_t)now.tm_min;
  tval[C_SCHED_HOUR_MASK] = (uint_t)now.tm_hour;
  tval[C_SCHED_DATE_MASK] = (uint_t)now.tm_mday;
  tval[C_SCHED_MONTH_MASK] = (uint_t)now.tm_mon;
  tval[C_SCHED_WEEKDAY_MASK] = (uint_t)now.tm_wday;

#ifdef DEBUG
  printf("mn = %d, hr = %d, dt = %d, mo = %d, wd = %d\n", tval[0],
         tval[1], tval[2], tval[3], tval[4]);
#endif

  for(C_linklist_move_head_r(__C_sched_scheduler->events, &lp);
      ! C_linklist_isend_r(__C_sched_scheduler->events, &lp);
      C_linklist_move_next_r(__C_sched_scheduler->events, &lp))
  {
    int i;
    c_bool_t trigger = TRUE;
    c_schedevt_t *evt
      = (c_schedevt_t *)C_linklist_restore_r(__C_sched_scheduler->events, &lp);

    for(i = 0; i < C_SCHED_MASK_COUNT; i++)
    {
      if(! C_bitstring_isset(evt->mask[i], tval[i]))
      {
        trigger = FALSE;
        break;
      }
    }

    if(trigger)
    {
#ifdef DEBUG
      printf(ctime(&t));
#endif

      if(evt->handler)
        evt->handler(evt, t);

      if(evt->once)
      {
        __C_sched_event_deactivate(__C_sched_scheduler, evt);
      }
    }
  }
}

/*
 */

void C_sched_poll(void)
{
#ifndef THREADED_LIBRARY
  __C_sched_default_handler(NULL);
#endif /* ! THREADED_LIBRARY */
}

/*
 */

#ifdef THREADED_LIBRARY

#if (defined(__SVR4) && defined(__sun))

static void *__C_sched_threaded_handler(void *arg)
{
  int old, sig;
  sigset_t sigmask;

  sigemptyset(&sigmask);
  sigaddset(&sigmask, SIGRTMIN);

  pthread_sigmask(SIG_UNBLOCK, &sigmask, NULL);

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  for(;;)
  {
    sigwait(&sigmask, &sig);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old);

    if(sig == SIGRTMIN)
      __C_sched_default_handler(NULL);

    pthread_setcancelstate(old, NULL);
  }
}

#else /* ! defined(__SVR4) && defined(__sun) */

/* Real-time interval timers appear to be broken on Linux, and they
 * don't exist at all on OS X, so we use nanosleep() instead.
 */

static void *__C_sched_threaded_handler(void *arg)
{
  int old;
  struct timeval tv;
  struct timezone tz;
  struct timespec pause, rem;

  pause.tv_sec = 0;
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  for(;;)
  {
    /* get current time, and sleep for remainder of current second */

    gettimeofday(&tv, &tz);
    uint_t nsec = tv.tv_usec * 1000; /* nanoseconds */
    uint_t sec = (tv.tv_sec % 60);

    if((sec > 0) || (nsec >= 1000)) /* if less than 1us left in minute;
                                     * don't bother sleeping */
    {
      pause.tv_nsec = (1E9 - nsec);
      pause.tv_sec = (60 - sec);
#ifdef DEBUG
      printf("sleeping for %d seconds, %d nanoseconds\n", pause.tv_sec,
             pause.tv_nsec);
#endif

      for(;;)
      {
        if(nanosleep(&pause, &rem) == 0)
          break;

        if(errno != EINTR)
          break;

        pause = rem;
      }
    }

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old);

    __C_sched_default_handler(NULL);

    pthread_setcancelstate(old, NULL);
  }

  return(NULL);
}

#endif /* ! defined(__SVR4) && defined(__sun)  */

#endif /* THREADED_LIBRARY */

/*
 */

#if (defined(__SVR4) && defined(__sun))

static void __C_sched_internal_handler(int sig, siginfo_t *extra, void *cruft)
{
#ifndef THREADED_LIBRARY
  c_schedevt_t *evt = NULL;
#endif

  /*
    int overrun = 0;

    evt = (schedevt_t *)(extra->si_value.sival_ptr);
    overrun = timer_getoverrun(sched->timer);

    printf("si_signo = %d\n", extra->si_signo);
    printf("si_errno = %d\n",  extra->si_errno);
    printf("si_code  = %d\n", extra->si_code);
    printf("si_value.sival_int = %d\n", extra->si_value.sival_int);

    printf("data value is %d\n", extra->si_value.sival_int);
  */

#ifndef THREADED_LIBRARY

  if(__C_sched_event_handler)
    __C_sched_event_handler(evt);

#endif /* ! THREADED_LIBRARY */
}

#endif /* (defined(__SVR4) && defined(__sun)) */


/*
 */

static c_bool_t __C_sched_parsetime(char *timespec, c_bitstring_t *mask,
                                    uint_t min)
{
  char *p, *q, *pp, *qq, buf[256];
  int start, end, x = 0;

  if(!strcmp(timespec, "*"))
    C_bitstring_set_all(mask);

  else
  {
    C_bitstring_clear_all(mask);

    q = NULL;

    for(p = strtok_r(timespec, ",", &q);
        p;
        p = strtok_r(NULL, ",", &q), ++x)
    {
      start = end = -1;

      strncpy(buf, p, sizeof(buf) - 1);
      buf[sizeof(buf) - 1] = NUL;
      qq = NULL;

      if(!((pp = strtok_r(buf, "-", &qq))))
        continue;

      if(! C_string_isnumeric(pp))
        return(FALSE); /* parse error! */

      start = atoi(pp);

      if((pp = strtok_r(NULL,  "-", &qq)))
      {
        if(! C_string_isnumeric(pp))
          return(FALSE); /* parse error! */

        end = atoi(pp);
      }

      if((start < min) || (start >= C_bitstring_size(mask)))
        return(FALSE); /* out of range! */

      if(end < 0)
        C_bitstring_set(mask, (uint_t)start);

      else if(end < start)
        return(FALSE); /* invalid range! */

      else if((end < min) || (end >= C_bitstring_size(mask)))
        return(FALSE); /* out of range! */

      else
        C_bitstring_set_range(mask, (uint_t)start, (uint_t)end);
    }

    if(x == 0)
      return(FALSE); /* empty input */
  }

  return(TRUE);
}

/*
 */

static c_sched_t *__C_sched_create(void *hook)
{
  time_t t = time(NULL);
#if defined(__SVR4) && defined(__sun)
  struct itimerspec interval;
#endif
  c_sched_t *sched;
  struct tm now;

  localtime_r(&t, &now);

  sched = C_new(c_sched_t);

  sched->hook = hook;
  sched->events = C_linklist_create();

#if defined(__SVR4) && defined(__sun)

  sched->evp.sigev_notify = SIGEV_SIGNAL;
  sched->evp.sigev_signo = SIGRTMIN;
  sched->evp.sigev_value.sival_int = 0; /* buggy in Solaris 7 */

  timer_create(CLOCK_REALTIME, &(sched->evp), &(sched->timer));

  interval.it_value.tv_sec = ONE_MINUTE - now.tm_sec;
  interval.it_value.tv_nsec = 0;

  interval.it_interval.tv_sec = C_SCHED_INTERVAL;
  interval.it_interval.tv_nsec = 0;

  timer_settime(sched->timer, 0, &interval, NULL);

#endif

  return(sched);
}

/*
 */

static c_bool_t __C_sched_destroy(c_sched_t *s)
{
  if(!s)
    return(FALSE);

#if defined(__SVR4) && defined(__sun)

  timer_delete(s->timer);

#endif

  /* clean up the linked list of events */

  C_linklist_destroy(s->events);

  C_free(s);

  return(TRUE);
}

/*
 */

static void __C_sched_set_handler(void (*func)(c_schedevt_t *))
{
  __C_sched_event_handler = func;
}

/* Functions */

c_bool_t C_sched_init(void)
{
#if defined(__SVR4) && defined(__sun)
#ifdef THREADED_LIBRARY
  struct sigaction sa;
#endif /* THREADED_LIBRARY */
#endif /* defined(__SVR4) && defined(__sun) */

  if(__C_sched_scheduler)
    return(FALSE); /* scheduler is already initialized! */

#if defined(__SVR4) && defined(__sun)
#ifdef THREADED_LIBRARY

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = __C_sched_internal_handler;

  if(sigaction(SIGRTMIN, &sa, &__C_sched_old_sigaction) < 0)
    return(FALSE);
#endif /* THREADED_LIBRARY */
#endif /* defined(__SVR4) && defined(__sun) */

  __C_sched_set_handler(__C_sched_default_handler);

  __C_sched_scheduler = __C_sched_create(NULL);

#ifdef THREADED_LIBRARY
  pthread_create(&__C_sched_wait_thread, NULL, __C_sched_threaded_handler,
                 NULL);
#endif /* THREADED_LIBRARY */

  return(TRUE);
}

/*
 */

c_schedevt_t *C_sched_event_find(uint_t id)
{
  c_link_t *lp = NULL;

  for(C_linklist_move_head_r(__C_sched_scheduler->events, &lp);
      ! C_linklist_isend_r(__C_sched_scheduler->events, &lp);
      C_linklist_move_next_r(__C_sched_scheduler->events, &lp))
  {
    c_schedevt_t *evt
      = (c_schedevt_t *)C_linklist_restore_r(__C_sched_scheduler->events, &lp);

    if(evt->id == id)
      return(evt);
  }

  return(NULL);
}

/*
 */

c_bool_t C_sched_shutdown(void)
{
  c_link_t *lp;

  if(! __C_sched_scheduler)
    return(FALSE);

#if defined(__SVR4) && defined(__sun)

  sigaction(SIGRTMIN, &__C_sched_old_sigaction, NULL);

#endif

#ifdef THREADED_LIBRARY
  pthread_cancel(__C_sched_wait_thread);
#endif /* THREADED_LIBRARY */

  for(C_linklist_move_head_r(__C_sched_scheduler->events, &lp);
      ! C_linklist_isend_r(__C_sched_scheduler->events, &lp);
      C_linklist_move_next_r(__C_sched_scheduler->events, &lp))
  {
    c_schedevt_t *evt
      = (c_schedevt_t *)C_linklist_restore_r(__C_sched_scheduler->events, &lp);

    __C_sched_event_deactivate(__C_sched_scheduler, evt);
  }

  __C_sched_destroy(__C_sched_scheduler);

  __C_sched_scheduler = NULL;

  return(TRUE);
}

/*
 */

c_schedevt_t *C_sched_event_create(const char *timespec, c_bool_t once,
                                   void *hook,
                                   void (*handler)(c_schedevt_t *, time_t),
                                   void (*destructor)(c_schedevt_t *),
                                   uint_t id)
{
  c_schedevt_t *evt;
  char *p, *q = NULL, buf[256];
  int i;

  evt = C_new(c_schedevt_t);
  evt->once = once;
  evt->hook = hook;
  evt->handler = handler;
  evt->destructor = destructor;
  evt->id = id;

  strncpy(buf, timespec, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = NUL;

  for(i = 0, p = strtok_r(buf, " :\t", &q);
      (i < C_SCHED_MASK_COUNT) && p;
      ++i, p = strtok_r(NULL, " :\t", &q))
  {
    evt->mask[i] = C_bitstring_create(__C_sched_range_max[i] + 1);
    if(!(__C_sched_parsetime(p, evt->mask[i], __C_sched_range_min[i])))
      break;
  }

  if(i != C_SCHED_MASK_COUNT)
  {
    C_sched_event_destroy(evt);
    return(NULL);
  }

  return(evt);
}

/*
 */

c_bool_t C_sched_event_destroy(c_schedevt_t *evt)
{
  int i;

  if(!evt)
    return(FALSE);

  for(i = 0; i < C_SCHED_MASK_COUNT; ++i)
    C_free(evt->mask[i]);

  C_free(evt);

  return(TRUE);
}

/*
 */

c_bool_t C_sched_event_activate(c_schedevt_t *e)
{
  if(! __C_sched_scheduler)
    return(FALSE);

  return(__C_sched_event_activate(__C_sched_scheduler, e));
}

/*
 */

c_bool_t C_sched_event_deactivate(c_schedevt_t *e)
{
  if(! __C_sched_scheduler)
    return(FALSE);

  return(__C_sched_event_deactivate(__C_sched_scheduler, e));
}

/* end of source file */

