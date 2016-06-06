#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <string.h>
#include <ctype.h>

#include "Eolian.h"
#include "types_generator.h"
#include "docs_generator.h"
#include "common_funcs.h"

static char *
_concat_name(const Eolian_Typedecl *tp)
{
   const char *name;
   char *str = NULL;
   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Iterator *itr = eolian_typedecl_namespaces_get(tp);
   EINA_ITERATOR_FOREACH(itr, name)
      if (name) eina_strbuf_append_printf(buf, "%s_", name);
   eina_iterator_free(itr);
   name = eolian_typedecl_name_get(tp);
   if (name) eina_strbuf_append_printf(buf, "%s", name);
   if (eina_strbuf_length_get(buf))
     {
        char *tmp = str = eina_strbuf_string_steal(buf);
        *tmp = toupper(*tmp);
        while (*tmp) if (*tmp++ == '_' && *tmp) *tmp = toupper(*tmp);
     }
   eina_strbuf_free(buf);
   return str;
}

static Eina_Strbuf *
_type_generate(const Eolian_Typedecl *tp, Eina_Bool full, Eina_Bool use_legacy)
{
   char *grp = strdup(eolian_typedecl_full_name_get(tp));
   char *p = strrchr(grp, '.');
   if (p) *p = '\0';
   Eina_Strbuf *buf = docs_generate_full(eolian_typedecl_documentation_get(tp),
                                         grp, 0, use_legacy);
   free(grp);
   if (!buf) buf = eina_strbuf_new();
   else eina_strbuf_append_char(buf, '\n');
   Eolian_Typedecl_Type tp_type = eolian_typedecl_type_get(tp);
   switch(tp_type)
     {
      case EOLIAN_TYPEDECL_ALIAS:
           {
              Eina_Stringshare *tn = eolian_typedecl_c_type_get(tp);
              eina_strbuf_append(buf, tn);
              eina_stringshare_del(tn);
              break;
           }
      case EOLIAN_TYPEDECL_STRUCT:
      case EOLIAN_TYPEDECL_STRUCT_OPAQUE:
           {
              const Eolian_Struct_Type_Field *member;
              char *name = _concat_name(tp);
              if (tp_type == EOLIAN_TYPEDECL_STRUCT_OPAQUE || !full)
                {
                   eina_strbuf_append_printf(buf, "typedef struct _%s %s", name, name);
                   free(name);
                   break;
                }
              eina_strbuf_append_printf(buf, "typedef struct _%s\n{\n", name);
              Eina_Iterator *members = eolian_typedecl_struct_fields_get(tp);
              EINA_ITERATOR_FOREACH(members, member)
                {
                   const Eolian_Type *type = eolian_typedecl_struct_field_type_get(member);
                   Eina_Stringshare *c_type = eolian_type_c_type_get(type);
                   Eina_Bool is_ref = eolian_typedecl_struct_field_is_ref(member);
                   eina_strbuf_append_printf(buf, "  %s%s%s%s;",
                         c_type, is_ref ? (strchr(c_type, '*') ? "*" : " *") : "", (is_ref || strchr(c_type, '*'))?"":" ",
                         eolian_typedecl_struct_field_name_get(member));
                   const Eolian_Documentation *fdoc
                       = eolian_typedecl_struct_field_documentation_get(member);
                   if (fdoc)
                     {
                        const char *nl = strrchr(eina_strbuf_string_get(buf), '\n');
                        if (nl)
                          {
                             Eina_Strbuf *fbuf = docs_generate_full(fdoc, NULL, strlen(nl), use_legacy);
                             if (fbuf)
                               eina_strbuf_append_printf(buf, " %s",
                                                         eina_strbuf_string_get(fbuf));
                             eina_strbuf_free(fbuf);
                          }
                     }
                   eina_strbuf_append(buf, "\n");
                }
              eina_iterator_free(members);
              eina_strbuf_append_printf(buf, "} %s", name);
              free(name);
              break;
           }
      case EOLIAN_TYPEDECL_ENUM:
           {
              const Eolian_Enum_Type_Field *member;
              char *name;
              if (!full)
                break;
              name = _concat_name(tp);
              char *pre = NULL;
              eina_strbuf_append_printf(buf, "typedef enum\n{\n");
              if (eolian_typedecl_enum_legacy_prefix_get(tp))
                pre = strdup(eolian_typedecl_enum_legacy_prefix_get(tp));
              else
                pre = strdup(name);
              eina_str_toupper(&pre);
              Eina_Iterator *members = eolian_typedecl_enum_fields_get(tp);
              Eina_Bool next = eina_iterator_next(members, (void**)&member);
              Eina_Strbuf *membuf = eina_strbuf_new();
              while (next)
                {
                   const Eolian_Expression *value = eolian_typedecl_enum_field_value_get(member, EINA_FALSE);
                   char *memb_u = strdup(eolian_typedecl_enum_field_name_get(member));
                   eina_str_toupper(&memb_u);
                   eina_strbuf_reset(membuf);
                   eina_strbuf_append(membuf, pre);
                   eina_strbuf_append_char(membuf, '_');
                   eina_strbuf_append(membuf, memb_u);
                   free(memb_u);
                   if (!value)
                     eina_strbuf_append_printf(buf, "  %s", eina_strbuf_string_get(membuf));
                   else
                     {
                        Eolian_Value val = eolian_expression_eval(value, EOLIAN_MASK_INT);
                        const char *lit = eolian_expression_value_to_literal(&val);
                        eina_strbuf_append_printf(buf, "  %s = %s", eina_strbuf_string_get(membuf), lit);
                        const char *exp = eolian_expression_serialize(value);
                        if (exp && strcmp(lit, exp))
                          {
                             eina_strbuf_append_printf(buf, " /* %s */", exp);
                             eina_stringshare_del(exp);
                          }
                        eina_stringshare_del(lit);
                     }
                   const Eolian_Documentation *fdoc
                       = eolian_typedecl_enum_field_documentation_get(member);
                   next = eina_iterator_next(members, (void**)&member);
                   if (next)
                     eina_strbuf_append(buf, ",");
                   if (fdoc)
                     {
                        const char *nl = strrchr(eina_strbuf_string_get(buf), '\n');
                        if (nl)
                          {
                             Eina_Strbuf *fbuf = docs_generate_full(fdoc, NULL, strlen(nl), use_legacy);
                             if (fbuf)
                               eina_strbuf_append_printf(buf, " %s",
                                                         eina_strbuf_string_get(fbuf));
                             eina_strbuf_free(fbuf);
                          }
                     }
                   eina_strbuf_append(buf, "\n");
                }
              eina_strbuf_append_printf(buf, "} %s", name);
              eina_strbuf_free(membuf);
              free(name);
              free(pre);
              eina_iterator_free(members);
              break;
           }
      default:
           {
              eina_strbuf_reset(buf);
           }
     }
   return buf;
}

