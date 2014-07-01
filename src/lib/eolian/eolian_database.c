#include <Eina.h>
#include "eo_parser.h"
#include "eolian_database.h"

#define PROP_GET_RETURN_DFLT_VAL "property_get_return_dflt_val"
#define PROP_SET_RETURN_DFLT_VAL "property_set_return_dflt_val"
#define METHOD_RETURN_DFLT_VAL "method_return_dflt_val"

#define EOLIAN_METHOD_RETURN_TYPE "method_return_type"
#define EOLIAN_PROP_GET_RETURN_TYPE "property_get_return_type"
#define EOLIAN_PROP_SET_RETURN_TYPE "property_set_return_type"

#define EOLIAN_METHOD_RETURN_COMMENT "method_return_comment"
#define EOLIAN_PROP_GET_RETURN_COMMENT "property_get_return_comment"
#define EOLIAN_PROP_SET_RETURN_COMMENT "property_set_return_comment"

static Eina_List *_classes = NULL;
static Eina_Hash *_types = NULL;
static Eina_Hash *_filenames = NULL; /* Hash: filename without extension -> full path */
static Eina_Hash *_tfilenames = NULL;
static int _database_init_count = 0;

typedef struct
{
   Eina_Stringshare *full_name;
   Eina_List *namespaces; /* List Eina_Stringshare * */
   Eina_Stringshare *name;
   Eina_Stringshare *file;
   Eolian_Class_Type type;
   Eina_Stringshare *description;
   Eina_Stringshare *legacy_prefix;
   Eina_Stringshare *eo_prefix;
   Eina_Stringshare *data_type;
   Eina_List *inherits; /* List Eina_Stringshare * */
   Eina_List *properties; /* List prop_name -> _Function_Id */
   Eina_List *methods; /* List meth_name -> _Function_Id */
   Eina_List *constructors; /* List constructor_name -> _Function_Id */
   Eina_List *implements; /* List implements name -> _Implement_Desc */
   Eina_List *events; /* List event_name -> _Event_Desc */
   Eina_Bool class_ctor_enable:1;
   Eina_Bool class_dtor_enable:1;
} _Class_Desc;

typedef struct
{
   Eina_Stringshare *alias;
   Eolian_Type type;
} Type_Desc;

typedef struct
{
   Eina_Stringshare *name;
   Eina_List *keys; /* list of _Parameter_Desc */
   Eina_List *params; /* list of _Parameter_Desc */
   Eolian_Function_Type type;
   Eolian_Function_Scope scope;
   Eolian_Type get_ret_type;
   Eolian_Type set_ret_type;
   Eina_Hash *data;
   Eina_Bool obj_is_const :1; /* True if the object has to be const. Useful for a few methods. */
   Eina_Bool get_virtual_pure :1;
   Eina_Bool set_virtual_pure :1;
   Eina_Bool get_return_warn_unused :1; /* also used for methods */
   Eina_Bool set_return_warn_unused :1;
} _Function_Id;

typedef struct
{
   Eina_Stringshare *name;
   Eolian_Type type;
   Eina_Stringshare *description;
   Eolian_Parameter_Dir param_dir;
   Eina_Bool is_const_on_get :1; /* True if const in this the get property */
   Eina_Bool is_const_on_set :1; /* True if const in this the set property */
   Eina_Bool nonull :1; /* True if this argument cannot be NULL */
} _Parameter_Desc;

/* maps directly to Eo_Type_Def */
typedef struct
{
   const char        *name;
   Eolian_Type_Type   type;
   union {
      struct {
         Eina_List   *subtypes;
         Eolian_Type  base_type;
      };
      struct {
         Eina_List   *arguments;
         Eolian_Type  ret_type;
      };
   };
   Eina_Bool is_const  :1;
   Eina_Bool is_own    :1;
} _Parameter_Type;

typedef struct
{
   Eina_Stringshare *full_name;
} _Implement_Desc;

typedef struct
{
   Eina_Stringshare *name;
   Eina_Stringshare *type;
   Eina_Stringshare *comment;
} _Event_Desc;

static void
_param_del(_Parameter_Desc *pdesc)
{
   eina_stringshare_del(pdesc->name);

   database_type_del(pdesc->type);
   eina_stringshare_del(pdesc->description);
   free(pdesc);
}

void
database_type_del(Eolian_Type type)
{
   _Parameter_Type *typep = (_Parameter_Type*)type;
   Eolian_Type stype;
   if (!type) return;
   if (typep->name) eina_stringshare_del(typep->name);
   /* for function types, this will map to arguments and ret_type */
   if (typep->subtypes)
      EINA_LIST_FREE(typep->subtypes, stype)
         database_type_del(stype);
   if (typep->base_type)
      database_type_del(typep->base_type);
   free(typep);
}

static void
_fid_del(_Function_Id *fid)
{
   _Parameter_Desc *param;
   if (!fid) return;
   eina_stringshare_del(fid->name);
   eina_hash_free(fid->data);
   EINA_LIST_FREE(fid->keys, param) _param_del(param);
   EINA_LIST_FREE(fid->params, param) _param_del(param);
   database_type_del(fid->get_ret_type);
   database_type_del(fid->set_ret_type);
   free(fid);
}

static void
_class_del(_Class_Desc *class)
{
   Eina_Stringshare *inherit_name;
   Eina_List *inherits = class->inherits;
   EINA_LIST_FREE(inherits, inherit_name)
      eina_stringshare_del(inherit_name);

   _Implement_Desc *impl;
   Eina_List *implements = class->implements;
   EINA_LIST_FREE(implements, impl)
     {
        eina_stringshare_del(impl->full_name);
        free(impl);
     }

   _Function_Id *fid;
   Eolian_Event ev;
   EINA_LIST_FREE(class->constructors, fid) _fid_del(fid);
   EINA_LIST_FREE(class->methods, fid) _fid_del(fid);
   EINA_LIST_FREE(class->properties, fid) _fid_del(fid);
   EINA_LIST_FREE(class->events, ev) database_event_free(ev);

   eina_stringshare_del(class->name);
   eina_stringshare_del(class->full_name);
   eina_stringshare_del(class->file);
   eina_stringshare_del(class->description);
   eina_stringshare_del(class->legacy_prefix);
   eina_stringshare_del(class->eo_prefix);
   eina_stringshare_del(class->data_type);
   free(class);
}

