#ifndef EOLIAN_GEN_TYPES_H
#define EOLIAN_GEN_TYPES_H

void eo_gen_types_header_gen(const Eolian_State *state, Eina_Iterator *itr, Eina_Strbuf *buf, Eina_Bool full, Eina_Bool legacy);
void eo_gen_types_source_gen(Eina_Iterator *itr, Eina_Strbuf *buf);
Eina_Strbuf *eo_gen_class_typedef_gen(const Eolian_State *eos, const char *eof);

#endif
