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

#ifndef __cbase_version_h
#define __cbase_version_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  extern const char *C_library_version(void);
  extern const char *C_library_info(void);
  extern const char **C_library_options(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __cbase_version_h */

/* end of library header */