static void _type_hash_free_cb(void *data)
{
   Type_Desc *type = data;
   eina_stringshare_del(type->alias);
   database_type_del(type->type);
   free(type);
}

int
database_init()
{
   if (_database_init_count > 0) return ++_database_init_count;
   eina_init();
   _types = eina_hash_stringshared_new(_type_hash_free_cb);
   _filenames = eina_hash_string_small_new(free);
   _tfilenames = eina_hash_string_small_new(free);
   return ++_database_init_count;
}

int
database_shutdown()
{
   if (_database_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   _database_init_count--;

   if (_database_init_count == 0)
     {
        Eolian_Class class;
        EINA_LIST_FREE(_classes, class)
           _class_del((_Class_Desc *)class);
        eina_hash_free(_types);
        eina_hash_free(_filenames);
        eina_hash_free(_tfilenames);
        eina_shutdown();
     }
   return _database_init_count;
}

Eina_Bool
database_type_add(const char *alias, Eolian_Type type)
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

EAPI Eolian_Type
eolian_type_find_by_alias(const char *alias)
{
   if (!_types) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(alias);
   Type_Desc *cl = eina_hash_find(_types, shr);
   eina_stringshare_del(shr);
   return cl?cl->type:NULL;
}

Eolian_Class
database_class_add(const char *class_name, Eolian_Class_Type type)
{
   char *full_name = strdup(class_name);
   char *name = full_name;
   char *colon = full_name;
   _Class_Desc *cl = calloc(1, sizeof(*cl));
   cl->full_name = eina_stringshare_add(class_name);
   cl->type = type;
   do
     {
        colon = strchr(colon, '.');
        if (colon)
          {
             *colon = '\0';
             cl->namespaces = eina_list_append(cl->namespaces, eina_stringshare_add(name));
             colon += 1;
             name = colon;
          }
     }
   while(colon);
   cl->name = eina_stringshare_add(name);
   _classes = eina_list_append(_classes, cl);
   free(full_name);
   return (Eolian_Class)cl;
}

Eina_Bool
database_class_file_set(Eolian_Class class, const char *file_name)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->file = eina_stringshare_add(file_name);
   return EINA_TRUE;
}

EAPI const char *
eolian_class_file_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   return cl ? cl->file : NULL;
}

EAPI const char *
eolian_class_full_name_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   return cl ? cl->full_name : NULL;
}

EAPI const char *
eolian_class_name_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   return cl ? cl->name : NULL;
}

EAPI const Eina_List *
eolian_class_namespaces_list_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   return cl ? cl->namespaces : NULL;
}

EAPI Eolian_Class
eolian_class_find_by_name(const char *class_name)
{
   Eina_List *itr;
   _Class_Desc *cl;
   Eina_Stringshare *shr_name = eina_stringshare_add(class_name);
   EINA_LIST_FOREACH(_classes, itr, cl)
      if (cl->full_name == shr_name) goto end;
   cl = NULL;
end:
   eina_stringshare_del(shr_name);
   return (Eolian_Class)cl;
}

/*
 * ret false -> clash, class = NULL
 * ret true && class -> only one class corresponding
 * ret true && !class -> no class corresponding
 */
Eina_Bool database_class_name_validate(const char *class_name, Eolian_Class *class)
{
   char *name = strdup(class_name);
   char *colon = name + 1;
   Eolian_Class found_class = NULL;
   Eolian_Class candidate;
   if (class) *class = NULL;
   do
     {
        colon = strchr(colon, '.');
        if (colon) *colon = '\0';
        candidate = eolian_class_find_by_name(name);
        if (candidate)
          {
             if (found_class)
               {
                  ERR("Name clash between class %s and class %s",
                        ((_Class_Desc *)candidate)->full_name,
                        ((_Class_Desc *)found_class)->full_name);
                  free(name);
                  return EINA_FALSE; // Names clash
               }
             found_class = candidate;
          }
        if (colon) *colon++ = '.';
     }
   while(colon);
   if (class) *class = found_class;
   free(name);
   return EINA_TRUE;
}

EAPI Eolian_Class
eolian_class_find_by_file(const char *file_name)
{
   Eina_List *itr;
   _Class_Desc *cl;
   Eina_Stringshare *shr_file = eina_stringshare_add(file_name);
   EINA_LIST_FOREACH(_classes, itr, cl)
      if (cl->file == shr_file) goto end;
   cl = NULL;
end:
   eina_stringshare_del(shr_file);
   return (Eolian_Class)cl;
}

EAPI Eolian_Class_Type
eolian_class_type_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EOLIAN_CLASS_UNKNOWN_TYPE);
   return cl->type;
}

Eina_Bool
database_class_del(Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   _classes = eina_list_remove(_classes, class);
   _class_del(cl);
   return EINA_TRUE;
}

EAPI const Eina_List *
eolian_class_names_list_get(void)
{
   Eina_List *itr;
   _Class_Desc *cl;
   Eina_List *list = NULL;
   EINA_LIST_FOREACH(_classes, itr, cl)
      list = eina_list_append(list, cl->name);
   return list;
}

Eina_Bool
database_class_inherit_add(Eolian_Class class, const char *inherit_class_name)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->inherits = eina_list_append(cl->inherits, eina_stringshare_add(inherit_class_name));
   return EINA_TRUE;
}

EAPI const char *
eolian_class_description_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->description;
}

void
database_class_description_set(Eolian_Class class, const char *description)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->description = eina_stringshare_add(description);
}

