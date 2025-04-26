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
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Macros */

#define C_FILE_BLOCKSZ 15
#define C_FILE_PATHBLOCKSZ 64

/* Functions */

c_bool_t C_file_readdir(const char *path, c_dirlist_t *dir, int flags)
{
  c_vector_t *vf, *vd = NULL;
  char *p, *buf, *z;
  struct stat st;
  DIR *dp;
  struct dirent *de;

  if(!path || !dir)
    return(FALSE);
  if(!(dp = opendir(path)))
    return(FALSE);

  buf = C_newstr(pathconf(".", _PC_NAME_MAX));
  strcpy(buf, path);
  strcat(buf, "/");
  z = buf + strlen(buf);

  vf = C_vector_start(C_FILE_BLOCKSZ);
  if(flags & C_FILE_SEPARATE)
    vd = C_vector_start(C_FILE_BLOCKSZ);
  dir->nfiles = dir->ndirs = 0;

  while((de = readdir(dp)) != NULL)
  {
    strcpy(z, de->d_name);

    if(lstat(buf, &st))
      continue;

    if(S_ISDIR(st.st_mode))
    {
      if(*(de->d_name) == '.')
      {
        if(!strcmp(de->d_name, "."))
        {
          if(flags & C_FILE_SKIPDOT)
            continue;
        }
        else if(!strcmp(de->d_name, ".."))
        {
          if(flags & C_FILE_SKIP2DOT)
            continue;
        }
        else if(flags & C_FILE_SKIPHIDDEN)
          continue;
      }
      p = ((flags & C_FILE_ADDSLASH)
           ? C_string_dup1(de->d_name, '/') : C_string_dup(de->d_name));

    }
    else if((S_ISREG(st.st_mode) && !(flags & C_FILE_SKIPFILES))
            || (S_ISDIR(st.st_mode) && !(flags & C_FILE_SKIPDIRS)))
    {
      if((*de->d_name == '.') && (flags & C_FILE_SKIPHIDDEN))
        continue;
      p = C_string_dup(de->d_name);
    }
    else
      continue;  /* skip all other types of files */

    if(S_ISDIR(st.st_mode) && (flags & C_FILE_SEPARATE))
      C_vector_store(vd, p);
    else
      C_vector_store(vf, p);
  }
  closedir(dp);
  C_free(buf);

  dir->files = C_vector_end(vf, &(dir->nfiles));
  if(flags & C_FILE_SEPARATE)
    dir->dirs = C_vector_end(vd, &(dir->ndirs));

  if(flags & C_FILE_SORT)
  {
    if(dir->nfiles)
      C_string_sortvec(dir->files, dir->nfiles);
    if(dir->ndirs)
      C_string_sortvec(dir->dirs, dir->ndirs);
  }

  C_free(de);

  return(TRUE);
}

/*
 */

