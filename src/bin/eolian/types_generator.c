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
_concat_name(const Eolian_Type *tp)
{
   const char *name;
   char *str = NULL;
   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Iterator *itr = eolian_type_namespaces_get(tp);
   EINA_ITERATOR_FOREACH(itr, name)
      if (name) eina_strbuf_append_printf(buf, "%s_", name);
   eina_iterator_free(itr);
   name = eolian_type_name_get(tp);
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

static void
_desc_generate(const char *desc, Eina_Strbuf *buf)
{
   if (desc)
     {
        eina_strbuf_append(buf, "/**\n");
        eina_strbuf_append(buf, desc);
        eina_strbuf_replace_all(buf, "\n", "\n * ");
        eina_strbuf_append(buf, "\n */\n");
        eina_strbuf_replace_all(buf, " * \n", " *\n"); /* Remove trailing whitespaces */
     }
}

static Eina_Strbuf *
_type_generate(const Eolian_Type *tp, Eina_Bool full)
{
   const char *cdesc = eolian_type_description_get(tp);
   Eina_Strbuf *buf;
   if (cdesc)
     {
        buf = eina_strbuf_new();
        _desc_generate(cdesc, buf);
     }
   else
     {
        buf = docs_generate_full(eolian_type_documentation_get(tp), 0);
        if (!buf) buf = eina_strbuf_new();
        else eina_strbuf_append_char(buf, '\n');
     }
   Eolian_Type_Type tp_type = eolian_type_type_get(tp);
   switch(tp_type)
     {
      case EOLIAN_TYPE_ALIAS:
           {
              char *name = _concat_name(tp);
              Eina_Stringshare *c_type = eolian_type_c_type_named_get(
                  eolian_type_base_type_get(tp), name);
              eina_strbuf_append_printf(buf, "typedef %s", c_type);
              eina_stringshare_del(c_type);
              free(name);
              break;
           }
      case EOLIAN_TYPE_STRUCT:
      case EOLIAN_TYPE_STRUCT_OPAQUE:
           {
              const Eolian_Struct_Type_Field *member;
              char *name = _concat_name(tp);
              if (tp_type == EOLIAN_TYPE_STRUCT_OPAQUE || !full)
                {
                   eina_strbuf_append_printf(buf, "typedef struct _%s %s", name, name);
                   free(name);
                   break;
                }
              eina_strbuf_append_printf(buf, "typedef struct _%s\n{\n", name);
              Eina_Iterator *members = eolian_type_struct_fields_get(tp);
              EINA_ITERATOR_FOREACH(members, member)
                {
                   const Eolian_Type *type = eolian_type_struct_field_type_get(member);
                   Eina_Stringshare *c_type = eolian_type_c_type_get(type);
                   eina_strbuf_append_printf(buf, "  %s%s%s;",
                         c_type, strchr(c_type, '*')?"":" ",
                         eolian_type_struct_field_name_get(member));
                   const char *fdesc = eolian_type_struct_field_description_get(member);
                   const Eolian_Documentation *fdoc
                       = eolian_type_struct_field_documentation_get(member);
                   if (fdesc) eina_strbuf_append_printf(buf, " /** %s */", fdesc);
                   else if (fdoc)
                     {
                        const char *nl = strrchr(eina_strbuf_string_get(buf), '\n');
                        if (nl)
                          {
                             Eina_Strbuf *fbuf = docs_generate_full(fdoc, strlen(nl));
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
      case EOLIAN_TYPE_ENUM:
           {
              const Eolian_Enum_Type_Field *member;
              char *name;
              if (!full)
                break;
              name = _concat_name(tp);
              char *pre = NULL;
              eina_strbuf_append_printf(buf, "typedef enum\n{\n");
              if (eolian_type_enum_legacy_prefix_get(tp))
                pre = strdup(eolian_type_enum_legacy_prefix_get(tp));
              else
                pre = strdup(name);
              eina_str_toupper(&pre);
              Eina_Iterator *members = eolian_type_enum_fields_get(tp);
              Eina_Bool next = eina_iterator_next(members, (void**)&member);
              Eina_Strbuf *membuf = eina_strbuf_new();
              while (next)
                {
                   const Eolian_Expression *value = eolian_type_enum_field_value_get(member, EINA_FALSE);
                   char *memb_u = strdup(eolian_type_enum_field_name_get(member));
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
                   const char *fdesc = eolian_type_enum_field_description_get(member);
                   const Eolian_Documentation *fdoc
                       = eolian_type_enum_field_documentation_get(member);
                   next = eina_iterator_next(members, (void**)&member);
                   if (next)
                     eina_strbuf_append(buf, ",");
                   if (fdesc) eina_strbuf_append_printf(buf, " /** %s */", fdesc);
                   else if (fdoc)
                     {
                        const char *nl = strrchr(eina_strbuf_string_get(buf), '\n');
                        if (nl)
                          {
                             Eina_Strbuf *fbuf = docs_generate_full(fdoc, strlen(nl));
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
types_header_generate(const char *eo_filename, Eina_Strbuf *buf, Eina_Bool full)
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

        const Eolian_Type *tp = eolian_declaration_data_type_get(decl);
        if (!tp || eolian_type_is_extern(tp))
          continue;

        if (eolian_type_type_get(tp) == EOLIAN_TYPE_ALIAS)
          {
             const Eolian_Type *btp = eolian_type_base_type_get(tp);
             if (eolian_type_type_get(btp) == EOLIAN_TYPE_REGULAR)
               if (!strcmp(eolian_type_full_name_get(btp), "__undefined_type"))
                 continue;
          }

        Eina_Strbuf *tbuf = _type_generate(tp, full);
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
