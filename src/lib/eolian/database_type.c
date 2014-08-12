#include <Eina.h>
#include "eo_lexer.h"

void
database_type_del(Eolian_Type *tp)
{
   if (!tp) return;
   const char *sp;
   Eolian_Type *stp;
   if (tp->base.file) eina_stringshare_del(tp->base.file);
   if (tp->subtypes) EINA_LIST_FREE(tp->subtypes, stp)
     database_type_del(stp);
   if (tp->base_type)
     database_type_del(tp->base_type);
   if (tp->name) eina_stringshare_del(tp->name);
   if (tp->full_name) eina_stringshare_del(tp->full_name);
   if (tp->fields) eina_hash_free(tp->fields);
   if (tp->namespaces) EINA_LIST_FREE(tp->namespaces, sp)
      eina_stringshare_del(sp);
   if (tp->comment) eina_stringshare_del(tp->comment);
   if (tp->legacy) eina_stringshare_del(tp->legacy);
   free(tp);
}

void
database_typedef_del(Eolian_Type *tp)
{
   if (!tp) return;
   Eolian_Type *btp = tp->base_type;
   /* prevent deletion of named structs/enums as they're deleted later on */
   if (btp)
     {
        if (btp->type == EOLIAN_TYPE_ENUM)
          tp->base_type = NULL;
        else if (btp->type == EOLIAN_TYPE_STRUCT && btp->name)
          tp->base_type = NULL;
     }
   database_type_del(tp);
}

Eina_Bool
database_type_add(Eolian_Type *def)
{
   if (!_aliases) return EINA_FALSE;
   eina_hash_set(_aliases, def->full_name, def);
   eina_hash_set(_aliasesf, def->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(_aliasesf, def->base.file), def));
   return EINA_TRUE;
}

Eina_Bool database_struct_add(Eolian_Type *tp)
{
   if (!_structs) return EINA_FALSE;
   eina_hash_set(_structs, tp->full_name, tp);
   eina_hash_set(_structsf, tp->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(_structsf, tp->base.file), tp));
   return EINA_TRUE;
}

Eina_Bool database_enum_add(Eolian_Type *tp)
{
   if (!_enums) return EINA_FALSE;
   eina_hash_set(_enums, tp->full_name, tp);
   eina_hash_set(_enumsf, tp->base.file, eina_list_append
                ((Eina_List*)eina_hash_find(_enumsf, tp->base.file), tp));
   return EINA_TRUE;
}

static void
_ftype_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name)
{
   Eina_List *l;
   Eolian_Type *stp;
   Eina_Bool first = EINA_TRUE;
   if (tp->ret_type)
     database_type_to_str(tp->ret_type, buf, NULL);
   else
     eina_strbuf_append(buf, "void");
   eina_strbuf_append(buf, " (*");
   if (name) eina_strbuf_append(buf, name);
   eina_strbuf_append(buf, ")(");
   EINA_LIST_FOREACH(tp->arguments, l, stp)
     {
        if (!first) eina_strbuf_append(buf, ", ");
        first = EINA_FALSE;
        database_type_to_str(stp, buf, NULL);
     }
}

static Eina_Bool
_stype_field_cb(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data,
                void *fdata)
{
   database_type_to_str((Eolian_Type*)((Eolian_Struct_Field*)data)->type,
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
        Eina_List *l;
        const char *sp;
        EINA_LIST_FOREACH(tp->namespaces, l, sp)
          {
             eina_strbuf_append(buf, sp);
             eina_strbuf_append_char(buf, '_');
          }
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

static Eina_Bool
_etype_field_cb(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data,
                void *fdata)
{
   Eina_Strbuf *buf = (Eina_Strbuf*)fdata;
   const char *fname = (const char*)key;
   Eolian_Enum_Field *ef = (Eolian_Enum_Field*)data;
   eina_strbuf_append(buf, fname);
   if (ef->value)
     {
        Eina_Value *val = NULL;
        Eolian_Expression_Type et = eolian_expression_eval(ef->value,
            EOLIAN_MASK_INT, &val);
        const char *ret;
        eina_strbuf_append(buf, " = ");
        ret = eolian_expression_value_to_literal(val, et);
        eina_strbuf_append(buf, ret);
        eina_stringshare_del(ret);
     }
   eina_strbuf_append(buf, ", ");
   return EINA_TRUE;
}

static void
_etype_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name)
{
   eina_strbuf_append(buf, "enum ");
   if (tp->name)
     {
        Eina_List *l;
        const char *sp;
        EINA_LIST_FOREACH(tp->namespaces, l, sp)
          {
             eina_strbuf_append(buf, sp);
             eina_strbuf_append_char(buf, '_');
          }
        eina_strbuf_append(buf, tp->name);
        eina_strbuf_append_char(buf, ' ');
     }
   eina_strbuf_append(buf, "{ ");
   eina_hash_foreach(tp->fields, _etype_field_cb, buf);
   eina_strbuf_append(buf, "}");
   if (name)
     {
        eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, name);
     }
}

static void
_atype_to_str(const Eolian_Type *tp, Eina_Strbuf *buf)
{
   Eina_Strbuf *fulln = eina_strbuf_new();
   Eina_List *l;
   const char *sp;

   eina_strbuf_append(buf, "typedef ");

   EINA_LIST_FOREACH(tp->namespaces, l, sp)
     {
        eina_strbuf_append(fulln, sp);
        eina_strbuf_append_char(fulln, '_');
     }
   eina_strbuf_append(fulln, tp->name);

   database_type_to_str(tp->base_type, buf, eina_strbuf_string_get(fulln));
   eina_strbuf_free(fulln);
}

