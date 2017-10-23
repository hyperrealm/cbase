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

#ifndef __cbase_util_h
#define __cbase_util_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#include <cbase/defs.h>

/* ----------------------------------------------------------------------------
 * bit strings
 * ----------------------------------------------------------------------------
 */

  typedef struct c_bitstring_t
  {
    c_byte_t *bits;
    size_t length;
    uint_t nbits;
  } c_bitstring_t;

  extern c_bitstring_t *C_bitstring_create(uint_t nbits);
  extern c_bool_t C_bitstring_destroy(c_bitstring_t *bs);

  extern c_bool_t C_bitstring_compare(c_bitstring_t *bs1, c_bitstring_t *bs2);
  extern c_bool_t C_bitstring_clear(c_bitstring_t *bs, uint_t bit);
  extern c_bool_t C_bitstring_set(c_bitstring_t *bs, uint_t bit);
  extern c_bool_t C_bitstring_clear_range(c_bitstring_t *bs, uint_t sbit,
                                          uint_t ebit);
  extern c_bool_t C_bitstring_set_range(c_bitstring_t *bs, uint_t sbit,
                                        uint_t ebit);
  extern c_bool_t C_bitstring_isset(c_bitstring_t *bs, uint_t bit);

#define C_bitstring_isclear(BS, B)              \
  (! C_bitstring_isset((BS), (B)))

#define C_bitstring_size(BS)                    \
  ((BS)->nbits)

#define C_bitstring_set_all(BS)                         \
  C_bitstring_set_range((BS), 0, ((BS)->nbits - 1))

#define C_bitstring_clear_all(BS)                       \
  C_bitstring_clear_range((BS), 0, ((BS)->nbits - 1))

/* ----------------------------------------------------------------------------
 * buffers
 * ----------------------------------------------------------------------------
 */

  typedef struct c_buffer_t
  {
    char *buf;
    size_t bufsz;
    size_t datalen;
    void *hook;
  } c_buffer_t;

  extern c_buffer_t *C_buffer_create(size_t bufsz);
  extern c_buffer_t *C_buffer_resize(c_buffer_t *buf, size_t newsz);
  extern void C_buffer_clear(c_buffer_t *buf);
  extern void C_buffer_destroy(c_buffer_t *buf);

#define C_buffer_data(B) ((B)->buf)
#define C_buffer_hook(B) ((B)->hook)
#define C_buffer_datalen(B) ((B)->datalen)
#define C_buffer_size(B) ((B)->bufsz)

/* ----------------------------------------------------------------------------
 * hex utilities
 * ----------------------------------------------------------------------------
 */

#define C_hex_isdigit(A)                        \
  ((((A) >= '0') && ((A) <= '9'))               \
   || (((A) >= 'A') && ((A) <= 'F'))            \
   || (((A) >= 'a') && ((A) <= 'f')))

  extern char C_hex_tonibble(int v);
  extern int C_hex_fromnibble(char c);

  extern c_bool_t C_hex_tobyte(char *s, int v);
  extern int C_hex_frombyte(char * const s);

  extern c_bool_t C_hex_encode(char * const data, size_t len, char *s);
  extern c_bool_t C_hex_decode(char * const s, size_t len, char *data);

/* ----------------------------------------------------------------------------
 * string and parsing functions
 * ----------------------------------------------------------------------------
 */

  extern char *C_string_clean(char *s, char fillc);
  extern char *C_string_tolower(char *s);
  extern char *C_string_toupper(char *s);
  extern c_bool_t C_string_endswith(const char *s, const char *suffix);
  extern c_bool_t C_string_startswith(const char *s, const char *prefix);
  extern c_bool_t C_string_isnumeric(const char *s);
  extern char *C_string_trim(char *s);
  extern char **C_string_split(char *s, const char *sep, size_t *len);
  extern char *C_string_dup(const char *s);
  extern char *C_string_dup1(const char *s, char c);
  extern char *C_string_chop(char *s, const char *termin);
  extern char *C_string_rchop(char *s, const char *termin);
  extern const char *C_string_tokenize(const char *s, const char *delim,
                                       const char **ctx, size_t *len);
  extern char **C_string_sortvec(char **v, size_t len);
  extern c_bool_t C_string_copy(char *buf, size_t bufsz, const char *s);
  extern c_bool_t C_string_va_copy(char *buf, size_t bufsz, ...);
  extern c_bool_t C_string_concat(char *buf, size_t bufsz, const char *s);
  extern c_bool_t C_string_va_concat(char *buf, size_t bufsz, ...);
  extern char *C_string_va_make(const char *first, ...);
  extern char **C_string_va_makevec(size_t *len, ...);
  extern char **C_string_valist2vec(const char *first, va_list vp, size_t *slen);
  extern uint_t C_string_hash(const char *s, uint_t modulo);
  extern int C_string_compare_len(const char *s1, size_t len1,
                                  const char *s2, size_t len2);
  extern int C_string_compare(const void *s1, const void *s2);

