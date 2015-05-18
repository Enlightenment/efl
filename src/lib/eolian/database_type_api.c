#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"
#include "eo_lexer.h"

EAPI const Eolian_Type *
eolian_type_alias_get_by_name(const char *name)
{
   if (!_aliases) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Type *tp = eina_hash_find(_aliases, shr);
   eina_stringshare_del(shr);
   return tp;
}

EAPI const Eolian_Type *
eolian_type_struct_get_by_name(const char *name)
{
   if (!_structs) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Type *tp = eina_hash_find(_structs, shr);
   eina_stringshare_del(shr);
   return tp;
}

EAPI const Eolian_Type *
eolian_type_enum_get_by_name(const char *name)
{
   if (!_enums) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Type *tp = eina_hash_find(_enums, shr);
   eina_stringshare_del(shr);
   return tp;
}

EAPI Eina_Iterator *
eolian_type_aliases_get_by_file(const char *fname)
{
   if (!_aliasesf) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(fname);
   Eina_List *l = eina_hash_find(_aliasesf, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_type_structs_get_by_file(const char *fname)
{
   if (!_structsf) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(fname);
   Eina_List *l = eina_hash_find(_structsf, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_type_enums_get_by_file(const char *fname)
{
   if (!_structsf) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(fname);
   Eina_List *l = eina_hash_find(_enumsf, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eolian_Type_Type
eolian_type_type_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, EOLIAN_TYPE_UNKNOWN_TYPE);
   return tp->type;
}

EAPI Eina_Iterator *
eolian_type_subtypes_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpt = tp->type;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpt == EOLIAN_TYPE_COMPLEX, NULL);
   if (!tp->subtypes) return NULL;
   return eina_list_iterator_new(tp->subtypes);
}

EAPI Eina_Iterator *
eolian_type_struct_fields_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_STRUCT, NULL);
   return eina_list_iterator_new(tp->field_list);
}

EAPI const Eolian_Struct_Type_Field *
eolian_type_struct_field_get(const Eolian_Type *tp, const char *field)
{
   Eolian_Struct_Type_Field *sf = NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(field, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_STRUCT, NULL);
   sf = eina_hash_find(tp->fields, field);
   if (!sf) return NULL;
   return sf;
}

EAPI Eina_Stringshare *
eolian_type_struct_field_name_get(const Eolian_Struct_Type_Field *fl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fl, NULL);
   return fl->name;
}

EAPI Eina_Stringshare *
eolian_type_struct_field_description_get(const Eolian_Struct_Type_Field *fl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fl, NULL);
   return fl->comment;
}

EAPI const Eolian_Type *
eolian_type_struct_field_type_get(const Eolian_Struct_Type_Field *fl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fl, NULL);
   return fl->type;
}

EAPI Eina_Iterator *
eolian_type_enum_fields_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_ENUM, NULL);
   return eina_list_iterator_new(tp->field_list);
}

EAPI const Eolian_Enum_Type_Field *
eolian_type_enum_field_get(const Eolian_Type *tp, const char *field)
{
   Eolian_Enum_Type_Field *ef = NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(field, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_ENUM, NULL);
   ef = eina_hash_find(tp->fields, field);
   if (!ef) return NULL;
   return ef;
}

EAPI Eina_Stringshare *
eolian_type_enum_field_name_get(const Eolian_Enum_Type_Field *fl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fl, NULL);
   return fl->name;
}

EAPI Eina_Stringshare *
eolian_type_enum_field_c_name_get(const Eolian_Enum_Type_Field *fl)
{
   Eina_Stringshare *ret;
   Eina_Strbuf *buf;
   char *bufp, *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fl, NULL);
   buf = eina_strbuf_new();
   if (fl->base_enum->legacy)
     eina_strbuf_append(buf, fl->base_enum->legacy);
   else
     eina_strbuf_append(buf, fl->base_enum->full_name);
   eina_strbuf_append_char(buf, '_');
   eina_strbuf_append(buf, fl->name);
   bufp = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   eina_str_toupper(&bufp);
   while ((p = strchr(bufp, '.'))) *p = '_';
   ret = eina_stringshare_add(bufp);
   free(bufp);
   return ret;
}

EAPI Eina_Stringshare *
eolian_type_enum_field_description_get(const Eolian_Enum_Type_Field *fl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fl, NULL);
   return fl->comment;
}

EAPI const Eolian_Expression *
eolian_type_enum_field_value_get(const Eolian_Enum_Type_Field *fl, Eina_Bool force)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fl, NULL);
   if (!force && !fl->is_public_value) return NULL;
   return fl->value;
}

EAPI Eina_Stringshare *
eolian_type_enum_legacy_prefix_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_ENUM, NULL);
   return tp->legacy;
}

EAPI Eina_Stringshare *
eolian_type_description_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpp = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpp != EOLIAN_TYPE_POINTER
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   return tp->comment;
}

EAPI Eina_Stringshare *
eolian_type_file_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpp = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpp != EOLIAN_TYPE_POINTER
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   return tp->base.file;
}

EAPI const Eolian_Type *
eolian_type_base_type_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpt = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpt == EOLIAN_TYPE_POINTER
                                || tpt == EOLIAN_TYPE_ALIAS
                                || tpt == EOLIAN_TYPE_REGULAR, NULL);
   if (tpt == EOLIAN_TYPE_REGULAR)
     {
        /* for regular types, try looking up if it belongs to a struct,
         * enum or an alias... otherwise return NULL
         * but first check for builtins
         */
        int  kw = eo_lexer_keyword_str_to_id(tp->full_name);
        if (!kw || kw < KW_byte || kw > KW_list)
          {
             Eolian_Declaration *decl = eina_hash_find(_decls, tp->full_name);
             if (decl && decl->type != EOLIAN_DECL_CLASS
                      && decl->type != EOLIAN_DECL_VAR)
               return decl->data;
          }
        return NULL;
     }
   return tp->base_type;
}

EAPI const Eolian_Class *
eolian_type_class_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eolian_type_type_get(tp) == EOLIAN_TYPE_CLASS, NULL);
   return eolian_class_get_by_name(tp->full_name);
}

EAPI Eina_Bool
eolian_type_is_own(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, EINA_FALSE);
   return tp->is_own;
}

EAPI Eina_Bool
eolian_type_is_const(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, EINA_FALSE);
   return tp->is_const;
}

EAPI Eina_Bool
eolian_type_is_extern(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, EINA_FALSE);
   return tp->is_extern;
}

EAPI Eina_Stringshare *
eolian_type_c_type_named_get(const Eolian_Type *tp, const char *name)
{
   Eina_Stringshare *ret;
   Eina_Strbuf *buf;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   buf = eina_strbuf_new();
   database_type_to_str(tp, buf, name);
   ret = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
}

EAPI Eina_Stringshare *
eolian_type_c_type_get(const Eolian_Type *tp)
{
   return eolian_type_c_type_named_get(tp, NULL);
}

EAPI Eina_Stringshare *
eolian_type_name_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpp = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpp != EOLIAN_TYPE_POINTER
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   return tp->name;
}

EAPI Eina_Stringshare *
eolian_type_full_name_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpp = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpp != EOLIAN_TYPE_POINTER
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   return tp->full_name;
}

EAPI Eina_Iterator *
eolian_type_namespaces_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpp = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpp != EOLIAN_TYPE_POINTER
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   if (!tp->namespaces) return NULL;
   return eina_list_iterator_new(tp->namespaces);
}

EAPI Eina_Stringshare *
eolian_type_free_func_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   return tp->freefunc;
}
