#ifndef DOCS_GENERATOR_H
#define DOCS_GENERATOR_H

#include <Eina.h>
#include <Eolian.h>

/*
 * @brief Generate standard documentation
 *
 * @param[in] doc the documentation
 * @param[in] indent by how many spaces to indent the comment from second line
 *
 * @return A documentation comment
 *
 */
Eina_Strbuf *docs_generate_full(const Eolian_Documentation *doc, int indent);

/*
 * @brief Generate function documentation
 *
 * @param[in] fid te function
 * @param[in] type the function type (either METHOD, PROP_GET, PROP_SET)
 * @param[in] indent by how many spaces to indent the comment from second line
 *
 * @return A documentation comment
 *
 */
Eina_Strbuf *docs_generate_function(const Eolian_Function *fid, Eolian_Function_Type ftype, int indent);

#endif