static c_bool_t __C_file_descend(const char *path,
                                 c_bool_t (*examine)(const char *file,
                                                     const struct stat *fst,
                                                     uint_t depth, void *hook),
                                 uint_t depth, void *hook)
{
  const char *opath = NULL;
  struct stat fst;
  DIR *dp;
  struct dirent *de;
  c_bool_t r = TRUE;

  if(!(opath = C_file_getcwd())) return(FALSE);

  if(chdir(path))
  {
    C_free(opath);
    return(TRUE);
  }

  if((dp = opendir(".")))
  {
    while((de = readdir(dp)) != NULL)
    {
      if(!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
        continue;
      lstat(de->d_name, &fst);
      if(S_ISDIR(fst.st_mode))
      {
        if(!examine(de->d_name, &fst, depth, hook))
        {
          r = FALSE;
          break;
        }

        if(!__C_file_descend(de->d_name, examine, depth + 1, hook))
        {
          r = FALSE;
          break;
        }
      }
      else if(S_ISREG(fst.st_mode))
      {
        if(!examine(de->d_name, &fst, depth, hook))
        {
          r = FALSE;
          break;
        }
      }
      else
        continue; /* skip all other types of files */
    }
    closedir(dp);
    C_free(de);
  }

  r = (chdir(opath) == 0);
  C_free(opath);

  return(r);
}

/*
 */

c_bool_t C_file_traverse(const char *path,
                         c_bool_t (*examine)(const char *file,
                                             const struct stat *fst,
                                             uint_t depth, void *hook),
                         void *hook)
{
  if(!path || !examine)
    return(FALSE);
  if(!*path)
    return(FALSE);

  return(__C_file_descend(path, examine, 0, hook));
}

/*
 */

const char *C_file_getcwd(void)
{
  char *p = C_newstr(C_FILE_PATHBLOCKSZ);
  size_t bufsz = C_FILE_PATHBLOCKSZ, l;

  for(;;)
  {
    if(getcwd(p, bufsz - 1))
    {
      l = strlen(p);
      *(p + l) = NUL;
      return(C_realloc(p, l + 1, char));
    }
    if(errno != ERANGE)
    {
      C_free(p);
      return(NULL);
    }
    bufsz += C_FILE_PATHBLOCKSZ;
    p = C_realloc(p, bufsz, char);
    continue;
  }
}

/*
 */

c_bool_t C_file_issymlink(const char *path)
{
  struct stat stbuf;

  if(stat(path, &stbuf) < 0)
    return(FALSE);

  return((S_ISLNK(stbuf.st_mode)) ? TRUE : FALSE);
}

/*
 */

c_bool_t C_file_isdir(const char *path)
{
  struct stat stbuf;

  if(stat(path, &stbuf) < 0)
    return(FALSE);

  return((S_ISDIR(stbuf.st_mode)) ? TRUE : FALSE);
}

/*
 */

c_bool_t C_file_isfile(const char *path)
{
  struct stat stbuf;

  if(stat(path, &stbuf) < 0)
    return(FALSE);

  return((S_ISREG(stbuf.st_mode)) ? TRUE : FALSE);
}

/*
 */

c_bool_t C_file_ispipe(const char *path)
{
  struct stat stbuf;

  if(stat(path, &stbuf) < 0)
    return(FALSE);

  return((S_ISFIFO(stbuf.st_mode)) ? TRUE : FALSE);
}

/*
 */

c_bool_t C_file_mkdirs(const char *path, mode_t mode)
{
  char *p = (char *)path, *q, *buf;

  if(!path)
    return(FALSE);

  p = buf = C_string_dup(path);
  while((q = strchr(p, '/')))
  {
    if(q != buf)
    {
      *q = NUL;

      if(access(buf, F_OK | R_OK | X_OK) != 0)
      {
        if(mkdir(buf, mode) < 0)
        {
          C_free(buf);
          return(FALSE);
        }
      }

      *q = '/';
    }
    p = ++q;
  }

  C_free(buf);

  return(TRUE);
}

/*
 */

c_bool_t C_file_lock(FILE *fp, int type)
{
  int r, fd;
  int t = (type == C_FILE_READ_LOCK) ? F_RDLCK : F_WRLCK;
  struct flock lock;

  lock.l_type = t;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;
  lock.l_pid = 0;

  fd = fileno(fp);

  r = fcntl(fd, F_SETLKW, lock);

  return(r >= 0);
}

/*
 */

c_bool_t C_file_trylock(FILE *fp, int type)
{
  int r, fd;
  int t = (type == C_FILE_READ_LOCK) ? F_RDLCK : F_WRLCK;
  struct flock lock;

  lock.l_type = t;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;
  lock.l_pid = 0;

  fd = fileno(fp);

  r = fcntl(fd, F_SETLKW, lock);

  return(r >= 0);
}

/*
 */

c_bool_t C_file_unlock(FILE *fp, int type)
{
  int r, fd;
  int t = (type == C_FILE_READ_LOCK) ? F_RDLCK : F_WRLCK;
  struct flock lock;

  lock.l_type = t;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;
  lock.l_pid = 0;

  fd = fileno(fp);

  r = fcntl(fd, F_SETLK, lock);

  return(r >= 0);
}

/*
 */

void *C_file_load(const char *path, size_t *len)
{
  struct stat stbuf;
  void *p;
  FILE *fp;

  if(!path || !len)
    return(NULL);

  if(!*path)
    return(NULL);

  if(stat(path, &stbuf) != 0)
    return(NULL);

  *len = (size_t)(stbuf.st_size);

  p = C_malloc(*len + 1, c_byte_t);
  if(!(fp = fopen(path, "r")))
    return(NULL);

  *len = fread(p, *len, 1, fp);
  fclose(fp);

  /* NUL-terminate the data, which is useful if it's a text file */

  *((char *)p + *len) = NUL;

  return(p);
}

/* end of source file */
