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

/* File scope variables */

static const char *__c_version = PACKAGE_VERSION;

static const char *__c_info = PACKAGE_STRING \
  " - (C) 1994-2025 Mark A Lindner - " PACKAGE_BUGREPORT;

static const char *__c_options[] = {
#ifdef THREADED_LIBRARY
"threaded",
#endif /* THREADED_LIBRARY */

#ifdef HAVE_LIBEXPAT
"xml",
#endif /* HAVE_LIBEXPAT */
NULL
};

/* External functions */

const char *C_library_version(void)
{
  return(__c_version);
}

/*
 */

const char *C_library_info(void)
{
  return(__c_info);
}

/*
 */

const char **C_library_options(void)
{
  return(__c_options);
}

/* end of source file */
