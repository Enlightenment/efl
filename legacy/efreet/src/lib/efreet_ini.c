#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#undef alloca
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

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

    /* This can validly be NULL at the moment as _parse() will return NULL
     * if the input file doesn't exist. Should we change _parse() to create
     * the hash and only return NULL on failed parse? */
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
    const char *buffer, *line_start;
    FILE *f;
    Eina_Hash *data, *section = NULL;
    struct stat file_stat;
    int line_length, left;

    if (!file) return NULL;

    f = fopen(file, "rb");
    if (!f) return NULL;

    if (fstat(fileno(f), &file_stat) || (file_stat.st_size < 1))
    {
        fclose(f);
        return NULL;
    }
    if (!S_ISREG(file_stat.st_mode)) /* if not a regular file - close */
    {
        fclose(f);
        return NULL;
    }

    left = file_stat.st_size;
    /* let's make mmap safe and just get 0 pages for IO erro */
    eina_mmap_safety_enabled_set(EINA_TRUE);
   
    buffer = mmap(NULL, left, PROT_READ, MAP_SHARED, fileno(f), 0);
    if (buffer == MAP_FAILED)
    {
        fclose(f);
        return NULL;
    }

    data = eina_hash_string_small_new(EINA_FREE_CB(eina_hash_free));

    line_start = buffer;
    while (left > 0)
    {
        int sep;

        /* find the end of line */
        for (line_length = 0;
                (line_length < left) &&
                (line_start[line_length] != '\n'); line_length++)
            ;

        /* check for all white space */
        while (isspace(line_start[0]) && (line_length > 0))
        {
            line_start++;
            line_length--;
        }

        /* skip empty lines and comments */
        if ((line_length == 0) || (line_start[0] == '\r') ||
                (line_start[0] == '\n') || (line_start[0] == '#') ||
                (line_start[0] == '\0'))
            goto next_line;

        /* new section */
        if (line_start[0] == '[')
        {
            int header_length;

            /* find the ']' */
            for (header_length = 1;
                    (header_length < line_length) &&
                    (line_start[header_length] != ']'); ++header_length)
                ;

            if (line_start[header_length] == ']')
            {
                const char *header;

                header = alloca(header_length * sizeof(unsigned char));
                if (!header) goto next_line;

                memcpy((char*)header, line_start + 1, header_length - 1);
                ((char*)header)[header_length - 1] = '\0';

                section = eina_hash_string_small_new(EINA_FREE_CB(eina_stringshare_del));

                eina_hash_del_by_key(data, header);
//                if (old) INF("[efreet] Warning: duplicate section '%s' "
  //                              "in file '%s'", header, file);

                eina_hash_add(data, header, section);
            }
            else
            {
                /* invalid file - skip line? or refuse to parse file? */
                /* just printf for now till we figure out what to do */
//                ERR("Invalid file (%s) (missing ] on group name)", file);
            }
            goto next_line;
        }

        if (!section)
        {
            INF("Invalid file (%s) (missing section)", file);
            goto error;
        }

        /* find for '=' */
        for (sep = 0; (sep < line_length) && (line_start[sep] != '='); ++sep)
            ;

        if (sep < line_length && line_start[sep] == '=')
        {
            char *key, *value;
            int key_end, value_start, value_end;

            /* trim whitespace from end of key */
            for (key_end = sep - 1;
                    (key_end > 0) && isspace(line_start[key_end]); --key_end)
                ;

            if (!isspace(line_start[key_end])) key_end++;

            /* trim whitespace from start of value */
            for (value_start = sep + 1;
                 (value_start < line_length) &&
                 isspace(line_start[value_start]); ++value_start)
                ;

            /* trim \n off of end of value */
            for (value_end = line_length;
                 (value_end > value_start) &&
                 ((line_start[value_end] == '\n') ||
                  (line_start[value_end] == '\r')); --value_end)
                ;

            if (line_start[value_end] != '\n'
                    && line_start[value_end] != '\r'
                    && value_end < line_length)
                value_end++;

            /* make sure we have a key. blank values are allowed */
            if (key_end == 0)
            {
                /* invalid file... */
//                INF("Invalid file (%s) (invalid key=value pair)", file);

                goto next_line;
            }

            key = alloca(key_end + 1);
            value = alloca(value_end - value_start + 1);
            if (!key || !value) goto next_line;

            memcpy(key, line_start, key_end);
            key[key_end] = '\0';

            memcpy(value, line_start + value_start,
                    value_end - value_start);
            value[value_end - value_start] = '\0';

            eina_hash_del_by_key(section, key);
            eina_hash_add(section, key, efreet_ini_unescape(value));
        }
        else
        {
            /* invalid file... */
            INF("Invalid file (%s) (missing = from key=value pair)", file);
            goto error;
        }

next_line:
        left -= line_length + 1;
        line_start += line_length + 1;
    }
    munmap((char*) buffer, file_stat.st_size);
    fclose(f);

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
    if (!ini || !ini->data) return 0;

    dir = ecore_file_dir_get(file);
    if (!ecore_file_mkpath(dir))
    {
        free(dir);
        return 0;
    }
    free(dir);
    f = fopen(file, "wb");
    if (!f) return 0;
    eina_hash_foreach(ini->data, efreet_ini_section_save, f);
    fclose(f);

    return 1;
}

