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

#include <assert.h>

/* Local headers */

#include "cbase/except.h"
#include "cbase/system.h"

/* File scope variables */

static C_except_context_t *__C_top_context = NULL;

/* External functions */

C_except_context_t *C_except_context_push(void)
{
  C_except_context_t *context = C_new(C_except_context_t);
  context->prev = __C_top_context;
  __C_top_context = context;
  return __C_top_context;
}

/*
 */

C_except_context_t *C_except_context_top(int exception,
                                         const char *file, int line)
{
  C_assert(__C_top_context != NULL); // assert if throw without try

  if(__C_top_context != NULL)
  {
    __C_top_context->exception = exception;
    __C_top_context->file = file;
    __C_top_context->line = line;
  }

  return __C_top_context;
}

/*
 */

int C_except_context_pop(void)
{
  C_assert(__C_top_context != NULL); // assert if catch without try

  C_except_context_t *top = __C_top_context;
  int exception = 0;

  if(top)
  {
    exception = top->exception;
    __C_top_context = top->prev;
    C_free(top);
  }

  return(exception);
}

/* end of source file */
