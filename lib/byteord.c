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

#include <sys/types.h>
#include <netinet/in.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"

/* Structs and unions */

union un32
{
  float f;
  uint32_t l;
};

union un64
{
  double d;
  uint64_t ll;
};

/* File scope variables */

#define __C_byteord_big_endian ((c_bool_t)(htonl(1) == 1))

/* Functions */

uint16_t C_byteord_htons(uint16_t val)
{
  return(htons(val));
}

/*
 */

uint16_t C_byteord_ntohs(uint16_t val)
{
  return(ntohs(val));
}

/*
 */

uint32_t C_byteord_htonl(uint32_t val)
{
  return(htonl(val));
}

/*
 */

uint32_t C_byteord_ntohl(uint32_t val)
{
  return(ntohl(val));
}

/*
 */

uint64_t C_byteord_htonll(uint64_t val)
{
  if(__C_byteord_big_endian)
    return(val);
  else
    return((((uint64_t)htonl((uint32_t)val)) << 32)
           + htonl((uint32_t)(val >> 32)));
}

/*
 */

uint64_t C_byteord_ntohll(uint64_t val)
{
  if(__C_byteord_big_endian)
    return(val);
  else
    return((((uint64_t)ntohl((uint32_t)val)) << 32)
           + ntohl((uint32_t)(val >> 32)));
}

/*
 */

float C_byteord_htonf(float val)
{
  union un32 u;

  u.f = val;
  u.l = C_byteord_htonl(u.l);

  return(u.f);
}

/*
 */

float C_byteord_ntohf(float val)
{
  union un32 u;

  u.f = val;
  u.l = C_byteord_ntohl(u.l);

  return(u.f);
}

/*
 */

double C_byteord_htond(double val)
{
  union un64 u;

  u.d = val;
  u.ll = C_byteord_htonll(u.ll);

  return(u.d);
}

/*
 */

double C_byteord_ntohd(double val)
{
  union un64 u;

  u.d = val;
  u.ll = C_byteord_ntohll(u.ll);

  return(u.d);
}

/* end of source file */
