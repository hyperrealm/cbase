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

#include <ctype.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/util.h"

/* Functions */

char C_hex_tonibble(int v)
{

  if(v < 0 || v > 15)
    return(NUL);

  return(v + ((v < 10) ? '0' : ('A' - 10)));
}

/*
 */

int C_hex_fromnibble(char c)
{

  if(!C_hex_isdigit(c))
    return(-1);

  c = toupper(c);

  return(c - ((c < 'A') ? '0' : ('A' - 10)));
}

/*
 */

c_bool_t C_hex_tobyte(char *s, int v)
{
  div_t d;

  if(v < 0 || v > 255 || !s)
    return(FALSE);

  d = div(v, 16);
  s[0] = C_hex_tonibble(d.quot);
  s[1] = C_hex_tonibble(d.rem);
  s[2] = NUL;

  return(TRUE);
}

/*
 */

int C_hex_frombyte(char * const s)
{
  int hi, lo;

  if(!s)
    return(-1);
  if((hi = C_hex_fromnibble(s[0])) < 0)
    return(-1);
  if((lo = C_hex_fromnibble(s[1])) < 0)
    return(-1);

  return(16 * hi + lo);
}

/*
 */

c_bool_t C_hex_encode(char * const data, size_t len, char *s)
{
  char *p, *q;
  int i;

  if(!data || !len || !s)
    return(FALSE);

  for(p = data, i = len, q = s; i--; ++p, q += 2)
    C_hex_tobyte(q, (int)*p);
  *q = NUL;

  return(TRUE);
}

/*
 */

c_bool_t C_hex_decode(char * const s, size_t len, char *data)
{
  char *p, *q;
  int i;

  if(!s || !len || (len % 2) || !data)
    return(FALSE);

  for(q = s, i = len, p = data; i--; q += 2, ++p)
    *p = (char)C_hex_frombyte(q);

  return(TRUE);
}

/* end of source file */

