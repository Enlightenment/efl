/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

static Ecore_Hash *efreet_ini_parse(const char *file);
static char *efreet_ini_unescape(const char *str);

static void efreet_ini_section_save(Ecore_Hash_Node *node, FILE *f);
static void efreet_ini_value_save(Ecore_Hash_Node *node, FILE *f);

/**
 * The number of times the Ini subsytem has been initialized
 */
static int init = 0;

/**
 * @internal
 * @return Returns > 0 on success or 0 on failure
 * @brief Initialize the Ini parser subsystem
 */
int
efreet_ini_init(void)
{
    if (init++) return init;
    if (!ecore_string_init()) return --init;
    return init;
}

/**
 * @internal
 * @returns the number of initializations left for this system
 * @brief Attempts to shut down the subsystem if nothing else is using it
 */
int
efreet_ini_shutdown(void)
{
    if (--init) return init;
    ecore_string_shutdown();
    return init;
}

/**
 * @internal
 * @param file: The file to parse
 * @return Returns a new Efreet_Ini structure initialized with the contents
 * of @a file, or NULL on failure
 * @brief Creates and initializes a new Ini structure with the contents of
 * @a file, or NULL on failure 
 */ 
Efreet_Ini *
efreet_ini_new(const char *file)
{
    Efreet_Ini *ini;

    ini = NEW(Efreet_Ini, 1);
    if (!ini) return NULL;

    ini->data = efreet_ini_parse(file);

    return ini;
}

/**
 * @internal
 * @param file The file to parse
 * @return Returns an Ecore_Hash with the contents of @a file, or NULL on failure
 * @brief Parses the ini file @a file into an Ecore_Hash
 */
static Ecore_Hash *
efreet_ini_parse(const char *file)
{
    FILE *f;

    /* a static buffer for quick reading of lines that fit */
    char static_buf[4096];
    int static_buf_len = 4096;

    /* a big buffer to allocate for lines that are larger than the static one */
    char *big_buf = NULL;
    int big_buf_len = 0;
    int big_buf_step = static_buf_len;

    /* the current location to read into (with fgets) and the amount to read */
    char *read_buf;
    int read_len;

    /* the current buffer to parse */
    char *buf;
   
    Ecore_Hash *data, *section = NULL;

    /* start with the static buffer */
    buf = read_buf = static_buf;
    read_len = static_buf_len;

    f = fopen(file, "r");
    if (!f) return NULL;

    data = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_set_free_key(data, ECORE_FREE_CB(ecore_string_release));
    ecore_hash_set_free_value(data, ECORE_FREE_CB(ecore_hash_destroy));

    /* if a line is longer than the buffer size, this \n will get overwritten. */
    read_buf[read_len - 2] = '\n';
    while(fgets(read_buf, read_len, f) != NULL) 
    {
        char *key, *value, *p;
        char *sep;

        /* handle lines longer than the buffer size */
        if (read_buf[read_len-2] != '\n')
        {
            int len;
            len = strlen(buf);

            if (!big_buf) 
            {
              /* create new big buffer and copy in contents of static buf */
              big_buf_len = 2 * big_buf_step;
              big_buf = malloc(big_buf_len * sizeof(char));
              strncpy(big_buf, buf, len + 1);
            }
            else if (buf == big_buf)
            {
              /* already using the big buffer. increase its size for the next read */
              big_buf_len += big_buf_step;
              big_buf = realloc(big_buf, big_buf_len);
            }
            else
            {
              /* the big buffer exists, but we aren't using it yet. copy contents of static buf in */
              strncpy(big_buf, buf, len);
            }

            /* use big_buffer for next fgets and subsequent parsing */
            buf = big_buf;
            read_buf = big_buf + len;
            read_len = big_buf_len - len;
            read_buf[read_len-2] = '\n';

            continue;
        }

        /* skip empty lines and comments */
        if (buf[0] == '\0' || buf[0] == '\n' || buf[0] == '#') goto next_line;

        /* new section */
        if (buf[0] == '[')
        {
            char *header, *p;
            header = buf + 1;

            p = strchr(header, ']');
            if (p)
            {
                Ecore_Hash *old;
                *p = '\0';
                section = ecore_hash_new(ecore_str_hash, ecore_str_compare);
                ecore_hash_set_free_key(section, ECORE_FREE_CB(ecore_string_release));
                ecore_hash_set_free_value(section, ECORE_FREE_CB(free));

                old = ecore_hash_remove(data, header);
                //if (old) printf("[efreet] Warning: duplicate section '%s' in file '%s'\n", header, file);
                IF_FREE_HASH(old);
                ecore_hash_set(data, (void *)ecore_string_instance(header), 
                                                                section);
            }
            else
            {
                /* invalid file - skip line? or refuse to parse file? */
                /* just printf for now till we figure out what to do */
                printf("Invalid file (%s) (missing ] on group name)\n", file);
            }
            goto next_line;
        }

        /* parse key=value pair */
        sep = strchr(buf, '=');
        key = buf;

        if (sep)
        {
            /* trim whitespace from end of key */
            p = sep;
            while (p > key && isspace(*(p - 1))) p--;
            *p = '\0';

            value = sep + 1;

            /* trim whitespace from start of value */
            while (*value && isspace(*value)) value++;

            /* trim \n off of end of value */
            p = value + strlen(value) - 1;
            while (p > value && (*p == '\n' || *p == '\r')) p--;
            *(p + 1) = '\0';

            if (key && value && *key && *value) 
            {
                char *old;

                old = ecore_hash_remove(section, key);
                //if (old) printf("[efreet] Warning: duplicate key '%s' in file '%s'\n", key, file);
                IF_FREE(old);

                ecore_hash_set(section, (void *)ecore_string_instance(key), 
                               efreet_ini_unescape(value));
            }
        }
        else
        {
            /* check if line is all whitespace, if so, skip it */
            int nonwhite = 0;
            p = buf;
            while (*p)
            {
                if (!isspace(*p))
                {
                    nonwhite = 1;
                    break;
                }
                p++;
            }
            if (!nonwhite) goto next_line;

            /* invalid file... */
            printf("Invalid file (%s) (missing = from key=value pair)\n", file);
        }

next_line:
        /* finished parsing a line. use static buffer for next line */
        buf = read_buf = static_buf;
        read_len = static_buf_len;
        read_buf[read_len - 2] = '\n';
    }

    fclose(f);
    if (big_buf) free(big_buf);

    return data;
}

