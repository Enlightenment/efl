#ifndef __EVIL_MACRO_H__
#define __EVIL_MACRO_H__

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


#ifndef PATH_MAX
# define PATH_MAX MAX_PATH
#endif /* PATH_MAX */

#endif /* __EVIL_MACRO_H__ */
