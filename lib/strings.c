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
#include <string.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Macros */

#define C_STRING_BLOCKSZ 40

/* Functions */

char *C_string_clean(char *s, char fillc)
{
  char *p;

  if(!s)
    return(NULL);

  for(p = s; *p; ++p)
    if(!isprint((int)*p) && !isspace((int)*p))
      *p = fillc;

  return(s);
}

/*
 */

char *C_string_tolower(char *s)
{
  char *p;

  if(!s)
    return(NULL);

  for(p = s; *p; ++p)
  {
    if(isupper((int)(*p)))
      *p = (char)tolower((int)(*p));
  }

  return(s);
}

/*
 */

char *C_string_toupper(char *s)
{
  char *p;

  if(!s)
    return(NULL);

  for(p = s; *p; ++p)
    if(islower((int)(*p)))
      *p = (char)toupper((int)(*p));

  return(s);
}

/*
 */

char **C_string_split(char *s, const char *sep, size_t *len)
{
  c_vector_t *vec;
  char *p;
  char *q = NULL;

  if(!s || !sep)
    return(NULL);
  if(!*s || !*sep)
    return(NULL);

  vec = C_vector_start(C_STRING_BLOCKSZ);
  if(!((p = strtok_r(s, sep, &q))))
  {
    C_vector_abort(vec);
    if(len)
      *len = 0;
    return(NULL);
  }
  else C_vector_store(vec, C_string_dup(p));

  do
  {
    if((p = strtok_r(NULL, sep, &q)))
      C_vector_store(vec, C_string_dup(p));
  }
  while(p);

  return(C_vector_end(vec, len));
}

/*
 */

char *C_string_dup(const char *s)
{
  char *r;
  size_t len;

  if(!s)
    return(NULL);

  len = strlen(s);
  r = C_newstr(len);

  return((char *)memcpy((void *)r, (void *)s, len));
}

/*
 */

char **C_string_sortvec(char **v, size_t len)
{

  qsort((void *)v, len, sizeof(char *), C_string_compare);
  return(v);
}

/*
 */

char *C_string_trim(char *s)
{
  char *p, *q;
  size_t l, ll;

  if(!s)
    return(NULL);

  l = ll = strlen(s);

  for(p = s; *p && isspace((int)*p); ++p, --l);

  if(! l)
    *s = NUL;
  else
  {
    for(q = s + (--ll); (l > 0) && isspace((int)*q); --q, --l);

    memmove((void *)s, (void *)p, (size_t)l);

    *(s + l) = NUL;
  }

  return(s);
}

/*
 */

c_bool_t C_string_copy(char *buf, size_t bufsz, const char *s)
{
  size_t l;

  if(!buf || !bufsz || !s)
    return(FALSE);

  l = strlen(s);

  --bufsz;
  strncpy(buf, s, bufsz);
  *(buf + bufsz) = NUL;

  return(l < bufsz);
}

/*
 */

c_bool_t C_string_va_copy(char *buf, size_t bufsz, ... /* , NULL */)
{
  va_list vp;
  size_t left = bufsz - 1;
  char *cp = buf, *p;
  c_bool_t all = FALSE;

  if(!buf)
    return(FALSE);

  va_start(vp, bufsz);
  while((p = va_arg(vp, char *)))
  {
    while(*p && (left > 0))
    {
      *(cp++) = *(p++);

      if(--left == 0)
        goto STR_VA_COPY;
    }
  }
  all = TRUE;

STR_VA_COPY:
  *cp = NUL;
  return(all);
}

/*
 */

c_bool_t C_string_concat(char *buf, size_t bufsz, const char *s)
{
  size_t l, bl, left, c;

  if(!buf || !s)
    return(FALSE);

  l = strlen(s);
  bl = strlen(buf);
  left = --bufsz - bl;
  c = C_min(left, l);

  strncpy(buf + bl, s, c);
  *(buf + bl + c) = NUL;

  return(l <= left);
}

/*
 */

c_bool_t C_string_va_concat(char *buf, size_t bufsz, ...)
{
  va_list vp;
  size_t len, left;
  char *cp, *p;
  c_bool_t all = FALSE;

  if(!buf)
    return(FALSE);

  len = strlen(buf);
  cp = buf + len;
  left = (bufsz - 1 - len);

  va_start(vp, bufsz);
  while((p = va_arg(vp, char *)))
  {
    while(*p && (left > 0))
    {
      *(cp++) = *(p++);

      if(--left == 0)
        goto STR_VA_CONCAT;
    }
  }

  all = TRUE;

STR_VA_CONCAT:
  *cp = NUL;
  return(all);
}

/*
 */

char *C_string_chop(char *s, const char *termin)
{
  char *p;

  if(!s || !termin)
    return(NULL);

  if((p = strpbrk(s, termin)))
    *p = NUL;

  return(s);
}

