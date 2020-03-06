#ifndef EVIL_EAPI
#define EVIL_EAPI

#ifdef EAPI
# undef EAPI
#endif

#ifdef EFL_BUILD
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif
#else
# define EAPI __declspec(dllimport)
#endif

#endif
