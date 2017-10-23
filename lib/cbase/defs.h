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

#ifndef __cbase_defs_h
#define __cbase_defs_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

/* common definitions */

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE (1)
#define FALSE (0)

#ifdef NUL
#undef NUL
#endif

#ifdef CRLF
#undef CRLF
#endif

#define NUL '\0'
#define CRLF "\r\n"

  typedef char c_bool_t;

  typedef unsigned char c_byte_t;

#if !(defined(__SVR4) && defined(__sun))
  typedef unsigned int uint_t;
#endif

  typedef off_t c_pointer_t;

#define C_offsetof(T, FIELD)                                            \
  ((size_t)(((void *)(&(((T *)NULL)->FIELD))) - ((void *)NULL)))

#define C_lengthof(A)                           \
  ((size_t)(sizeof(A) / sizeof(A[0])))

/* convenience macros */

#define C_max(A, B)                             \
  (((A) > (B)) ? (A) : (B))
#define C_min(A, B)                             \
  (((A) < (B)) ? (A) : (B))
#define C_sgn(A)                                \
  (((A) < 0) ? -1 : ((A) ? 1 : 0))

#define C_bit_set(I, B)                         \
  ((I) |= (1L << (B)))
#define C_bit_clear(I, B)                       \
  ((I) &= ~(1L << (B)))
#define C_bit_isset(I, B)                       \
  (((I) & (1L << (B))) ? TRUE : FALSE)

/* terminal attributes */

#define C_TERMATTR_NORMAL     "\033[0m"
#define C_TERMATTR_BOLD       "\033[1m"
#define C_TERMATTR_UNDERLINE  "\033[2m"
#define C_TERMATTR_BLINK      "\033[3m"
#define C_TERMATTR_INVERSE    "\033[4m"

#define C_TERMATTR_FG_BLACK   "\033[30m"
#define C_TERMATTR_FG_RED     "\033[31m"
#define C_TERMATTR_FG_GREEN   "\033[32m"
#define C_TERMATTR_FG_YELLOW  "\033[33m"
#define C_TERMATTR_FG_BLUE    "\033[34m"
#define C_TERMATTR_FG_MAGENTA "\033[35m"
#define C_TERMATTR_FG_CYAN    "\033[36m"
#define C_TERMATTR_FG_WHITE   "\033[37m"
#define C_TERMATTR_FG_DEFAULT "\033[39m"

#define C_TERMATTR_BG_BLACK   "\033[40m"
#define C_TERMATTR_BG_RED     "\033[41m"
#define C_TERMATTR_BG_GREEN   "\033[42m"
#define C_TERMATTR_BG_YELLOW  "\033[43m"
#define C_TERMATTR_BG_BLUE    "\033[44m"
#define C_TERMATTR_BG_MAGENTA "\033[45m"
#define C_TERMATTR_BG_CYAN    "\033[46m"
#define C_TERMATTR_BG_WHITE   "\033[47m"
#define C_TERMATTR_BG_DEFAULT "\033[49m"

#define C_TERMATTR_CS_LINE    "\033(0"
#define C_TERMATTR_CS_ASCII   "\033(B"


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_defs_h */

/* end of library header */
