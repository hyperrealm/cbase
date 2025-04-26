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

#ifdef linux
#define _XOPEN_SOURCE 500
#endif

/* System headers */

#include <time.h>

/* Local headers */

#include "cbase/defs.h"

/* Functions */

c_bool_t C_time_format(time_t t, char *buf, size_t bufsz, const char *format)
{
  time_t t1;
  struct tm t2;

  t1 = (t ? t : time(NULL));
  (void)localtime_r(&t1, &t2);

  return((strftime(buf, bufsz, format, &t2) > 0) ? TRUE : FALSE);
}

/*
 */

time_t C_time_parse(const char *buf, const char *format)
{
  struct tm t;

  if(! strptime(buf, format, &t))
    return((time_t)0);

  return(mktime(&t));
}

/* end of source file */
