#ifndef EINA_FNMATCH_H
#define EINA_FNMATCH_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32

/* Bits set in the FLAGS argument to `fnmatch'.  */
#define FNM_PATHNAME (1 << 0) /* No wildcard can ever match `/'.  */
#define FNM_NOESCAPE (1 << 1) /* Backslashes don't quote special chars.  */
#define FNM_PERIOD   (1 << 2) /* Leading `.' is matched only explicitly.  */

#define FNM_FILE_NAME   FNM_PATHNAME /* Preferred GNU name.  */
#define FNM_LEADING_DIR (1 << 3)     /* Ignore `/...' after a match.  */
#define FNM_CASEFOLD    (1 << 4)     /* Compare without regard to case.  */

/* Value returned by `fnmatch' if STRING does not match PATTERN.  */
#define FNM_NOMATCH 1

int eina_fnmatch(const char *pattern, const char *string, int flags);

#else /* _WIN32 */

//#define eina_fnmatch(a,b,c) fnmatch(a,b,c)
#include <fnmatch.h>
#define eina_fnmatch fnmatch

#endif /* _WIN32 */

#ifdef __cplusplus
}
#endif 

#endif /* EINA_FNMATCH_H */
