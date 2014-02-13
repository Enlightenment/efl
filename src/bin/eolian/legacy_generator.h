#ifndef __EOLIAN_LEGACY_GENERATOR_H
#define __EOLIAN_LEGACY_GENERATOR_H

#include <Eina.h>

/*
 * @brief Generate legacy EAPI header for Eo class
 *
 * This function needs to be used in case we want to generate a function
 * from scratch.
 * There will not be respect of the order of the Eo Op Ids.
 *
 * @param[in] classname class name
 * @param[in] eo_version Eo version to generate
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool legacy_header_generate(const char *classname, int eo_version, Eina_Strbuf *buf);

/*
 * @brief Append legacy EAPI header for Eo class
 *
 * This function needs to be used in case we want to add new functions
 * to an existing class.
 *
 * @param[in] classname class name
 * @param[in] eo_version Eo version to generate
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool legacy_header_append(const char *classname, int eo_version, Eina_Strbuf *buf);

/*
 * @brief Generate C source code for Eo class
 *
 * This function needs to be used to generate C source code. It is generating
 * code from scratch.
 *
 * @param[in] classname class name
 * @param[in] legacy indicates if the legacy has to be generated
 * @param[in] eo_version Eo version to generate
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool legacy_source_generate(const char *classname, Eina_Bool legacy, int eo_version, Eina_Strbuf *buf);

#endif

