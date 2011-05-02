/**
 * @internal
 * @addtogroup Evas_Utils
 *
 * @{
 */
/**
 * @internal
 * @defgroup Evas_Script Evas Script (language) utility functions
 *
 * This set of functions and types helps evas handle scripts correctly.
 * @todo Document types, structures and macros.
 *
 * @{
 */
#include <Eina.h>

#include "evas_language_utils.h"
#include "evas_bidi_utils.h" /* Used for fallback. */
#include "../evas_font_ot.h" /* Used for harfbuzz info */

#ifdef USE_HARFBUZZ
# include <hb.h>
#endif

#include "evas_script_table.h"

static Evas_Script_Type
_evas_common_language_char_script_search(Eina_Unicode unicode)
{
   int min = 0;
   int max  = (sizeof(_evas_script_slow_table) /
      sizeof(_evas_script_slow_table[0])) - 1;
   int mid;

   do
     {
        mid = (min + max) / 2;

        if (unicode < _evas_script_slow_table[mid].start)
           max = mid - 1;
        else if (unicode >= _evas_script_slow_table[mid].start +
              _evas_script_slow_table[mid].len)
           min = mid + 1;
        else
           return _evas_script_slow_table[mid].script;
     }
   while (min <= max);

   return EVAS_SCRIPT_UNKNOWN;
}

Evas_Script_Type
evas_common_language_char_script_get(Eina_Unicode unicode)
{
   if (unicode < EVAS_SCRIPT_DIRECT_TABLE_LIMIT)
      return _evas_script_fast_table[unicode];
   else
      return _evas_common_language_char_script_search(unicode);
}

int
evas_common_language_script_end_of_run_get(const Eina_Unicode *str,
      const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, int len)
{
   /* FIXME: Use the standard segmentation instead */
   Evas_Script_Type first = EVAS_SCRIPT_UNKNOWN;
   int i;
   for (i = 0 ; i < len ; i++, str++)
     {
        Evas_Script_Type tmp;
        tmp = evas_common_language_char_script_get(*str);
        /* Arabic is the first script in the array that's not
         * common/inherited. */
        if ((first == EVAS_SCRIPT_UNKNOWN) && (tmp >= EVAS_SCRIPT_ARABIC))
          {
             first = tmp;
             continue;
          }
        if ((first != tmp) && (tmp >= EVAS_SCRIPT_ARABIC))
          {
             break;
          }
     }
#ifdef BIDI_SUPPORT
     {
        int bidi_end;
        bidi_end = evas_bidi_end_of_run_get(bidi_props, start, len);
        if (bidi_end > 0)
          {
             i = (i < bidi_end) ? i : bidi_end;
          }
     }
#else
   (void) bidi_props;
#endif
   return (i < len) ? i : 0;
}

Evas_Script_Type
evas_common_language_script_type_get(const Eina_Unicode *str, size_t len)
{
   Evas_Script_Type script = EVAS_SCRIPT_COMMON;
   const Eina_Unicode *end = str + len;
   /* Arabic is the first script in the array that's not a common/inherited */
   for ( ; str < end && ((script = evas_common_language_char_script_get(*str)) < EVAS_SCRIPT_ARABIC) ; str++)
     ;
   return script;
}

const char *
evas_common_language_from_locale_get(void)
{
   static char lang[6]; /* FIXME: Maximum length I know about */
   if (*lang) return lang;

   const char *locale;
   locale = getenv("LANG");
   if (locale && *locale)
     {
        char *itr;
        strncpy(lang, locale, 5);
        lang[5] = '\0';
        itr = lang;
        while (*itr)
          {
             if (*itr == '_')
               {
                  *itr = '\0';
               }
             itr++;
          }
        return lang;
     }

   return "";
}

/*
 * @}
 */
/*
 * @}
 */