EAPI const char*
eolian_class_legacy_prefix_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->legacy_prefix;
}

void
database_class_legacy_prefix_set(Eolian_Class class, const char *legacy_prefix)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->legacy_prefix = eina_stringshare_add(legacy_prefix);
}

EAPI const char*
eolian_class_eo_prefix_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->eo_prefix;
}

void
database_class_eo_prefix_set(Eolian_Class class, const char *eo_prefix)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->eo_prefix = eina_stringshare_add(eo_prefix);
}

EAPI const char*
eolian_class_data_type_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->data_type;
}

void
database_class_data_type_set(Eolian_Class class, const char *data_type)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->data_type= eina_stringshare_add(data_type);
}

EAPI const Eina_List *
eolian_class_inherits_list_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   //FIXME: create list here
   return cl->inherits;
}

EAPI const Eina_List*
eolian_class_implements_list_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->implements;
}

Eolian_Function
database_function_new(const char *function_name, Eolian_Function_Type foo_type)
{
   _Function_Id *fid = calloc(1, sizeof(*fid));
   fid->name = eina_stringshare_add(function_name);
   fid->type = foo_type;
   fid->data = eina_hash_string_superfast_new(free);
   return (Eolian_Function) fid;
}

EAPI Eolian_Function_Scope
eolian_function_scope_get(Eolian_Function function_id)
{
   _Function_Id *fid = (_Function_Id *)function_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EOLIAN_SCOPE_PUBLIC);
   return fid->scope;
}

void
database_function_scope_set(Eolian_Function function_id, Eolian_Function_Scope scope)
{
   _Function_Id *fid = (_Function_Id *)function_id;
   EINA_SAFETY_ON_NULL_RETURN(fid);
   fid->scope = scope;
}

void
database_function_type_set(Eolian_Function function_id, Eolian_Function_Type foo_type)
{
   _Function_Id *fid = (_Function_Id *)function_id;
   EINA_SAFETY_ON_NULL_RETURN(fid);
   switch (foo_type)
     {
      case EOLIAN_PROP_SET:
         if (fid->type == EOLIAN_PROP_GET) foo_type = EOLIAN_PROPERTY;
         break;
      case EOLIAN_PROP_GET:
         if (fid->type == EOLIAN_PROP_SET) foo_type = EOLIAN_PROPERTY;
         break;
      default:
         break;
     }
   fid->type = foo_type;
}

Eina_Bool database_class_function_add(Eolian_Class class, Eolian_Function foo_id)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(foo_id && cl, EINA_FALSE);
   _Function_Id *fid = (_Function_Id *) foo_id;
   switch (fid->type)
     {
      case EOLIAN_PROPERTY:
      case EOLIAN_PROP_SET:
      case EOLIAN_PROP_GET:
         cl->properties = eina_list_append(cl->properties, foo_id);
         break;
      case EOLIAN_METHOD:
         cl->methods = eina_list_append(cl->methods, foo_id);
         break;
      case EOLIAN_CTOR:
         cl->constructors = eina_list_append(cl->constructors, foo_id);
         break;
      default:
         ERR("Bad function type %d.", fid->type);
         return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eolian_Implement
database_implement_new(const char *impl_name)
{
   _Implement_Desc *impl_desc = calloc(1, sizeof(_Implement_Desc));
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl_desc, NULL);
   impl_desc->full_name = eina_stringshare_add(impl_name);
   return (Eolian_Implement) impl_desc;
}

Eina_Bool
database_class_implement_add(Eolian_Class class, Eolian_Implement impl_desc)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl_desc, EINA_FALSE);
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->implements = eina_list_append(cl->implements, impl_desc);
   return EINA_TRUE;
}

EAPI Eina_Stringshare *
eolian_implement_full_name_get(const Eolian_Implement impl)
{
   _Implement_Desc *_impl = (_Implement_Desc *)impl;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_impl, NULL);
   return _impl->full_name;
}

EAPI Eina_Bool
eolian_implement_information_get(const Eolian_Implement impl, Eolian_Class *class_out, Eolian_Function *func_out, Eolian_Function_Type *type_out)
{
   _Implement_Desc *_impl = (_Implement_Desc *)impl;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_impl, EINA_FALSE);
   Eolian_Class class;
   if (!database_class_name_validate(_impl->full_name, &class) || !class) return EINA_FALSE;
   const char *class_name = ((_Class_Desc *)class)->full_name;
   if (class_out) *class_out = class;

   char *func_name = strdup(_impl->full_name + strlen(class_name) + 1);
   char *colon = strchr(func_name, '.');
   Eolian_Function_Type type = EOLIAN_UNRESOLVED;
   if (colon)
     {
        *colon = '\0';
        if (!strcmp(colon+1, "set")) type = EOLIAN_PROP_SET;
        else if (!strcmp(colon+1, "get")) type = EOLIAN_PROP_GET;
     }

   Eolian_Function fid = eolian_class_function_find_by_name(class, func_name, type);
   if (func_out) *func_out = fid;
   if (type == EOLIAN_UNRESOLVED) type = eolian_function_type_get(fid);
   if (type_out) *type_out = type;
   free(func_name);
   return EINA_TRUE;
}

