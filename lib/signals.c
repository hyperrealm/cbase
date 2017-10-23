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

/* Local headers */

#include "cbase/defs.h"
#include "cbase/ipc.h"
#include "cbase/system.h"

/* System headers */

#include <signal.h>

/* File scope variables */

static const char *__C_sig_names[] =
{
  "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGEMT",
  "SIGFPE", "SIGKILL", "SIGBUS", "SIGSEGV", "SIGSYS", "SIGPIPE", "SIGALRM",
  "SIGTERM", "SIGUSR1", "SIGUSR2", "SIGCHLD", "SIGPWR", "SIGWINCH", "SIGURG",
  "SIGPOLL", "SIGIO", "SIGSTOP", "SIGTSTP", "SIGCONT", "SIGTTIN", "SIGTTOU",
  "SIGVTALRM", "SIGPROF", "SIGXCPU", "SIGXFSZ"
};

/* Functions */

const char *C_signal_name(int sig)
{
  if(sig < C_SIGNAL_MIN || sig > C_SIGNAL_MAX)
    return(NULL);

  return(__C_sig_names[--sig]);
}

/* end of source file */
