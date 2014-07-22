#include <Eina.h>
#include "eolian_database.h"
#include "eo_definitions.h"

EAPI const Eolian_Type *
eolian_type_alias_find_by_name(const char *name)
{
   if (!_aliases) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Type *tp = eina_hash_find(_aliases, shr);
   eina_stringshare_del(shr);
   return tp;
}

EAPI const Eolian_Type *
eolian_type_struct_find_by_name(const char *name)
{
   if (!_structs) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Type *tp = eina_hash_find(_structs, shr);
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

EAPI Eolian_Type_Type
eolian_type_type_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, EOLIAN_TYPE_UNKNOWN_TYPE);
   return tp->type;
}

EAPI Eina_Iterator *
eolian_type_arguments_list_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eolian_type_type_get(tp) == EOLIAN_TYPE_FUNCTION, NULL);
   if (!tp->arguments) return NULL;
   return eina_list_iterator_new(tp->arguments);
}

EAPI Eina_Iterator *
eolian_type_subtypes_list_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpt = tp->type;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpt == EOLIAN_TYPE_REGULAR
                                || tpt == EOLIAN_TYPE_POINTER
                                || tpt == EOLIAN_TYPE_REGULAR_STRUCT, NULL);
   if (!tp->subtypes) return NULL;
   return eina_list_iterator_new(tp->subtypes);
}

EAPI Eina_Iterator *
eolian_type_struct_field_names_list_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_STRUCT, NULL);
   return eina_hash_iterator_key_new(tp->fields);
}

EAPI const Eolian_Type *
eolian_type_struct_field_get(const Eolian_Type *tp, const char *field)
{
   Eolian_Struct_Field *sf = NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(field, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_STRUCT, NULL);
   sf = eina_hash_find(tp->fields, field);
   if (!sf) return NULL;
   return sf->type;
}

EAPI Eina_Stringshare *
eolian_type_struct_field_description_get(const Eolian_Type *tp, const char *field)
{
   Eolian_Struct_Field *sf = NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(field, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_STRUCT, NULL);
   sf = eina_hash_find(tp->fields, field);
   if (!sf) return NULL;
   return sf->comment;
}

EAPI Eina_Stringshare *
eolian_type_description_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpp = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpp != EOLIAN_TYPE_POINTER
                                && tpp != EOLIAN_TYPE_FUNCTION
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   return eina_stringshare_ref(tp->comment);
}

EAPI Eina_Stringshare *
eolian_type_file_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpp = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpp != EOLIAN_TYPE_POINTER
                                && tpp != EOLIAN_TYPE_FUNCTION
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   return eina_stringshare_ref(tp->file);
}

EAPI const Eolian_Type *
eolian_type_return_type_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eolian_type_type_get(tp) == EOLIAN_TYPE_FUNCTION, NULL);
   return tp->ret_type;
}

EAPI const Eolian_Type *
eolian_type_base_type_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpt = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpt == EOLIAN_TYPE_POINTER || tpt == EOLIAN_TYPE_ALIAS, NULL);
   return tp->base_type;
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
                                && tpp != EOLIAN_TYPE_FUNCTION
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   return eina_stringshare_ref(tp->name);
}

EAPI Eina_Stringshare *
eolian_type_full_name_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpp = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpp != EOLIAN_TYPE_POINTER
                                && tpp != EOLIAN_TYPE_FUNCTION
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   return eina_stringshare_ref(tp->full_name);
}

EAPI Eina_Iterator *
eolian_type_namespaces_list_get(const Eolian_Type *tp)
{
   Eolian_Type_Type tpp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpp = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpp != EOLIAN_TYPE_POINTER
                                && tpp != EOLIAN_TYPE_FUNCTION
                                && tpp != EOLIAN_TYPE_VOID, NULL);
   if (!tp->namespaces) return NULL;
   return eina_list_iterator_new(tp->namespaces);
}
