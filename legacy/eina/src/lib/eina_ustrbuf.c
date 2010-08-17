#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_strbuf_common.h"
#include "eina_unicode.h"
#include "eina_ustrbuf.h"


#define _STRBUF_DATA_TYPE         Eina_Unicode
#define _STRBUF_CSIZE             sizeof(_STRBUF_DATA_TYPE)
#define _STRBUF_STRUCT_NAME       Eina_UStrbuf
#define _STRBUF_STRLEN_FUNC(x)    eina_unicode_strlen(x)
#define _STRBUF_STRESCAPE_FUNC(x) eina_unicode_escape(x)
#define _STRBUF_MAGIC             EINA_MAGIC_USTRBUF
#define _STRBUF_MAGIC_STR         __USTRBUF_MAGIC_STR
static const char __USTRBUF_MAGIC_STR[] = "Eina UStrbuf";

#define _FUNC_EXPAND(y) eina_ustrbuf_ ## y

#include "eina_strbuf_template_c.x"