EAPI int
efreet_ini_section_set(Efreet_Ini *ini, const char *section)
{
    if (!ini || !ini->data || !section) return 0;

    ini->section = eina_hash_find(ini->data, section);
    return (ini->section ? 1 : 0);
}

EAPI void
efreet_ini_section_add(Efreet_Ini *ini, const char *section)
{
    Eina_Hash *hash;

    if (!ini || !section) return;

    if (!ini->data)
        ini->data = eina_hash_string_small_new(EINA_FREE_CB(eina_hash_free));
    if (eina_hash_find(ini->data, section)) return;

    hash = eina_hash_string_small_new(EINA_FREE_CB(eina_stringshare_del));
    eina_hash_add(ini->data, section, hash);
}

EAPI const char *
efreet_ini_string_get(Efreet_Ini *ini, const char *key)
{
    if (!ini || !key || !ini->section) return NULL;

    return eina_hash_find(ini->section, key);
}

EAPI void
efreet_ini_string_set(Efreet_Ini *ini, const char *key, const char *value)
{
    if (!ini || !key || !ini->section) return;

    eina_hash_del_by_key(ini->section, key);
    eina_hash_add(ini->section, key, eina_stringshare_add(value));
}

EAPI int
efreet_ini_int_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    if (!ini || !key || !ini->section) return -1;

    str = efreet_ini_string_get(ini, key);
    if (str) return atoi(str);

    return -1;
}

EAPI void
efreet_ini_int_set(Efreet_Ini *ini, const char *key, int value)
{
    char str[12];

    if (!ini || !key || !ini->section) return;

    snprintf(str, 12, "%d", value);
    efreet_ini_string_set(ini, key, str);
}

EAPI double
efreet_ini_double_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    if (!ini || !key || !ini->section) return -1;

    str = efreet_ini_string_get(ini, key);
    if (str) return atof(str);

    return -1;
}

EAPI void
efreet_ini_double_set(Efreet_Ini *ini, const char *key, double value)
{
    char str[512];
    size_t len;

    if (!ini || !key || !ini->section) return;

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

    if (!ini || !key || !ini->section) return 0;

    str = efreet_ini_string_get(ini, key);
    if (str && !strcmp("true", str)) return 1;

    return 0;
}

EAPI void
efreet_ini_boolean_set(Efreet_Ini *ini, const char *key, unsigned int value)
{
    if (!ini || !key || !ini->section) return;

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

    if (!ini || !key || !ini->section) return NULL;

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

    if (!ini || !key || !ini->section) return;

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
    if (!ini || !key || !ini->section) return;

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
efreet_ini_section_save(const Eina_Hash *hash __UNUSED__, const void *key, void *value, void *fdata)
{
    FILE *f = fdata;

    fprintf(f, "[%s]\n", (char *)key);
    eina_hash_foreach(value, efreet_ini_value_save, f);
    return EINA_TRUE;
}

static Eina_Bool
efreet_ini_value_save(const Eina_Hash *hash __UNUSED__, const void *key, void *value, void *fdata)
{
    FILE *f = fdata;

    fprintf(f, "%s=%s\n", (char *)key, (char *)value);
    return EINA_TRUE;
}
