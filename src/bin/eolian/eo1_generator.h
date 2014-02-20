#ifndef EO1_GENERATOR_H
#define EO1_GENERATOR_H

#include<Eina.h>

/*
 * @brief Generate beginning of Eo1 source code for Eo class
 *
 * This function generates the base id definition and the list of events.
 *
 * @param[in] classname class name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool
eo1_source_beginning_generate(const char *classname, Eina_Strbuf *buf);

/*
 * @brief Generate end of Eo1 source code for Eo class
 *
 * This function generates the constructors, the class constructor, the function
 * descriptions and the class description.
 *
 * @param[in] classname class name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool
eo1_source_end_generate(const char *classname, Eina_Strbuf *buf);

/*
 * @brief Generate the source code for a specific Eo function.
 *
 * This function generates for a given function id the corresponding
 * Eo function.
 *
 * @param[in] classname class name
 * @param[in] funcid Function Id
 * @param[in] ftype type of the function (SET/GET/METHOD...)
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool
eo1_bind_func_generate(const char *classname, Eolian_Function funcid, Eolian_Function_Type ftype, Eina_Strbuf *buf);

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
Eina_Bool eo1_header_generate(const char *classname, Eina_Strbuf *buf);

/*
 * @brief Append the header code for a specific Eo class.
 *
 * This function generates header code by appending it into an existing class.
 *
 * @param[in] classname class name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool eo1_header_append(const char *classname, Eina_Strbuf *buf);

/*
 * @brief Fill the given buffer with the Eo enum of a given function.
 *
 * @param[in] classname class name
 * @param[in] funcname function name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool eo1_enum_append(const char *classname, const char *funcname, Eina_Strbuf *buf);

/*
 * @brief Generate the function definition (header) for a specific Eo function.
 *
 * This function generates for a given function the corresponding
 * Eo function definition.
 *
 * @param[in] classname class name
 * @param[in] funcid Function Id
 * @param[in] ftype type of the function (SET/GET/METHOD...)
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool
eo1_fundef_generate(const char *classname, Eolian_Function func, Eolian_Function_Type ftype, Eina_Strbuf *buf);

/*
 * @brief Generate the function description for a specific Eo function.
 *
 * This function generates for a given function the corresponding
 * Eo function description.
 *
 * @param[in] classname class name
 * @param[in] funcname function name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool
eo1_eo_func_desc_generate(const char *classname, const char *funcname, Eina_Strbuf *buf);

/*
 * @brief Generate the Eo op description for a specific Eo function.
 *
 * This function generates for a given function the corresponding
 * Eo function definition.
 *
 * @param[in] classname class name
 * @param[in] funcname function name
 * @param[inout] buf buffer to fill
 *
 * @return EINA_TRUE on success, EINA_FALSE on error.
 *
 */
Eina_Bool
eo1_eo_op_desc_generate(const char *classname, const char *funcname, Eina_Strbuf *buf);

#endif