EAPI Eolian_Function
eolian_class_function_find_by_name(const Eolian_Class class, const char *func_name, Eolian_Function_Type f_type)
{
   Eina_List *itr;
   Eolian_Function foo_id;
   _Class_Desc *cl = (_Class_Desc *)class;
   if (!cl) return NULL;

   if (f_type == EOLIAN_UNRESOLVED || f_type == EOLIAN_METHOD)
      EINA_LIST_FOREACH(cl->methods, itr, foo_id)
        {
           _Function_Id *fid = (_Function_Id *) foo_id;
           if (!strcmp(fid->name, func_name))
              return foo_id;
        }

   if (f_type == EOLIAN_UNRESOLVED || f_type == EOLIAN_PROPERTY ||
         f_type == EOLIAN_PROP_SET || f_type == EOLIAN_PROP_GET)
     {
        EINA_LIST_FOREACH(cl->properties, itr, foo_id)
          {
             _Function_Id *fid = (_Function_Id *) foo_id;
             if (!strcmp(fid->name, func_name))
                return foo_id;
          }
     }

   if (f_type == EOLIAN_UNRESOLVED || f_type == EOLIAN_CTOR)
     {
        EINA_LIST_FOREACH(cl->constructors, itr, foo_id)
          {
             _Function_Id *fid = (_Function_Id *) foo_id;
             if (!strcmp(fid->name, func_name))
                return foo_id;
          }
     }

   ERR("Function %s not found in class %s", func_name, cl->name);
   return NULL;
}

EAPI const Eina_List *
eolian_class_functions_list_get(const Eolian_Class class, Eolian_Function_Type foo_type)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   switch (foo_type)
     {
      case EOLIAN_PROPERTY:
         return cl->properties;
      case EOLIAN_METHOD:
         return cl->methods;
      case EOLIAN_CTOR:
         return cl->constructors;
      default: return NULL;
     }
}

EAPI Eolian_Function_Type
eolian_function_type_get(Eolian_Function function_id)
{
   _Function_Id *fid = (_Function_Id *)function_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EOLIAN_UNRESOLVED);
   return fid->type;
}

EAPI const char *
eolian_function_name_get(Eolian_Function function_id)
{
   _Function_Id *fid = (_Function_Id *)function_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return fid->name;
}

EAPI const char *
eolian_function_full_c_name_get(Eolian_Function foo_id, const char *prefix)
{
   const char  *funcn = eolian_function_name_get(foo_id);
   const char  *last_p = strrchr(prefix, '_');
   const char  *func_p = strchr(funcn, '_');
   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Stringshare *ret;
   int   len;

   if (!last_p) last_p = prefix;
   else last_p++;
   if (!func_p) len = strlen(funcn);
   else len = func_p - funcn;

   if ((int)strlen(last_p) != len || strncmp(last_p, funcn, len))
     {
        eina_strbuf_append(buf, prefix);
        eina_strbuf_append_char(buf, '_');
        eina_strbuf_append(buf, funcn);
        ret = eina_stringshare_add(eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
        return ret;
     }

   if (last_p != prefix)
      eina_strbuf_append_n(buf, prefix, last_p - prefix); /* includes _ */

   eina_strbuf_append(buf, funcn);
   ret = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
}

Eina_Bool
database_function_set_as_virtual_pure(Eolian_Function function_id, Eolian_Function_Type ftype)
{
   _Function_Id *fid = (_Function_Id *)function_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: fid->get_virtual_pure = EINA_TRUE; break;
      case EOLIAN_PROP_SET: fid->set_virtual_pure = EINA_TRUE; break;
      default: return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_function_is_virtual_pure(Eolian_Function function_id, Eolian_Function_Type ftype)
{
   _Function_Id *fid = (_Function_Id *)function_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: return fid->get_virtual_pure; break;
      case EOLIAN_PROP_SET: return fid->set_virtual_pure; break;
      default: return EINA_FALSE;
     }
}

void
database_function_data_set(Eolian_Function function_id, const char *key, const char *data)
{
   EINA_SAFETY_ON_NULL_RETURN(key);
   _Function_Id *fid = (_Function_Id *)function_id;
   EINA_SAFETY_ON_NULL_RETURN(fid);
   if (data)
     {
        if (!eina_hash_find(fid->data, key))
          eina_hash_set(fid->data, key, strdup(data));
     }
   else
     {
        eina_hash_del(fid->data, key, NULL);
     }
}

EAPI const char *
eolian_function_data_get(Eolian_Function function_id, const char *key)
{
   _Function_Id *fid = (_Function_Id *)function_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return eina_hash_find(fid->data, key);
}

static _Parameter_Desc *
_parameter_new(Eolian_Type type, const char *name, const char *description)
{
   _Parameter_Desc *param = NULL;
   param = calloc(1, sizeof(*param));
   param->name = eina_stringshare_add(name);
   param->type = type;
   param->description = eina_stringshare_add(description);
   return param;
}

Eolian_Function_Parameter
database_property_key_add(Eolian_Function foo_id, Eolian_Type type, const char *name, const char *description)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   _Parameter_Desc *param = _parameter_new(type, name, description);
   fid->keys = eina_list_append(fid->keys, param);
   return (Eolian_Function_Parameter)param;
}

Eolian_Function_Parameter
database_property_value_add(Eolian_Function foo_id, Eolian_Type type, const char *name, const char *description)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   _Parameter_Desc *param = _parameter_new(type, name, description);
   fid->params = eina_list_append(fid->params, param);
   return (Eolian_Function_Parameter)param;
}

Eolian_Function_Parameter
database_method_parameter_add(Eolian_Function foo_id, Eolian_Parameter_Dir param_dir, Eolian_Type type, const char *name, const char *description)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   _Parameter_Desc *param = _parameter_new(type, name, description);
   param->param_dir = param_dir;
   fid->params = eina_list_append(fid->params, param);
   return (Eolian_Function_Parameter)param;
}

EAPI Eolian_Function_Parameter
eolian_function_parameter_get(const Eolian_Function foo_id, const char *param_name)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   Eina_List *itr;
   _Parameter_Desc *param;
   EINA_LIST_FOREACH(fid->keys, itr, param)
      if (!strcmp(param->name, param_name)) return (Eolian_Function_Parameter)param;
   EINA_LIST_FOREACH(fid->params, itr, param)
      if (!strcmp(param->name, param_name)) return (Eolian_Function_Parameter)param;
   return NULL;
}

EAPI Eolian_Type
eolian_parameter_type_get(const Eolian_Function_Parameter param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   return ((_Parameter_Desc*)param)->type;
}