/* ----------------------------------------------------------------------------
 * string buffer functions
 * ----------------------------------------------------------------------------
 */

  typedef struct c_strbuffer_t
  {
    char *buf;
    size_t bufsz;
    char *pos;
    size_t left;
  } c_strbuffer_t;

  extern c_strbuffer_t *C_strbuffer_create(size_t bufsz);
  extern c_bool_t C_strbuffer_destroy(c_strbuffer_t *sb);

  extern c_bool_t C_strbuffer_clear(c_strbuffer_t *sb);
  extern c_bool_t C_strbuffer_strcpy(c_strbuffer_t *sb, const char *s);
  extern c_bool_t C_strbuffer_strcat(c_strbuffer_t *sb, const char *s);
  extern c_bool_t C_strbuffer_sprintf(c_strbuffer_t *sb, const char *s, ...);
  extern c_bool_t C_strbuffer_putc(c_strbuffer_t *sb, char c);
  extern size_t C_strbuffer_strlen(c_strbuffer_t *sb);

#define C_strbuffer_size(S)                     \
  ((S)->bufsz)

#define C_strbuffer_string(S)                   \
  (const char *)((S)->buf)

/* ----------------------------------------------------------------------------
 * time formatting/parsing functions
 * ----------------------------------------------------------------------------
 */

  extern c_bool_t C_time_format(time_t t, char *buf, size_t bufsz,
                                const char *format);
  extern time_t C_time_parse(const char *buf, const char *format);

/* ----------------------------------------------------------------------------
 * timer functions
 * ----------------------------------------------------------------------------
 */

  typedef struct c_timer_t
  {
    time_t created;
    struct tms t1;
    struct tms t2;
    struct timeval tv1;
    struct timeval tv2;
    clock_t usr;
    clock_t sys;
    long real;
    float usr_time;
    float sys_time;
    long real_time;
    c_bool_t running;
    char pad[3];
  } c_timer_t;

#define C_timer_user(T)                         \
  ((T)->user_time)

#define C_timer_system(T)                       \
  ((T)->system_time)

#define C_timer_elapsed(T)                      \
  ((T)->real_time)

#define C_timer_created(T)                      \
  ((T)->created)

#define C_timer_isrunning(T)                    \
  ((T)->running)

  extern c_timer_t *C_timer_create(void);
  extern void C_timer_destroy(c_timer_t *timer);

  extern void C_timer_start(c_timer_t *timer);
  extern void C_timer_resume(c_timer_t *timer);
  extern void C_timer_stop(c_timer_t *timer);
  extern void C_timer_reset(c_timer_t *timer);

/* ----------------------------------------------------------------------------
 * vector management
 * ----------------------------------------------------------------------------
 */

  typedef struct c_vector_t
  {
    uint_t len;
    uint_t c;
    char **vs;
    char **v;
    uint_t blk;
    uint_t blksz;
  } c_vector_t;

  extern c_vector_t *C_vector_start(uint_t resize_rate);
  extern c_bool_t C_vector_store(c_vector_t *v, const char *s);
  extern c_bool_t C_vector_contains(c_vector_t *v, const char *s);
  extern char **C_vector_end(c_vector_t *v, size_t *len);
  extern void C_vector_abort(c_vector_t *v);

#define C_vector_free C_free_vec

/* ----------------------------------------------------------------------------
 * random numbers
 * ----------------------------------------------------------------------------
 */

  extern void C_random_seed(void);
  extern uint_t C_random(uint_t range);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_util_h */

/* end of library header */
