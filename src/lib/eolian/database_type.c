#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eo_lexer.h"

void
database_type_del(Eolian_Type *tp)
{
   if (!tp) return;
   const char *sp;
   if (tp->base.file) eina_stringshare_del(tp->base.file);
   database_type_del(tp->base_type);
   database_type_del(tp->next_type);
   if (tp->name) eina_stringshare_del(tp->name);
   if (tp->full_name) eina_stringshare_del(tp->full_name);
   if (tp->namespaces) EINA_LIST_FREE(tp->namespaces, sp)
     eina_stringshare_del(sp);
   if (tp->freefunc) eina_stringshare_del(tp->freefunc);
   free(tp);
}

void
database_typedecl_del(Eolian_Typedecl *tp)
{
   if (!tp) return;
   const char *sp;
   if (tp->base.file) eina_stringshare_del(tp->base.file);
   database_type_del(tp->base_type);
   if (tp->name) eina_stringshare_del(tp->name);
   if (tp->full_name) eina_stringshare_del(tp->full_name);
   if (tp->fields) eina_hash_free(tp->fields);
   if (tp->field_list) eina_list_free(tp->field_list);
   if (tp->namespaces) EINA_LIST_FREE(tp->namespaces, sp)
     eina_stringshare_del(sp);
   if (tp->legacy) eina_stringshare_del(tp->legacy);
   if (tp->freefunc) eina_stringshare_del(tp->freefunc);
   database_doc_del(tp->doc);
   free(tp);
}

void
database_type_add(Eolian_Typedecl *def)
{
   eina_hash_set(_aliases, def->full_name, def);
   eina_hash_set(_aliasesf, def->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(_aliasesf, def->base.file), def));
   database_decl_add(def->full_name, EOLIAN_DECL_ALIAS, def->base.file, def);
}

void
database_struct_add(Eolian_Typedecl *tp)
{
   eina_hash_set(_structs, tp->full_name, tp);
   eina_hash_set(_structsf, tp->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(_structsf, tp->base.file), tp));
   database_decl_add(tp->full_name, EOLIAN_DECL_STRUCT, tp->base.file, tp);
}

void
database_enum_add(Eolian_Typedecl *tp)
{
   eina_hash_set(_enums, tp->full_name, tp);
   eina_hash_set(_enumsf, tp->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(_enumsf, tp->base.file), tp));
   database_decl_add(tp->full_name, EOLIAN_DECL_ENUM, tp->base.file, tp);
}

void
database_type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name)
{
   if ((tp->type == EOLIAN_TYPE_REGULAR
     || tp->type == EOLIAN_TYPE_COMPLEX
     || tp->type == EOLIAN_TYPE_VOID
     || tp->type == EOLIAN_TYPE_CLASS)
     && tp->is_const)
     {
        eina_strbuf_append(buf, "const ");
     }
   if (tp->type == EOLIAN_TYPE_REGULAR
    || tp->type == EOLIAN_TYPE_COMPLEX
    || tp->type == EOLIAN_TYPE_CLASS)
     {
        Eina_List *l;
        const char *sp;
        EINA_LIST_FOREACH(tp->namespaces, l, sp)
          {
             eina_strbuf_append(buf, sp);
             eina_strbuf_append_char(buf, '_');
          }
        int kw = eo_lexer_keyword_str_to_id(tp->name);
        if (kw && eo_lexer_is_type_keyword(kw))
          eina_strbuf_append(buf, eo_lexer_get_c_type(kw));
        else
          eina_strbuf_append(buf, tp->name);
     }
   else if (tp->type == EOLIAN_TYPE_VOID)
     eina_strbuf_append(buf, "void");
   else if (tp->type == EOLIAN_TYPE_UNDEFINED)
     eina_strbuf_append(buf, "__undefined_type");
   else
     {
        Eolian_Type *btp = tp->base_type;
        database_type_to_str(tp->base_type, buf, NULL);
        if (btp->type != EOLIAN_TYPE_POINTER || btp->is_const)
           eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append_char(buf, '*');
        if (tp->is_const) eina_strbuf_append(buf, " const");
     }
   if (tp->type == EOLIAN_TYPE_COMPLEX || tp->type == EOLIAN_TYPE_CLASS)
     eina_strbuf_append(buf, " *");
   if (tp->is_ref)
     {
        if (eina_strbuf_string_get(buf)[eina_strbuf_length_get(buf) - 1] != '*')
          eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append_char(buf, '*');
     }
   if (name)
     {
        if (eina_strbuf_string_get(buf)[eina_strbuf_length_get(buf) - 1] != '*')
          eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, name);
     }
}

