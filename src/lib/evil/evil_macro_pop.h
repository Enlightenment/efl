#ifndef __EVIL_MACRO_POP_H__
#define __EVIL_MACRO_POP_H__


#ifdef EVIL_CDECL_IS_DEFINED
# undef __cdecl
# undef EVIL_CDECL_IS_DEFINED
#endif

#define fprintf   _evil_fprintfa
#define printf    _evil_printfa
#define snprintf  _evil_snprintfa
#define sprintf   _evil_sprintfa
#define vfprintf  _evil_vfprintfa
#define vprintf   _evil_vprintfa
#define vsnprintf _evil_vsnprintfa
#define vsprintf  _evil_vsprintfa

#define fscanf    _evil_fscanf
#define scanf     _evil_scanf
#define sscanf    _evil_sscanf
#define vfscanf   _evil_vfscanf
#define vscanf    _evil_vscanf
#define vsscanf   _evil_vsscanf

#define asprintf  _evil_asprintf
#define vasprintf _evil_vasprintf

/* Redefine to GNU specific PRI... and SCN... macros.  */

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


#endif /* __EVIL_MACRO_POP_H__ */
