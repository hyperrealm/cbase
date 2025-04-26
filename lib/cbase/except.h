/* ----------------------------------------------------------------------------
   cbase - A C Foundation Library
   Copyright (C) 1994-2025 Mark A Lindner

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

#ifndef __cbase_except_h
#define __cbase_except_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <setjmp.h>
  
/* ----------------------------------------------------------------------------
 * exception handling structures
 * ----------------------------------------------------------------------------
 */

  struct C_except_context_ds {
    int exception;
    jmp_buf buf;
    const char *file;
    int line;
    struct C_except_context_ds *prev;
  };

  typedef struct C_except_context_ds C_except_context_t;
 
/* ----------------------------------------------------------------------------
 * exception handling functions
 * ----------------------------------------------------------------------------
 */

  extern C_except_context_t *C_except_context_push(void);
  extern C_except_context_t *C_except_context_top(int exception,
                                                  const char *file, int line);
  extern int C_except_context_pop(void);
  
  
/* ----------------------------------------------------------------------------
 * exception handling macros
 * ----------------------------------------------------------------------------
 */

#define C_try                                                   \
  if(setjmp(C_except_context_push()->buf) == 0)                 \
    for(int __C_except_once = 1;                                \
        __C_except_once;                                        \
        (void)C_except_context_pop(), --__C_except_once)

#define C_catch(E)                                              \
  else for(int __C_except_once = 1, E = C_except_context_pop(); \
           __C_except_once--; )
  
#define C_throw(E)                                              \
  longjmp(C_except_context_top(E, __FILE__, __LINE__)->buf, 1)

#define C_throws(...)
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_except_h */

/* end of library header */
