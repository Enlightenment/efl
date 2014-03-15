#ifndef __EVIL_STDIO_H__
#define __EVIL_STDIO_H__


/**
 * @file evil_stdio.h
 * @brief The file that provides functions ported from Unix in stdio.h.
 * @defgroup Evil_Stdio_Group Stdio.h functions
 * @ingroup Evil
 *
 * This header provides functions ported from Unix in stdio.h.
 *
 * 
 */

#define EVIL_PATH_SEP_SWAP(p, s1, s2) \
   do { \
     char *_evil_path_tmp; \
     _evil_path_tmp = p; \
     while (*_evil_path_tmp) \
       { \
          if (*_evil_path_tmp == s1) \
            *_evil_path_tmp = s2; \
          _evil_path_tmp++; \
       } \
   } while (0)

#define EVIL_PATH_SEP_WIN32_TO_UNIX(p) EVIL_PATH_SEP_SWAP(p, '\\', '/')
#define EVIL_PATH_SEP_UNIX_TO_WIN32(p) EVIL_PATH_SEP_SWAP(p, '/', '\\')

#ifdef _WIN32_WCE

/*
 * Error related functions
 *
 */

/**
 * @brief Print the given string to stderr.
 *
 * @param s The string to print.
 *
 * This function just printf the string @p s to stderr.
 *
 * Conformity: None.
 *
 * Supported OS: Windows CE.
 */
EAPI void evil_perror (const char *s);

/**
 * @def perror(s)
 *
 * Wrapper around evil_perror().
 */
# define perror(s) evil_perror(s)

/*
 * Stream related functions
 *
 */

/**
 * @brief Emulate the fopen() function on Windows CE.
 *
 * @param path The file to open.
 * @param mode The mode.
 * @return A FILE pointer on success, @c NULL otherwise.
 *
 * This function emulates the fopen() function on Windows CE using the
 * Windows libc. The main problem is that the filesytem on Windows CE
 * is like on Sys V : the top level directory is beginning by '/' or
 * '\' and the full path must be calculated.. This function takes care
 * of this feature, and replace all the '/' by '/'. Otherwise, it
 * calls the Windows CE fopen() function once the filename has been
 * correctly set. If @p path is @c NULL or empty, this function
 * returns @c NULL. On success, this function returns a FILE stream,
 * @c NULL otherwise.
 *
 * Conformity: None.
 *
 * Supported OS: Windows CE.
 */
EAPI FILE *evil_fopen(const char *path, const char *mode);

/**
 * @def fopen(path, mode)
 *
 * Wrapper around evil_fopen().
 */
# define fopen(path, mode) evil_fopen(path, mode)

/**
 * @brief Emulate the rewind() function on Windows CE.
 *
 * @param stream The FILE stream.
 *
 * This function emulates the rewind() function on Windows CE by just
 * calling fseek().
 *
 * Conformity: None.
 *
 * Supported OS: Windows CE.
 */
EAPI void evil_rewind(FILE *stream);

/**
 * @def rewind(f)
 *
 * Wrapper around evil_rewind().
 */
# define rewind(f) evil_rewind(f)

/**
 * @brief Emulate the remove() function on Windows CE.
 *
 * @param path The path to remove.
 * @return 0 on success, -1 otherwise.
 *
 * This function emulates the remove() function on Windows CE. If
 * @p path is an empty directory, it removes it. If @p path is a file,
 * it deletes it. On success, 0 is returns, -1 otherwise.
 *
 * Conformity: None.
 *
 * Supported OS: Windows CE.
 */
EAPI int evil_remove(const char *path);

/**
 * @def remove(p)
 *
 * Wrapper around evil_remove().
 */
# define remove(p) evil_remove(p)

#endif /* _WIN32_WCE */


#ifdef _WIN32_WCE

/**
 * @brief Emulate the fopen() function on Windows CE using Windows API.
 *
 * @param path The file to open.
 * @param mode The mode.
 * @return A FILE pointer on success, @c NULL otherwise.
 *
 * This function emulates the fopen() function on Windows CE using the
 * Windows API and not the libc. It is similar to evil_fopen().
 *
 * @see evil_fopen()
 * @see evil_fread_native()
 * @see evil_fclose_native()
 *
 * Conformity: None.
 *
 * Supported OS: Windows CE.
 */
EAPI FILE *evil_fopen_native(const char *path, const char *mode);

/**
 * @brief Read the given stream on Windows CE using Windows API.
 *
 * @param buffer The buffer to store the data.
 * @param size The size of each element of data to read.
 * @param count The number of elements of data to read.
 * @param stream The stream to read.
 * @return The size read on success, 0 otherwise.
 *
 * This function read @p stream using the Windows API. It reads
 * @p size elements of data, each  @ size bytes long. The data is
 * stored in the buffer @p buffer. On success, the size read is
 * returned, 0 otherwise.
 *
 * @see evil_fopen_native()
 * @see evil_fclose_native()
 *
 * Conformity: None.
 *
 * Supported OS: Windows CE.
 */
EAPI size_t evil_fread_native(void* buffer, size_t size, size_t count, FILE* stream);

/**
 * @brief Close the given stream on Windows CE using Windows API.
 *
 * @param stream The stream to close.
 * @return 0 on success, -1 otherwise.
 *
 * This function closes @p stream using the Windows API. On success, 0
 * is returned, -1 otherwise.
 *
 * @see evil_fopen_native()
 * @see evil_fread_native()
 *
 * Conformity: None.
 *
 * Supported OS: Windows CE.
 */
EAPI int evil_fclose_native(FILE *stream);

#endif /* _WIN32_WCE */


/**
 * @brief Emulate the rename() function on Windows.
 *
 * @param src The old pathname.
 * @param dst The new pathname.
 * @return 0 on success, -1 otherwise.
 *
 * This function emulates the POSIX rename() function on Windows.
 * The difference with the POSIX function is that the rename() function
 * on windows fails if the destination exists.
 *
 * @since 1.8
 */
EAPI int evil_rename(const char *src, const char *dst);

/**
 * @def rename(src, dest)
 *
 * Wrapper around evil_rename().
 */
# define rename(src, dst) evil_rename(src, dst)


#endif /* __EVIL_STDIO_H__ */
