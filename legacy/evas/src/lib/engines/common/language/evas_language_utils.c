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

/* FIXME: rename and move */
void *
evas_common_language_unicode_funcs_get(void)
{
#if defined(USE_HARFBUZZ)
   return hb_unicode_funcs_get_default();
#endif
   return NULL;
}

Evas_Script_Type
evas_common_language_char_script_get(Eina_Unicode unicode)
{
#ifdef USE_HARFBUZZ
   static hb_unicode_funcs_t *funcs;
   if (!funcs)
        funcs = evas_common_language_unicode_funcs_get();
   if (funcs)
      return hb_unicode_get_script(funcs, unicode);
#else
   (void) unicode;
#endif
   return EVAS_SCRIPT_COMMON;
}

int
evas_common_language_script_end_of_run_get(const Eina_Unicode *str,
      const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, int len)
{
   /* FIXME: Use the standard segmentation instead */
#ifdef OT_SUPPORT
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
# ifdef BIDI_SUPPORT
     {
        int bidi_end;
        bidi_end = evas_bidi_end_of_run_get(bidi_props, start, len);
        if (bidi_end > 0)
          {
             i = (i < bidi_end) ? i : bidi_end;
          }
     }
# else
   (void) bidi_props;
   (void) start;
# endif
   return (i < len) ? i : 0;
#elif defined(BIDI_SUPPORT)
   (void) str;
   return evas_bidi_end_of_run_get(bidi_props, start, len);
#else
   (void) bidi_props;
   (void) start;
   (void) str;
   (void) len;
   return 0;
#endif
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

