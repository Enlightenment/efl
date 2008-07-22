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
 * @param file: The file to parse
 * @return Returns a new Efreet_Ini structure initialized with the contents
 * of @a file, or NULL on memory allocation failure
 * @brief Creates and initializes a new Ini structure with the contents of
 * @a file, or NULL on failure
 */
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
 * @return Returns an Ecore_Hash with the contents of @a file, or NULL if the
 *         file fails to parse or if the file doesn't exist
 * @brief Parses the ini file @a file into an Ecore_Hash
 */
static Ecore_Hash *
efreet_ini_parse(const char *file)
{
    const char *buffer, *line_start;
    FILE *f;
    Ecore_Hash *data, *section = NULL;
    struct stat file_stat;
    int line_length, left;

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
    buffer = mmap(NULL, left, PROT_READ, MAP_SHARED, fileno(f), 0);
    if (buffer == MAP_FAILED)
    {
        fclose(f);
        return NULL;
    }

    data = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(data, ECORE_FREE_CB(ecore_string_release));
    ecore_hash_free_value_cb_set(data, ECORE_FREE_CB(ecore_hash_destroy));

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
                Ecore_Hash *old;
                const char *header;

                header = alloca(header_length * sizeof(unsigned char));
                if (!header) goto next_line;

                memcpy((char*)header, line_start + 1, header_length - 1);
                ((char*)header)[header_length - 1] = '\0';

                section = ecore_hash_new(ecore_str_hash, ecore_str_compare);
                ecore_hash_free_key_cb_set(section,
                            ECORE_FREE_CB(ecore_string_release));
                ecore_hash_free_value_cb_set(section, ECORE_FREE_CB(free));

                old = ecore_hash_remove(data, header);
//                if (old) printf("[efreet] Warning: duplicate section '%s' "
  //                              "in file '%s'\n", header, file);

                IF_FREE_HASH(old);
                ecore_hash_set(data, (void *)ecore_string_instance(header),
                                section);
            }
            else
            {
                /* invalid file - skip line? or refuse to parse file? */
                /* just printf for now till we figure out what to do */
//                printf("Invalid file (%s) (missing ] on group name)\n", file);
            }
            goto next_line;
        }

        if (section == NULL)
        {
//            printf("Invalid file (%s) (missing section)\n", file);
            goto next_line;
        }

        /* find for '=' */
        for (sep = 0; (sep < line_length) && (line_start[sep] != '='); ++sep)
            ;

        if (sep < line_length)
        {
            const char *key, *value;
            char *old;
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
//                printf("Invalid file (%s) (invalid key=value pair)\n", file);

                goto next_line;
            }

            key = alloca((key_end + 1) * sizeof(unsigned char));
            value = alloca((value_end - value_start + 1) * sizeof(unsigned char));
            if (!key || !value) goto next_line;

            memcpy((char*)key, line_start, key_end);
            ((char*)key)[key_end] = '\0';

            memcpy((char*)value, line_start + value_start, 
                    value_end - value_start);
            ((char*)value)[value_end - value_start] = '\0';

            old = ecore_hash_remove(section, key);
            IF_FREE(old);

            ecore_hash_set(section, (void *)ecore_string_instance(key),
                           efreet_ini_unescape(value));
        }
//        else
//        {
//            /* invalid file... */
//            printf("Invalid file (%s) (missing = from key=value pair)\n", file);
//        }

next_line:
        left -= line_length + 1;
        line_start += line_length + 1;
    }
    munmap((char*) buffer, file_stat.st_size);
    fclose(f);

    return data;
}

/**
 * @param ini: The Efreet_Ini to work with
 * @return Returns no value
 * @brief Frees the given Efree_Ini structure.
 */
EAPI void
efreet_ini_free(Efreet_Ini *ini)
{
    if (!ini) return;

    IF_FREE_HASH(ini->data);
    FREE(ini);
}

/**
 * @param ini: The Efreet_Ini to work with
 * @param file: The file to load
 * @return Returns no value
 * @brief Saves the given Efree_Ini structure.
 */
EAPI int
efreet_ini_save(Efreet_Ini *ini, const char *file)
{
    FILE *f;
    if (!ini || !ini->data) return 0;

    f = fopen(file, "wb");
    if (!f) return 0;
    ecore_hash_for_each_node(ini->data, ECORE_FOR_EACH(efreet_ini_section_save), f);
    fclose(f);

    return 1;
}

/**
 * @param ini: The Efreet_Ini to work with
 * @param section: The section of the ini file we want to get values from
 * @return Returns 1 if the section exists, otherwise 0
 * @brief Sets the current working section of the ini file to @a section
 */
