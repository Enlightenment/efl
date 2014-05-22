#ifndef IMPL_GENERATOR_H
#define IMPL_GENERATOR_H

#include<Eina.h>

/*
 * @brief Generate the implementation source code of a class
 *
 * This function generates all the source code of a class.
 *
 * @param[in] classname class name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool
impl_source_generate(const Eolian_Class class, Eina_Strbuf *buf);

#endif


