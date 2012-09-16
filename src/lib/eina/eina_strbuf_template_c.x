/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/* This file should be included from files implementing strbuf.
   The including file should define the following macros:
 * _STRBUF_DATA_TYPE
 * _STRBUF_CSIZE
 * _STRBUF_STRUCT_NAME
 * _STRBUF_STRLEN_FUNC(x)
 * _STRBUF_STRESCAPE_FUNC(x)
 * _STRBUF_STRSTR_FUNC(x, y)
 * _STRBUF_MAGIC
 * _STRBUF_MAGIC_STR
 * See how it's done in eina_ustrbuf.c and eina_strbuf.c. This just makes things
 * a lot easier since those are essentially the same just with different sizes.
 */

#include "eina_binbuf_template_c.x"

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI _STRBUF_STRUCT_NAME *
_FUNC_EXPAND(manage_new)(_STRBUF_DATA_TYPE *str)
{
   _STRBUF_STRUCT_NAME *buf = eina_strbuf_common_manage_new(_STRBUF_CSIZE,
	 (void *) str, _STRBUF_STRLEN_FUNC(str));
   EINA_MAGIC_SET(buf, _STRBUF_MAGIC);
   return buf;
}

EAPI Eina_Bool
_FUNC_EXPAND(append)(_STRBUF_STRUCT_NAME *buf, const _STRBUF_DATA_TYPE *str)
{
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);
   return eina_strbuf_common_append(_STRBUF_CSIZE, buf, (const void *) str, _STRBUF_STRLEN_FUNC(str));
}

EAPI Eina_Bool
_FUNC_EXPAND(append_escaped)(_STRBUF_STRUCT_NAME *buf, const _STRBUF_DATA_TYPE *str)
{
   _STRBUF_DATA_TYPE *esc;
   Eina_Bool ret;

   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);
   esc = _STRBUF_STRESCAPE_FUNC(str);
   if (!esc) {
      return _FUNC_EXPAND(append)(buf, str);
   }
   ret = _FUNC_EXPAND(append)(buf, esc);
   if (esc)
      free(esc);

   return ret;
}

EAPI Eina_Bool
_FUNC_EXPAND(append_n)(_STRBUF_STRUCT_NAME *buf, const _STRBUF_DATA_TYPE *str, size_t maxlen)
{
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);
   return eina_strbuf_common_append_n(_STRBUF_CSIZE, buf, (const void *) str, _STRBUF_STRLEN_FUNC(str), maxlen);
}

EAPI Eina_Bool
_FUNC_EXPAND(insert)(_STRBUF_STRUCT_NAME *buf, const _STRBUF_DATA_TYPE *str, size_t pos)
{
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);
   return eina_strbuf_common_insert(_STRBUF_CSIZE, buf, (const void *) str, _STRBUF_STRLEN_FUNC(str), pos);
}

EAPI Eina_Bool
_FUNC_EXPAND(insert_escaped)(_STRBUF_STRUCT_NAME *buf, const _STRBUF_DATA_TYPE *str, size_t pos)
{
   _STRBUF_DATA_TYPE *esc;
   Eina_Bool ret;
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);

   esc = _STRBUF_STRESCAPE_FUNC(str);
   if (!esc) {
      return _FUNC_EXPAND(insert)(buf, str, pos);
   }
   ret = _FUNC_EXPAND(insert)(buf, esc, pos);
   if (esc)
      free(esc);

   return ret;
}

EAPI Eina_Bool
_FUNC_EXPAND(insert_n)(_STRBUF_STRUCT_NAME *buf, const _STRBUF_DATA_TYPE *str, size_t maxlen, size_t pos)
{
   EINA_MAGIC_CHECK_STRBUF(buf, EINA_FALSE);
   return eina_strbuf_common_insert_n(_STRBUF_CSIZE, buf, (const void *) str, _STRBUF_STRLEN_FUNC(str), maxlen, pos);
}