Eina_Bool
types_header_generate(const char *eo_filename, Eina_Strbuf *buf, Eina_Bool full, Eina_Bool use_legacy)
{
   const Eolian_Declaration *decl;

   Eina_Iterator *itr = eolian_declarations_get_by_file(eo_filename);
   EINA_ITERATOR_FOREACH(itr, decl)
     {
        Eolian_Declaration_Type dt = eolian_declaration_type_get(decl);
        if (dt != EOLIAN_DECL_ALIAS &&
            dt != EOLIAN_DECL_STRUCT &&
            dt != EOLIAN_DECL_ENUM)
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

        Eina_Strbuf *tbuf = _type_generate(tp, full, use_legacy);
        if (tbuf)
          {
             eina_strbuf_append(buf, eina_strbuf_string_get(tbuf));
             eina_strbuf_append(buf, ";\n\n");
             eina_strbuf_free(tbuf);
          }
     }
   eina_iterator_free(itr);

   return EINA_TRUE;
}

Eina_Bool
types_class_typedef_generate(const char *eo_filename, Eina_Strbuf *buf)
{
   const Eolian_Class *class = eolian_class_get_by_file(eo_filename);
   if (!class)
     return EINA_FALSE;

   static _eolian_class_vars class_env;
   _class_env_create(class, NULL, &class_env);

   eina_strbuf_append_printf(buf, "typedef Eo %s;\n", class_env.full_classname);
   return EINA_TRUE;
}
