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

#ifndef __cbase_fortune_h
#define __cbase_fortune_h

#include <cbase/defs.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  typedef struct c_fortune_db_t
  {
    FILE *data;
    FILE *index;
    int count;
    off_t filelen;
  } c_fortune_db_t;

  extern c_fortune_db_t *C_fortune_opendb(const char *basename);
  extern c_bool_t C_fortune_closedb(c_fortune_db_t *db);

  extern c_bool_t C_fortune_indexdb(const char *basename);

  extern const char *C_fortune_select(c_fortune_db_t *db);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_fortune_h */

/* end of library header */
