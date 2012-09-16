#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_private.h"
#include "eina_strbuf_common.h"
#include "eina_binbuf.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#ifdef _STRBUF_DATA_TYPE
# undef _STRBUF_DATA_TYPE
#endif

#ifdef _STRBUF_CSIZE
# undef _STRBUF_CSIZE
#endif

#ifdef _STRBUF_STRUCT_NAME
# undef _STRBUF_STRUCT_NAME
#endif

#ifdef _STRBUF_MAGIC
# undef _STRBUF_MAGIC
#endif

#ifdef _STRBUF_MAGIC_STR
# undef _STRBUF_MAGIC_STR
#endif

#ifdef _FUNC_EXPAND
# undef _FUNC_EXPAND
#endif


#define _STRBUF_DATA_TYPE         unsigned char
#define _STRBUF_CSIZE             sizeof(_STRBUF_DATA_TYPE)
#define _STRBUF_STRUCT_NAME       Eina_Binbuf
#define _STRBUF_MAGIC             EINA_MAGIC_BINBUF
#define _STRBUF_MAGIC_STR         __BINBUF_MAGIC_STR
static const char __BINBUF_MAGIC_STR[] = "Eina Binbuf";

#define _FUNC_EXPAND(y) eina_binbuf_ ## y

#include "eina_binbuf_template_c.x"

/**
 * @endcond
 */
