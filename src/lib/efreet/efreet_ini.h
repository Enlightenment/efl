#ifndef EFREET_INI_H
#define EFREET_INI_H

/**
 * @internal
 * @file efreet_ini.h
 * @brief A simple and fast INI parser
 *
 * @defgroup Efreet_Ini Efreet_Ini: An INI parser
 * @ingroup Efreet_Group
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
 * @brief Creates and initializes a new Ini structure with the contents of
 * @a file, or NULL on failure
 *
 * @param[in] file The file to parse
 * @return Returns a new Efreet_Ini structure initialized with the contents
 * of @a file, or NULL on memory allocation failure
 */
EAPI Efreet_Ini  *efreet_ini_new(const char *file);

/**
 * @brief Frees the given Efree_Ini structure.
 *
 * @param[in] ini The Efreet_Ini to work with
 * @return Returns no value
 */
EAPI void         efreet_ini_free(Efreet_Ini *ini);

/**
 * @brief Saves the given Efree_Ini structure.
 *
 * @param[in] ini The Efreet_Ini to work with
 * @param[in] path The file path to load
 * @return Returns no value
 */
EAPI int          efreet_ini_save(Efreet_Ini *ini, const char *path);


/**
 * @brief Sets the current working section of the ini file to @a section
 *
 * @param[in] ini The Efreet_Ini to work with
 * @param[in] section The section of the ini file we want to get values from
 * @return Returns 1 if the section exists, otherwise 0
 */
EAPI int          efreet_ini_section_set(Efreet_Ini *ini, const char *section);

/**
 * @brief Adds a new working section of the ini file to @a section
 *
 * @param[in] ini The Efreet_Ini to work with
 * @param[in] section The section of the ini file we want to add
 * @return Returns no value
 */
EAPI void         efreet_ini_section_add(Efreet_Ini *ini, const char *section);


/**
 * @brief Retrieves the value for the given key or NULL if none found.
 *
 * @param[in] ini The Efree_Ini to work with
 * @param[in] key The key to lookup
 * @return Returns the string associated with the given key or NULL if not
 * found.
 */
EAPI const char  *efreet_ini_string_get(Efreet_Ini *ini, const char *key);

/**
 * @brief Sets the value for the given key
 *
 * @param[in] ini The Efree_Ini to work with
 * @param[in] key The key to use
 * @param[in] value The value to set
 * @return Returns no value
 */
EAPI void         efreet_ini_string_set(Efreet_Ini *ini, const char *key,
                                                    const char *value);


/**
 * @brief Retrieves the utf8 encoded string associated with @a key in the current locale or NULL if none found
 *
 * @param[in] ini The ini struct to work with
 * @param[in] key The key to search for
 * @return Returns the utf8 encoded string associated with @a key, or NULL
 *         if none found
 */
EAPI const char  *efreet_ini_localestring_get(Efreet_Ini *ini, const char *key);

/**
 * @brief Sets the value for the given key
 *
 * @param[in] ini The ini struct to work with
 * @param[in] key The key to use
 * @param[in] value The value to set
 * @return Returns no value
 */
EAPI void         efreet_ini_localestring_set(Efreet_Ini *ini, const char *key,
                                                    const char *value);


/**
 * @brief Retrieves the boolean value at key @a key from the ini @a ini
 *
 * @param[in] ini The ini struct to work with
 * @param[in] key The key to search for
 * @return Returns 1 if the boolean is true, 0 otherwise
 */
EAPI unsigned int efreet_ini_boolean_get(Efreet_Ini *ini, const char *key);

/**
 * @brief Sets the value for the given key
 *
 * @param[in] ini The ini struct to work with
 * @param[in] key The key to use
 * @param[in] value The value to set
 * @return Returns no value
 */
EAPI void         efreet_ini_boolean_set(Efreet_Ini *ini, const char *key,
                                                    unsigned int value);


/**
 * @brief Retrieves the value for the given key or -1 if none found.
 *
 * @param[in] ini The Efree_Ini to work with
 * @param[in] key The key to lookup
 * @return Returns the integer associated with the given key or -1 if not
 * found.
 */
EAPI int          efreet_ini_int_get(Efreet_Ini *ini, const char *key);

/**
 * @brief Sets the value for the given key
 *
 * @param[in] ini The Efree_Ini to work with
 * @param[in] key The key to use
 * @param[in] value The value to set
 * @return Returns no value
 */
EAPI void         efreet_ini_int_set(Efreet_Ini *ini, const char *key, int value);


/**
 * @brief Retrieves the value for the given key or -1 if none found.
 *
 * @param[in] ini The Efree_Ini to work with
 * @param[in] key The key to lookup
 * @return Returns the double associated with the given key or -1 if not
 * found.
 */
EAPI double       efreet_ini_double_get(Efreet_Ini *ini, const char *key);

/**
 * @brief Sets the value for the given key
 *
 * @param[in] ini The Efree_Ini to work with
 * @param[in] key The key to use
 * @param[in] value The value to set
 * @return Returns no value
 */
EAPI void         efreet_ini_double_set(Efreet_Ini *ini, const char *key,
                                                    double value);


/**
 * @brief Remove the given key from the ini struct
 *
 * @param[in] ini The ini struct to work with
 * @param[in] key The key to remove
 * @return Returns no value
 */
EAPI void         efreet_ini_key_unset(Efreet_Ini *ini, const char *key);

/**
 * @}
 */

#endif
