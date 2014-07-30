#include <Eina.h>
#include "eolian_database.h"

static void
_implements_print(Eolian_Implement *impl, int nb_spaces)
{
   const Eolian_Class *class;
   const Eolian_Function *func;
   const char *t;
   Eolian_Function_Type ft;

   eolian_implement_information_get(impl, &class, &func, &ft);
   switch (ft)
     {
      case EOLIAN_PROP_SET: t = "SET"; break;
      case EOLIAN_PROP_GET: t = "GET"; break;
      case EOLIAN_METHOD: t = "METHOD"; break;
      case EOLIAN_UNRESOLVED:
        {
           t = "Type is the same as function being overriden";
           break;
        }
      default:
         return;
     }
   printf("%*s <%s> <%s>\n", nb_spaces + 5, "", eolian_implement_full_name_get(impl), t);
}

static void
_event_print(Eolian_Event *ev, int nb_spaces)
{
   const char *name, *comment;
   const Eolian_Type *type;

   eolian_class_event_information_get(ev, &name, &type, &comment);
   printf("%*s <%s> <",  nb_spaces + 5, "", name);
   if (type) database_type_print((Eolian_Type*)type);
   printf("> <%s>\n", comment);
}

static Eina_Bool _function_print(const Eolian_Function *fid, int nb_spaces)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   const char *ret_desc = eolian_function_return_comment_get(fid, fid->type);
   const Eolian_Type *tp;
   switch (fid->type)
     {
      case EOLIAN_PROPERTY:
           {
              printf("%*s<%s> %s\n", nb_spaces, "", ret_desc ? ret_desc : "", fid->name);
              const char *str = eolian_function_description_get(fid, EOLIAN_COMMENT_GET);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_description_get(fid, EOLIAN_COMMENT_SET);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(fid, EOLIAN_LEGACY_GET);
              if (str) printf("%*slegacy_get: <%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(fid, EOLIAN_LEGACY_SET);
              if (str) printf("%*slegacy_set: <%s>\n", nb_spaces + 5, "", str);
              tp = eolian_function_return_type_get(fid, EOLIAN_PROP_GET);
              if (tp)
                {
                   printf("%*sreturn type for get: <", nb_spaces + 5, "");
                   database_type_print((Eolian_Type*)tp);
                   printf(">\n");
                }
              tp = eolian_function_return_type_get(fid, EOLIAN_PROP_SET);
              if (tp)
                {
                   printf("%*sreturn type for set: <", nb_spaces + 5, "");
                   database_type_print((Eolian_Type*)tp);
                   printf(">\n");
                }
              break;
           }
      case EOLIAN_PROP_GET:
           {
              printf("%*sGET:<%s> %s\n", nb_spaces, "", ret_desc ? ret_desc : "", fid->name);
              const char *str = eolian_function_description_get(fid, EOLIAN_COMMENT_GET);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(fid, EOLIAN_LEGACY_GET);
              if (str) printf("%*slegacy: <%s>\n", nb_spaces + 5, "", str);
              tp = eolian_function_return_type_get(fid, EOLIAN_PROP_GET);
              if (tp)
                {
                   printf("%*sreturn type: <", nb_spaces + 5, "");
                   database_type_print((Eolian_Type*)tp);
                   printf(">\n");
                }
              break;
           }
      case EOLIAN_PROP_SET:
           {
              printf("%*sSET:<%s> %s\n", nb_spaces, "", ret_desc ? ret_desc : "", fid->name);
              const char *str = eolian_function_description_get(fid, EOLIAN_COMMENT_SET);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(fid, EOLIAN_LEGACY_SET);
              if (str) printf("%*slegacy: <%s>\n", nb_spaces + 5, "", str);
              tp = eolian_function_return_type_get(fid, EOLIAN_PROP_SET);
              if (tp)
                {
                   printf("%*sreturn type: <", nb_spaces + 5, "");
                   database_type_print((Eolian_Type*)tp);
                   printf(">\n");
                }
              break;
           }
      case EOLIAN_METHOD:
           {
              printf("%*s<%s> %s\n", nb_spaces, "", ret_desc ? ret_desc : "", fid->name);
              const char *str = eolian_function_description_get(fid, EOLIAN_COMMENT);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(fid, EOLIAN_LEGACY);
              if (str) printf("%*slegacy: <%s>\n", nb_spaces + 5, "", str);
              tp = eolian_function_return_type_get(fid, EOLIAN_METHOD);
              if (tp)
                {
                   printf("%*sreturn type: <", nb_spaces + 5, "");
                   database_type_print((Eolian_Type*)tp);
                   printf(">\n");
                }
              if (fid->obj_is_const) printf("%*sobj const: <true>\n", nb_spaces + 5, "");
              break;
           }
      case EOLIAN_CTOR:
           {
              //char *str = eina_hash_find(fid->data, "comment");
              const char *str = eolian_function_description_get(fid, EOLIAN_COMMENT);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(fid, EOLIAN_LEGACY);
              if (str) printf("%*slegacy: <%s>\n", nb_spaces + 5, "", str);
              tp = eolian_function_return_type_get(fid, EOLIAN_CTOR);
              if (tp)
                {
                   printf("%*sreturn type: <", nb_spaces + 5, "");
                   database_type_print((Eolian_Type*)tp);
                   printf(">\n");
                }
              break;
           }
      default:
         return EINA_FALSE;
     }
   Eina_List *itr;
   Eolian_Function_Parameter *param;
   EINA_LIST_FOREACH(fid->params, itr, param)
     {
        char *param_dir = NULL;
        switch (param->param_dir)
          {
           case EOLIAN_IN_PARAM:
             param_dir = "IN";
             break;
           case EOLIAN_OUT_PARAM:
             param_dir = "OUT";
             break;
           case EOLIAN_INOUT_PARAM:
             param_dir = "INOUT";
             break;
          }
         printf("%*s%s <%s> <", nb_spaces + 5, "", param_dir, param->name);
         database_type_print((Eolian_Type*)param->type);
         printf("> <%s>\n", param->description?param->description:"");
     }
   return EINA_TRUE;
}

