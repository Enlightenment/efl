#ifndef __EVIL_PRINT_H__
#define __EVIL_PRINT_H__


#include "evil_macro.h"


EAPI int __cdecl _evil_fprintfa(FILE *, const char *, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_printfa(const char *, ...) __EVIL_PRINTF(1, 2) __EVIL_NOTHROW;
EAPI int __cdecl _evil_sprintfa(char *, const char *, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_snprintfa(char *, size_t, const char *, ...) __EVIL_PRINTF(3, 4) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vfprintfa(FILE *, const char *, va_list) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vprintfa(const char *, va_list) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vsprintfa(char *, const char *, va_list) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vsnprintfa(char *, size_t, const char *, va_list) __EVIL_NOTHROW;

EAPI int __cdecl _evil_fscanf (FILE *, const char *, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_scanf (const char *, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_sscanf (const char *, const char *, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vfscanf(FILE *, const char *, va_list) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vscanf (const char *, va_list) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vsscanf(const char *, const char *, va_list) __EVIL_NOTHROW;

EAPI int __cdecl _evil_asprintf(char ** __restrict__, const char * __restrict__, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vasprintf(char ** __restrict__, const char * __restrict__, va_list) __EVIL_NOTHROW;

EAPI int __cdecl _evil_fprintfw(FILE *, const wchar_t *, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_printfw(const wchar_t *, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_sprintfw(wchar_t *, const wchar_t *, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_snprintfw(wchar_t *, size_t, const wchar_t *, ...) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vfprintfw(FILE *, const wchar_t *, va_list) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vprintfw(const wchar_t *, va_list) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vsprintfw(wchar_t *, const wchar_t *, va_list) __EVIL_NOTHROW;
EAPI int __cdecl _evil_vsnprintfw(wchar_t *, size_t, const wchar_t *, va_list) __EVIL_NOTHROW;

/* Redefine to GNU specific PRI... and SCN... macros.  */
/* #if defined(_INTTYPES_H_) && defined(PRId64) */

#define PRId64 "lld"
#define PRIdLEAST64 "lld"
#define PRIdFAST64 "lld"
#define PRIdMAX "lld"
#define PRIi64 "lli"
#define PRIiLEAST64 "lli"
#define PRIiFAST64 "lli"
#define PRIiMAX "lli"
#define PRIo64 "llo"
#define PRIoLEAST64 "llo"
#define PRIoFAST64 "llo"
#define PRIoMAX "llo"
#define PRIu64 "llu"
#define PRIuLEAST64 "llu"
#define PRIuFAST64 "llu"
#define PRIuMAX "llu"
#define PRIx64 "llx"
#define PRIxLEAST64 "llx"
#define PRIxFAST64 "llx"
#define PRIxMAX "llx"
#define PRIX64 "llX"
#define PRIXLEAST64 "llX"
#define PRIXFAST64 "llX"
#define PRIXMAX "llX"

#define SCNd64 "lld"
#define SCNdLEAST64 "lld"
#define SCNdFAST64 "lld"
#define SCNdMAX "lld"
#define SCNi64 "lli"
#define SCNiLEAST64 "lli"
#define SCNiFAST64 "lli"
#define SCNiMAX "lli"
#define SCNo64 "llo"
#define SCNoLEAST64 "llo"
#define SCNoFAST64 "llo"
#define SCNoMAX "llo"
#define SCNx64 "llx"
#define SCNxLEAST64 "llx"
#define SCNxFAST64 "llx"
#define SCNxMAX "llx"
#define SCNu64 "llu"
#define SCNuLEAST64 "llu"
#define SCNuFAST64 "llu"
#define SCNuMAX "llu"

#ifdef _WIN64
#define PRIdPTR "lld"
#define PRIiPTR "lli"
#define PRIoPTR "llo"
#define PRIuPTR "llu"
#define PRIxPTR "llx"
#define PRIXPTR "llX"

#define SCNdPTR "lld"
#define SCNiPTR "lli"
#define SCNoPTR "llo"
#define SCNxPTR "llx"
#define SCNuPTR "llu"
#endif /* _WIN64 */

/* #endif /\* defined(_INTTYPES_H_) && defined(PRId64) *\/ */


#endif /* __EVIL_PRINT_H__ */