EAPI Eina_Stringshare *
eolian_parameter_name_get(const Eolian_Function_Parameter param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   eina_stringshare_ref(((_Parameter_Desc*)param)->name);
   return ((_Parameter_Desc*)param)->name;
}

EAPI const Eina_List *
eolian_property_keys_list_get(Eolian_Function foo_id)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return fid->keys;
}

EAPI const Eina_List *
eolian_property_values_list_get(Eolian_Function foo_id)
{
   return eolian_parameters_list_get(foo_id);
}

EAPI const Eina_List *
eolian_parameters_list_get(Eolian_Function foo_id)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return fid->params;
}

/* Get parameter information */
EAPI void
eolian_parameter_information_get(const Eolian_Function_Parameter param_desc, Eolian_Parameter_Dir *param_dir, Eolian_Type *type, const char **name, const char **description)
{
   _Parameter_Desc *param = (_Parameter_Desc *)param_desc;
   EINA_SAFETY_ON_NULL_RETURN(param);
   if (param_dir) *param_dir = param->param_dir;
   if (type) *type = param->type;
   if (name) *name = param->name;
   if (description) *description = param->description;
}

void
database_parameter_const_attribute_set(Eolian_Function_Parameter param_desc, Eina_Bool is_get, Eina_Bool is_const)
{
   _Parameter_Desc *param = (_Parameter_Desc *)param_desc;
   EINA_SAFETY_ON_NULL_RETURN(param);
   if (is_get)
      param->is_const_on_get = is_const;
   else
      param->is_const_on_set = is_const;
}

void
database_parameter_type_set(Eolian_Function_Parameter param_desc, Eolian_Type types)
{
   _Parameter_Desc *param = (_Parameter_Desc *)param_desc;
   EINA_SAFETY_ON_NULL_RETURN(param);
   param->type = types;
}

EAPI Eina_Bool
eolian_parameter_const_attribute_get(Eolian_Function_Parameter param_desc, Eina_Bool is_get)
{
   _Parameter_Desc *param = (_Parameter_Desc *)param_desc;
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EINA_FALSE);
   if (is_get)
      return param->is_const_on_get;
   else
      return param->is_const_on_set;
}

void
database_parameter_nonull_set(Eolian_Function_Parameter param_desc, Eina_Bool nonull)
{
   _Parameter_Desc *param = (_Parameter_Desc *)param_desc;
   EINA_SAFETY_ON_NULL_RETURN(param);
   param->nonull = nonull;
}

EAPI Eina_Bool
eolian_parameter_is_nonull(Eolian_Function_Parameter param_desc)
{
   _Parameter_Desc *param = (_Parameter_Desc *)param_desc;
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EINA_FALSE);
   return param->nonull;
}

void database_function_return_type_set(Eolian_Function foo_id, Eolian_Function_Type ftype, Eolian_Type ret_type)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: fid->set_ret_type = ret_type; break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: fid->get_ret_type = ret_type; break;
      default: return;
     }
}

EAPI Eolian_Type
eolian_function_return_type_get(Eolian_Function foo_id, Eolian_Function_Type ftype)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: return fid->set_ret_type;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: return fid->get_ret_type;
      default: return NULL;
     }
}

void database_function_return_dflt_val_set(Eolian_Function foo_id, Eolian_Function_Type ftype, const char *ret_dflt_value)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = PROP_SET_RETURN_DFLT_VAL; break;
      case EOLIAN_PROP_GET: key = PROP_GET_RETURN_DFLT_VAL; break;
      case EOLIAN_METHOD: key = METHOD_RETURN_DFLT_VAL; break;
      default: return;
     }
   database_function_data_set(foo_id, key, ret_dflt_value);
}

EAPI const char *
eolian_function_return_dflt_value_get(Eolian_Function foo_id, Eolian_Function_Type ftype)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = PROP_SET_RETURN_DFLT_VAL; break;
      case EOLIAN_PROP_GET: key = PROP_GET_RETURN_DFLT_VAL; break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: key = METHOD_RETURN_DFLT_VAL; break;
      default: return NULL;
     }
   return eolian_function_data_get(foo_id, key);
}

EAPI const char *
eolian_function_return_comment_get(Eolian_Function foo_id, Eolian_Function_Type ftype)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = EOLIAN_PROP_SET_RETURN_COMMENT; break;
      case EOLIAN_PROP_GET: key = EOLIAN_PROP_GET_RETURN_COMMENT; break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: key = EOLIAN_METHOD_RETURN_COMMENT; break;
      default: return NULL;
     }
   return eolian_function_data_get(foo_id, key);
}

void database_function_return_comment_set(Eolian_Function foo_id, Eolian_Function_Type ftype, const char *ret_comment)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = EOLIAN_PROP_SET_RETURN_COMMENT; break;
      case EOLIAN_PROP_GET: key = EOLIAN_PROP_GET_RETURN_COMMENT; break;
      case EOLIAN_METHOD: key = EOLIAN_METHOD_RETURN_COMMENT; break;
      default: return;
     }
   database_function_data_set(foo_id, key, ret_comment);
}

void database_function_return_flag_set_as_warn_unused(Eolian_Function foo_id,
      Eolian_Function_Type ftype, Eina_Bool warn_unused)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN(fid);
   switch (ftype)
     {
      case EOLIAN_METHOD: case EOLIAN_PROP_GET: fid->get_return_warn_unused = warn_unused; break;
      case EOLIAN_PROP_SET: fid->set_return_warn_unused = warn_unused; break;
      default: return;
     }
}

EAPI Eina_Bool
eolian_function_return_is_warn_unused(Eolian_Function foo_id,
      Eolian_Function_Type ftype)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_METHOD: case EOLIAN_PROP_GET: return fid->get_return_warn_unused;
      case EOLIAN_PROP_SET: return fid->set_return_warn_unused;
      default: return EINA_FALSE;
     }
}

