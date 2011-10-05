
/* Copyright (C) 2005 by Daniel Stenberg
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#ifndef __EVIL_INET_H__
#define __EVIL_INET_H__


/**
 * @file evil_inet.h
 * @brief The file that provides functions ported from Unix in arpa/inet.h.
 * @defgroup Evil_Inet_Group Inet.h functions
 *
 * This header provides functions ported from Unix in stdio.h.
 *
 * @{
 */

EAPI int evil_inet_pton(int af, const char *src, void *dst);

#define inet_pton(x,y,z) evil_inet_pton(x,y,z)

EAPI const char *evil_inet_ntop(int af, const char *src, void *dst);

#define inet_ntop(x,y,z) evil_inet_ntop(x,y,z)



/**
 * @}
 */


#endif /* __EVIL_INET_H__ */
