#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <ctype.h>

#include <Ecore_File.h>

/* define macros and variable for using the eina logging system  */
#define EFREET_MODULE_LOG_DOM _efreet_ini_log_dom
static int _efreet_ini_log_dom = -1;

#include "Efreet.h"
#include "efreet_private.h"

static Eina_Hash *efreet_ini_parse(const char *file);
static const char *efreet_ini_unescape(const char *str) EINA_ARG_NONNULL(1);
static Eina_Bool
efreet_ini_section_save(const Eina_Hash *hash, const void *key, void *data, void *fdata);
static Eina_Bool
efreet_ini_value_save(const Eina_Hash *hash, const void *key, void *data, void *fdata);

/**
 * @internal
 * @return Returns > 0 on success or 0 on failure
 * @brief Initialize the Ini parser subsystem
 */
int
efreet_ini_init(void)
{
    _efreet_ini_log_dom = eina_log_domain_register
      ("efreet_ini", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_ini_log_dom < 0)
    {
        EINA_LOG_ERR("Efreet: Could not create a log domain for efreet_ini");
        return 0;
    }
    return 1;
}

/**
 * @internal
 * @returns the number of initializations left for this system
 * @brief Attempts to shut down the subsystem if nothing else is using it
 */
void
efreet_ini_shutdown(void)
{
    eina_log_domain_unregister(_efreet_ini_log_dom);
    _efreet_ini_log_dom = -1;
}

EAPI Efreet_Ini *
efreet_ini_new(const char *file)
{
    Efreet_Ini *ini;

    ini = NEW(Efreet_Ini, 1);
    if (!ini) return NULL;

    if (file)
      ini->data = efreet_ini_parse(file);

    return ini;
}

/**
 * @internal
 * @param file The file to parse
 * @return Returns an Eina_Hash with the contents of @a file, or NULL if the
 *         file fails to parse or if the file doesn't exist
 * @brief Parses the ini file @a file into an Eina_Hash
 */
static Eina_Hash *
efreet_ini_parse(const char *file)
{
    Eina_Hash *data = NULL, *section = NULL;
    Eina_Iterator *it = NULL;
    Eina_File_Line *line;
    Eina_File *f;

    EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
    f = eina_file_open(file, EINA_FALSE);
    if (!f)
      return NULL;

    data = eina_hash_string_small_new(EINA_FREE_CB(eina_hash_free));
    if (!data) goto error;

    /* let's make mmap safe and just get 0 pages for IO erro */
    eina_mmap_safety_enabled_set(EINA_TRUE);

    it = eina_file_map_lines(f);
    if (!it) goto error;
    EINA_ITERATOR_FOREACH(it, line)
    {
        const char *eq;
        unsigned int start = 0;

        /* skip empty lines */
        if (line->length == 0) continue;
        /* skip white space at start of line */
        while ((start < line->length) && (isspace((unsigned char)line->start[start])))
            start++;
        /* skip empty lines */
        if (start == line->length) continue;
        /* skip comments */
        if (line->start[start] == '#') continue;

        /* new section */
        if (line->start[start] == '[')
        {
            const char *head_start;
            const char *head_end;

            head_start = &(line->start[start]) + 1;
            head_end = memchr(line->start, ']', line->length);

            if (head_end)
            {
                char *header;
                size_t len;

                len = head_end - head_start + 1;
                header = alloca(len);

                memcpy(header, head_start, len - 1);
                header[len - 1] = '\0';

                section = eina_hash_string_small_new(EINA_FREE_CB(eina_stringshare_del));

                eina_hash_del_by_key(data, header);
                eina_hash_add(data, header, section);
            }
            else
            {
                /* invalid file - skip line? or refuse to parse file? */
                /* just printf for now till we figure out what to do */
//                ERR("Invalid file (%s) (missing ] on group name)", file);
            }
            continue;
        }

        if (!section)
        {
            INF("Invalid file '%s' (missing section)", file);
            goto error;
        }

        eq = memchr(line->start, '=', line->length);

        if (eq)
        {
            const char *key_start, *key_end;
            const char *value_start, *value_end;
            char *key, *value;
            size_t len;

            key_start = &(line->start[start]);
            key_end = eq - 1;

            /* trim whitespace from end of key */
            while ((isspace((unsigned char)*key_end)) && (key_end > key_start))
                key_end--;
            key_end++;

            /* make sure we have a key */
            if (key_start == key_end) continue;

            value_start = eq + 1;
            value_end = line->end;

            /* line->end points to char after '\n' or '\r' */
            value_end--;
            /* trim whitespace from end of value */
            while ((isspace((unsigned char)*value_end)) && (value_end > value_start))
                value_end--;
            value_end++;

            /* trim whitespace from start of value */
            while ((isspace((unsigned char)*value_start)) && (value_start < value_end))
                value_start++;

            len = key_end - key_start + 1;
            key = alloca(len);

            memcpy(key, key_start, len - 1);
            key[len - 1] = '\0';

            /* empty value allowed */
            if (value_end == value_start)
            {
                eina_hash_del_by_key(section, key);
                eina_hash_add(section, key, "");
            }
            else
            {
                len = value_end - value_start + 1;
                value = alloca(len);
                memcpy(value, value_start, len - 1);
                value[len - 1] = '\0';

                eina_hash_del_by_key(section, key);
                eina_hash_add(section, key, efreet_ini_unescape(value));
            }
        }
        else
        {
            /* invalid file... */
            INF("Invalid file '%s' (missing = from key=value pair)", file);
            goto error;
        }
    }
    eina_iterator_free(it);
    eina_file_close(f);

#if 0
    if (!eina_hash_population(data))
    {
        eina_hash_free(data);
        return NULL;
    }
#endif
    return data;
error:
    if (data) eina_hash_free(data);
    if (it) eina_iterator_free(it);

    eina_file_close(f);
    return NULL;
}

