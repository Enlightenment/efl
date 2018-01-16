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
database_type_add(Eolian *state, Eolian_Typedecl *def)
{
   eina_hash_set(state->unit.aliases, def->full_name, def);
   eina_hash_set(state->aliases_f, def->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(state->aliases_f, def->base.file),
                def));
   database_decl_add(state, def->full_name, EOLIAN_DECL_ALIAS, def->base.file, def);
}

void
database_struct_add(Eolian *state, Eolian_Typedecl *tp)
{
   eina_hash_set(state->unit.structs, tp->full_name, tp);
   eina_hash_set(state->structs_f, tp->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(state->structs_f, tp->base.file), tp));
   database_decl_add(state, tp->full_name, EOLIAN_DECL_STRUCT, tp->base.file, tp);
}

void
database_enum_add(Eolian *state, Eolian_Typedecl *tp)
{
   eina_hash_set(state->unit.enums, tp->full_name, tp);
   eina_hash_set(state->enums_f, tp->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(state->enums_f, tp->base.file), tp));
   database_decl_add(state, tp->full_name, EOLIAN_DECL_ENUM, tp->base.file, tp);
}

Eina_Bool
database_type_is_ownable(const Eolian_Unit *unit, const Eolian_Type *tp)
{
   if (tp->is_ptr)
     return EINA_TRUE;
   if (tp->type == EOLIAN_TYPE_REGULAR)
     {
        int kw = eo_lexer_keyword_str_to_id(tp->name);
        const char *ct = eo_lexer_get_c_type(kw);
        if (!ct)
          {
             const Eolian_Typedecl *tpp = database_type_decl_find(unit, tp);
             if (!tpp)
               return EINA_FALSE;
             if (tpp->type == EOLIAN_TYPEDECL_FUNCTION_POINTER)
               return EINA_TRUE;
             if (tpp->type == EOLIAN_TYPEDECL_ALIAS)
               return database_type_is_ownable(unit, tpp->base_type);
             return EINA_FALSE;
          }
        return (ct[strlen(ct) - 1] == '*');
     }
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
                     Eolian_C_Type_Type ctype)
{
   if ((tp->type == EOLIAN_TYPE_REGULAR
     || tp->type == EOLIAN_TYPE_CLASS
     || tp->type == EOLIAN_TYPE_VOID)
     && tp->is_const
     && ((ctype != EOLIAN_C_TYPE_RETURN) || database_type_is_ownable(NULL, tp)))
     {
        eina_strbuf_append(buf, "const ");
     }
   if (tp->type == EOLIAN_TYPE_REGULAR
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
        /* handles arrays and pointers as they all serialize to pointers */
        database_type_to_str(tp->base_type, buf, NULL,
                             EOLIAN_C_TYPE_DEFAULT);
        _buf_add_suffix(buf, "*");
        if (tp->is_const && (ctype != EOLIAN_C_TYPE_RETURN))
          eina_strbuf_append(buf, " const");
     }
   if (tp->type == EOLIAN_TYPE_CLASS)
     _buf_add_suffix(buf, "*");
   if (tp->is_ptr)
     _buf_add_suffix(buf, "*");
   _buf_add_suffix(buf, name);
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
        database_type_to_str(sf->type, buf, sf->name,
                             EOLIAN_C_TYPE_DEFAULT);
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

   if (tp->base_type->type == EOLIAN_TYPE_REGULAR)
     {
        if (!strcmp(tp->base_type->name, "__builtin_free_cb"))
          {
             eina_strbuf_append(buf, "void (*");
             _append_name(tp, buf);
             eina_strbuf_append(buf, ")(void *data)");
             return;
          }
     }

   Eina_Strbuf *fulln = eina_strbuf_new();
   _append_name(tp, fulln);
   database_type_to_str(tp->base_type, buf, eina_strbuf_string_get(fulln),
                        EOLIAN_C_TYPE_DEFAULT);
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

Eolian_Typedecl *database_type_decl_find(const Eolian_Unit *unit, const Eolian_Type *tp)
{
   if (tp->type != EOLIAN_TYPE_REGULAR)
     return NULL;
   if (tp->tdecl)
     return tp->tdecl;
   /* try looking up if it belongs to a struct, enum or an alias... otherwise
    * return NULL, but first check for builtins
    */
   int  kw = eo_lexer_keyword_str_to_id(tp->full_name);
   if (!kw || kw < KW_byte || kw >= KW_true)
     {
        Eolian_Declaration *decl = eina_hash_find(unit->state->unit.decls, tp->full_name);
        if (decl && decl->type != EOLIAN_DECL_CLASS
                 && decl->type != EOLIAN_DECL_VAR)
          return decl->data;
     }
   return NULL;
}
