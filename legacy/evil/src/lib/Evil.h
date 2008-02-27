#ifndef __E_WIN32_H__
#define __E_WIN32_H__

#ifdef EAPI
# undef EAPI
#endif /* EAPI */

#ifdef _WIN32
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#endif /* _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @mainpage Evil
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

#include <stdlib.h>
#include <sys/time.h>

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

/**
 * @brief Make temporay unique file name.
 *
 * @param template Template of the file to create.
 * @return A file descriptor on success, -1 otherwise.
 *
 * Take the given file name @p template and overwrite a portion of it
 * to create a file name. This file is guaranted not to exist at the
 * time invocation and is suitable for use by the function.
 *
 * The @p template parameter can be any file name with some number of
 * 'Xs' appended to it, for example @em baseXXXXXX, where @em base is
 * the part of the new file that you supply and eacg 'X' is a placeholder
 * for a character supplied by mkstemp(). The trailing 'Xs' are replaced
 * with a five-digit value; this value is a unique number. Each successful
 * call to mkstemp() modifes @p template.
 *
 * When mkstemp() succeeds, it creates and opens the template file for
 * reading and writing.
 *
 * On success, the function returns the file descriptor of the
 * temporary file. Otherwise, it returns -1 and errno is set to the
 * following values:
 * - EACCESS: Given path is a directory, or file is read-only, but an
 * open-for-writing operation was attempted.
 * - EEXISTS: File name already exists.
 * - EMFILE: No more file descriptors available.
 * - ENOENT: File or path not found.
 *
 * Conformity: Should follow BSD conformity.
 *
 * Supported OS: Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int mkstemp(char *template);

/**
 * @brief Create a shell link.
 *
 * @param oldpath The file name to be linked.
 * @param newpath The file name to create.
 * @return 0 on success, -1 otherwise.
 *
 * Create a shell link @p newpath to @p oldpath (@p newpath is the
 * name of the file created, @p oldpath is the string used in
 * creating the shell link).
 *
 * On success, this function returns 0. Otherwise, it returns -1 and
 * errno may be set to the following value:
 * - ENOMEM: Not enough memory.
 *
 * On Windows, the symbolic links do not exist. Nevertheless
 * shell links can be created. This function is named like the Unix
 * function for portability reasons.
 *
 * Conformity: None.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int symlink(const char *oldpath, const char *newpath);

/**
 * @brief Read value of a shell link.
 *
 * @param path The file name to be linked.
 * @param buf The file name to create.
 * @param bufsiz The size of the buffer.
 * @return 0 on success, -1 otherwise.
 *
 * Place the content of the shell link @p path in the buffer
 * @p buf, which has size @p bufzsiz.
 *
 * On success, this function returns the count of characters
 * placed in the buffer. Otherwise, it returns -1 and errno may
 * be set to the following value:
 * - ENOMEM: Not enough memory.
 *
 * On Windows, the symbolic links do not exist. Nevertheless
 * shell links can be managed. This function is named like the Unix
 * function for portability reasons.
 *
 * Conformity: None.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI ssize_t readlink(const char *path, char *buf, size_t bufsiz);

EAPI int pipe(int *fds);

#endif /* ! __CEGCC__ */

#if defined(__MSDOS__) || defined(__EMX__) || \
   (defined(_WIN32) && !defined(_UWIN) && !defined(__CYGWIN__) && !defined(__CEGCC__))
# if defined(_MSC_VER) || defined(__MINGW32__)
#  define open(path,flag,mode) EAPI _open((path),(flag),(mode))
#  define close(fd) EAPI _close(fd)
#  define read(fd,buffer,count) EAPI _read((fd),(buffer),(count))
#  define write(fd,buffer,count) EAPI _write((fd),(buffer),(count))
#  define mkdir(p,m) EAPI _mkdir(p)
# endif
#endif

#define realpath(file_name, resolved_name) EAPI _fullpath((resolved_name), (file_name), PATH_MAX)

EAPI char *evil_tmpdir_get(void);


#ifdef __cplusplus
}
#endif

#endif /* __E_WIN32_H__ */