EAPI void
efreet_ini_free(Efreet_Ini *ini)
{
    if (!ini) return;

    IF_FREE_HASH(ini->data);
    FREE(ini);
}

EAPI int
efreet_ini_save(Efreet_Ini *ini, const char *file)
{
    char *dir;
    FILE *f;

    EINA_SAFETY_ON_NULL_RETURN_VAL(ini, 0);
    EINA_SAFETY_ON_NULL_RETURN_VAL(ini->data, 0);
    EINA_SAFETY_ON_NULL_RETURN_VAL(file, 0);

    dir = ecore_file_dir_get(file);
    if (!ecore_file_mkpath(dir))
    {
        free(dir);
        return 0;
    }
    free(dir);
    f = fopen(file, "wb");
    if (!f) return 0;
    if (ini->data)
      eina_hash_foreach(ini->data, efreet_ini_section_save, f);
    fclose(f);

    return 1;
}

EAPI int
efreet_ini_section_set(Efreet_Ini *ini, const char *section)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(ini, 0);
    EINA_SAFETY_ON_NULL_RETURN_VAL(ini->data, 0);
    EINA_SAFETY_ON_NULL_RETURN_VAL(section, 0);

    ini->section = eina_hash_find(ini->data, section);
    return (ini->section ? 1 : 0);
}

EAPI void
efreet_ini_section_add(Efreet_Ini *ini, const char *section)
{
    Eina_Hash *hash;

    EINA_SAFETY_ON_NULL_RETURN(ini);
    EINA_SAFETY_ON_NULL_RETURN(section);

    if (!ini->data)
        ini->data = eina_hash_string_small_new(EINA_FREE_CB(eina_hash_free));
    if (eina_hash_find(ini->data, section)) return;

    hash = eina_hash_string_small_new(EINA_FREE_CB(eina_stringshare_del));
    eina_hash_add(ini->data, section, hash);
}

EAPI const char *
efreet_ini_string_get(Efreet_Ini *ini, const char *key)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(ini, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(ini->section, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);

    return eina_hash_find(ini->section, key);
}

