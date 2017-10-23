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

#ifndef __cbase_ipc_h
#define __cbase_ipc_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <semaphore.h>

#include <cbase/defs.h>

/* ----------------------------------------------------------------------------
 * file descriptor passing
 * ----------------------------------------------------------------------------
 */

  extern c_bool_t C_fd_send(int sd, int fd);
  extern c_bool_t C_fd_recv(int sd, int *fd);

/* ----------------------------------------------------------------------------
 * terminal control
 * ----------------------------------------------------------------------------
 */

  extern c_bool_t C_tty_raw(int fd);
  extern c_bool_t C_tty_unraw(int fd);
  extern c_bool_t C_tty_store(int fd);
  extern c_bool_t C_tty_restore(int fd);
  extern c_bool_t C_tty_sane(int fd);

  extern c_bool_t C_tty_getsize(uint_t *columns, uint_t *rows);

/* ----------------------------------------------------------------------------
 * pseudoterminals
 * ----------------------------------------------------------------------------
 */

  typedef struct c_pty_t
  {
    int master_fd;
    int slave_fd;
    char pts_name[11];
  } c_pty_t;

  extern c_pty_t *C_pty_create(void);
  extern c_bool_t C_pty_destroy(c_pty_t *pty);

#define C_pty_master_fd(P)                      \
  ((P)->master_fd)

#define C_pty_slave_fd(P)                       \
  ((P)->slave_fd)

#define C_pty_slave_name(P)                     \
  ((const char *)(P)->pts_name)

/* ----------------------------------------------------------------------------
 * signals
 * ----------------------------------------------------------------------------
 */

#define C_SIGNAL_MIN SIGHUP

#ifdef SIGXFSZ
#define C_SIGNAL_MAX SIGXFSZ
#else
#define C_SIGNAL_MAX 31
#endif

  typedef void (*c_sighandler_t)(int /* sig */);

  extern const char *C_signal_name(int sig);

/* ----------------------------------------------------------------------------
 * shared memory
 * ----------------------------------------------------------------------------
 */

  typedef struct c_shmem_t
  {
    int fd;
    void *base;
    size_t size;
    char *name;
  } c_shmem_t;

  extern c_shmem_t *C_shmem_create(const char *name, size_t size, mode_t mode);
  extern void C_shmem_destroy(c_shmem_t *mem);
  extern c_bool_t C_shmem_resize(c_shmem_t *mem, size_t size);

#define /* void * */ C_shmem_base(M)            \
  (M)->base
#define /* size_t */ C_shmem_size(M)            \
  (M)->size
#define C_shmem_name(M)                         \
  (const char *)((M)->name)

/* ----------------------------------------------------------------------------
 * counting semaphores
 * ----------------------------------------------------------------------------
 */

  typedef struct c_sem_t
  {
    sem_t *sem;
    char *name;
    int initial_value;
  } c_sem_t;

  extern c_sem_t *C_sem_create(const char *name, mode_t mode, uint_t value);
  extern void C_sem_destroy(c_sem_t *sem);

  extern c_bool_t C_sem_wait(c_sem_t *sem);
  extern c_bool_t C_sem_trywait(c_sem_t *sem);
  extern c_bool_t C_sem_post(c_sem_t *sem);
  extern int C_sem_value(c_sem_t *sem);

#define C_sem_name(S)                           \
  (const char *)((S)->name)

#define C_sem_initial_value(S)                  \
  ((S)->initial_value)

#if defined SEM_VALUE_MAX
#define C_SEM_MAX_VALUE SEM_VALUE_MAX
#elif defined _SEM_VALUE_MAX
#define C_SEM_MAX_VALUE _SEM_VALUE_MAX
#else
#define C_SEM_MAX_VALUE 32767 /* a safe default value */
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_ipc_h */

/* end of library header */
