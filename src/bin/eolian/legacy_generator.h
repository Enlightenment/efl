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
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool legacy_header_generate(const Eolian_Class class, Eina_Strbuf *buf);

/*
 * @brief Generate C source code for Eo class
 *
 * This function needs to be used to generate C source code. It is generating
 * code from scratch.
 *
 * @param[in] classname class name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool legacy_source_generate(const Eolian_Class class, Eina_Strbuf *buf);

#endif