/**
 * @internal
 * @param ini: The Efreet_Ini to work with
 * @return Returns no value
 * @brief Frees the given Efree_Ini structure.
 */
void
efreet_ini_free(Efreet_Ini *ini)
{
    if (!ini) return;

    IF_FREE_HASH(ini->data);
    FREE(ini);
}

/**
 * @internal
 * @param ini: The Efreet_Ini to work with
 * @param file: The file to load
 * @return Returns no value
 * @brief Saves the given Efree_Ini structure.
 */
int
efreet_ini_save(Efreet_Ini *ini, const char *file)
{
    FILE *f;
    if (!ini) return 0;

    f = fopen(file, "w");
    if (!f) return 0;
    ecore_hash_for_each_node(ini->data, ECORE_FOR_EACH(efreet_ini_section_save), f);
    fclose(f);

    return 1;
}

/**
 * @internal
 * @param ini: The Efreet_Ini to work with
 * @param section: The section of the ini file we want to get values from
 * @return Returns 1 if the section exists, otherwise 0
 * @brief Sets the current working section of the ini file to @a section
 */
int
efreet_ini_section_set(Efreet_Ini *ini, const char *section)
{
    if (!ini || !section) return 0;

    ini->section = ecore_hash_get(ini->data, section);
    return (ini->section ? 1 : 0);
}

/**
 * @internal
 * @param ini: The Efreet_Ini to work with
 * @param section: The section of the ini file we want to add
 * @return Returns no value
 * @brief Adds a new working section of the ini file to @a section
 */
void
efreet_ini_section_add(Efreet_Ini *ini, const char *section)
{
    Ecore_Hash *hash;

    if (!ini || !section) return;

    if (!ini->data)
    {
        ini->data = ecore_hash_new(ecore_str_hash, ecore_str_compare);
        ecore_hash_set_free_key(ini->data, ECORE_FREE_CB(ecore_string_release));
        ecore_hash_set_free_value(ini->data, ECORE_FREE_CB(ecore_hash_destroy));
    }
    if (ecore_hash_get(ini->data, section)) return;

    hash = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_set_free_key(hash, ECORE_FREE_CB(ecore_string_release));
    ecore_hash_set_free_value(hash, ECORE_FREE_CB(free));
    ecore_hash_set(ini->data, (void *)ecore_string_instance(section), hash);
}

/**
 * @internal
 * @param ini: The Efree_Ini to work with
 * @param key: The key to lookup
 * @return Returns the string associated with the given key or NULL if not
 * found.
 * @brief Retrieves the value for the given key or NULL if none found.
 */
const char *
efreet_ini_string_get(Efreet_Ini *ini, const char *key)
{
    if (!ini || !key || !ini->section) return NULL;

    return ecore_hash_get(ini->section, key);
}

/**
 * @internal
 * @param ini: The Efree_Ini to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
void
efreet_ini_string_set(Efreet_Ini *ini, const char *key, const char *value)
{
    if (!ini || !key || !ini->section) return;

    ecore_hash_set(ini->section, (void *)ecore_string_instance(key), strdup(value));
}

/**
 * @internal
 * @param ini: The Efree_Ini to work with
 * @param key: The key to lookup
 * @return Returns the integer associated with the given key or -1 if not
 * found.
 * @brief Retrieves the value for the given key or -1 if none found.
 */
