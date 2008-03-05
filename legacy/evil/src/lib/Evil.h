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

#ifdef __cplusplus
extern "C" {
#endif


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

#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>

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
 * - EINVAL: @p template has an invalid format.
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

/**
 * @brief Create a pair of sockets.
 *
 * @param fds A pointer that contains two sockets.
 *
 * Create a pair of sockets that can be use with select().
 * Hence, evil_sockets_init() must have been caled at least
 * once before. Contrary to Unix, that functions does not
 * create a pair of file descriptors.
 *
 * Conformity: Not applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int pipe(int *fds);

#endif /* ! __CEGCC__ */

#if defined(__MSDOS__) || defined(__EMX__) || \
   (defined(_WIN32) && !defined(_UWIN) && !defined(__CYGWIN__) && !defined(__CEGCC__))
# if defined(_MSC_VER) || defined(__MINGW32__)
#  define S_IRGRP S_IRUSR
#  define S_IROTH S_IRUSR
#  define S_IWGRP S_IWUSR
#  define S_IWOTH S_IWUSR
#  define S_IXGRP S_IXUSR
#  define S_IXOTH S_IXUSR
#  define open(path,flag,mode) _open((path),(flag),(mode))
#  define close(fd) _close(fd)
#  define read(fd,buffer,count) _read((fd),(buffer),(count))
#  define write(fd,buffer,count) _write((fd),(buffer),(count))
#  define unlink(filename) _unlink((filename))
#  define mkdir(p,m) _mkdir(p)
# endif
#endif

#if ! ( defined(__CEGCC__) || defined(__MINGW32CE__) )
/**
 * @brief Return an absolute or full path name for a specified relative path name.
 *
 * @param file_name The absolute path name.
 * @param resolved_name The relative path name.
 * @return @c NULL on failure, a pointer to the absolute path name otherwise.
 *
 * The function expands the relative path name @p file_name to its
 * fully qualified or absolute path and store it in the buffer pointed
 * by @p resolved_name. The buffer is at most @c PATH_MAX bytes long.
 * If @p resolved_name is @c NULL, malloc() is used to allocate a
 * buffer of sufficient length to hold the path name. In that case, it
 * is the responsability of the caller to free this buffer with free().
 *
 * That function can be used to obtain the absolute path name for
 * relative paths (relPath) that include "./" or "../" in their names.
 *
 * Conformity: None.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI char *realpath(const char *file_name, char *resolved_name);
#endif /* ! __CEGCC__  && ! __MINGW32CE__ */

/**
 * @brief Initiates the use of Windows sockets.
 *
 * @return 1 on success, 0 otherwise.
 *
 * Initiates the use of Windows sockets. If the function succeeds,
 * it returns 1, otherwise it return 0.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int evil_sockets_init(void);

/**
 * @brief Shutdown the Windows socket system.
 *
 * Shutdown the Windows socket system.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI void evil_sockets_shutdown(void);

/**
 * @brief Return a dir to store temporary files.
 *
 * @return The directory to store temporary files.
 *
 * Return a directory to store temporary files. The function gets
 * the value of the followig environment variables, and in that order:
 * - TMP
 * - TEMP
 * - USERPROFILE
 * - WINDIR
 * and returns its value if it exists. If none exists, the function
 * returns "C:\".
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI const char *evil_tmpdir_get(void);


#ifdef __cplusplus
}
#endif

#ifdef _WIN32
# undef EAPI
# define EAPI
#endif /* _WIN32 */

#endif /* __EVIL_H__ */
