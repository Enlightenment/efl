#ifndef EINA_STRBUF_H
#define EINA_STRBUF_H

#include <stddef.h>

#include "eina_types.h"

typedef struct _Eina_Strbuf Eina_Strbuf;

EAPI Eina_Strbuf *eina_strbuf_new(void);
EAPI void eina_strbuf_free(Eina_Strbuf *buf);
EAPI void eina_strbuf_append(Eina_Strbuf *buf, const char *str);
EAPI void eina_strbuf_append_char(Eina_Strbuf *buf, char c);
EAPI void eina_strbuf_insert(Eina_Strbuf *buf, const char *str, 
                              size_t pos);
#define eina_strbuf_prepend(buf, str) eina_strbuf_insert(buf, str, 0)
EAPI const char *eina_strbuf_string_get(Eina_Strbuf *buf);
EAPI size_t eina_strbuf_length_get(Eina_Strbuf *buf);
EAPI int eina_strbuf_replace(Eina_Strbuf *buf, const char *str, 
                                 const char *with, unsigned int n);
#define eina_strbuf_replace_first(buf, str, with) \
	eina_strbuf_replace(buf, str, with, 1)
EAPI int eina_strbuf_replace_all(Eina_Strbuf *buf, const char *str,
                                  const char *with);

#endif /* EINA_STRBUF_H */