EAPI void
efreet_ini_string_set(Efreet_Ini *ini, const char *key, const char *value)
{
    EINA_SAFETY_ON_NULL_RETURN(ini);
    EINA_SAFETY_ON_NULL_RETURN(ini->section);
    EINA_SAFETY_ON_NULL_RETURN(key);

    eina_hash_del_by_key(ini->section, key);
    eina_hash_add(ini->section, key, eina_stringshare_add(value));
}

EAPI int
efreet_ini_int_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    EINA_SAFETY_ON_NULL_RETURN_VAL(ini, -1);
    EINA_SAFETY_ON_NULL_RETURN_VAL(ini->section, -1);
    EINA_SAFETY_ON_NULL_RETURN_VAL(key, -1);

    str = efreet_ini_string_get(ini, key);
    if (str) return atoi(str);

    return -1;
}

EAPI void
efreet_ini_int_set(Efreet_Ini *ini, const char *key, int value)
{
    char str[12];

    EINA_SAFETY_ON_NULL_RETURN(ini);
    EINA_SAFETY_ON_NULL_RETURN(ini->section);
    EINA_SAFETY_ON_NULL_RETURN(key);

    snprintf(str, 12, "%d", value);
    efreet_ini_string_set(ini, key, str);
}

EAPI double
efreet_ini_double_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    EINA_SAFETY_ON_NULL_RETURN_VAL(ini, -1);
    EINA_SAFETY_ON_NULL_RETURN_VAL(ini->section, -1);
    EINA_SAFETY_ON_NULL_RETURN_VAL(key, -1);

    str = efreet_ini_string_get(ini, key);
    if (str) return atof(str);

    return -1;
}

EAPI void
efreet_ini_double_set(Efreet_Ini *ini, const char *key, double value)
{
    char str[512];
    size_t len;

    EINA_SAFETY_ON_NULL_RETURN(ini);
    EINA_SAFETY_ON_NULL_RETURN(ini->section);
    EINA_SAFETY_ON_NULL_RETURN(key);

    snprintf(str, 512, "%.6f", value);
    len = strlen(str) - 1;
    /* Strip trailing zero's */
    while (str[len] == '0' && str[len - 1] != '.') str[len--] = '\0';
    efreet_ini_string_set(ini, key, str);
}

EAPI unsigned int
efreet_ini_boolean_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    EINA_SAFETY_ON_NULL_RETURN_VAL(ini, 0);
    EINA_SAFETY_ON_NULL_RETURN_VAL(ini->section, 0);
    EINA_SAFETY_ON_NULL_RETURN_VAL(key, 0);

    str = efreet_ini_string_get(ini, key);
    if (str && !strcmp("true", str)) return 1;

    return 0;
}

EAPI void
efreet_ini_boolean_set(Efreet_Ini *ini, const char *key, unsigned int value)
{
    EINA_SAFETY_ON_NULL_RETURN(ini);
    EINA_SAFETY_ON_NULL_RETURN(ini->section);
    EINA_SAFETY_ON_NULL_RETURN(key);

    if (value) efreet_ini_string_set(ini, key, "true");
    else efreet_ini_string_set(ini, key, "false");
}