void
database_function_object_set_as_const(Eolian_Function foo_id, Eina_Bool is_const)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN(fid);
   fid->obj_is_const = is_const;
}

EAPI Eina_Bool
eolian_function_object_is_const(Eolian_Function foo_id)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   return fid->obj_is_const;
}

Eolian_Event
database_event_new(const char *event_name, const char *event_type, const char *event_comment)
{
   if (!event_name) return NULL;
   _Event_Desc *event_desc = calloc(1, sizeof(_Event_Desc));
   if (!event_desc) return NULL;
   event_desc->name = eina_stringshare_add(event_name);
   if (event_type) event_desc->type = eina_stringshare_add(event_type);
   event_desc->comment = eina_stringshare_add(event_comment);
   return (Eolian_Event) event_desc;
}

void
database_event_free(Eolian_Event event)
{
   _Event_Desc *event_desc = (_Event_Desc *)event;
   eina_stringshare_del(event_desc->name);
   eina_stringshare_del(event_desc->comment);
   free(event_desc);
}

Eina_Bool
database_class_event_add(Eolian_Class class, Eolian_Event event_desc)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(event_desc && cl, EINA_FALSE);
   cl->events = eina_list_append(cl->events, event_desc);
   return EINA_TRUE;
}

EAPI const Eina_List*
eolian_class_events_list_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->events;
}

EAPI Eina_Bool
eolian_class_event_information_get(Eolian_Event event, const char **event_name, const char **event_type, const char **event_comment)
{
   _Event_Desc *_event_desc = (_Event_Desc *) event;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_event_desc, EINA_FALSE);
   if (event_name) *event_name = _event_desc->name;
   if (event_type) *event_type = _event_desc->type;
   if (event_comment) *event_comment = _event_desc->comment;
   return EINA_TRUE;
}

Eina_Bool
database_class_ctor_enable_set(Eolian_Class class, Eina_Bool enable)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->class_ctor_enable = enable;
   return EINA_TRUE;
}

Eina_Bool
database_class_dtor_enable_set(Eolian_Class class, Eina_Bool enable)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->class_dtor_enable = enable;
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_class_ctor_enable_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   return cl->class_ctor_enable;
}

EAPI Eina_Bool
eolian_class_dtor_enable_get(const Eolian_Class class)
{
   _Class_Desc *cl = (_Class_Desc *)class;
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   return cl->class_dtor_enable;
}

EAPI Eolian_Type_Type
eolian_type_type_get(Eolian_Type tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, EOLIAN_TYPE_UNKNOWN_TYPE);
   return ((_Parameter_Type*)tp)->type;
}

EAPI Eina_Iterator *
eolian_type_arguments_list_get(Eolian_Type tp)
{
   _Parameter_Type *tpp = (_Parameter_Type*)tp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eolian_type_type_get(tp) == EOLIAN_TYPE_FUNCTION, NULL);
   if (!tpp->arguments) return NULL;
   return eina_list_iterator_new(tpp->arguments);
}

EAPI Eina_Iterator *
eolian_type_subtypes_list_get(Eolian_Type tp)
{
   _Parameter_Type *tpp = (_Parameter_Type*)tp;
   Eolian_Type_Type tpt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpt = tpp->type;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpt == EOLIAN_TYPE_REGULAR || tpt == EOLIAN_TYPE_POINTER, NULL);
   if (!tpp->subtypes) return NULL;
   return eina_list_iterator_new(tpp->subtypes);
}

EAPI Eolian_Type
eolian_type_return_type_get(Eolian_Type tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eolian_type_type_get(tp) == EOLIAN_TYPE_FUNCTION, NULL);
   return ((_Parameter_Type*)tp)->ret_type;
}

EAPI Eolian_Type
eolian_type_base_type_get(Eolian_Type tp)
{
   Eolian_Type_Type tpt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   tpt = eolian_type_type_get(tp);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(tpt == EOLIAN_TYPE_REGULAR || tpt == EOLIAN_TYPE_POINTER, NULL);
   return ((_Parameter_Type*)tp)->base_type;
}

EAPI Eina_Bool
eolian_type_is_own(Eolian_Type tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, EINA_FALSE);
   return ((_Parameter_Type*)tp)->is_own;
}

EAPI Eina_Bool
eolian_type_is_const(Eolian_Type tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, EINA_FALSE);
   return ((_Parameter_Type*)tp)->is_const;
}

static void _type_to_str(Eolian_Type tp, Eina_Strbuf *buf, const char *name);

static void
_ftype_to_str(Eolian_Type tp, Eina_Strbuf *buf, const char *name)
{
   _Parameter_Type *tpp = (_Parameter_Type*)tp;
   Eina_List *l;
   Eolian_Type stp;
   Eina_Bool first = EINA_TRUE;
   if (tpp->ret_type)
     _type_to_str(tpp->ret_type, buf, NULL);
   else
     eina_strbuf_append(buf, "void");
   eina_strbuf_append(buf, " (*");
   if (name) eina_strbuf_append(buf, name);
   eina_strbuf_append(buf, ")(");
   EINA_LIST_FOREACH(tpp->arguments, l, stp)
     {
        if (!first) eina_strbuf_append(buf, ", ");
        first = EINA_FALSE;
        _type_to_str(stp, buf, NULL);
     }
}

static void
_type_to_str(Eolian_Type tp, Eina_Strbuf *buf, const char *name)
{
   _Parameter_Type *tpp = (_Parameter_Type*)tp;
   if (tpp->type == EOLIAN_TYPE_FUNCTION)
     {
        _ftype_to_str(tp, buf, name);
        return;
     }
   if ((tpp->type == EOLIAN_TYPE_REGULAR || tpp->type == EOLIAN_TYPE_VOID)
     && tpp->is_const)
      eina_strbuf_append(buf, "const ");
   if (tpp->type == EOLIAN_TYPE_REGULAR)
     eina_strbuf_append(buf, tpp->name);
   else if (tpp->type == EOLIAN_TYPE_VOID)
     eina_strbuf_append(buf, "void");
   else
     {
        _Parameter_Type *btpp = (_Parameter_Type*)tpp->base_type;
        _type_to_str(tpp->base_type, buf, NULL);
        if (btpp->type != EOLIAN_TYPE_POINTER || btpp->is_const)
           eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append_char(buf, '*');
        if (tpp->is_const) eina_strbuf_append(buf, " const");
     }
   if (name)
     {
        eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, name);
     }
}

