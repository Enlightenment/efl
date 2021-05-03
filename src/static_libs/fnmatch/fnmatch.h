/*
 * LICENSE: MIT, see COPYRIGHT file in the directory
 */

#ifndef	_FNMATCH_H
#define	_FNMATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define	__FNM_PATHNAME 0x1
#define	__FNM_NOESCAPE 0x2
#define	__FNM_PERIOD   0x4
#define	__FNM_LEADING_DIR	0x8
#define	__FNM_CASEFOLD	0x10
#define	__FNM_FILE_NAME	__FNM_PATHNAME

#define	__FNM_NOMATCH 1
#define __FNM_NOSYS   (-1)

int __fnmatch(const char *, const char *, int);

#ifdef __cplusplus
}
#endif

#endif
