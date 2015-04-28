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
