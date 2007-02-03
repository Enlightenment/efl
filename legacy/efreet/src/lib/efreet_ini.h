/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_INI_H
#define EFREET_INI_H

/**
 * @internal
 * @file efreet_ini.h
 * @brief A simple and fast INI parser
 * @addtogroup Efreet_Ini Efreet_Ini: An INI parser 
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
  Ecore_Hash *data;     /**< Hash of string => (Hash of string => string) */
  Ecore_Hash *section;  /**< currently selected section */
};

Efreet_Ini  *efreet_ini_new(const char *file);
void         efreet_ini_free(Efreet_Ini *ini);
int          efreet_ini_save(Efreet_Ini *ini, const char *path);

int          efreet_ini_section_set(Efreet_Ini *ini, const char *section);
void         efreet_ini_section_add(Efreet_Ini *ini, const char *section);

const char  *efreet_ini_string_get(Efreet_Ini *ini, const char *key);
void         efreet_ini_string_set(Efreet_Ini *ini, const char *key, 
                                                    const char *value);

const char  *efreet_ini_localestring_get(Efreet_Ini *ini, const char *key);
void         efreet_ini_localestring_set(Efreet_Ini *ini, const char *key, 
                                                    const char *value);

unsigned int efreet_ini_boolean_get(Efreet_Ini *ini, const char *key);
void         efreet_ini_boolean_set(Efreet_Ini *ini, const char *key, 
                                                    unsigned int value);

int          efreet_ini_int_get(Efreet_Ini *ini, const char *key);
void         efreet_ini_int_set(Efreet_Ini *ini, const char *key, int value);

double       efreet_ini_double_get(Efreet_Ini *ini, const char *key);
void         efreet_ini_double_set(Efreet_Ini *ini, const char *key, 
                                                    double value);

/**
 * @}
 */

#endif

