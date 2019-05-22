


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
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <locale.h>

#ifdef ENABLE_NLS
# include <libintl.h>
#else
# define dgettext(domain, x) (x)
#endif

#include <Eina.h>

#include "evas_language_utils.h"
#include "evas_bidi_utils.h" /* Used for splitting according to bidi */
#include "../evas_font_ot.h" /* Used for harfbuzz info */

#ifdef USE_HARFBUZZ
# include <hb.h>
#endif

#include "evas_script_table.h"

/* The given 'script' of a character is an EXPLICIT_SCRIPT if it is not one of
 * the three special values: INHERITED, COMMON and UNKNOWN.
 * See http://www.unicode.org/reports/tr24 */
#define EXPLICIT_SCRIPT(script) \
   (((script) != EVAS_SCRIPT_UNKNOWN) && ((script) > EVAS_SCRIPT_INHERITED))

static char lang[6]; /* FIXME: Maximum length I know about */
static char lang_full[32];
static Evas_BiDi_Direction lang_dir = EVAS_BIDI_DIRECTION_NEUTRAL;

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
evas_common_language_char_script_get(Eina_Unicode _unicode)
{
   uint32_t unicode = _unicode;
   if (unicode < EVAS_SCRIPT_DIRECT_TABLE_LIMIT)
      return _evas_script_fast_table[unicode];
   return _evas_common_language_char_script_search(unicode);
}

int
evas_common_language_script_end_of_run_get(const Eina_Unicode *str,
      const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, int len)
{
   /* FIXME: Use the standard segmentation instead */
   Evas_Script_Type first = EVAS_SCRIPT_UNKNOWN;
   int i;

   /* Find the first EXPLICIT_SCRIPT (see documented macro above), and
    * use it to determine the script run */
   for (i = 0 ; (i < len) && !EXPLICIT_SCRIPT(first) ; i++, str++)
     {
        first = evas_common_language_char_script_get(*str);
     }

   /* At this point either (i == len) or 'first' is an EXPLICIT_SCRIPT. */
   /* NOTE: ++i, ++str is to start at the character after 'first', if exists. */
   for ( ; i < len ; ++i, ++str)
     {
        Evas_Script_Type tmp = evas_common_language_char_script_get(*str);
        if (EXPLICIT_SCRIPT(tmp) && (tmp != first))
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
   (void) start;
#endif
   return (i < len) ? i : 0;
}

Evas_Script_Type
evas_common_language_script_type_get(const Eina_Unicode *str, size_t len)
{
   Evas_Script_Type script = EVAS_SCRIPT_COMMON;
   const Eina_Unicode *end = str + len;
   for ( ; (str < end) && !EXPLICIT_SCRIPT(script) ; str++)
     {
        script = evas_common_language_char_script_get(*str);
     }
   return script;
}

const char *
evas_common_language_from_locale_get(void)
{
   if (*lang) return lang;

   const char *locale;
   locale = setlocale(LC_MESSAGES, NULL);
   if (locale && *locale)
     {
        char *itr;
        const size_t size = sizeof(lang);
        strncpy(lang, locale, size - 1);
        lang[size - 1] = '\0';
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

const char *
evas_common_language_from_locale_full_get(void)
{
   if (*lang_full) return lang_full;

   const char *locale;
   locale = setlocale(LC_MESSAGES, NULL);
   if (locale && *locale)
     {
        const size_t size = sizeof(lang_full);
        size_t i;
        for (i = 0 ; locale[i] ; i++)
          {
             const char c = locale[i];
             if ((c == '.') || (c == '@') || (c == ' ')) /* Looks like en_US.UTF8 or de_DE@euro or aa_ER UTF-8*/
                break;
          }

        if (i >= size)
          {
             i = size - 1;
          }

        strncpy(lang_full, locale, i);
        lang_full[i] = '\0';
        return lang_full;
     }

   return "";
}

Evas_BiDi_Direction
evas_common_language_direction_get(void)
{
   if (lang_dir == EVAS_BIDI_DIRECTION_NEUTRAL)
     {
        const char *dir_str = dgettext(PACKAGE, "default:LTR");

        if (dir_str && !strcmp(dir_str, "default:RTL"))
          lang_dir = EVAS_BIDI_DIRECTION_RTL;
        else
          lang_dir = EVAS_BIDI_DIRECTION_LTR;
     }

   return lang_dir;
}

void
evas_common_language_reinit(void)
{
   *lang = *lang_full = '\0';
   lang_dir = EVAS_BIDI_DIRECTION_NEUTRAL;
}

/*
 * @}
 */
/*
 * @}
 */