/*
 */

char *C_string_rchop(char *s, const char *termin)
{
  char *p;
  int len;

  if(!s || !termin)
    return(NULL);

  len = strlen(s);
  for(p = s + len - 1; --len >= 0; p--)
  {
    if(strchr(termin, *p) != NULL)
    {
      *p = NUL;
      break;
    }
  }

  return(s);
}

/*
 */

const char *C_string_tokenize(const char *s, const char *delim,
                              const char **ctx, size_t *len)
{
  const char *p, *r = NULL;

  if(!delim || !ctx || !len)
    return(NULL);

  p = (s ? s : *ctx);
  *len = 0;

  for(; *p; ++p)
  {
    if(! strchr(delim, *p))
    {
      r = p;
      break;
    }
  }

  if(r)
  {
    for(; *p; ++p, ++(*len))
    {
      if(strchr(delim, *p))
        break;
    }
  }

  *ctx = p;

  return(r);
}

/*
 */

char **C_string_valist2vec(const char *first, va_list vp, size_t *len)
{
  c_vector_t *vec;
  char *p;

  if(vp == NULL)
    return(NULL);

  vec = C_vector_start(C_STRING_BLOCKSZ);
  if(first)
    C_vector_store(vec, first);

  while((p = va_arg(vp, char *)))
    C_vector_store(vec, p);

  return(C_vector_end(vec, len));
}

/*
 */

char *C_string_va_make(const char *first, ... /* , NULL */)
{
  size_t len = 0;
  char *p, *q, *s;
  va_list vp;

  if(! first)
    return(NULL);

  len += strlen(first);

  va_start(vp, first);

  while((p = va_arg(vp, char *)))
    len += strlen(p);

  va_end(vp);

  q = s = C_newstr(len);

  len = strlen(first);
  memcpy((void *)q, (void *)first, len);
  q += len;

  va_start(vp, first);

  while((p = va_arg(vp, char *)))
  {
    len = strlen(p);
    memcpy((void *)q, (void *)p, len);
    q += len;
  }

  va_end(vp);

  *q = NUL;

  return(s);
}

/*
 */

char **C_string_va_makevec(size_t *len, ... /* , NULL */)
{
  va_list vp;
  char **v;

  va_start(vp, len);
  v = C_string_valist2vec(NULL, vp, len);
  va_end(vp);

  return(v);
}

/*
 */

char *C_string_dup1(const char *s, char c)
{
  char *r, *p;
  size_t len;

  if(!s || !c)
    return(NULL);

  len = strlen(s);
  p = r = C_newstr(len + 1);
  memcpy((void *)r, (void *)s, len);
  r += len;
  *(r++) = c;
  *r = NUL;

  return(p);
}

/*
 */

c_bool_t C_string_endswith(const char *s, const char *suffix)
{
  int len, slen;

  if(!s || !suffix)
    return(FALSE);

  slen = strlen(suffix);
  len = strlen(s);

  if(len >= slen)
    if(!strcmp(s + (len - slen), suffix))
      return(TRUE);

  return(FALSE);
}

/*
 */

c_bool_t C_string_startswith(const char *s, const char *prefix)
{
  int slen;

  if(!s || !prefix)
    return(FALSE);

  slen = strlen(prefix);

  return(!strncmp(s, prefix, slen));
}

/*
 */

int C_string_compare_len(const char *s1, size_t len1,
                         const char *s2, size_t len2)
{
  const char *p, *q;

  for(p = s1, q = s2; *p && len1 && *q && len2; ++p, ++q, --len1, --len2)
  {
    if(*p < *q)
      return(-1);
    else if (*p > *q)
      return(1);
  }

  if(!len1 || !*p)
  {
    if(len2 || *q)
      return(-1);
    else
      return(0);
  }
  else
    return(1);
}

/*
 */

int C_string_compare(const void *s1, const void *s2)
{
  char **p = (char **)s1, **q = (char **)s2;

  return(strcmp(*p, *q));
}

/*
 */

uint_t C_string_hash(const char *s, uint_t modulo)
{
  uint_t hashval = 0, i;
  char *p;

  if(!s || !modulo)
    return(0);
  if(!*s)
    return(0);

  for(p = (char *)s; *p; ++p)
  {
    hashval = (hashval << 4) + *p;
    if((i = (hashval & 0xf0000000)))
    {
      hashval ^= (i >> 24);
      hashval ^= i;
    }
  }

  return(hashval % modulo);
}

/*
 */

c_bool_t C_string_isnumeric(const char *s)
{
  char *p;

  if(! *s)
    return(FALSE);

  for(p = (char *)s; *p; ++p)
    if(! isdigit((int)*p))
      return(FALSE);

  return(TRUE);
}

/* end of source file */
