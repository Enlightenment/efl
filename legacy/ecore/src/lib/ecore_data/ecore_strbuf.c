/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore_Data.h"

/**
 * Create a new string buffer
 *
 * @deprecated use eina_strbuf_new() instead.
 */
EAPI Ecore_Strbuf *
ecore_strbuf_new(void)
{
   return eina_strbuf_new();
}

/**
 * Free a string buffer
 * @param buf the buffer to free
 *
 * @deprecated use eina_strbuf_free() instead.
 */
EAPI void
ecore_strbuf_free(Ecore_Strbuf *buf)
{
   eina_strbuf_free(buf);
}

/**
 * Append a string to a buffer, reallocating as necessary.
 * @param buf the Ecore_Strbuf to append to
 * @param str the string to append
 *
 * @deprecated use eina_strbuf_append() instead.
 */
EAPI void
ecore_strbuf_append(Ecore_Strbuf *buf, const char *str)
{
   eina_strbuf_append(buf, str);
}

/**
 * Insert a string to a buffer, reallocating as necessary.
 * @param buf the Ecore_Strbuf to insert
 * @param str the string to insert
 * @param pos the position to insert the string
 *
 * @deprecated use eina_strbuf_insert() instead.
 */
EAPI void
ecore_strbuf_insert(Ecore_Strbuf *buf, const char *str, size_t pos)
{
   eina_strbuf_insert(buf, str, pos);
}

/**
 * Append a character to a string buffer, reallocating as necessary.
 * @param buf the Ecore_Strbuf to append to
 * @param c the char to append
 *
 * @deprecated use eina_strbuf_append_char() instead.
 */
EAPI void
ecore_strbuf_append_char(Ecore_Strbuf *buf, char c)
{
   eina_strbuf_append_char(buf, c);
}

/**
 * Retrieve a pointer to the contents of a string buffer
 * @param buf the buffer
 *
 * This pointer must not be modified, and will no longer be valid if
 * the Ecore_Strbuf is modified.
 *
 * @deprecated use eina_strbuf_string_get() instead.
 */
EAPI const char *
ecore_strbuf_string_get(Ecore_Strbuf *buf)
{
   return eina_strbuf_string_get(buf);
}

/**
 * Retrieve the length of the string buffer content
 * @param buf the buffer
 *
 * @deprecated use eina_strbuf_length_get() instead.
 */
EAPI size_t
ecore_strbuf_length_get(Ecore_Strbuf *buf)
{
   return eina_strbuf_length_get(buf);
}

/**
 * Replace the n-th string with an other string.
 * @param buf the Ecore_Strbuf to work with
 * @param str the string to replace
 * @param with the replaceing string
 * @param n the number of the fitting string
 *
 * @return true on success
 *
 * @deprecated use eina_strbuf_replace() instead.
 */
EAPI int
ecore_strbuf_replace(Ecore_Strbuf *buf, const char *str, const char *with,
                     unsigned int n)
{
   return eina_strbuf_replace(buf, str, with, n);
}

/**
 * Replace all strings with an other string.
 * @param buf the Ecore_Strbuf to work with
 * @param str the string to replace
 * @param with the replaceing string
 *
 * @return how often the string was replaced
 *
 * @deprecated use eina_strbuf_replace_all() instead.
 */
EAPI int
ecore_strbuf_replace_all(Ecore_Strbuf *buf, const char *str, const char *with)
{
   return eina_strbuf_replace_all(buf, str, with);
}
