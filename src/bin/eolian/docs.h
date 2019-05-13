#ifndef EOLIAN_GEN_DOCS_H
#define EOLIAN_GEN_DOCS_H

#include "main.h"

/*
 * @brief Generate standard documentation
 *
 * @param[in] doc the documentation
 * @param[in] group the group to use (can be NULL)
 * @param[in] indent by how many spaces to indent the comment from second line
 *
 * @return A documentation comment
 *
 */
Eina_Strbuf *eo_gen_docs_full_gen(const Eolian_State *state, const Eolian_Documentation *doc, const char *group, int indent);

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
Eina_Strbuf *eo_gen_docs_func_gen(const Eolian_State *state, const Eolian_Function *fid, Eolian_Function_Type ftype, int indent);

/*
 * @brief Generate event documentation
 *
 * @param[in] ev the event
 * @param[in] group the group to use (can be NULL);
 *
 * @return A documentation comment
 *
 */
Eina_Strbuf *eo_gen_docs_event_gen(const Eolian_State *state, const Eolian_Event *ev, const char *group);

#endif

