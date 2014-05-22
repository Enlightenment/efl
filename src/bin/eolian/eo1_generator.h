#ifndef EO1_GENERATOR_H
#define EO1_GENERATOR_H

#include<Eina.h>

/*
 * @brief Generate Eo source code for Eo class
 *
 * This function generates all the source code for Eo.
 *
 * @param[in] classname class name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool
eo_source_generate(const Eolian_Class class, Eina_Strbuf *buf);

/*
 * @brief Generate the header code for a specific Eo class.
 *
 * This function generates header code from scratch.
 *
 * @param[in] classname class name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool eo_header_generate(const Eolian_Class class, Eina_Strbuf *buf);

#endif

