#ifndef __EVIL_H__
#define __EVIL_H__

#ifdef EAPI
# undef EAPI
#endif /* EAPI */

#ifdef _WIN32
# ifdef EFL_EVIL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVIL_BUILD */
#endif /* _WIN32 */


/**
 * @mainpage Evil
 * @author Vincent Torri
 * @date 2008
 *
 * @section intro_sec Introduction
 *
 * The Evil library is an evil library that ports some evil Unix
 * functions to the Windows (XP or Mobile) platform. The evilness is
 * so huge that the most of the functions are not POSIX or BSD
 * compliant.
 *
 * These functions are intended to be used in the Enlightenment
 * Fundations Libraries only and can be compiled only on Windows.
 *
 * @section evil_sec Evil API Documentation
 *
 * Take a look at the evil documentation of the @ref Dlfcn.
 *
 * Take a look at the demoniac documentation of the @ref Mman.
 *
 * Take a look at the satanic documentation of the @ref Evil.
 */

/**
 * @file evil.h
 * @brief The file that provides miscellaneous functions ported from Unix.
 * @defgroup Evil Miscellaneous functions ported from Unix.
 *
 * This header provides miscallenaous functions that exist on Unix
 * but not on Windows platform. They try to follow the conformance of
 * the Unix versions.
 */

#ifdef __cplusplus
extern "C" {
#endif


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef PATH_MAX
# undef PATH_MAX
#endif /* PATH_MAX */

#define PATH_MAX MAX_PATH


#ifdef _MSC_VER

#define F_OK 0  /* Check for file existence */
#define X_OK 1  /* MS access() doesn't check for execute permission. */
#define W_OK 2  /* Check for write permission */
#define R_OK 4  /* Check for read permission */

#define _S_IRWXU (_S_IREAD | _S_IWRITE | _S_IEXEC)
#define _S_IXUSR _S_IEXEC
#define _S_IWUSR _S_IWRITE
#define _S_IRUSR _S_IREAD

typedef int            pid_t;
typedef SSIZE_T        ssize_t;
typedef unsigned short mode_t;

#endif /* _MSC_VER */


#ifndef __CEGCC__

typedef unsigned long  uid_t;
typedef unsigned long  gid_t;

#endif /* ! __CEGCC__ */


#include "evil_fcntl.h"
#include "evil_langinfo.h"
#include "evil_stdlib.h"
#include "evil_stdio.h"
#include "evil_string.h"
#include "evil_unistd.h"
#include "evil_util.h"


#if defined(__MSDOS__) || defined(__EMX__) || \
   (defined(_WIN32) && !defined(_UWIN) && !defined(__CYGWIN__) && !defined(__CEGCC__))
# if defined(_MSC_VER) || defined(__MINGW32__)

#  ifdef S_ISDIR
#   undef S_ISDIR
#  endif
#  ifdef S_ISREG
#   undef S_ISREG
#  endif
#  define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#  define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)

#  define S_IRUSR _S_IRUSR
#  define S_IWUSR _S_IWUSR
#  define S_IXUSR _S_IXUSR
#  define S_IRGRP S_IRUSR
#  define S_IROTH S_IRUSR
#  define S_IWGRP S_IWUSR
#  define S_IWOTH S_IWUSR
#  define S_IXGRP S_IXUSR
#  define S_IXOTH S_IXUSR

#  define open(path, flag, ...) _open((path), _O_BINARY | (flag), __VA_ARGS__)
#  define close(fd) _close(fd)
#  define read(fd,buffer,count) _read((fd),(buffer),(count))
#  define write(fd,buffer,count) _write((fd),(buffer),(count))
#  define unlink(filename) _unlink((filename))
#  define mkdir(p,m) _mkdir(p)
#  define getcwd(b,s) evil_getcwd((b),(s))
#  define lstat(f,s) _stat((f),(s))
#  define strdup(s) _strdup(s)

# endif
#endif

#define pipe(fd) evil_pipe(fd)

#if defined (_WIN32_WCE) && ! defined (__CEGCC__)
# define fopen(path, mode) evil_fopen(path, mode)
#endif /* _WIN32_WCE && ! __CEGCC__ */


#ifdef __cplusplus
}
#endif

#endif /* __EVIL_H__ */
