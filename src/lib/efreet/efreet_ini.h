#ifndef EFREET_INI_H
#define EFREET_INI_H

/**
 * @internal
 * @file efreet_ini.h
 * @brief A simple and fast INI parser
 * @addtogroup Efreet_Ini Efreet_Ini: An INI parser
 * @ingroup Efreet
 *
 * @{
 */

/**
 * Efreet_Ini
 */
typedef struct Efreet_Ini Efreet_Ini;

/**
 * Efreet_Ini
 * @brief Contains all the information about an ini file.
 */
struct Efreet_Ini
{
  Eina_Hash *data;     /**< Hash of string => (Hash of string => string) */
  Eina_Hash *section;  /**< currently selected section */
};


/**
 * @param file The file to parse
 * @return Returns a new Efreet_Ini structure initialized with the contents
 * of @a file, or NULL on memory allocation failure
 * @brief Creates and initializes a new Ini structure with the contents of
 * @a file, or NULL on failure
 */
EAPI Efreet_Ini  *efreet_ini_new(const char *file);

/**
 * @param ini The Efreet_Ini to work with
 * @return Returns no value
 * @brief Frees the given Efree_Ini structure.
 */
EAPI void         efreet_ini_free(Efreet_Ini *ini);

/**
 * @param ini The Efreet_Ini to work with
 * @param file The file to load
 * @return Returns no value
 * @brief Saves the given Efree_Ini structure.
 */
EAPI int          efreet_ini_save(Efreet_Ini *ini, const char *path);


/**
 * @param ini The Efreet_Ini to work with
 * @param section The section of the ini file we want to get values from
 * @return Returns 1 if the section exists, otherwise 0
 * @brief Sets the current working section of the ini file to @a section
 */
EAPI int          efreet_ini_section_set(Efreet_Ini *ini, const char *section);

/**
 * @param ini The Efreet_Ini to work with
 * @param section The section of the ini file we want to add
 * @return Returns no value
 * @brief Adds a new working section of the ini file to @a section
 */
EAPI void         efreet_ini_section_add(Efreet_Ini *ini, const char *section);


/**
 * @param ini The Efree_Ini to work with
 * @param key The key to lookup
 * @return Returns the string associated with the given key or NULL if not
 * found.
 * @brief Retrieves the value for the given key or NULL if none found.
 */
EAPI const char  *efreet_ini_string_get(Efreet_Ini *ini, const char *key);

/**
 * @param ini The Efree_Ini to work with
 * @param key The key to use
 * @param value The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
EAPI void         efreet_ini_string_set(Efreet_Ini *ini, const char *key,
                                                    const char *value);


/**
 * @param ini The ini struct to work with
 * @param key The key to search for
 * @return Returns the utf8 encoded string associated with @a key, or NULL
 *         if none found
 * @brief Retrieves the utf8 encoded string associated with @a key in the current locale or NULL if none found
 */
EAPI const char  *efreet_ini_localestring_get(Efreet_Ini *ini, const char *key);

/**
 * @param ini The ini struct to work with
 * @param key The key to use
 * @param value The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
EAPI void         efreet_ini_localestring_set(Efreet_Ini *ini, const char *key,
                                                    const char *value);


/**
 * @param ini The ini struct to work with
 * @param key The key to search for
 * @return Returns 1 if the boolean is true, 0 otherwise
 * @brief Retrieves the boolean value at key @a key from the ini @a ini
 */
EAPI unsigned int efreet_ini_boolean_get(Efreet_Ini *ini, const char *key);

/**
 * @param ini The ini struct to work with
 * @param key The key to use
 * @param value The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
EAPI void         efreet_ini_boolean_set(Efreet_Ini *ini, const char *key,
                                                    unsigned int value);


/**
 * @param ini The Efree_Ini to work with
 * @param key The key to lookup
 * @return Returns the integer associated with the given key or -1 if not
 * found.
 * @brief Retrieves the value for the given key or -1 if none found.
 */
EAPI int          efreet_ini_int_get(Efreet_Ini *ini, const char *key);

/**
 * @param ini The Efree_Ini to work with
 * @param key The key to use
 * @param value The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
EAPI void         efreet_ini_int_set(Efreet_Ini *ini, const char *key, int value);


/**
 * @param ini The Efree_Ini to work with
 * @param key The key to lookup
 * @return Returns the double associated with the given key or -1 if not
 * found.
 * @brief Retrieves the value for the given key or -1 if none found.
 */
EAPI double       efreet_ini_double_get(Efreet_Ini *ini, const char *key);

/**
 * @param ini The Efree_Ini to work with
 * @param key The key to use
 * @param value The value to set
 * @return Returns no value
 * @brief Sets the value for the given key
 */
EAPI void         efreet_ini_double_set(Efreet_Ini *ini, const char *key,
                                                    double value);


/**
 * @param ini The ini struct to work with
 * @param key The key to remove
 * @return Returns no value
 * @brief Remove the given key from the ini struct
 */
EAPI void         efreet_ini_key_unset(Efreet_Ini *ini, const char *key);

/**
 * @}
 */

#endif
