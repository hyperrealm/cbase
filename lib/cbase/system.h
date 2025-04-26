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

#ifndef __cbase_system_h
#define __cbase_system_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <sys/stat.h>
#include <utime.h>
#include <inttypes.h>

#include <cbase/defs.h>
#include <cbase/util.h>

/* ----------------------------------------------------------------------------
 * byte order functions
 * ----------------------------------------------------------------------------
 */

  extern uint16_t C_byteord_htons(uint16_t val);
  extern uint16_t C_byteord_ntohs(uint16_t val);

  extern uint32_t C_byteord_htonl(uint32_t val);
  extern uint32_t C_byteord_ntohl(uint32_t val);

  extern uint64_t C_byteord_htonll(uint64_t val);
  extern uint64_t C_byteord_ntohll(uint64_t val);

  extern float C_byteord_htonf(float val);
  extern float C_byteord_ntohf(float val);

  extern double C_byteord_htond(double val);
  extern double C_byteord_ntohd(double val);

/* ----------------------------------------------------------------------------
 * debugging/tracing functions
 * ----------------------------------------------------------------------------
 */

  extern void C_debug_printf_x(const char *file, int line, int severity,
                               const char *format, ...);
  extern void C_debug_set_trace(c_bool_t flag);
  extern void C_debug_set_stream(FILE *stream);
  extern void C_debug_set_termattr(c_bool_t flag);
  extern c_bool_t C_debug_doassert(char *file, int line, char *expression);

