#ifndef __EVIL_PRIVATE_H__
#define __EVIL_PRIVATE_H__


#include "evil_macro.h"


#ifdef __BUILD_WIDEAPI
#define APICHAR	wchar_t
#else
#define APICHAR char
#endif

/* The following are the declarations specific to the `pformat' API...
 */
#define PFORMAT_TO_FILE     0x1000
#define PFORMAT_NOLIMIT     0x2000


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __BUILD_WIDEAPI
# define __fputc(X,STR)  fputwc((wchar_t) (X), (STR))
# define _evil_pformat   _evil_pformatw

# define _evil_fprintf   _evil_fprintfw
# define _evil_printf    _evil_printfw
# define _evil_snprintf  _evil_snprintfw
# define _evil_sprintf   _evil_sprintfw

# define _evil_vfprintf  _evil_vfprintfw
# define _evil_vprintf   _evil_vprintfw
# define _evil_vsnprintf _evil_vsnprintfw
# define _evil_vsprintf  _evil_vsprintfw
#else
# define __fputc(X,STR)  fputc((X), (STR))
# define _evil_pformat   _evil_pformata

# define _evil_fprintf   _evil_fprintfa
# define _evil_printf    _evil_printfa
# define _evil_snprintf  _evil_snprintfa
# define _evil_sprintf   _evil_sprintfa

# define _evil_vfprintf  _evil_vfprintfa
# define _evil_vprintf   _evil_vprintfa
# define _evil_vsnprintf _evil_vsnprintfa
# define _evil_vsprintf  _evil_vsprintfa
#endif

int __cdecl _evil_pformat(int, void *, int, const APICHAR *, va_list) __EVIL_NOTHROW;


#ifdef __cplusplus
}
#endif


#endif /* __EVIL_PRIVATE_H__ */