EAPI const char *
efreet_ini_localestring_get(Efreet_Ini *ini, const char *key)
{
    const char *lang, *country, *modifier;
    const char *val = NULL;
    char *buf;
    int maxlen = 5; /* _, @, [, ] and \0 */
    int found = 0;

    EINA_SAFETY_ON_NULL_RETURN_VAL(ini, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(ini->section, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);

    lang = efreet_lang_get();
    country = efreet_lang_country_get();
    modifier = efreet_lang_modifier_get();

    maxlen += strlen(key);
    if (lang) maxlen += strlen(lang);
    if (country) maxlen += strlen(country);
    if (modifier) maxlen += strlen(modifier);

    buf = alloca(maxlen);

    if (lang && modifier && country)
    {
        snprintf(buf, maxlen, "%s[%s_%s@%s]", key, lang, country, modifier);
        val = efreet_ini_string_get(ini, buf);
        if (val && (*val != '\0')) found = 1;
    }

    if (!found && lang && country)
    {
        snprintf(buf, maxlen, "%s[%s_%s]", key, lang, country);
        val = efreet_ini_string_get(ini, buf);
        if (val && (*val != '\0')) found = 1;
    }

    if (!found && lang && modifier)
    {
        snprintf(buf, maxlen, "%s[%s@%s]", key, lang, modifier);
        val = efreet_ini_string_get(ini, buf);
        if (val && (*val != '\0')) found = 1;
    }

    if (!found && lang)
    {
        snprintf(buf, maxlen, "%s[%s]", key, lang);
        val = efreet_ini_string_get(ini, buf);
        if (val && (*val != '\0')) found = 1;
    }

    if (!found)
        val = efreet_ini_string_get(ini, key);

    return val;
}

EAPI void
efreet_ini_localestring_set(Efreet_Ini *ini, const char *key, const char *value)
{
    const char *lang, *country, *modifier;
    char *buf;
    int maxlen = 5; /* _, @, [, ] and \0 */

    EINA_SAFETY_ON_NULL_RETURN(ini);
    EINA_SAFETY_ON_NULL_RETURN(ini->section);
    EINA_SAFETY_ON_NULL_RETURN(key);

    lang = efreet_lang_get();
    country = efreet_lang_country_get();
    modifier = efreet_lang_modifier_get();

    maxlen += strlen(key);
    if (lang) maxlen += strlen(lang);
    if (country) maxlen += strlen(country);
    if (modifier) maxlen += strlen(modifier);

    buf = alloca(maxlen);

    if (lang && modifier && country)
        snprintf(buf, maxlen, "%s[%s_%s@%s]", key, lang, country, modifier);
    else if (lang && country)
        snprintf(buf, maxlen, "%s[%s_%s]", key, lang, country);
    else if (lang && modifier)
        snprintf(buf, maxlen, "%s[%s@%s]", key, lang, modifier);
    else if (lang)
        snprintf(buf, maxlen, "%s[%s]", key, lang);
    else
        return;

    efreet_ini_string_set(ini, buf, value);
}

EAPI void
efreet_ini_key_unset(Efreet_Ini *ini, const char *key)
{
    EINA_SAFETY_ON_NULL_RETURN(ini);
    EINA_SAFETY_ON_NULL_RETURN(ini->section);
    EINA_SAFETY_ON_NULL_RETURN(key);

    eina_hash_del_by_key(ini->section, key);
}

/**
 * @param str The string to unescape
 * @return An allocated unescaped string
 * @brief Unescapes backslash escapes in a string
 */
static const char *
efreet_ini_unescape(const char *str)
{
    char *buf, *dest;
    const char *p;

    if (!strchr(str, '\\')) return eina_stringshare_add(str);
    buf = alloca(strlen(str) + 1);

    p = str;
    dest = buf;
    while (*p)
    {
        if ((*p == '\\') && (p[1] != '\0'))
        {
            p++;
            switch (*p)
            {
                case 's':
                    *(dest++) = ' ';
                    break;
                case 'n':
                    *(dest++) = '\n';
                    break;
                case 't':
                    *(dest++) = '\t';
                    break;
                case 'r':
                    *(dest++) = '\r';
                    break;
                case '\\':
                    *(dest++) = '\\';
                    break;
                default:
                    (*dest++) = '\\';
                    (*dest++) = *p;
            }
        }
        else
            *(dest++) = *p;

        p++;
    }

    *(dest) = '\0';
    return eina_stringshare_add(buf);
}

static Eina_Bool
efreet_ini_section_save(const Eina_Hash *hash EINA_UNUSED, const void *key, void *value, void *fdata)
{
    FILE *f = fdata;

    fprintf(f, "[%s]\n", (char *)key);
    eina_hash_foreach(value, efreet_ini_value_save, f);
    return EINA_TRUE;
}

static Eina_Bool
efreet_ini_value_save(const Eina_Hash *hash EINA_UNUSED, const void *key, void *value, void *fdata)
{
    FILE *f = fdata;

    fprintf(f, "%s=%s\n", (char *)key, (char *)value);
    return EINA_TRUE;
}
