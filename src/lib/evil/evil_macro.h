#ifndef __EVIL_MACRO_H__
#define __EVIL_MACRO_H__


#ifndef __cdecl
# define EVIL_CDECL_IS_DEFINED
# ifdef __GNUC__
#  define __cdecl __attribute__((__cdecl__))
# else
#  define __cdecl
# endif
#endif /* __cdecl */


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


#ifndef __EVIL_GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __EVIL_GNUC_PREREQ( major, minor )\
     (__GNUC__ > (major) || (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
# else
#  define __EVIL_GNUC_PREREQ( major, minor )
# endif
#endif /* __EVIL_GNUC_PREREQ */


#ifndef  __EVIL_NOTHROW
# if __EVIL_GNUC_PREREQ( 3, 3 )
#  define __EVIL_NOTHROW  __attribute__((__nothrow__))
# else
#  define __EVIL_NOTHROW
# endif
#endif /* __EVIL_NOTHROW */


#ifndef  __EVIL_PRINTF
# if __EVIL_GNUC_PREREQ( 2, 4 )
#  define __EVIL_PRINTF(fmt, arg) __attribute__((__format__ (__gnu_printf__, fmt, arg)))
# else
#  define __EVIL_PRINTF(fmt, arg)
# endif
#endif /* __EVIL_PRINTF */


#ifndef PATH_MAX
# define PATH_MAX MAX_PATH
#endif /* PATH_MAX */
#ifdef fprintf
# undef fprintf
#endif

#ifdef printf
# undef printf
#endif

#ifdef sprintf
# undef sprintf
#endif

#ifdef snprintf
# undef snprintf
#endif

#ifdef vfprintf
# undef vfprintf
#endif

#ifdef vprintf
# undef vprintf
#endif

#ifdef vsprintf
# undef vsprintf
#endif

#ifdef vsnprintf
# undef vsnprintf
#endif

#ifdef fscanf
# undef fscanf
#endif

#ifdef scanf
# undef scanf
#endif

#ifdef sscanf
# undef sscanf
#endif

#ifdef vfscanf
# undef vfscanf
#endif

#ifdef vscanf
# undef vscanf
#endif

#ifdef vsscanf
# undef vsscanf
#endif

#ifdef asprintf
# undef asprintf
#endif

#ifdef vasprintf
# undef vasprintf
#endif


#if defined(_INTTYPES_H_) && defined(PRId64)

#undef PRId64
#undef PRIdLEAST64
#undef PRIdFAST64
#undef PRIdMAX
#undef PRIi64
#undef PRIiLEAST64
#undef PRIiFAST64
#undef PRIiMAX
#undef PRIo64
#undef PRIoLEAST64
#undef PRIoFAST64
#undef PRIoMAX
#undef PRIu64
#undef PRIuLEAST64
#undef PRIuFAST64
#undef PRIuMAX
#undef PRIx64
#undef PRIxLEAST64
#undef PRIxFAST64
#undef PRIxMAX
#undef PRIX64
#undef PRIXLEAST64
#undef PRIXFAST64
#undef PRIXMAX

#undef SCNd64
#undef SCNdLEAST64
#undef SCNdFAST64
#undef SCNdMAX
#undef SCNi64
#undef SCNiLEAST64
#undef SCNiFAST64
#undef SCNiMAX
#undef SCNo64
#undef SCNoLEAST64
#undef SCNoFAST64
#undef SCNoMAX
#undef SCNx64
#undef SCNxLEAST64
#undef SCNxFAST64
#undef SCNxMAX
#undef SCNu64
#undef SCNuLEAST64
#undef SCNuFAST64
#undef SCNuMAX

#ifdef _WIN64
#undef PRIdPTR
#undef PRIiPTR
#undef PRIoPTR
#undef PRIuPTR
#undef PRIxPTR
#undef PRIXPTR

#undef SCNdPTR
#undef SCNiPTR
#undef SCNoPTR
#undef SCNxPTR
#undef SCNuPTR
#endif /* _WIN64 */

#endif /* defined(_INTTYPES_H_) && defined(PRId64) */

#endif /* __EVIL_MACRO_H__ */
