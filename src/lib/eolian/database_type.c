#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eo_lexer.h"

void
database_type_del(Eolian_Type *tp)
{
   if (!tp || eolian_object_unref(&tp->base)) return;
   eina_stringshare_del(tp->base.file);
   eina_stringshare_del(tp->base.name);
   eina_stringshare_del(tp->base.c_name);
   database_type_del(tp->base_type);
   database_type_del(tp->next_type);
   free(tp);
}

void
database_typedecl_del(Eolian_Typedecl *tp)
{
   if (!tp || eolian_object_unref(&tp->base)) return;
   eina_stringshare_del(tp->base.file);
   eina_stringshare_del(tp->base.name);
   eina_stringshare_del(tp->base.c_name);
   database_type_del(tp->base_type);
   if (tp->fields) eina_hash_free(tp->fields);
   if (tp->field_list) eina_list_free(tp->field_list);
   database_function_del(tp->function_pointer);
   eina_stringshare_del(tp->freefunc);
   database_doc_del(tp->doc);
   free(tp);
}

void
database_type_add(Eolian_Unit *unit, Eolian_Typedecl *tp)
{
   EOLIAN_OBJECT_ADD(unit, tp->base.name, tp, aliases);
   eina_hash_set(unit->state->staging.aliases_f, tp->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(unit->state->staging.aliases_f, tp->base.file),
                tp));
   database_object_add(unit, &tp->base);
}

void
database_struct_add(Eolian_Unit *unit, Eolian_Typedecl *tp)
{
   EOLIAN_OBJECT_ADD(unit, tp->base.name, tp, structs);
   Eina_Hash *sh = unit->state->staging.structs_f;
   eina_hash_set(sh, tp->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(sh, tp->base.file), tp));
   database_object_add(unit, &tp->base);
}

void
database_enum_add(Eolian_Unit *unit, Eolian_Typedecl *tp)
{
   EOLIAN_OBJECT_ADD(unit, tp->base.name, tp, enums);
   eina_hash_set(unit->state->staging.enums_f, tp->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(unit->state->staging.enums_f, tp->base.file), tp));
   database_object_add(unit, &tp->base);
}

Eina_Bool
database_type_is_ownable(const Eolian_Unit *unit, const Eolian_Type *tp, Eina_Bool allow_void, const Eolian_Type **otp)
{
   if (otp) *otp = tp;
   if (tp->is_ptr)
     return EINA_TRUE;
   if (tp->type == EOLIAN_TYPE_REGULAR)
     {
        int kw = eo_lexer_keyword_str_to_id(tp->base.name);
        const char *ct = eo_lexer_get_c_type(kw);
        if (!ct)
          {
             const Eolian_Typedecl *tpp = database_type_decl_find(unit, tp);
             if (!tpp)
               return EINA_FALSE;
             if (tpp->type == EOLIAN_TYPEDECL_FUNCTION_POINTER)
               return EINA_TRUE;
             if (tpp->type == EOLIAN_TYPEDECL_ALIAS)
               return database_type_is_ownable(unit, tpp->base_type, allow_void, otp);
             return EINA_FALSE;
          }
        return (ct[strlen(ct) - 1] == '*');
     }
   if (allow_void && (tp->type == EOLIAN_TYPE_VOID))
     return EINA_TRUE;
   return (tp->type == EOLIAN_TYPE_CLASS);
}

static void
_buf_add_suffix(Eina_Strbuf *buf, const char *suffix)
{
   if (!suffix) return;
   if (eina_strbuf_string_get(buf)[eina_strbuf_length_get(buf) - 1] != '*')
     eina_strbuf_append_char(buf, ' ');
   eina_strbuf_append(buf, suffix);
}

