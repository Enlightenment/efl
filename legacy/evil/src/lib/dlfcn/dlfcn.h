#ifndef __E_WIN32_DLFCN_H__
#define __E_WIN32_DLFCN_H__

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
 * @def RTLD_LAZY
 * Lazy function call binding
 */

/**
 * @def RTLD_NOW
 * Immediate function call binding
 */

/**
 * @def RTLD_GLOBAL
 * Symbols in this dlopen'ed obj are visible to other dlopen'ed objs
 */

# define RTLD_LAZY 1   /* lazy function call binding */
# define RTLD_NOW 2    /* immediate function call binding */
# define RTLD_GLOBAL 4 /* symbols in this dlopen'ed obj are visible
			 to other dlopen'ed objs */

/**
 * @file dlfcn.h
 * @brief The file that provides functions to manage dynamic-link libraries
 * @defgroup Dlfcn Functions that manage dynamic-link libraries.
 *
 * This header provides functions to load and unload dynamic-link
 * libaries, to get the address of a symbol, and to get diagnostic
 * information.
 *
 */

/**
 * Map a specified executable module (either a .dll or .exe file)
 * into the address space of the user process.
 *
 * @param path Name of the module.
 * @param mode Unused.
 * @return A pointer that represent the module, or @c NULL on failure.
 *
 * Map a specified executable module (either a .dll or .exe file)
 * into the address space of the user process. If @p path is @c NULL,
 * then the module corresponding to the current process is returned.
 * Otherwise the module specified by @p path is loaded if it exists.
 * If not, @c NULL is returned. The directory separators can be forward
 * slash, or backward ones. Mapping a module can map other modules.
 * @p mode is unused.
 *
 * If an error occurred, an error string can be retrived with dlerror().
 *
 * According to the OS, the search order of the module can change,
 * according to the value of SafeDllSearchMode.
 *
 *  - For Windows Vista, Windows Server 2003, and Windows XP SP2:
 *    SafeDLLSearchMode is enabled by default.
 *  - For Windows XP and Windows 2000 SP4:  SafeDLLSearchMode is disabled
 *    by default.
 *
 * If SafeDllSearchMode is enabled
 *  - The directory from which the application loaded.
 *  - The system directory. Use the GetSystemDirectory() function
 *    to get the path of this directory.
 *  - The 16-bit system directory. There is no function that obtains
 *    the path of this directory, but it is searched.
 *  - The Windows directory. Use the GetWindowsDirectory() function
 *    to get the path of this directory.
 *  - The current directory.
 *  - The directories that are listed in the PATH environment variable.
 *    Note that this does not include the per-application path specified
 *    by the App Paths registry key.
 *
 * If SafeDllSearchMode is disabled
 *  - The directory from which the application loaded.
 *  - The current directory.
 *  - The system directory. Use the GetSystemDirectory() function
 *    to get the path of this directory.
 *  - The 16-bit system directory. There is no function that obtains
 *    the path of this directory, but it is searched.
 *  - The Windows directory. Use the GetWindowsDirectory() function
 *    to get the path of this directory.
 *  - The directories that are listed in the PATH environment variable.
 *    Note that this does not include the per-application path specified
 *    by the App Paths registry key.
 *
 * Conformity: None.
 *
 * Supported OS: Windows Vista, Windows XP or Windows 2000
 * Professional.
 *
 * @ingroup Dlfcn
 */
EAPI void *dlopen(const char* path, int mode);

/**
 * Close a dynamic-link library.
 *
 * @param handle Handle that references a dynamic-link library.
 * @return O on sucess, -1 otherwise.
 *
 * Release a reference to the dynamic-link library referenced
 * by @p handle.  If the reference count drops to 0, the handle is
 * removed from the address space and is rendered invalid. @p handle
 * is the value returned by a previous call to dlopen().
 *
 * If no error occurred, the returned value is 0, otherwise the
 * returned value is -1 and an error string can be retrived with
 * dlerror().
 *
 * Conformity: None.
 *
 * Supported OS: Windows Vista, Windows XP or Windows 2000
 * Professional.
 *
 * @ingroup Dlfcn
 */
EAPI int   dlclose(void* handle);

/**
 * Get the address of a symbol.
 *
 * @param handle Handle that references a dynamic-link library.
 * @param symbol @c NULL-terminated string.
 * @return O on sucess, NULL otherwise.
 *
 * Return the address of the code or data location specified by the
 * string @p symbol.  @p handle references a library that contains
 * the function or variable @p symbol.
 *
 * If no error occurred, the returned value is the code or data
 * location, otherwise the returned value is NULL and an error
 * string can be retrived with dlerror().
 *
 * Conformity: None.
 *
 * Supported OS: Windows Vista, Windows XP or Windows 2000
 * Professional.
 *
 * @ingroup Dlfcn
 */
EAPI void *dlsym(void* handle, const char* symbol);

/**
 * Get diagnostic information
 *
 * @return A @c NULL-terminated string if an error occured, @c NULL
 * otherwise.
 *
 * Return a @c NULL-terminated character string describing the last
 * error that occurred on this thread during a call to dlopen(),
 * dlsym(), or dlclose(). If no such error has occurred, dlerror()
 * returns a null pointer.  At each call to dlerror(), the error
 * indication is reset.  Thus in the case of two calls to dlerror(),
 * where the second call follows the first immediately, the second
 * call will always return a null pointer.
 *
 * Conformity: None.
 *
 * Supported OS: Windows Vista, Windows XP or Windows 2000
 * Professional.
 *
 * @ingroup Dlfcn
 */
EAPI char *dlerror (void);


#ifdef __cplusplus
}
#endif

#ifdef _WIN32
# undef EAPI
# define EAPI
#endif /* _WIN32 */

#endif /* __E_WIN32_DLFCN_H__ */
