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

/* Feature test switches */

/* System headers */

#include <sys/wait.h>
#include <sys/times.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Macros */

#define C_EXEC_NULLDEV "/dev/null"

/* Functions */

int C_exec_run_cwd(char **argv, int fdin, int fdout, c_bool_t waitf,
                   const char *cwd)
{
  pid_t pid;
  int fdz = -1, r = 0, x;

  pid = fork();
  switch(pid)
  {
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);

    case 0: /* child process */
      if(fdin < 0 || fdout < 0)
        fdz = open(C_EXEC_NULLDEV, O_RDWR);
      dup2(((fdin < 0) ? fdz : fdin), STDIN_FILENO);
      dup2(((fdout < 0) ? fdz : fdout), STDOUT_FILENO);
      dup2(((fdout < 0) ? fdz : fdout), STDERR_FILENO);

      if(cwd)
        x = chdir(cwd);

      execvp(*argv, argv);
      if(fdz >= 0)
        close(fdz);
      perror("execvp");
      exit(EXIT_FAILURE);

    default: /* parent process */
      if(waitf)
        r = C_exec_wait(pid);
      return(r);
  }
}

/*
 */

int C_exec_pipefrom_cwd(char **argv, int *fd, const char *cwd)
{
  int pfd[2];

  if(pipe(pfd))
  {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  *fd = pfd[0];
  fcntl(pfd[0], F_SETFD, FD_CLOEXEC);

  return(C_exec_run_cwd(argv, -1, pfd[1], FALSE, cwd));
}

/*
 */

int C_exec_pipeto_cwd(char **argv, int *fd, const char *cwd)
{
  int pfd[2];

  if(pipe(pfd))
  {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  *fd = pfd[1];
  fcntl(pfd[1], F_SETFD, FD_CLOEXEC);

  return(C_exec_run_cwd(argv, pfd[0], -1, FALSE, cwd));
}

/*
 */

int C_exec_wait(pid_t pid)
{
  int st = 0;
  pid_t r;

  if(pid)
  {
  WAITPID:
    r = waitpid(pid, &st, 0);
    if((r < 0) && (errno == EINTR))
      goto WAITPID;
  }
  else
    wait(&st);

  return(WEXITSTATUS(st));
}

/*
 */

int C_exec_va_run_cwd(int fdin, int fdout, c_bool_t waitf, const char *cwd,
                      ... /* , NULL */)
{
  char **v;
  va_list vp;
  int r;

  va_start(vp, cwd);
  v = C_string_valist2vec(NULL, vp, NULL);
  va_end(vp);

  r = (v ? C_exec_run_cwd(v, fdin, fdout, waitf, cwd) : -1);
  if(v)
    C_free(v);

  return(r);
}

/*
 */

int C_exec_va_call(const char *arg, ... /* , NULL */)
{
  char **v;
  va_list vp;
  int r;

  va_start(vp, arg);
  v = C_string_valist2vec(arg, vp, NULL);
  va_end(vp);

  r = (v ? C_exec_run(v, STDIN_FILENO, STDOUT_FILENO, TRUE) : -1);
  if(v)
    C_free(v);

  return(r);
}

/* end of source file */