static Eina_Bool
_class_print(const Eolian_Class *cl)
{
   Eina_List *itr;
   Eolian_Function *function;
   const char *types[5] = {"", "Regular", "Regular Non Instantiable", "Mixin", "Interface"};

   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   printf("Class %s:\n", cl->name);
   if (cl->description)
     printf("  description: <%s>\n", cl->description);

   printf("  type: %s\n", types[cl->type]);

   // Inherits
   if (cl->inherits)
     {
        printf("  inherits: ");
        char *word;
        EINA_LIST_FOREACH(cl->inherits, itr, word)
          printf("%s ", word);
        printf("\n");
     }

   // Legacy prefix
   if (cl->legacy_prefix)
     {
        printf("  legacy prefix: <%s>\n", cl->legacy_prefix);
     }

   // Eo prefix
   if (cl->eo_prefix)
     {
        printf("  Eo prefix: <%s>\n", cl->eo_prefix);
     }

   // Data type
   if (cl->data_type)
     {
        printf("  Data type: <%s>\n", cl->data_type);
     }

   // Constructors
   printf("  constructors:\n");
   EINA_LIST_FOREACH(cl->constructors, itr, function)
     {
        _function_print(function, 4);
     }
   printf("\n");

   // Properties
   printf("  properties:\n");
   EINA_LIST_FOREACH(cl->properties, itr, function)
     {
        _function_print(function, 4);
     }
   printf("\n");

   // Methods
   printf("  methods:\n");
   EINA_LIST_FOREACH(cl->methods, itr, function)
     {
        _function_print(function, 4);
     }
   // Implement
   printf("  implements:\n");
   Eolian_Implement *impl;
   EINA_LIST_FOREACH(cl->implements, itr, impl)
     {
        _implements_print(impl, 4);
     }
   printf("\n");
   // Implement
   printf("  events:\n");
   Eolian_Event *ev;
   EINA_LIST_FOREACH(cl->events, itr, ev)
     {
        _event_print(ev, 4);
     }
   printf("\n");
   return EINA_TRUE;
}

static Eina_Bool
_cl_print_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
             void *data, void *fdata EINA_UNUSED)
{
   _class_print((Eolian_Class*)data);
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_show_class(const Eolian_Class *class)
{
   if (!class)
     eina_hash_foreach(_classes, _cl_print_cb, NULL);
   else
     _class_print(class);
   return EINA_TRUE;
}

static Eina_Bool
_typedef_cb(Eina_Hash *hash EINA_UNUSED, const char *alias,
            const Eolian_Type *tp, const void *fdata EINA_UNUSED)
{
   printf("Typedef: %s\n", alias);
   printf("  type: <");
   database_type_print(tp->base_type);
   printf(">\n");
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_show_typedef(const char *alias)
{
   if (!alias)
     eina_hash_foreach(_aliases, (Eina_Hash_Foreach)_typedef_cb, NULL);
   else
     {
        Eina_Stringshare *shr = eina_stringshare_add(alias);
        Eolian_Type *tp = eina_hash_find(_aliases, shr);
        eina_stringshare_del(shr);
        if (!tp) return EINA_FALSE;
        _typedef_cb(NULL, alias, tp, NULL);
     }
   return EINA_TRUE;
}

static Eina_Bool
_struct_cb(Eina_Hash *hash EINA_UNUSED, const char *name,
            const Eolian_Type *tp, const void *fdata EINA_UNUSED)
{
   printf("Struct: %s\n", name);
   printf("  type: <");
   database_type_print((Eolian_Type*)tp);
   printf(">\n");
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_show_struct(const char *name)
{
   if (!name)
     eina_hash_foreach(_structs, (Eina_Hash_Foreach)_struct_cb, NULL);
   else
     {
        Eina_Stringshare *shr = eina_stringshare_add(name);
        Eolian_Type *tp = eina_hash_find(_structs, shr);
        eina_stringshare_del(shr);
        if (!tp) return EINA_FALSE;
        _struct_cb(NULL, name, tp, NULL);
     }
   return EINA_TRUE;
}

EAPI void
eolian_show_all()
{
   eolian_show_class(NULL);
   eolian_show_typedef(NULL);
   eolian_show_struct(NULL);
}