void
database_type_to_str(const Eolian_Type *tp,
                     Eina_Strbuf *buf, const char *name,
                     Eolian_C_Type_Type ctype, Eina_Bool by_ref)
{
   if ((tp->type == EOLIAN_TYPE_REGULAR
     || tp->type == EOLIAN_TYPE_CLASS
     || tp->type == EOLIAN_TYPE_VOID)
     && tp->is_const
     && ((ctype != EOLIAN_C_TYPE_RETURN) || by_ref || database_type_is_ownable(NULL, tp, EINA_FALSE, NULL)))
     {
        eina_strbuf_append(buf, "const ");
     }
   if (tp->type == EOLIAN_TYPE_REGULAR
    || tp->type == EOLIAN_TYPE_CLASS)
     {
        int kw = eo_lexer_keyword_str_to_id(tp->base.name);
        if (kw && eo_lexer_is_type_keyword(kw))
          eina_strbuf_append(buf, eo_lexer_get_c_type(kw));
        else
          eina_strbuf_append(buf, tp->base.c_name);
     }
   else if (tp->type == EOLIAN_TYPE_ERROR)
     eina_strbuf_append(buf, "Eina_Error");
   else if (tp->type == EOLIAN_TYPE_VOID)
     eina_strbuf_append(buf, "void");
   else if (tp->type == EOLIAN_TYPE_UNDEFINED)
     eina_strbuf_append(buf, "__undefined_type");
   else
     {
        /* handles arrays and pointers as they all serialize to pointers */
        database_type_to_str(tp->base_type, buf, NULL,
                             EOLIAN_C_TYPE_DEFAULT, EINA_FALSE);
        _buf_add_suffix(buf, "*");
        if (tp->is_const && (ctype != EOLIAN_C_TYPE_RETURN))
          eina_strbuf_append(buf, " const");
     }
   if (tp->type == EOLIAN_TYPE_CLASS)
     _buf_add_suffix(buf, "*");
   if (tp->is_ptr)
     _buf_add_suffix(buf, "*");
   if (by_ref)
     _buf_add_suffix(buf, "*");
   _buf_add_suffix(buf, name);
}

static void
_stype_to_str(const Eolian_Typedecl *tp, Eina_Strbuf *buf)
{
   eina_strbuf_append(buf, "struct ");
   eina_strbuf_append(buf, tp->base.c_name);
   if (tp->type == EOLIAN_TYPEDECL_STRUCT_OPAQUE)
     return;
   eina_strbuf_append(buf, " { ");
   Eina_List *l;
   Eolian_Struct_Type_Field *sf;
   EINA_LIST_FOREACH(tp->field_list, l, sf)
     {
        database_type_to_str(sf->type, buf, sf->base.name,
                             EOLIAN_C_TYPE_DEFAULT, sf->by_ref);
        eina_strbuf_append(buf, "; ");
     }
   eina_strbuf_append(buf, "}");
}

static void
_etype_to_str(const Eolian_Typedecl *tp, Eina_Strbuf *buf)
{
   eina_strbuf_append(buf, "enum ");
   eina_strbuf_append(buf, tp->base.c_name);
   eina_strbuf_append(buf, " { ");
   Eina_List *l;
   Eolian_Enum_Type_Field *ef;
   EINA_LIST_FOREACH(tp->field_list, l, ef)
     {
        eina_strbuf_append(buf, ef->base.name);
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
_atype_to_str(const Eolian_Typedecl *tp, Eina_Strbuf *buf)
{
   eina_strbuf_append(buf, "typedef ");
   database_type_to_str(tp->base_type, buf, tp->base.c_name,
                        EOLIAN_C_TYPE_DEFAULT, EINA_FALSE);
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

Eolian_Typedecl *database_type_decl_find(const Eolian_Unit *unit, const Eolian_Type *tp)
{
   if (tp->type != EOLIAN_TYPE_REGULAR)
     return NULL;
   if (tp->tdecl)
     return tp->tdecl;
   /* try looking up if it belongs to a struct, enum or an alias... otherwise
    * return NULL, but first check for builtins
    */
   int  kw = eo_lexer_keyword_str_to_id(tp->base.name);
   if (!kw || kw < KW_byte || kw >= KW_true)
     {
        Eolian_Object *decl = eina_hash_find(unit->objects, tp->base.name);
        if (decl && decl->type == EOLIAN_OBJECT_TYPEDECL)
          return (Eolian_Typedecl *)decl;
     }
   return NULL;
}
