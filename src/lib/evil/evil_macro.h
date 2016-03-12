#ifndef __EVIL_MACRO_H__
#define __EVIL_MACRO_H__

#ifdef EAPI
# undef EAPI
#endif

#ifdef EFL_EVIL_BUILD
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif
#else
# define EAPI __declspec(dllimport)
#endif


#ifndef PATH_MAX
# define PATH_MAX MAX_PATH
#endif

#endif /* __EVIL_MACRO_H__ */
