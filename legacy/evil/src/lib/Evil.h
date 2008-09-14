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
#include <locale.h>


#ifdef PATH_MAX
# undef PATH_MAX
#endif /* PATH_MAX */

#define PATH_MAX MAX_PATH


#ifdef _MSC_VER

typedef SSIZE_T        ssize_t;
typedef unsigned short mode_t;

#endif /* _MSC_VER */


#include "evil_stdlib.h"
#include "evil_unistd.h"
#include "evil_util.h"


#ifndef __CEGCC__

# include <sys/types.h>

/**
 * @def FD_CLOEXEC
 * Specifies that the file descriptor should be closed when an exec()
 * function is invoked.
 */
# define FD_CLOEXEC 1

/**
 * @def F_SETFD
 * Specifies that fcntl() should set the file descriptor flags
 * associated with the filedes argument.
 */

/**
 * @def F_SETLK
 * Specifies that fcntl() should set or clear a file segment lock
 * according to the lock description pointed to by the third argument.
 */

/**
 * @def F_SETLKW
 * Equivalent to F_SETLK except that if a shared or exclusive lock
 * is blocked by other locks, the thread shall wait until the request
 * can be satisfied.
 */

# define F_SETFD    2
# define F_SETLK    6
# define F_SETLKW   7

/**
 * @def F_RDLCK
 * Read (or shared) lock
 */

/**
 * @def F_WRLCK
 * Write (or exclusive) lock
 */

/**
 * @def F_UNLCK
 * Remove lock
 */

# ifndef F_RDLCK
#  define F_RDLCK     0
#  define F_WRLCK     1
#  define F_UNLCK     2
# endif /* ! F_RDLCK */

/**
 * @struct flock
 * @brief A structure that control the lock of a file descriptor.
 */
struct flock
{
   short int l_type;   /**< lock type: read, write, ... */
   short int l_whence; /**< type of l_start */
   off_t     l_start;  /**< starting offset */
   off_t     l_len;    /**< 0 means end of the file */
   pid_t     l_pid;    /**< lock owner */
};


/**
 * @brief Provide control over file descriptors.
 *
 * @param fd The file descriptor.
 * @param cmd The type of control.
 * @return 0 on success, -1 otherwise.
 *
 * Performs one of various miscellaneous operations on @p fd.
 * The operation in question is determined by @p cmd:
 *
 * - F_SETFD: Set the close-on-exec flag to the value specified
 *   by the argument after command (only the least significant
 *   bit is used).
 * - F_SETLK and F_SETLKW: used to manage discretionary file locks.
 *   The third argument must be a pointer to a struct flock (that
 *   may be overwritten by this call).
 *
 * This function returns 0 on success, -1 otherwise.
 *
 * Conformity: None.
 *
 * Supported OS: Windows Vista, Windows XP or Windows 2000
 * Professional.
 *
 * @ingroup Evil
 */
EAPI int fcntl(int fd, int cmd, ...);


#endif /* ! __CEGCC__ */

typedef int            nl_item;

#define __NL_ITEM( CATEGORY, INDEX )  ((CATEGORY << 16) | INDEX)
#define __NL_ITEM_CATEGORY( ITEM )    (ITEM >> 16)
#define __NL_ITEM_INDEX( ITEM )       (ITEM & 0xffff)

enum {
  /*
   * LC_CTYPE category...
   * Character set classification items.
   */
  _NL_CTYPE_CODESET = __NL_ITEM( LC_CTYPE, 0 ),

  /*
   * Dummy entry, to terminate the list.
   */
  _NL_ITEM_CLASSIFICATION_END
};

/*
 * Define the public aliases for the enumerated classification indices...
 */
# define CODESET       _NL_CTYPE_CODESET

EAPI char *nl_langinfo(nl_item index);


#ifndef __CEGCC__
typedef unsigned long  uid_t;
typedef unsigned long  gid_t;
#endif /* ! __CEGCC__ */

#ifdef _MSC_VER

#define F_OK 0  /* Check for file existence */
#define X_OK 1  /* MS access() doesn't check for execute permission. */
#define W_OK 2  /* Check for write permission */
#define R_OK 4  /* Check for read permission */

#define _S_IRWXU (_S_IREAD | _S_IWRITE | _S_IEXEC)
#define _S_IXUSR _S_IEXEC
#define _S_IWUSR _S_IWRITE
#define _S_IRUSR _S_IREAD

#endif /* _MSC_VER */


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

#  define open(path,...) _open((path),__VA_ARGS__)
#  define close(fd) _close(fd)
#  define read(fd,buffer,count) _read((fd),(buffer),(count))
#  define write(fd,buffer,count) _write((fd),(buffer),(count))
#  define unlink(filename) _unlink((filename))
#  define mkdir(p,m) _mkdir(p)
#  define getcwd(b,s) _getcwd((b),(s))
#  define lstat(f,s) _stat((f),(s))
#  define strdup(s) _strdup(s)

# endif
#endif

#define pipe(fd) evil_pipe(fd)


#ifdef __cplusplus
}
#endif

#endif /* __EVIL_H__ */
