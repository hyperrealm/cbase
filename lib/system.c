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

#include <sys/types.h>
#include <sys/utsname.h>
#include <grp.h>
#include <pwd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif
#include <string.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"

/* Macros */

#define C_SYSTEM_SALT_STRLEN 64

/* File scope variables */

static const char *__C_system_salt_string = "abcdefghijklmnopqrstuvwxyz" \
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

static c_sysinfo_t *__C_system_info = NULL;

/* Functions */

c_bool_t C_system_ingroup(const char *login, const char *group)
{
  c_bool_t foundf = FALSE;
  struct group gr;
  struct group *rgr;
  struct passwd pw;
  struct passwd *rpw;
  char **u, *buf = NULL;
  gid_t gid;
  int blk = 128, x;

  if(!login || !group)
    return(FALSE);
  if(!*login || !*group)
    return(FALSE);

  buf = C_malloc(blk, char);

GETPWNAM:
  x = getpwnam_r(login, &pw, buf, blk, &rpw);

  if(x == ERANGE)
  {
    blk += 128;
    buf = C_realloc(buf, blk, char);
    goto GETPWNAM;
  }
  else if(rpw == NULL)
  {
    C_free(buf);
    return(FALSE);
  }

  gid = rpw->pw_gid;

GETGRNAM:
#ifndef HAVE_GETGRNAM_R
#warning "getgrnam_r() not available; C_system_isingroup() won't be re-entrant"
  rgr = getgrnam(group);
#else
  x = getgrnam_r(group, &gr, buf, blk, &rgr);
#endif
  if(x == ERANGE)
  {
    blk += 128;
    buf = C_realloc(buf, blk, char);
    goto GETGRNAM;
  }
  else if(x != 0)
  {
    C_free(buf);
    return(FALSE);
  }

  if(gid == rgr->gr_gid) /* primary group? */
    foundf = TRUE;
  else
  {
    for(u = rgr->gr_mem; *u; u++)
    {
      if(!strcmp(*u, login))
      {
        foundf = TRUE;
        break;
      }
    }
  }

  C_free(buf);
  return(foundf);
}

/*
 */

c_sysinfo_t *C_system_getinfo(void)
{
  static struct passwd *pw = NULL;
  struct passwd *rpw;
  struct utsname uts;
  char *buf;
  int blk = 128, x;

  if(!pw)
    pw = C_new(struct passwd);

  if(!__C_system_info)
  {
    __C_system_info = C_new(c_sysinfo_t);

    buf = C_newstr(blk);

    __C_system_info->uid = getuid();

  GETPWUID:
    x = getpwuid_r(__C_system_info->uid, pw, buf, blk, &rpw);

    if(x == ERANGE)
    {
      blk += 128;
      buf = C_realloc(buf, blk, char);
      goto GETPWUID;
    }

    if(pw)
    {
      char *p;

      __C_system_info->login = C_string_dup(pw->pw_name);
      __C_system_info->fullname = C_string_dup(pw->pw_gecos);

      if((p = strchr(__C_system_info->fullname, ',')) != NULL)
        *p = NUL;
      
      __C_system_info->homedir = C_string_dup(pw->pw_dir);
      __C_system_info->shell = C_string_dup(pw->pw_shell);
    }
    else
    {
      __C_system_info->login = NULL;
      __C_system_info->fullname = NULL;
      __C_system_info->homedir = NULL;
      __C_system_info->shell = NULL;
    }

    C_free(buf);

    __C_system_info->gid = getgid();
    __C_system_info->euid = geteuid();
    __C_system_info->egid = getegid();

    uname(&uts);
    __C_system_info->hostname = C_string_dup(uts.nodename);
    __C_system_info->osname = C_string_dup(uts.sysname);
    __C_system_info->osver = C_string_dup(uts.version);
    __C_system_info->osrel = C_string_dup(uts.release);
    __C_system_info->arch = C_string_dup(uts.machine);

    __C_system_info->pid = getpid();
    __C_system_info->ppid = getppid();
    __C_system_info->stime = time(NULL);
    __C_system_info->term = C_newstr(L_ctermid);
#ifdef HAVE_CTERMID_R
    ctermid_r(__C_system_info->term);
#else
    ctermid(__C_system_info->term);
#endif
    __C_system_info->term[L_ctermid] = NUL;
  }

  return(__C_system_info);
}

/*
 */

c_bool_t C_system_cdhome(void)
{
  char *p = C_system_get_homedir();
  c_bool_t r = (chdir(p) ? FALSE : TRUE);

  C_free(p);
  return(r);
}

/*
 */

uid_t C_system_get_uid(void)
{
  return(C_system_getinfo()->uid);
}

/*
 */

gid_t C_system_get_gid(void)
{
  return(C_system_getinfo()->gid);
}

/*
 */

pid_t C_system_get_pid(void)
{
  return(C_system_getinfo()->pid);
}

/*
 */

char *C_system_get_login(void)
{
  return(C_system_getinfo()->login);
}

/*
 */

char *C_system_get_fullname(void)
{
  return(C_system_getinfo()->fullname);
}

/*
 */

char *C_system_get_homedir(void)
{
  return(C_system_getinfo()->homedir);
}

/*
 */

char *C_system_get_hostname(void)
{
  return(C_system_getinfo()->hostname);
}

/*
 */

char *C_system_get_term(void)
{
  return(C_system_getinfo()->term);
}

/*
 */

c_bool_t C_system_passwd_validate(const char *plaintext,
                                  const char *ciphertext)
{
  char salt[2], *c;

  strncpy(salt, ciphertext, 2);

  c = (char *)crypt(plaintext, salt);

  return(!strcmp(c, ciphertext));
}

/*
 */

c_bool_t C_system_passwd_generate(const char *plaintext, char *buf,
                                  size_t bufsz)
{
  char salt[2], *cp;

  if(!plaintext || !buf || !bufsz)
    return(FALSE);

  salt[0] = __C_system_salt_string[C_random(C_SYSTEM_SALT_STRLEN)];
  salt[1] = __C_system_salt_string[C_random(C_SYSTEM_SALT_STRLEN)];

  cp = (char *)crypt(plaintext, salt);

  strncpy(buf, cp, bufsz - 1);
  *(buf + bufsz - 1) = NUL;

  return(TRUE);
}

/* end of source file */