void
database_type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name)
{
   if (tp->type == EOLIAN_TYPE_ALIAS)
     {
        _atype_to_str(tp, buf);
        return;
     }
   else if (tp->type == EOLIAN_TYPE_FUNCTION)
     {
        _ftype_to_str(tp, buf, name);
        return;
     }
   else if (tp->type == EOLIAN_TYPE_STRUCT)
     {
        _stype_to_str(tp, buf, name);
        return;
     }
   else if (tp->type == EOLIAN_TYPE_ENUM)
     {
        _etype_to_str(tp, buf, name);
        return;
     }
   if ((tp->type == EOLIAN_TYPE_REGULAR
     || tp->type == EOLIAN_TYPE_REGULAR_STRUCT
     || tp->type == EOLIAN_TYPE_REGULAR_ENUM
     || tp->type == EOLIAN_TYPE_VOID
     || tp->type == EOLIAN_TYPE_CLASS)
     && tp->is_const)
     {
        eina_strbuf_append(buf, "const ");
     }
   if (tp->type == EOLIAN_TYPE_REGULAR
    || tp->type == EOLIAN_TYPE_CLASS
    || tp->type == EOLIAN_TYPE_REGULAR_STRUCT
    || tp->type == EOLIAN_TYPE_REGULAR_ENUM)
     {
        Eina_List *l;
        const char *sp;
        if (tp->type == EOLIAN_TYPE_REGULAR_STRUCT)
          {
             eina_strbuf_append(buf, "struct ");
          }
        else if (tp->type == EOLIAN_TYPE_REGULAR_ENUM)
          {
             eina_strbuf_append(buf, "enum ");
          }
        EINA_LIST_FOREACH(tp->namespaces, l, sp)
          {
             eina_strbuf_append(buf, sp);
             eina_strbuf_append_char(buf, '_');
          }
        int kw = eo_lexer_keyword_str_to_id(tp->name);
        if (kw) eina_strbuf_append(buf, eo_lexer_get_c_type(kw));
        else eina_strbuf_append(buf, tp->name);
     }
   else if (tp->type == EOLIAN_TYPE_VOID)
     eina_strbuf_append(buf, "void");
   else
     {
        Eolian_Type *btp = tp->base_type;
        database_type_to_str(tp->base_type, buf, NULL);
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

static Eina_Bool
_print_field(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data,
             void *fdata EINA_UNUSED)
{
   Eolian_Struct_Field *sf = (Eolian_Struct_Field*)data;
   printf("%s: ", (const char*)key);
   database_type_print(sf->type);
   printf("; ");
   return EINA_TRUE;
}

static Eina_Bool
_print_enum_field(const Eina_Hash *hash EINA_UNUSED, const void *key,
                  void *data, void *fdata EINA_UNUSED)
{
   Eolian_Enum_Field *ef = (Eolian_Enum_Field*)data;
   printf("%s", (const char*)key);
   if (ef->value)
     {
        Eina_Value *val = NULL;
        Eolian_Expression_Type et = eolian_expression_eval(ef->value,
            EOLIAN_MASK_INT, &val);
        const char *ret;
        printf(" = ");
        ret = eolian_expression_value_to_literal(val, et);
        printf("%s", ret);
        eina_stringshare_del(ret);
     }
   printf(", ");
   return EINA_TRUE;
}

static void
_typedef_print(Eolian_Type *tp)
{
   printf("type %s: ", tp->full_name);
   database_type_print(tp->base_type);
}

void
database_type_print(Eolian_Type *tp)
{
   Eina_List *l;
   Eolian_Type *stp;
   if (tp->type == EOLIAN_TYPE_ALIAS)
     {
        _typedef_print(tp);
        return;
     }
   if (tp->is_own)
     printf("own(");
   if (tp->is_const)
     printf("const(");
   if (tp->type == EOLIAN_TYPE_REGULAR || tp->type == EOLIAN_TYPE_CLASS)
     printf("%s", tp->full_name);
   else if (tp->type == EOLIAN_TYPE_VOID)
     printf("void");
   else if (tp->type == EOLIAN_TYPE_REGULAR_STRUCT)
     printf("struct %s", tp->full_name);
   else if (tp->type == EOLIAN_TYPE_REGULAR_ENUM)
     printf("enum %s", tp->full_name);
   else if (tp->type == EOLIAN_TYPE_POINTER)
     {
        database_type_print(tp->base_type);
        putchar('*');
     }
   else if (tp->type == EOLIAN_TYPE_FUNCTION)
     {
        Eina_Bool first = EINA_TRUE;
        printf("func");
        if (tp->ret_type)
          {
             putchar(' ');
             database_type_print(tp->ret_type);
          }
        else
          printf(" void");
        printf(" (");
        EINA_LIST_FOREACH(tp->arguments, l, stp)
          {
             if (!first) printf(", ");
             first = EINA_FALSE;
             database_type_print(stp);
          }
        putchar(')');
     }
   else if (tp->type == EOLIAN_TYPE_STRUCT)
     {
        printf("struct ");
        if (tp->full_name) printf("%s ", tp->full_name);
        printf("{ ");
        eina_hash_foreach(tp->fields, _print_field, NULL);
        printf("}");
     }
   else if (tp->type == EOLIAN_TYPE_ENUM)
     {
        printf("enum %s ", tp->full_name);
        printf("{ ");
        eina_hash_foreach(tp->fields, _print_enum_field, NULL);
        printf("}");
     }
   if (tp->is_own)
     putchar(')');
   if (tp->is_const)
     putchar(')');
}
