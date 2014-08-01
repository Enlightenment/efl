#ifndef TYPES_GENERATOR_H
#define TYPES_GENERATOR_H

#include<Eina.h>

/*
 * @brief Generate the header code for the types of a specific file.
 *
 * @param[in] eo_filename Eo filename
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool types_header_generate(const char *eo_filename, Eina_Strbuf *buf);

Eina_Bool types_class_typedef_generate(const char *eo_filename, Eina_Strbuf *buf);

#endif

