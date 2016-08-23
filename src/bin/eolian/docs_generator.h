#ifndef DOCS_GENERATOR_H
#define DOCS_GENERATOR_H

#include <Eina.h>
#include <Eolian.h>

/*
 * @brief Generate standard documentation
 *
 * @param[in] doc the documentation
 * @param[in] group the group to use (can be NULL)
 * @param[in] indent by how many spaces to indent the comment from second line
 * @param[in] use_legacy whether to use legacy names
 *
 * @return A documentation comment
 *
 */
Eina_Strbuf *docs_generate_full(const Eolian_Documentation *doc, const char *group, int indent, Eina_Bool use_legacy);

/*
 * @brief Generate function documentation
 *
 * @param[in] fid te function
 * @param[in] type the function type (either METHOD, PROP_GET, PROP_SET)
 * @param[in] indent by how many spaces to indent the comment from second line
 * @param[in] use_legacy whether to use legacy names
 *
 * @return A documentation comment
 *
 */
Eina_Strbuf *docs_generate_function(const Eolian_Function *fid, Eolian_Function_Type ftype, int indent, Eina_Bool use_legacy);

/*
 * @brief Generate event documentation
 *
 * @param[in] ev the event
 * @param[in] group the group to use (can be NULL);
 *
 * @return A documentation comment
 *
 */
Eina_Strbuf *docs_generate_event(const Eolian_Event *ev, const char *group);

#endif

