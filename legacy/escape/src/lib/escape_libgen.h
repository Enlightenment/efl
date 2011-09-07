#ifndef __ESCAPE_LIBGEN_H__
#define __ESCAPE_LIBGEN_H__


/**
 * @file escape_libgen.h
 * @brief The file that provides functions ported from Unix in libgen.h.
 * @defgroup Escape_Libgen_Group Libgen.h functions.
 *
 * This header provides functions ported from Unix in libgen.h.
 *
 * @{
 */

EAPI char *escape_basename(char *path);

#ifdef basename
#undef basename
#endif
#define basename(p) escape_basename(p)

EAPI char *escape_dirname(char *path);

#ifdef dirname
#undef dirname
#endif
#define dirname(p) escape_dirname(p)


/**
 * @}
 */


#endif /* __ESCAPE_LIBGEN_H__ */
