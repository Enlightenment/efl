#include "main.h"

static Eina_Strbuf *
_type_generate(const Eolian_Typedecl *tp, Eina_Bool full, Eina_Bool legacy)
{
   return NULL;
}

void eo_gen_types_header_gen(const char *eof, Eina_Strbuf *buf,
                             Eina_Bool full, Eina_Bool legacy)
{
   const Eolian_Declaration *decl;

   Eina_Iterator *itr = eolian_declarations_get_by_file(eof);
   EINA_ITERATOR_FOREACH(itr, decl)
     {
        Eolian_Declaration_Type dt = eolian_declaration_type_get(decl);
        if ((dt != EOLIAN_DECL_ALIAS) &&
            (dt != EOLIAN_DECL_STRUCT) &&
            (dt != EOLIAN_DECL_ENUM))
          continue;
        if (dt == EOLIAN_DECL_ENUM && !full)
          continue;

        const Eolian_Typedecl *tp = eolian_declaration_data_type_get(decl);
        if (!tp || eolian_typedecl_is_extern(tp))
          continue;

        if (eolian_typedecl_type_get(tp) == EOLIAN_TYPEDECL_ALIAS)
          {
             const Eolian_Type *btp = eolian_typedecl_base_type_get(tp);
             if (eolian_type_type_get(btp) == EOLIAN_TYPE_UNDEFINED)
               continue;
          }

        Eina_Strbuf *tbuf = _type_generate(tp, full, legacy);
        if (tbuf)
          {
             eina_strbuf_append(buf, eina_strbuf_string_get(tbuf));
             eina_strbuf_append(buf, ";\n\n");
             eina_strbuf_free(tbuf);
          }
     }
}

Eina_Strbuf *eo_gen_class_typedef_gen(const char *eof)
{
   const Eolian_Class *cl = eolian_class_get_by_file(eof);
   if (!cl)
     return NULL;
   char *clfn = eo_gen_class_full_name_get(cl);
   if (!clfn)
     return NULL;
   Eina_Strbuf *ret = eina_strbuf_new();
   eina_strbuf_append_printf(ret, "typedef Eo %s;\n", clfn);
   free(clfn);
   return ret;
}
