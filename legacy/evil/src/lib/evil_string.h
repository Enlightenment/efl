#ifndef __EVIL_STRING_H__
#define __EVIL_STRING_H__


#ifdef __MINGW32CE__

/*
 * Environment variable related functions
 *
 */

EAPI char *strerror (int errnum);

#endif /* __MINGW32CE__ */

#ifndef __CEGCC__

/*
 * bit related functions
 *
 */

EAPI int ffs(int i);

#endif /* ! __CEGCC__ */


#endif /* __EVIL_STRING_H__ */