EAPI Eina_Stringshare *
eolian_type_c_type_named_get(Eolian_Type tp, const char *name)
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
eolian_type_c_type_get(Eolian_Type tp)
{
   return eolian_type_c_type_named_get(tp, NULL);
}

EAPI Eina_Stringshare *
eolian_type_name_get(Eolian_Type tp)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tp, NULL);
   eina_stringshare_ref(((_Parameter_Type*)tp)->name);
   return ((_Parameter_Type*)tp)->name;
}

static void
_implements_print(Eolian_Implement impl, int nb_spaces)
{
   Eolian_Class class;
   Eolian_Function func;
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
_event_print(Eolian_Event ev, int nb_spaces)
{
   const char *name, *comment, *type;

   eolian_class_event_information_get(ev, &name, &type, &comment);
   printf("%*s <%s> <%s> <%s>\n", nb_spaces + 5, "", name, type, comment);
}

static void
_type_print(Eolian_Type tp, Eina_Strbuf *buf)
{
   _Parameter_Type *tpp = (_Parameter_Type*)tp;
   Eina_List *l;
   Eolian_Type stp;
   if (tpp->is_own)
     eina_strbuf_append(buf, "@own(");
   if (tpp->is_const)
     eina_strbuf_append(buf, "const(");
   if (tpp->type == EOLIAN_TYPE_REGULAR)
     eina_strbuf_append(buf, tpp->name);
   else if (tpp->type == EOLIAN_TYPE_POINTER)
     {
        _type_print(tpp->base_type, buf);
        eina_strbuf_append_char(buf, '*');
     }
   else if (tpp->type == EOLIAN_TYPE_FUNCTION)
     {
        Eina_Bool first = EINA_TRUE;
        eina_strbuf_append(buf, "fn");
        if (tpp->ret_type)
          {
             eina_strbuf_append(buf, " -> ");
             _type_print(tpp->ret_type, buf);
          }
        eina_strbuf_append(buf, " (");
        EINA_LIST_FOREACH(tpp->arguments, l, stp)
          {
             if (!first) eina_strbuf_append(buf, ", ");
             first = EINA_FALSE;
             _type_print(stp, buf);
          }
        eina_strbuf_append_char(buf, ')');
     }
   if (tpp->is_own)
     eina_strbuf_append_char(buf, ')');
   if (tpp->is_const)
      eina_strbuf_append_char(buf, ')');
}

static Eina_Bool _function_print(const _Function_Id *fid, int nb_spaces)
{
   Eolian_Function foo_id = (Eolian_Function) fid;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   const char *ret_desc = eolian_function_return_comment_get(foo_id, fid->type);
   switch (fid->type)
     {
      case EOLIAN_PROPERTY:
           {
              printf("%*s<%s> %s\n", nb_spaces, "", ret_desc ? ret_desc : "", fid->name);
              const char *str = eolian_function_description_get(foo_id, EOLIAN_COMMENT_GET);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_description_get(foo_id, EOLIAN_COMMENT_SET);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_LEGACY_GET);
              if (str) printf("%*slegacy_get: <%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_LEGACY_SET);
              if (str) printf("%*slegacy_set: <%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_PROP_GET_RETURN_TYPE);
              if (str) printf("%*sreturn type for get: <%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_PROP_SET_RETURN_TYPE);
              if (str) printf("%*sreturn type for set: <%s>\n", nb_spaces + 5, "", str);
              break;
           }
      case EOLIAN_PROP_GET:
           {
              printf("%*sGET:<%s> %s\n", nb_spaces, "", ret_desc ? ret_desc : "", fid->name);
              const char *str = eolian_function_description_get(foo_id, EOLIAN_COMMENT_GET);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_LEGACY_GET);
              if (str) printf("%*slegacy: <%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_PROP_GET_RETURN_TYPE);
              if (str) printf("%*sreturn type: <%s>\n", nb_spaces + 5, "", str);
              break;
           }
      case EOLIAN_PROP_SET:
           {
              printf("%*sSET:<%s> %s\n", nb_spaces, "", ret_desc ? ret_desc : "", fid->name);
              const char *str = eolian_function_description_get(foo_id, EOLIAN_COMMENT_SET);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_LEGACY_SET);
              if (str) printf("%*slegacy: <%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_PROP_SET_RETURN_TYPE);
              if (str) printf("%*sreturn type: <%s>\n", nb_spaces + 5, "", str);
              break;
           }
      case EOLIAN_METHOD:
           {
              printf("%*s<%s> %s\n", nb_spaces, "", ret_desc ? ret_desc : "", fid->name);
              const char *str = eolian_function_description_get(foo_id, EOLIAN_COMMENT);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_LEGACY);
              if (str) printf("%*slegacy: <%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_METHOD_RETURN_TYPE);
              if (str) printf("%*sreturn type: <%s>\n", nb_spaces + 5, "", str);
              if (fid->obj_is_const) printf("%*sobj const: <true>\n", nb_spaces + 5, "");
              break;
           }
      case EOLIAN_CTOR:
           {
              //char *str = eina_hash_find(fid->data, "comment");
              const char *str = eolian_function_description_get(foo_id, EOLIAN_COMMENT);
              if (str) printf("%*s<%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_LEGACY);
              if (str) printf("%*slegacy: <%s>\n", nb_spaces + 5, "", str);
              str = eolian_function_data_get(foo_id, EOLIAN_METHOD_RETURN_TYPE);
              if (str) printf("%*sreturn type: <%s>\n", nb_spaces + 5, "", str);
              break;
           }
      default:
         return EINA_FALSE;
     }
   Eina_List *itr;
   _Parameter_Desc *param;
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
         Eina_Strbuf *type_buf = eina_strbuf_new();
         _type_print(param->type, type_buf);
         printf("%*s%s <%s> <%s> <%s>\n", nb_spaces + 5, "",
               param_dir, param->name,
               eina_strbuf_string_get(type_buf),
               param->description?param->description:"");
         eina_strbuf_free(type_buf);
     }
   return EINA_TRUE;
}

static Eina_Bool
_class_print(const Eolian_Class class)
{
   Eina_List *itr;
   _Function_Id *function;
   const char *types[5] = {"", "Regular", "Regular Non Instantiable", "Mixin", "Interface"};

   _Class_Desc *cl = (_Class_Desc *)class;
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
   Eolian_Implement impl;
   EINA_LIST_FOREACH(cl->implements, itr, impl)
     {
        _implements_print(impl, 4);
     }
   printf("\n");
   // Implement
   printf("  events:\n");
   Eolian_Event ev;
   EINA_LIST_FOREACH(cl->events, itr, ev)
     {
        _event_print(ev, 4);
     }
   printf("\n");
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_show(const Eolian_Class class)
{
   if (!class)
     {
        Eina_List *itr;
        Eolian_Class cl;
        EINA_LIST_FOREACH(_classes, itr, cl)
          _class_print(cl);
     }
   else
     {
        _class_print(class);
     }
   return EINA_TRUE;
}

#define EO_SUFFIX ".eo"
#define EOT_SUFFIX ".eot"
#define EO_SCAN_BODY(suffix, hash) \
if (eina_str_has_suffix(file, suffix)) \
  { \
     int len = strlen(file); \
     int idx = len - 1; \
     while (idx >= 0 && file[idx] != '/' && file[idx] != '\\') idx--; \
     eina_hash_add(hash, eina_stringshare_add_length(file+idx+1, len - idx - sizeof(suffix)), strdup(file)); \
  }

EAPI Eina_Bool
eolian_directory_scan(const char *dir)
{
   if (!dir) return EINA_FALSE;
   char *file;
   /* Get all files from directory. Not recursively!!! */
   Eina_Iterator *dir_files = eina_file_ls(dir);
   EINA_ITERATOR_FOREACH(dir_files, file) EO_SCAN_BODY(EO_SUFFIX, _filenames);
   eina_iterator_free(dir_files);
   dir_files = eina_file_ls(dir);
   EINA_ITERATOR_FOREACH(dir_files, file) EO_SCAN_BODY(EOT_SUFFIX, _tfilenames);
   eina_iterator_free(dir_files);
   return EINA_TRUE;
}

static char *
_eolian_class_to_filename(const char *filename)
{
   char *ret;
   Eina_Strbuf *strbuf = eina_strbuf_new();
   eina_strbuf_append(strbuf, filename);
   eina_strbuf_replace_all(strbuf, ".", "_");

   ret = eina_strbuf_string_steal(strbuf);
   eina_strbuf_free(strbuf);

   eina_str_tolower(&ret);

   return ret;
}

EAPI Eina_Bool
eolian_eot_file_parse(const char *filepath)
{
   return eo_parser_database_fill(filepath, EINA_TRUE);
}

EAPI Eina_Bool
eolian_eo_file_parse(const char *filepath)
{
   const Eina_List *itr;
   Eolian_Class class = eolian_class_find_by_file(filepath);
   const char *inherit_name;
   Eolian_Implement impl;
   if (!class)
     {
        if (!eo_parser_database_fill(filepath, EINA_FALSE)) return EINA_FALSE;
        class = eolian_class_find_by_file(filepath);
        if (!class)
          {
             ERR("No class for file %s", filepath);
             return EINA_FALSE;
          }
     }
   EINA_LIST_FOREACH(eolian_class_inherits_list_get(class), itr, inherit_name)
     {
        if (!eolian_class_find_by_name(inherit_name))
          {
             char *filename = _eolian_class_to_filename(inherit_name);
             filepath = eina_hash_find(_filenames, filename);
             if (!filepath)
               {
                  ERR("Unable to find a file for class %s", inherit_name);
                  return EINA_FALSE;
               }
             if (!eolian_eo_file_parse(filepath)) return EINA_FALSE;
             if (!eolian_class_find_by_name(inherit_name))
               {
                  ERR("Unable to find class %s", inherit_name);
                  return EINA_FALSE;
               }
             free(filename);
          }
     }
   EINA_LIST_FOREACH(eolian_class_implements_list_get(class), itr, impl)
     {
        Eolian_Class impl_class;
        Eolian_Function impl_func;
        Eolian_Function_Type impl_type = EOLIAN_UNRESOLVED;
        eolian_implement_information_get(impl, &impl_class, &impl_func, &impl_type);
        if (!impl_func)
          {
             ERR("Unable to find function %s", eolian_implement_full_name_get(impl));
             return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}

static Eina_Bool _tfile_parse(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Eina_Bool *ret = fdata;
   if (*ret) *ret = eolian_eot_file_parse(data);
   return *ret;
}

EAPI Eina_Bool
eolian_all_eot_files_parse()
{
   Eina_Bool ret = EINA_TRUE;
   eina_hash_foreach(_tfilenames, _tfile_parse, &ret);
   return ret;
}

static Eina_Bool _file_parse(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Eina_Bool *ret = fdata;
   if (*ret) *ret = eolian_eo_file_parse(data);
   return *ret;
}

EAPI Eina_Bool
eolian_all_eo_files_parse()
{
   Eina_Bool ret = EINA_TRUE;
   eina_hash_foreach(_filenames, _file_parse, &ret);
   return ret;
}

