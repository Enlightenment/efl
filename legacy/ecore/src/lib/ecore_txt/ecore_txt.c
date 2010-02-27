/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore_Txt.h"


/**
 * @deprecated use eina_str_convert() instead.
 */
EAPI char *
ecore_txt_convert(const char *enc_from, const char *enc_to, const char *text)
{
   EINA_LOG_ERR("use eina_str_convert() instead.");
   return eina_str_convert(enc_from, enc_to, text);
}
