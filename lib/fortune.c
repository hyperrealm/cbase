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
#include <netinet/in.h>
#include <string.h>

/* Local headers */

#include "cbase/defs.h"
#include "cbase/system.h"
#include "cbase/util.h"
#include "cbase/fortune.h"

/* Macros */

#define C_FORTUNE_SEPARATOR "%"
#define C_FORTUNE_INDEX_EXT "idx"

/* Functions */

c_bool_t C_fortune_indexdb(const char *basename)
{
  char *buf, databuf[128];
  FILE *fp, *ip;
  long offset = 0;
  size_t bufsz;

  if(!(fp = fopen(basename, "r")))
    return(FALSE);

  bufsz = strlen(basename) + strlen(C_FORTUNE_INDEX_EXT) + 2;
  buf = C_newstr(bufsz);
  snprintf(buf, bufsz, "%s.%s", basename, C_FORTUNE_INDEX_EXT);

  if(!(ip = fopen(buf, "w+")))
  {
    fclose(fp);
    C_free(buf);
    return(FALSE);
  }

  fwrite(&offset, sizeof(offset), 1, ip);

  while(C_io_gets(fp, databuf, sizeof(databuf), '\n') != EOF)
  {
    if(!strcmp(databuf, C_FORTUNE_SEPARATOR))
    {
      offset = htonl(ftell(fp));
      fwrite(&offset, sizeof(offset), 1, ip);
    }
  }

  C_free(buf);

  fclose(fp);
  fclose(ip);
  return(TRUE);
}

/*
 */

c_fortune_db_t *C_fortune_opendb(const char *basename)
{
  c_fortune_db_t *db;
  struct stat stbuf;
  char *buf;
  size_t len;

  len = strlen(basename) + strlen(C_FORTUNE_INDEX_EXT) + 2;

  buf = C_newstr(len);
  snprintf(buf, len, "%s.%s", basename, C_FORTUNE_INDEX_EXT);

  if(access(basename, (F_OK | R_OK)) || access(buf, (F_OK | R_OK)))
  {
    C_free(buf);
    return(NULL);
  }

  db = C_new(c_fortune_db_t);

  db->data = fopen(basename, "r");
  db->index = fopen(buf, "r");

  stat(basename, &stbuf);
  db->filelen = stbuf.st_size;

  stat(buf, &stbuf);
  db->count = (stbuf.st_size / sizeof(long));

  C_free(buf);

  return(db);
}

/*
 */

c_bool_t C_fortune_closedb(c_fortune_db_t *db)
{
  if(!db)
    return(FALSE);

  fclose(db->data);
  fclose(db->index);

  C_free(db);

  return(TRUE);
}

/*
 */

const char *C_fortune_select(c_fortune_db_t *db)
{
  char *buf;
  int idx;
  unsigned long offset, end, fsize;

  if(!db)
    return(NULL);

  idx = C_random(db->count);

  fseek(db->index, (idx * sizeof(unsigned long)), SEEK_SET);
  if(fread(&offset, sizeof(unsigned long), 1, db->index) != 1)
    return(NULL);

  offset = ntohl(offset);

  if(idx == (db->count - 1))
    fsize = db->filelen - offset;
  else
  {
    if(fread(&end, sizeof(unsigned long), 1, db->index) != 1)
      return(NULL);

    fsize = ntohl(end) - offset - 2;
  }

  fseek(db->data, offset, SEEK_SET);

  buf = C_newb(fsize + 1);
  if(fread(buf, fsize, 1, db->data) != 1)
  {
    C_free(buf);
    return(NULL);
  }

  return(buf);
}

/* end of source file */