#define C_assert(EX)                                            \
  (void)((EX) || C_debug_doassert(__FILE__, __LINE__, #EX))

#define C_DEBUG_INFO    0
#define C_DEBUG_ERROR   1

#ifdef DEBUG

#define C_debug_printf(F, args...)                                      \
  C_debug_printf_x(__FILE__, __LINE__, C_DEBUG_INFO, (F), ## args)

#else

#define C_debug_printf(F, args...)

#endif /* DEBUG */

/* ----------------------------------------------------------------------------
 * error handling functions
 * ----------------------------------------------------------------------------
 */

  extern void C_error_init(const char *progname);
  extern void C_error_printf(const char *fmt, ...);
  extern void C_error_usage(const char *usage);
  extern void C_error_syserr(void);

  extern const char *C_error_string(void);

  extern int C_error_get_errno(void);
  extern void C_error_set_errno(int err);

#define c_errno (C_error_get_errno())

/* ----------------------------------------------------------------------------
 * logging functions
 * ----------------------------------------------------------------------------
 */

  extern void C_log_set_console(c_bool_t flag);
  extern void C_log_set_stream(FILE *stream);
  extern void C_log_set_termattr(c_bool_t flag);

  extern void C_log_message_x(int level, const char *format, ...);

#define C_LOG_INFO    0
#define C_LOG_WARNING 1
#define C_LOG_ERROR   2

#define C_log_info(F, args...)                  \
  C_log_message_x(C_LOG_INFO, (F), ## args)

#define C_log_warning(F, args...)               \
  C_log_message_x(C_LOG_WARNING, (F), ## args)

#define C_log_error(F, args...)                 \
  C_log_message_x(C_LOG_ERROR, (F), ## args)

/* ----------------------------------------------------------------------------
 * subprocess control functions
 * ----------------------------------------------------------------------------
 */

  extern int C_exec_run_cwd(char **argv, int fdin, int fdout, c_bool_t waitf,
                            const char *cwd);
  extern int C_exec_va_run(int fdin, int fdout, c_bool_t waitf, ...);
  extern int C_exec_runpty(char **argv, FILE **fpin, FILE **fpout);
  extern int C_exec_pipefrom_cwd(char **argv, int *fd, const char *cwd);
  extern int C_exec_pipeto_cwd(char **argv, int *fd, const char *cwd);
  extern int C_exec_va_call(const char *arg, ...);
  extern int C_exec_wait(pid_t pid);

#define C_exec_run(A, FI, FO, W)                \
  C_exec_run_cwd(A, FI, FO, W, NULL)
#define C_exec_va_run(FI, FO, W, args...)               \
  C_exec_va_run_cwd(A, FI, FO, W, NULL, ## args)
#define C_exec_pipefrom(A, F)                   \
  C_exec_pipefrom_cwd(A, F, NULL)
#define C_exec_pipeto(A, F)                     \
  C_exec_pipeto_cwd(A, F, NULL)

/* ----------------------------------------------------------------------------
 * filesystem functions
 * ----------------------------------------------------------------------------
 */

  typedef struct c_dirlist_t
  {
    char **files;
    char **dirs;
    size_t nfiles;
    size_t ndirs;
  } c_dirlist_t;

  extern c_bool_t C_file_readdir(const char *path, c_dirlist_t *dir,
                                 int flags);
  extern c_bool_t C_file_traverse(const char *path,
                                  c_bool_t (*examine)(const char *file,
                                                      const struct stat *fst,
                                                      uint_t depth,
                                                      void *hook),
                                  void *hook);
  extern const char *C_file_getcwd(void);

  extern c_bool_t C_file_issymlink(const char *path);
  extern c_bool_t C_file_isdir(const char *path);
  extern c_bool_t C_file_isfile(const char *path);
  extern c_bool_t C_file_ispipe(const char *path);

  extern c_bool_t C_file_mkdirs(const char *path, mode_t mode);
  extern void *C_file_load(const char *path, size_t *len);

#define C_FILE_SKIPDOT    0x01
#define C_FILE_SKIP2DOT   0x02
#define C_FILE_SKIPHIDDEN 0x04
#define C_FILE_ADDSLASH   0x08
#define C_FILE_SEPARATE   0x10
#define C_FILE_SORT       0x20
#define C_FILE_SKIPDIRS   0x40
#define C_FILE_SKIPFILES  0x80

#define C_FILE_READ_LOCK 0
#define C_FILE_WRITE_LOCK 1

  extern c_bool_t C_file_lock(FILE *fp, int type);
  extern c_bool_t C_file_trylock(FILE *fp, int type);
  extern c_bool_t C_file_unlock(FILE *fp, int type);

/* ----------------------------------------------------------------------------
 * memory handling functions & macros
 * ----------------------------------------------------------------------------
 */

  extern size_t C_mem_defrag(void *p, size_t elemsz, size_t len,
                             c_bool_t (*isempty)(void *elem));
  extern void *C_mem_manage(void *p, size_t n, c_bool_t clearf);
  extern void C_mem_set_errorfunc(c_bool_t (*func)(void));
  extern void *C_mem_free(void *p);
  extern void C_mem_free_vec(char **p);
  extern uint_t C_mem_va_free(uint_t n, ...);

  extern void C_mem_set_alloc_hook(void (*hook)(const void *p_old,
                                                const void *p_new,
                                                size_t len));
  extern void C_mem_default_alloc_hook(const void *p_old, const void *p_new,
                                       size_t len);

#define C_malloc(N, T)                                  \
  (T *)C_mem_manage(NULL, (N) * sizeof(T), FALSE)
#define C_calloc(N, T)                                  \
  (T *)C_mem_manage(NULL, (N) * sizeof(T), TRUE)
#define C_realloc(P, N, T)                                      \
  (T *)C_mem_manage((void *)(P), (N) * sizeof(T), FALSE)

#define C_zero(P, T)                            \
  memset((void *)P, 0, sizeof(T))
#define C_zeroa(P, N, T)                        \
  memset((void *)P, 0, (N) * sizeof(T))

#define C_free(P)                               \
  C_mem_free((void *)(P))
#define C_new(T)                                \
  C_calloc(1, T)
#define C_newa(N, T)                            \
  C_calloc((N), T)
#define C_newb(N)                               \
  C_mem_manage(NULL, (N), TRUE)
#define C_newstr(N)                             \
  C_calloc(((N) + 1), char)
#define C_free_vec(V)                           \
  C_mem_free_vec((char **)(V))

#define C_va_free C_mem_va_free

/* ----------------------------------------------------------------------------
 * memory pool functions & macros
 * ----------------------------------------------------------------------------
 */

  typedef struct c_mempool_t
  {
    void *base; 
    size_t size;
    size_t pos;
  } c_mempool_t;

  extern c_mempool_t *C_mempool_create(size_t size);
  extern void C_mempool_destroy(c_mempool_t *pool);
  extern void *C_mempool_alloc(c_mempool_t *pool, size_t size);
  extern size_t C_mempool_avail(c_mempool_t *pool);

#define C_palloc1(P, T)                         \
  (T *)C_mempool_alloc((P), sizeof(T))

#define C_palloc(P, N, T)                       \
  (T *)C_mempool_alloc((P), (N) * sizeof(T))

#define C_pallocstr(P, N)                       \
  C_palloc((P), (N), char)

/* ----------------------------------------------------------------------------
 * I/O functions & macros
 * ----------------------------------------------------------------------------
 */

  extern int C_io_getchar(uint_t delay);
  extern int C_io_gets(FILE *fp, char *buf, size_t bufsz, char termin);
  extern int C_io_getpasswd(const char *prompt, char *buf, size_t bufsz);
  extern char *C_io_getline(FILE *fp, char termin, int *len);
  extern char *C_io_getline_buf(FILE *fp, char termin, c_buffer_t *buf);
  extern int C_io_fprintf(FILE *fp, const char *format, ...);

#define C_printf(S, args...)                    \
  C_io_fprintf(stdout, S, ## args)

#define C_IO_GETCHAR_DELAY 10

#define C_getchar()                             \
  C_io_getchar(C_IO_GETCHAR_DELAY)
#define C_gets(B, L)                            \
  C_io_gets(stdin, (B), (L), '\n')
#define C_getline(F, L)                         \
  C_io_getline((F), '\n', (L))

/* ----------------------------------------------------------------------------
 * dynamic linker functions
 * ----------------------------------------------------------------------------
 */

  typedef struct c_dlobject_t
  {
    void *handle;
    char *path;
    char *error;
    c_bool_t loaded;
    time_t last_access;
    void *hook;
  } c_dlobject_t;

  extern c_dlobject_t *C_dlobject_create(const char *path);
  extern c_bool_t C_dlobject_load(c_dlobject_t *obj, c_bool_t lazy);
  extern c_bool_t C_dlobject_unload(c_dlobject_t *obj);
  extern c_bool_t C_dlobject_destroy(c_dlobject_t *obj);
  extern void *C_dlobject_lookup(c_dlobject_t *obj, const char *symbol);

#define C_dlobject_isloaded(OBJ)                \
  ((OBJ)->loaded)
#define C_dlobject_error(OBJ)                   \
  (const char *)((OBJ)->error)
#define C_dlobject_path(OBJ)                    \
  (const char *)((OBJ)->path)

/* ----------------------------------------------------------------------------
 * memory-mapped files
 * ----------------------------------------------------------------------------
 */

  typedef struct c_memfile_t
  {
    int fd;
    void *base;
    off_t length;
  } c_memfile_t;

  extern c_memfile_t *C_memfile_open(const char *file, c_bool_t readonly);
  extern c_bool_t C_memfile_close(c_memfile_t *f);
  extern c_bool_t C_memfile_resize(c_memfile_t *f, off_t length);
  extern c_bool_t C_memfile_sync(c_memfile_t *f, c_bool_t async);

#define C_memfile_base(F)                       \
  ((F)->base)

#define C_memfile_length(F)                     \
  ((F)->length)

#define C_memfile_pointer(F, O)                 \
  ((F)->base + (O))

/* system info functions */

  typedef struct c_sysinfo_t
  {
    char *login;
    char *fullname;
    char *homedir;
    char *shell;
    uid_t uid;
    gid_t gid;
    uid_t euid;
    gid_t egid;
    char *hostname;
    char *osname;
    char *osver;
    char *osrel;
    char *arch;
    pid_t pid;
    pid_t ppid;
    char *term;
    time_t stime;
  } c_sysinfo_t;

  extern c_bool_t C_system_ingroup(const char *login, const char *group);
  extern c_sysinfo_t *C_system_getinfo(void);
  extern c_bool_t C_system_cdhome(void);

  extern c_bool_t C_system_passwd_validate(const char *plaintext,
                                           const char *ciphertext);
  extern c_bool_t C_system_passwd_generate(const char *plaintext, char *buf,
                                           size_t bufsz);

  extern uid_t C_system_get_uid(void);
  extern gid_t C_system_get_gid(void);
  extern pid_t C_system_get_pid(void);
  extern char *C_system_get_login(void);
  extern char *C_system_get_fullname(void);
  extern char *C_system_get_homedir(void);
  extern char *C_system_get_hostname(void);
  extern char *C_system_get_term(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_system_h */

/* end of library header */
