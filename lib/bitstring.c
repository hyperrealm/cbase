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

/* Local headers */

#include "cbase/defs.h"
#include "cbase/util.h"
#include "cbase/system.h"

/* Macros */

#define _C_bitstring_byte_offset(bit)           \
  ((bit) >> 3)

#define _C_bitstring_byte_mask(bit)             \
  (c_byte_t)(1 << ((bit) & 0x7))

#define _C_bitstring_getsize(nbits)             \
  (size_t)((((nbits) - 1) >> 3) + 1)

/* Functions */

c_bitstring_t *C_bitstring_create(uint_t nbits)
{
  c_bitstring_t *bs;
  int len = _C_bitstring_getsize(nbits);

  if(len < 1)
    return(NULL);

  bs = C_new(c_bitstring_t);
  bs->length = len;
  bs->nbits = nbits;
  bs->bits = C_newa(len, c_byte_t);

  return(bs);
}

/*
 */

c_bool_t C_bitstring_destroy(c_bitstring_t *bs)
{
  if(! bs)
    return(FALSE);

  C_free(bs->bits);
  C_free(bs);

  return(TRUE);
}

/*
 */

c_bool_t C_bitstring_compare(c_bitstring_t *bs1, c_bitstring_t *bs2)
{
  int r = 0, i;
  c_byte_t *p, *q;

  if(! bs1 || ! bs2)
    return(FALSE);

  if(bs1->nbits != bs2->nbits)
    return(FALSE);

  for(i = bs1->length, p = bs1->bits, q = bs2->bits; i--; ++i, ++p, ++q)
    r |= ((*p & *q) != 0);

  return(r != 0);
}

/*
 */

c_bool_t C_bitstring_clear(c_bitstring_t *bs, uint_t bit)
{
  c_byte_t *b;

  if(! bs)
    return(FALSE);

  b = &(bs->bits[_C_bitstring_byte_offset(bit)]);
  *b &= ~(_C_bitstring_byte_mask(bit));

  return(TRUE);
}

/*
 */

c_bool_t C_bitstring_set(c_bitstring_t *bs, uint_t bit)
{
  c_byte_t *b;

  if(! bs)
    return(FALSE);

  b = &(bs->bits[_C_bitstring_byte_offset(bit)]);
  *b |= _C_bitstring_byte_mask(bit);

  return(TRUE);
}

/*
 */

c_bool_t C_bitstring_clear_range(c_bitstring_t *bs, uint_t sbit, uint_t ebit)
{
  uint_t sbyte, ebyte;

  if(!bs || (ebit < sbit))
    return(FALSE);

  sbyte = _C_bitstring_byte_offset(sbit);
  ebyte = _C_bitstring_byte_offset(ebit);

  if(sbyte == ebyte)
  {
    bs->bits[sbyte] &= ((0xFF >> (8 - (sbit & 0x07)))
                        | (0xFF << ((ebit & 0x07) + 1)));
  }
  else
  {
    bs->bits[sbyte] &= 0xFF >> (8 - (sbit & 0x07));

    while(++sbyte < ebyte)
      bs->bits[sbyte] = 0;

    bs->bits[ebyte] &= 0xFF << ((ebit & 0x07) + 1);
  }

  return(TRUE);
}

/*
 */

c_bool_t C_bitstring_set_range(c_bitstring_t *bs, uint_t sbit, uint_t ebit)
{
  uint_t sbyte, ebyte;

  if(!bs || (ebit < sbit))
    return(FALSE);

  sbyte = _C_bitstring_byte_offset(sbit);
  ebyte = _C_bitstring_byte_offset(ebit);

  if(sbyte == ebyte)
  {
    bs->bits[sbyte] |= ((0xFF << (sbit & 0x07))
                        & (0xFF >> (7 - (ebit & 0x07))));
  }
  else
  {
    bs->bits[sbyte] |= (0xFF << (sbit & 0x07));

    while(++sbyte < ebyte)
      bs->bits[sbyte] = 0xFF;

    bs->bits[ebyte] |= 0xFF >> (7 - (ebit & 0x07));
  }

  return(TRUE);
}

/*
 */

c_bool_t C_bitstring_isset(c_bitstring_t *bs, uint_t bit)
{
  uint_t b;

  if(! bs)
    return(FALSE);

  b = bs->bits[_C_bitstring_byte_offset(bit)];

  return((b & (_C_bitstring_byte_mask(bit))) != 0);
}

/* end of source file */
