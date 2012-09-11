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


#endif /* __EVIL_PRINT_H__ */
