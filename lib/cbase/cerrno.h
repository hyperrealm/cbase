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

#ifndef __cbase_errno_h
#define __cbase_errno_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cbase/defs.h>

#define C_EFAILED        -1     /* general failure                      */
#define C_EOK             0     /* success (no error)                   */
#define C_EINVAL          1     /* invalid argument(s)                  */
#define C_ESOCKET         2     /* socket() error                       */
#define C_EBADSTATE       3     /* call not valid in this state         */
#define C_EADDRINFO       4     /* unable to resolve network address    */
#define C_EBIND           5     /* bind() error                         */
#define C_EBLOCKED        6     /* operation would block                */
#define C_EACCEPT         7     /* accept() error                       */
#define C_EBADTYPE        8     /* wrong socket type for operation      */
#define C_ELISTEN         9     /* listen() error                       */
#define C_ESOCKINFO      10     /* unable to obtain socket info         */
#define C_EFCNTL         11     /* fcntl() error                        */
#define C_ELOSTCONN      12     /* connection to peer lost              */
#define C_EFDOPEN        13     /* fdopen() error                       */
#define C_ECONNECT       14     /* connect() error                      */
#define C_ENOCONN        15     /* no connection available              */
#define C_ESEND          16     /* send() error                         */
#define C_ERECV          17     /* recv() error                         */
#define C_EMSG2BIG       18     /* UDP message too big                  */
#define C_ESENDTO        19     /* sendto() error                       */
#define C_ERECVFROM      20     /* recvfrom() error                     */
#define C_ETIMEOUT       21     /* connection or I/O timed out          */
#define C_ESVCINFO       22     /* unable to obtain service info        */
#define C_EFORK          23     /* unable to fork                       */
#define C_ETBLFULL       24     /* table full (depends on context)      */
#define C_ESELECT        25     /* select() error                       */
#define C_EIOCTL         26     /* ioctl() error                        */
#define C_ETCATTR        27     /* unable to get/set tty attributes     */
#define C_ENOTTY         28     /* descriptor does not refer to a tty   */
#define C_EGETPTY        29     /* getpty() error                       */
#define C_EOPEN          30     /* open() error                         */
#define C_EPTY           31     /* general pseudoterminal error         */
#define C_EEXECV         32     /* execv() error                        */
#define C_ENOTIMPL       33     /* function or feature not implemented  */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cbase_errno_h */

/* end of header file */
