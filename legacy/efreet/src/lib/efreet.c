/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

static int init = 0;
static int efreet_parsed_locale = 0;
static char *efreet_lang = NULL;
static char *efreet_lang_country = NULL;
static char *efreet_lang_modifier = NULL;

static void efreet_parse_locale(void);
static int efreet_parse_locale_setting(const char *env);

/**
 * @return Returns > 0 if the initialization was successful, 0 otherwise
 * @brief Initializes the Efreet system
 */
EAPI int
efreet_init(void)
{
    if (init++) return init;
    if (!eina_stringshare_init()) return --init;
    if (!efreet_base_init()) return --init;
    if (!efreet_xml_init()) return --init;
    if (!efreet_icon_init()) return --init;
    if (!efreet_ini_init()) return --init;
    if (!efreet_desktop_init()) return --init;
    if (!efreet_menu_init()) return --init;
    return init;
}

/**
 * @return Returns the number of times the init function as been called
 * minus the corresponding init call.
 * @brief Shuts down Efreet if a balanced number of init/shutdown calls have
 * been made
 */
EAPI int
efreet_shutdown(void)
{
    if (--init) return init;
    efreet_menu_shutdown();
    efreet_desktop_shutdown();
    efreet_ini_shutdown();
    efreet_icon_shutdown();
    efreet_xml_shutdown();
    efreet_base_shutdown();
    eina_stringshare_shutdown();

    IF_FREE(efreet_lang);
    IF_FREE(efreet_lang_country);
    IF_FREE(efreet_lang_modifier);
    efreet_parsed_locale = 0;  /* reset this in case they init efreet again */

    return init;
}

/**
 * @internal
 * @return Returns the current users language setting or NULL if none set
 * @brief Retrieves the current language setting
 */
const char *
efreet_lang_get(void)
{
    if (efreet_parsed_locale) return efreet_lang;

    efreet_parse_locale();
    return efreet_lang;
}

/**
 * @internal
 * @return Returns the current language country setting or NULL if none set
 * @brief Retrieves the current country setting for the current language or
 */
const char *
efreet_lang_country_get(void)
{
    if (efreet_parsed_locale) return efreet_lang_country;

    efreet_parse_locale();
    return efreet_lang_country;
}

/**
 * @internal
 * @return Returns the current language modifier setting or NULL if none
 * set.
 * @brief Retrieves the modifier setting for the language.
 */
const char *
efreet_lang_modifier_get(void)
{
    if (efreet_parsed_locale) return efreet_lang_modifier;

    efreet_parse_locale();
    return efreet_lang_modifier;
}

/**
 * @internal
 * @return Returns no value
 * @brief Parses out the language, country and modifer setting from the
 * LC_MESSAGES environment variable
 */
static void
efreet_parse_locale(void)
{
    efreet_parsed_locale = 1;

    if (efreet_parse_locale_setting("LC_ALL"))
        return;

    if (efreet_parse_locale_setting("LC_MESSAGES"))
        return;

    efreet_parse_locale_setting("LANG");
}

/**
 * @internal
 * @param env: The environment variable to grab
 * @return Returns 1 if we parsed something of @a env, 0 otherwise
 * @brief Tries to parse the lang settings out of the given environment
 * variable
 */
static int
efreet_parse_locale_setting(const char *env)
{
    int found = 0;
    char *setting;
    char *p;

    setting = getenv(env);
    if (!setting) return 0;
    setting = strdup(setting);

    /* pull the modifier off the end */
    p = strrchr(setting, '@');
    if (p)
    {
        *p = '\0';
        efreet_lang_modifier = strdup(p + 1);
        found = 1;
    }

    /* if there is an encoding we ignore it */
    p = strrchr(setting, '.');
    if (p) *p = '\0';

    /* get the country if available */
    p = strrchr(setting, '_');
    if (p)
    {
        *p = '\0';
        efreet_lang_country = strdup(p + 1);
        found = 1;
    }

    if (setting && (*setting != '\0'))
    {
        efreet_lang = strdup(setting);
        found = 1;
    }

    FREE(setting);

    return found;
}

/**
 * @internal
 * @param buffer: The destination buffer
 * @param size: The destination buffer size
 * @param strs: The strings to concatenate together
 * @return Returns the size of the string in @a buffer
 * @brief Concatenates the strings in @a strs into the given @a buffer not
 * exceeding the given @a size.
 */
size_t
efreet_array_cat(char *buffer, size_t size, const char *strs[])
{
    int i;
    size_t n;
    for (i = 0, n = 0; n < size && strs[i]; i++)
    {
        n += ecore_strlcpy(buffer + n, strs[i], size - n);
    }
    return n;
}
