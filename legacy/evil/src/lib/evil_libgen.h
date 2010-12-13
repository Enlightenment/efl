#ifndef __EVIL_LIBGEN_H__
#define __EVIL_LIBGEN_H__


/**
 * @file evil_libgen.h
 * @brief The file that provides functions ported from Unix in libgen.h.
 * @defgroup Evil_Libgen_Group Libgen.h functions.
 *
 * This header provides functions ported from Unix in libgen.h.
 *
 * @{
 */

EAPI char *evil_basename(char *path);

#define basename(p) evil_basename(p)

EAPI char *evil_dirname(char *path);

#define dirname(p) evil_dirname(p)


/**
 * @}
 */


#endif /* __EVIL_LIBGEN_H__ */
