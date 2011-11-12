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
 * @image html  e_big.png
 * @author Vincent Torri
 * @date 2008-2011
 *
 * @section intro_sec Introduction
 *
 * The Evil library is an evil library that ports some evil Unix
 * functions to the Windows (XP or above, or Mobile) platform. The
 * evilness is so huge that the most of the functions are not POSIX or
 * BSD compliant.
 *
 * These functions are intended to be used in the Enlightenment
 * Fundations Libraries only and can be compiled only on Windows,
 * using MSYS/MinGW on Windows, and cross-compilation on Unix. This
 * library is minimal in the sense that only the functions needed to
 * compile the EFL are available. The purpose of this library is NOT
 * to have a full POSIX emulation et it is NOT a replacement of
 * cygwin. To compare the size of the DLL themselves, Evil is around
 * 33 KB and cygwin DLL is around 800 KB.
 *
 * @section acknowledgments_sec Acknowledgments
 *
 * This library has receive some from people interested in the EFL or
 * not. Among them, evil thanks to Lars Munch, Raoul Hecky, Nicolas
 * Aguirre, Tor Lillqvist, Lance Fetters, Vincent Richomme, Paul
 * Vixie, Daniel Stenberg, who helped the author of the library in
 * different fields (code and tests).
 *
 * @section license_sec license
 *
 * The Evil library is distributes under a modified BSD license. See
 * the files COPYING and COPYING-PLAIN in the top level directory for
 * the full license text.
 *
 * @section reference_sec Reference API
 *
 * Use the horizontal menu above to navigate into the reference API
 */

/**
 * @file Evil.h
 * @brief The file that provides miscellaneous functions ported from Unix.
 * @defgroup Evil Miscellaneous functions ported from Unix.
 *
 * This header provides miscallenaous functions that exist on Unix
 * but not on Windows platform. They try to follow the conformance of
 * the Unix versions.
 */

/**
 * @cond LOCAL
 */

#ifdef __cplusplus
extern "C" {
#endif


#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>


#ifdef PATH_MAX
# undef PATH_MAX
#endif /* PATH_MAX */

#define PATH_MAX MAX_PATH


#ifdef _MSC_VER

# include <io.h>

# define F_OK 0  /* Check for file existence */
# define X_OK 1  /* MS access() doesn't check for execute permission. */
# define W_OK 2  /* Check for write permission */
# define R_OK 4  /* Check for read permission */

typedef DWORD          pid_t;
typedef unsigned short mode_t;

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef SSIZE_T ssize_t;

# define strdup(s) _strdup(s)
# define unlink(filename) _unlink(filename)
# define fileno(f) _fileno(f)
# define fdopen(fd,m) _fdopen((fd),(m))
# define access(p,m) _access((p),(m))
# define hypot(x,y) _hypot((x),(y))
# define tzset _tzset

#endif /* _MSC_VER */

#ifdef _WIN32_WCE
# ifndef offsetof
#  define offsetof(type, ident) ((size_t)&(((type*)0)->ident))
# endif
#endif

typedef unsigned long  uid_t;
typedef unsigned long  gid_t;


#include "evil_fcntl.h"
#include "evil_inet.h"
#include "evil_langinfo.h"
#include "evil_libgen.h"
#include "evil_main.h"
#include "evil_stdlib.h"
#include "evil_stdio.h"
#include "evil_string.h"
#include "evil_time.h"
#include "evil_unistd.h"
#include "evil_util.h"


#if (defined(_WIN32) && !defined(_UWIN) && !defined(__CYGWIN__))
# if defined(_MSC_VER) || defined(__MINGW32__)

# ifdef S_ISDIR
#  undef S_ISDIR
# endif
# ifdef S_ISREG
#  undef S_ISREG
# endif
# define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
# define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)

# define S_IRUSR _S_IRUSR
# define S_IWUSR _S_IWUSR
# define S_IXUSR _S_IXUSR
# define S_IRGRP S_IRUSR
# define S_IROTH S_IRUSR
# define S_IWGRP S_IWUSR
# define S_IWOTH S_IWUSR
# define S_IXGRP S_IXUSR
# define S_IXOTH S_IXUSR

# define _S_IRWXU (_S_IREAD | _S_IWRITE | _S_IEXEC)
# define _S_IXUSR _S_IEXEC
# define _S_IWUSR _S_IWRITE
# define _S_IRUSR _S_IREAD

#  define mkdir(p,m) _mkdir(p)
  /*
#  define close(fd) _close(fd)
#  define read(fd,buffer,count) _read((fd),(buffer),(count))
#  define write(fd,buffer,count) _write((fd),(buffer),(count))
#  define unlink(filename) _unlink((filename))
#  define lstat(f,s) _stat((f),(s))
  */

# endif
#endif


#ifdef __cplusplus
}
#endif

/**
 * @endcond
 */

#endif /* __EVIL_H__ */