EAPI int
efreet_ini_section_set(Efreet_Ini *ini, const char *section)
{
    if (!ini || !ini->data || !section) return 0;

    ini->section = ecore_hash_get(ini->data, section);
    return (ini->section ? 1 : 0);
}

/**
 * @param ini: The Efreet_Ini to work with
 * @param section: The section of the ini file we want to add
 * @return Returns no value
 * @brief Adds a new working section of the ini file to @a section
 */
EAPI void
efreet_ini_section_add(Efreet_Ini *ini, const char *section)
{
    Ecore_Hash *hash;

    if (!ini || !section) return;

    if (!ini->data)
    {
        ini->data = ecore_hash_new(ecore_str_hash, ecore_str_compare);
        ecore_hash_free_key_cb_set(ini->data, ECORE_FREE_CB(ecore_string_release));
        ecore_hash_free_value_cb_set(ini->data, ECORE_FREE_CB(ecore_hash_destroy));
    }
    if (ecore_hash_get(ini->data, section)) return;

    hash = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(hash, ECORE_FREE_CB(ecore_string_release));
    ecore_hash_free_value_cb_set(hash, ECORE_FREE_CB(free));
    ecore_hash_set(ini->data, (void *)ecore_string_instance(section), hash);
}

/**
 * @param ini: The Efree_Ini to work with
 * @param key: The key to lookup
 * @return Returns the string associated with the given key or NULL if not
 * found.
 * @brief Retrieves the value for the given key or NULL if none found.
 */
EAPI const char *
efreet_ini_string_get(Efreet_Ini *ini, const char *key)
{
    if (!ini || !key || !ini->section) return NULL;

    return ecore_hash_get(ini->section, key);
}

/**
 * @param ini: The Efree_Ini to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
EAPI void
efreet_ini_string_set(Efreet_Ini *ini, const char *key, const char *value)
{
    if (!ini || !key || !ini->section) return;

    ecore_hash_set(ini->section, (void *)ecore_string_instance(key), strdup(value));
}

/**
 * @param ini: The Efree_Ini to work with
 * @param key: The key to lookup
 * @return Returns the integer associated with the given key or -1 if not
 * found.
 * @brief Retrieves the value for the given key or -1 if none found.
 */
EAPI int
efreet_ini_int_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    if (!ini || !key || !ini->section) return -1;

    str = efreet_ini_string_get(ini, key);
    if (str) return atoi(str);

    return -1;
}

/**
 * @param ini: The Efree_Ini to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
EAPI void
efreet_ini_int_set(Efreet_Ini *ini, const char *key, int value)
{
    char str[12];

    if (!ini || !key || !ini->section) return;

    snprintf(str, 12, "%d", value);
    efreet_ini_string_set(ini, key, str);
}

/**
 * @param ini: The Efree_Ini to work with
 * @param key: The key to lookup
 * @return Returns the double associated with the given key or -1 if not
 * found.
 * @brief Retrieves the value for the given key or -1 if none found.
 */
EAPI double
efreet_ini_double_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    if (!ini || !key || !ini->section) return -1;

    str = efreet_ini_string_get(ini, key);
    if (str) return atof(str);

    return -1;
}

/**
 * @param ini: The Efree_Ini to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
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

/**
 * @param ini: The ini struct to work with
 * @param key: The key to search for
 * @return Returns 1 if the boolean is true, 0 otherwise
 * @brief Retrieves the boolean value at key @a key from the ini @a ini
 */
EAPI unsigned int
efreet_ini_boolean_get(Efreet_Ini *ini, const char *key)
{
    const char *str;

    if (!ini || !key || !ini->section) return 0;

    str = efreet_ini_string_get(ini, key);
    if (str && !strcmp("true", str)) return 1;

    return 0;
}

/**
 * @param ini: The ini struct to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
EAPI void
efreet_ini_boolean_set(Efreet_Ini *ini, const char *key, unsigned int value)
{
    if (!ini || !key || !ini->section) return;

    if (value) efreet_ini_string_set(ini, key, "true");
    else efreet_ini_string_set(ini, key, "false");
}

/**
 * @param ini: The ini struct to work with
 * @param key: The key to search for
 * @return Returns the utf8 encoded string associated with @a key, or NULL
 *         if none found
 * @brief Retrieves the utf8 encoded string associated with @a key in the current locale or NULL if none found
 */
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

    buf = malloc(maxlen * sizeof(char));

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

    FREE(buf);

    return val;
}

/**
 * @param ini: The ini struct to work with
 * @param key: The key to use
 * @param value: The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
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