static void
_stype_to_str(const Eolian_Typedecl *tp, Eina_Strbuf *buf)
{
   Eolian_Struct_Type_Field *sf;
   Eina_List *l;
   const char *sp;
   eina_strbuf_append(buf, "struct ");
   EINA_LIST_FOREACH(tp->namespaces, l, sp)
     {
        eina_strbuf_append(buf, sp);
        eina_strbuf_append_char(buf, '_');
     }
    eina_strbuf_append(buf, tp->name);
   if (tp->type == EOLIAN_TYPEDECL_STRUCT_OPAQUE)
     return;
   eina_strbuf_append(buf, " { ");
   EINA_LIST_FOREACH(tp->field_list, l, sf)
     {
        database_type_to_str(sf->type, buf, sf->name);
        eina_strbuf_append(buf, "; ");
     }
   eina_strbuf_append(buf, "}");
}

static void
_etype_to_str(const Eolian_Typedecl *tp, Eina_Strbuf *buf)
{
   Eolian_Enum_Type_Field *ef;
   Eina_List *l;
   const char *sp;
   eina_strbuf_append(buf, "enum ");
   EINA_LIST_FOREACH(tp->namespaces, l, sp)
     {
        eina_strbuf_append(buf, sp);
        eina_strbuf_append_char(buf, '_');
     }
   eina_strbuf_append(buf, tp->name);
   eina_strbuf_append(buf, " { ");
   EINA_LIST_FOREACH(tp->field_list, l, ef)
     {
        eina_strbuf_append(buf, ef->name);
        if (ef->value)
          {
             Eolian_Value val = eolian_expression_eval(ef->value,
                 EOLIAN_MASK_INT);
             const char *ret;
             eina_strbuf_append(buf, " = ");
             ret = eolian_expression_value_to_literal(&val);
             eina_strbuf_append(buf, ret);
             eina_stringshare_del(ret);
          }
        if (l != eina_list_last(tp->field_list))
          eina_strbuf_append(buf, ", ");
     }
   eina_strbuf_append(buf, " }");
}

static void
_append_name(const Eolian_Typedecl *tp, Eina_Strbuf *buf)
{
   Eina_List *l;
   const char *sp;

   EINA_LIST_FOREACH(tp->namespaces, l, sp)
     {
        eina_strbuf_append(buf, sp);
        eina_strbuf_append_char(buf, '_');
     }
   eina_strbuf_append(buf, tp->name);
}

static void
_atype_to_str(const Eolian_Typedecl *tp, Eina_Strbuf *buf)
{
   eina_strbuf_append(buf, "typedef ");

   if (tp->base_type->type == EOLIAN_TYPE_REGULAR &&
       !strcmp(tp->base_type->name, "__builtin_event_cb"))
     {
        eina_strbuf_append(buf, "Eina_Bool (*");
        _append_name(tp, buf);
        eina_strbuf_append(buf, ")(void *data, const Eo_Event *event)");
        return;
     }

   Eina_Strbuf *fulln = eina_strbuf_new();
   _append_name(tp, fulln);
   database_type_to_str(tp->base_type, buf, eina_strbuf_string_get(fulln));
   eina_strbuf_free(fulln);
}

void
database_typedecl_to_str(const Eolian_Typedecl *tp, Eina_Strbuf *buf)
{
   switch (tp->type)
     {
      case EOLIAN_TYPEDECL_ALIAS:
        _atype_to_str(tp, buf);
        break;
      case EOLIAN_TYPEDECL_ENUM:
        _etype_to_str(tp, buf);
        break;
      case EOLIAN_TYPEDECL_STRUCT:
      case EOLIAN_TYPEDECL_STRUCT_OPAQUE:
        _stype_to_str(tp, buf);
        break;
      default:
        break;
     }
}
