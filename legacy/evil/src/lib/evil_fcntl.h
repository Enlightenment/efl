#ifndef __EVIL_FCNTL_H__
#define __EVIL_FCNTL_H__


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


#endif /* __EVIL_FCNTL_H__ */
