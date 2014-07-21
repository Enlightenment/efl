#include <Eina.h>
#include "eolian_database.h"
#include "eo_definitions.h"

void
database_type_del(Eolian_Type *tp)
{
   if (!tp) return;
   if (tp->type == EOLIAN_TYPE_POINTER || tp->type == EOLIAN_TYPE_FUNCTION)
     {
        Eolian_Type *stp;
        if (tp->subtypes) EINA_LIST_FREE(tp->subtypes, stp)
          database_type_del(stp);
        if (tp->base_type)
          database_type_del(tp->base_type);
     }
   else
     {
        const char *sp;
        if (tp->name) eina_stringshare_del(tp->name);
        if (tp->full_name) eina_stringshare_del(tp->full_name);
        if (tp->fields) eina_hash_free(tp->fields);
        if (tp->namespaces) EINA_LIST_FREE(tp->namespaces, sp)
           eina_stringshare_del(sp);
        if (tp->comment) eina_stringshare_del(tp->comment);
        if (tp->file) eina_stringshare_del(tp->file);
     }
   free(tp);
}

Eina_Bool
database_type_add(Eolian_Type *def)
{
   if (!_aliases) return EINA_FALSE;
   eina_hash_set(_aliases, def->full_name, def);
   return EINA_TRUE;
}

Eina_Bool database_struct_add(Eolian_Type *tp)
{
   if (!_structs) return EINA_FALSE;
   eina_hash_set(_structs, tp->full_name, tp);
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

void
database_type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name)
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
   printf("%s: ", (const char*)key);
   database_type_print((Eolian_Type*)data);
   printf("; ");
   return EINA_TRUE;
}

void
database_type_print(Eolian_Type *tp)
{
   Eina_List *l;
   Eolian_Type *stp;
   if (tp->is_own)
     printf("own(");
   if (tp->is_const)
     printf("const(");
   if (tp->type == EOLIAN_TYPE_REGULAR)
     printf("%s", tp->name);
   else if (tp->type == EOLIAN_TYPE_VOID)
     printf("void");
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
        if (tp->name) printf("%s ", tp->name);
        printf("{ ");
        eina_hash_foreach(tp->fields, _print_field, NULL);
        printf("}");
     }
   if (tp->is_own)
     putchar(')');
   if (tp->is_const)
     putchar(')');
}
