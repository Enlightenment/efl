#include <Eina.h>
#include "eolian_database.h"
#include "eo_definitions.h"

void
database_type_del(Eolian_Type *type)
{
   if (!type) return;
   eo_definitions_type_free((Eo_Type_Def*)type);
}

Eina_Bool
database_type_add(const char *alias, Eolian_Type *type)
{
   if (_types)
     {
        Type_Desc *desc = calloc(1, sizeof(*desc));
        desc->alias = eina_stringshare_add(alias);
        desc->type = type;
        eina_hash_set(_types, desc->alias, desc);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

Eina_Bool database_struct_add(Eolian_Type *tp)
{
   if (_structs)
     {
        eina_hash_set(_structs, tp->name, tp);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EAPI const Eolian_Type *
eolian_type_find_by_alias(const char *alias)
{
   if (!_types) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(alias);
   Type_Desc *cl = eina_hash_find(_types, shr);
   eina_stringshare_del(shr);
   return cl?cl->type:NULL;
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
   _Struct_Field_Type *sf = NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(field, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_STRUCT, NULL);
   sf = eina_hash_find(tp->fields, field);
   if (!sf) return NULL;
   return sf->type;
}

EAPI const char *
eolian_type_struct_field_description_get(const Eolian_Type *tp, const char *field)
{
   _Struct_Field_Type *sf = NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(field, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_STRUCT, NULL);
   sf = eina_hash_find(tp->fields, field);
   if (!sf) return NULL;
   return sf->comment;
}

EAPI const char *
eolian_type_struct_description_get(const Eolian_Type *tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tp->type == EOLIAN_TYPE_STRUCT, NULL);
   return tp->comment;
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
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpt == EOLIAN_TYPE_POINTER, NULL);
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

static void _type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name);

static void
_ftype_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name)
{
   Eina_List *l;
   Eolian_Type *stp;
   Eina_Bool first = EINA_TRUE;
   if (tp->ret_type)
     _type_to_str(tp->ret_type, buf, NULL);
   else
     eina_strbuf_append(buf, "void");
   eina_strbuf_append(buf, " (*");
   if (name) eina_strbuf_append(buf, name);
   eina_strbuf_append(buf, ")(");
   EINA_LIST_FOREACH(tp->arguments, l, stp)
     {
        if (!first) eina_strbuf_append(buf, ", ");
        first = EINA_FALSE;
        _type_to_str(stp, buf, NULL);
     }
}

static Eina_Bool
_stype_field_cb(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data,
                void *fdata)
{
   _type_to_str((Eolian_Type*)((_Struct_Field_Type*)data)->type,
                (Eina_Strbuf*)fdata, (const char*)key);
   eina_strbuf_append((Eina_Strbuf*)fdata, "; ");
   return EINA_TRUE;
}

static void
_stype_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name)
{
   eina_strbuf_append(buf, "struct ");
   if (tp->name)
     {
        eina_strbuf_append(buf, tp->name);
        eina_strbuf_append_char(buf, ' ');
     }
   eina_strbuf_append(buf, "{ ");
   eina_hash_foreach(tp->fields, _stype_field_cb, buf);
   eina_strbuf_append(buf, "}");
   if (name)
     {
        eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, name);
     }
}

static void
_type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name)
{
   if (tp->type == EOLIAN_TYPE_FUNCTION)
     {
        _ftype_to_str(tp, buf, name);
        return;
     }
   else if (tp->type == EOLIAN_TYPE_STRUCT)
     {
        _stype_to_str(tp, buf, name);
        return;
     }
   if ((tp->type == EOLIAN_TYPE_REGULAR
     || tp->type == EOLIAN_TYPE_REGULAR_STRUCT
     || tp->type == EOLIAN_TYPE_VOID)
     && tp->is_const)
     {
        eina_strbuf_append(buf, "const ");
     }
   if (tp->type == EOLIAN_TYPE_REGULAR)
     eina_strbuf_append(buf, tp->name);
   else if (tp->type == EOLIAN_TYPE_REGULAR_STRUCT)
     {
        eina_strbuf_append(buf, "struct ");
        eina_strbuf_append(buf, tp->name);
     }
   else if (tp->type == EOLIAN_TYPE_VOID)
     eina_strbuf_append(buf, "void");
   else
     {
        Eolian_Type *btp = tp->base_type;
        _type_to_str(tp->base_type, buf, NULL);
        if (btp->type != EOLIAN_TYPE_POINTER || btp->is_const)
           eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append_char(buf, '*');
        if (tp->is_const) eina_strbuf_append(buf, " const");
     }
   if (name)
     {
        eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, name);
     }
}

EAPI Eina_Stringshare *
eolian_type_c_type_named_get(const Eolian_Type *tp, const char *name)
{
   Eina_Stringshare *ret;
   Eina_Strbuf *buf;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   buf = eina_strbuf_new();
   _type_to_str(tp, buf, name);
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
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   eina_stringshare_ref(tp->name);
   return tp->name;
}

static Eina_Bool
_print_field(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data,
             void *fdata EINA_UNUSED)
{
   printf("%s: ", (const char*)key);
   database_type_print((Eolian_Type*)data);
   puts("; ");
   return EINA_TRUE;
}

void
database_type_print(Eolian_Type *tp)
{
   Eina_List *l;
   Eolian_Type *stp;
   if (tp->is_own)
     puts("own(");
   if (tp->is_const)
     puts("const(");
   if (tp->type == EOLIAN_TYPE_REGULAR)
     puts(tp->name);
   else if (tp->type == EOLIAN_TYPE_REGULAR_STRUCT)
     printf("struct %s", tp->name);
   else if (tp->type == EOLIAN_TYPE_POINTER)
     {
        database_type_print(tp->base_type);
        putchar('*');
     }
   else if (tp->type == EOLIAN_TYPE_FUNCTION)
     {
        Eina_Bool first = EINA_TRUE;
        puts("func");
        if (tp->ret_type)
          {
             putchar(' ');
             database_type_print(tp->ret_type);
          }
        else
          puts(" void");
        puts(" (");
        EINA_LIST_FOREACH(tp->arguments, l, stp)
          {
             if (!first) puts(", ");
             first = EINA_FALSE;
             database_type_print(stp);
          }
        putchar(')');
     }
   else if (tp->type == EOLIAN_TYPE_STRUCT)
     {
        puts("struct ");
        if (tp->name) printf("%s ", tp->name);
        puts("{ ");
        eina_hash_foreach(tp->fields, _print_field, NULL);
        puts("}");
     }
   if (tp->is_own)
     putchar(')');
   if (tp->is_const)
     putchar(')');
}
