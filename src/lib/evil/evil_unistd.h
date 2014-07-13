#ifndef __EVIL_UNISTD_H__
#define __EVIL_UNISTD_H__


/**
 * @file evil_unistd.h
 * @brief The file that provides functions ported from Unix in unistd.h.
 * @defgroup Evil_Unistd_Group Unistd.h functions
 * @ingroup Evil
 *
 * This header provides functions ported from Unix in unistd.h.
 *
 * @{
 */


/*
 * Time related functions
 *
 */

/**
 * @brief Return the time spent since the Evil library has been initialized.
 *
 * @return The time spent since the Evil library has been initialized.
 *
 * This function returns the time spent since the Evil library has
 * been initialized. It uses a high-resolution timer and then can have
 * a precision up to the nano-second. The precision is processor
 * dependant. This function can be used to benchmark parts of code in
 * with high precision.
 *
 * Conformity: Not appliclable.
 *
 * Supported OS: Windows XP.
 */
EAPI double evil_time_get(void);

#ifdef _MSC_VER

/**
 * @brief Retrieve the time since the Evil library has been
 * initialized.
 *
 * @param tp Structure to fill.
 * @param tzp Unused.
 * @return Always 1.
 *
 * The function fills @p tp with the time spent since the Evil library
 * has been initialized. It uses a high-resolution timer and then can
 * have a precision up to the nano-second. The precision is processor
 * dependant. This function can be used to benchmark parts of code in
 * with high precision. This function always returns 1.
 *
 * Conformity: Not appliclable.
 *
 * Supported OS: Windows XP.
 */
EAPI int evil_gettimeofday(struct timeval * tp, void * tzp);

/**
 * @def gettimeofday(tp,tzp)
 *
 * Wrapper around evil_gettimeofday().
 */
#define gettimeofday(tp,tzp) evil_gettimeofday((tp),(tzp))

/**
 * @brief Suspend the execution of the calling process for microsecond
 * intervals.
 *
 * @param usec The interval in microseconds.
 * @return Always 0 (success).
 *
 * This function suspends the execution of the calling process for (at
 * least) @p usec microseconds.
 *
 * Conformity: Not appliclable.
 *
 * Supported OS: Windows XP.
 */
EAPI int evil_usleep(unsigned long usec);

/**
 * @def usleep(usec)
 *
 * Wrapper around evil_usleep().
 */
#define usleep(usec) evil_usleep(usec);

/*
 * Process identifer related functions
 *
 */

/**
 * @brief Return the process identifier of the calling process.
 *
 * @return The process ID.
 *
 * Return the process identifier of the calling process. Until
 * the process terminates, the process identifier uniquely
 * identifies the process throughout the system.
 *
 * Conformity: Not appliclable.
 *
 * Supported OS: Windows XP.
 */
EAPI pid_t getpid(void);

#endif


/*
 * Symbolic links and directory related functions
 *
 */

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
 * Supported OS: Windows XP.
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
 * On success, this function returns 0. Otherwise, it returns -1 and
 * errno may be set to the following value:
 * - ENOMEM: Not enough memory.
 *
 * On Windows, the symbolic links do not exist. Nevertheless
 * shell links can be managed. This function is named like the Unix
 * function for portability reasons.
 *
 * Conformity: None.
 *
 * Supported OS: Windows XP.
 */
EAPI ssize_t readlink(const char *path, char *buf, size_t bufsiz);


/*
 * file related functions
 *
 */

/**
 * @brief Get the current directory.
 *
 * @param buffer Buffer to store the current directory.
 * @param size Size of the buffer.
 * @return The current directory.
 *
 * Use the _getcwd function in MSVCRT.
 *
 * Conformity: Almost POSIX.1 (no errno set)
 *
 * Supported OS: Windows XP.
 */
EAPI char *evil_getcwd(char *buffer, size_t size);

/**
 * @def getcwd(b,s)
 *
 * Wrapper around evil_getcwd().
 */
#define getcwd(b,s) evil_getcwd((b),(s))

/*
 * Sockets and pipe related functions
 *
 */

/**
 * @brief Initiates the use of Windows sockets.
 *
 * @return 1 on success, 0 otherwise.
 *
 * Initiates the use of Windows sockets. If the function succeeds,
 * it returns 1, otherwise it returns 0.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI int evil_sockets_init(void);

/**
 * @brief Shutdown the Windows socket system.
 *
 * Shutdown the Windows socket system.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI void evil_sockets_shutdown(void);

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
 * Supported OS: Windows XP.
 */
EAPI int evil_pipe(int *fds);

/**
 * @def pipe(fds)
 *
 * Wrapper around evil_pipe().
 */
#define pipe(fds) evil_pipe(fds)


/**
 * @}
 */


#endif /* __EVIL_UNISTD_H__ */