int
efreet_ini_int_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    if (!ini || !key || !ini->section) return -1;

    str = efreet_ini_string_get(ini, key);
    if (str) return atoi(str);

    return -1;
}

/**
 * @internal
 * @param ini: The Efree_Ini to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
void
efreet_ini_int_set(Efreet_Ini *ini, const char *key, int value)
{
    char str[12];

    if (!ini || !key || !ini->section) return;

    snprintf(str, 12, "%d", value);
    efreet_ini_string_set(ini, key, str);
}

/**
 * @internal
 * @param ini: The Efree_Ini to work with
 * @param key: The key to lookup
 * @return Returns the double associated with the given key or -1 if not
 * found.
 * @brief Retrieves the value for the given key or -1 if none found.
 */
double
efreet_ini_double_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    if (!ini || !key || !ini->section) return -1;

    str = efreet_ini_string_get(ini, key);
    if (str) return atof(str);

    return -1;
}

/**
 * @internal
 * @param ini: The Efree_Ini to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
void
efreet_ini_double_set(Efreet_Ini *ini, const char *key, double value)
{
    char str[512];
    size_t len;

    if (!ini || !key || !ini->section) return;

    snprintf(str, 512, "%.6f", value);
    len = strlen(str) - 1;
    /* Strip trailing zero's */
    while (str[len] == '0' && str[len - 1] != '.') str[len--] = 0;
    efreet_ini_string_set(ini, key, str);
}

/**
 * @internal
 * @param ini: The ini struct to work with
 * @param key: The key to search for
 * @return Returns 1 if the boolean is true, 0 otherwise
 * @brief Retrieves the boolean value at key @a key from the ini @a ini
 */
unsigned int
efreet_ini_boolean_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    if (!ini || !key || !ini->section) return 0;

    str = efreet_ini_string_get(ini, key);
    if (str && !strcmp("true", str)) return 1;

    return 0;
}

/**
 * @internal
 * @param ini: The ini struct to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
void
efreet_ini_boolean_set(Efreet_Ini *ini, const char *key, unsigned int value)
{
    if (!ini || !key || !ini->section) return;

    if (value) efreet_ini_string_set(ini, key, "true");
    else efreet_ini_string_set(ini, key, "false");
}

/**
 * @internal
 * @param ini: The ini struct to work with
 * @param key: The key to search for
 * @return Returns the utf8 encoded string associated with @a key, or NULL 
 *         if none found
 * @brief Retrieves the utf8 encoded string associated with @a key in the current locale or NULL if none found
 */
const char *
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

    buf = malloc(maxlen * sizeof(char));

    if (lang && modifier && country)
    {
        snprintf(buf, maxlen, "%s[%s_%s@%s]", key, lang, country, modifier);
        val = efreet_ini_string_get(ini, buf);
        if (val) found = 1;
    }

    if (!found && lang && country)
    {
        snprintf(buf, maxlen, "%s[%s_%s]", key, lang, country);
        val = efreet_ini_string_get(ini, buf);
        if (val) found = 1;
    }

    if (!found && lang && modifier)
    {
        snprintf(buf, maxlen, "%s[%s@%s]", key, lang, modifier);
        val = efreet_ini_string_get(ini, buf);
        if (val) found = 1;
    }

    if (!found && lang)
    {
        snprintf(buf, maxlen, "%s[%s]", key, lang);
        val = efreet_ini_string_get(ini, buf);
        if (val) found = 1;

    }

    if (!found)
        val = efreet_ini_string_get(ini, key);

    FREE(buf);

    return val;
}

/**
 * @internal
 * @param ini: The ini struct to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
void
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

    buf = malloc(maxlen * sizeof(char));

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
    FREE(buf);
}

/**
 * @param str The string to unescape
 * @return An allocated unescaped string
 * @brief Unescapes backslash escapes in a string
 */
static char *
efreet_ini_unescape(const char *str)
{
    char *buf, *dest;
    const char *p;

    if (!str) return NULL;
    if (!strchr(str, '\\')) return strdup(str);
    buf = malloc(strlen(str) + 1);

    p = str;
    dest = buf;
    while(*p)
    {
        if (*p == '\\')
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
    return buf;
}

static void
efreet_ini_section_save(Ecore_Hash_Node *node, FILE *f)
{
    fprintf(f, "[%s]\n", (char *)node->key);
    ecore_hash_for_each_node(node->value, ECORE_FOR_EACH(efreet_ini_value_save), f);
}

static void
efreet_ini_value_save(Ecore_Hash_Node *node, FILE *f)
{
    fprintf(f, "%s=%s\n", (char *)node->key, (char *)node->value);
}
