/* A small helper header defining ELM_API for elementary modules, it should be
 * included last in the modules C files.
 */

#ifndef ELM_MODULE_HELPER_H
#define ELM_MODULE_HELPER_H

#ifdef _WIN32
# ifndef EFL_MODULE_STATIC
#  define EMODAPI __declspec(dllexport)
# else
#  define EMODAPI
# endif
# define EMODAPI_WEAK
#elif defined(__GNUC__)
# if __GNUC__ >= 4
#  define EMODAPI __attribute__ ((visibility("default")))
#  define EMODAPI_WEAK __attribute__ ((weak))
# else
#  define EMODAPI
#  define EMODAPI_WEAK
# endif
#else
# define EMODAPI
# define EMODAPI_WEAK
#endif

#endif
