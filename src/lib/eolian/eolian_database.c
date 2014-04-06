#include <Eina.h>
#include "eolian_database.h"

#define PROP_GET_RETURN_DFLT_VAL "property_get_return_dflt_val"
#define PROP_SET_RETURN_DFLT_VAL "property_set_return_dflt_val"
#define METHOD_RETURN_DFLT_VAL "method_return_dflt_val"

static Eina_Hash *_classes = NULL;
static int _database_init_count = 0;

typedef struct
{
   Eina_Stringshare *name;
   Eina_Stringshare *file;
   Eolian_Class_Type type;
   Eina_Stringshare *description;
   Eina_Stringshare *legacy_prefix;
   Eina_Stringshare *eo_prefix;
   Eina_Stringshare *data_type;
   Eolian_Function dflt_ctor;
   Eolian_Function dflt_dtor;
   Eina_List *inherits; /* List Eina_Stringshare * */
   Eina_List *properties; /* List prop_name -> _Function_Id */
   Eina_List *methods; /* List meth_name -> _Function_Id */
   Eina_List *constructors; /* List constructor_name -> _Function_Id */
   Eina_List *destructors; /* List destructor_name -> _Function_Id */
   Eina_List *implements; /* List implements name -> _Implement_Desc */
   Eina_List *events; /* List event_name -> _Event_Desc */
   Eina_Bool class_ctor_enable:1;
   Eina_Bool class_dtor_enable:1;
} Class_desc;

typedef struct
{
   Eina_Stringshare *name;
   Eina_List *keys; /* list of _Parameter_Desc */
   Eina_List *params; /* list of _Parameter_Desc */
   Eolian_Function_Type type;
   Eina_Hash *data;
   Eina_Bool obj_is_const :1; /* True if the object has to be const. Useful for a few methods. */
   Eina_Bool get_virtual_pure :1;
   Eina_Bool set_virtual_pure :1;
   Eina_Bool get_return_warn_unused :1; /* also used for methods */
   Eina_Bool set_return_warn_unused :1;
   Eina_Bool get_return_own :1; /* also used for methods */
   Eina_Bool set_return_own :1;
} _Function_Id;

typedef struct
{
   Eina_Stringshare *name;
   Eina_Stringshare *type;
   Eina_Stringshare *description;
   Eolian_Parameter_Dir param_dir;
   Eina_Bool is_const_on_get :1; /* True if const in this the get property */
   Eina_Bool is_const_on_set :1; /* True if const in this the set property */
   Eina_Bool nonull :1; /* True if this argument cannot be NULL */
   Eina_Bool own :1; /* True if the ownership of this argument passes to the caller/callee */
} _Parameter_Desc;

typedef struct
{
   Eina_Stringshare *eo_param;
   Eina_Stringshare *leg_param;
   Eina_Stringshare *comment;
} _Implement_Legacy_Param;

typedef struct
{
   Eina_Stringshare *legacy_function_name;
   Eina_List *params; /* List of Eolian_Implement_Legacy_Parameter */
   Eina_Stringshare *ret_type;
   Eina_Stringshare *ret_value;
} _Implement_Legacy_Desc;

typedef struct
{
   Eina_Stringshare *class_name;
   Eina_Stringshare *func_name;
   Eolian_Function_Type type;
   Eolian_Implement_Legacy leg_desc;
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
   eina_stringshare_del(pdesc->type);
   eina_stringshare_del(pdesc->description);
   free(pdesc);
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
   free(fid);
}

static void
_class_del(Class_desc *class)
{
   Eina_Stringshare *inherit_name;
   Eina_List *inherits = class->inherits;
   EINA_LIST_FREE(inherits, inherit_name)
      eina_stringshare_del(inherit_name);

   _Implement_Desc *impl;
   Eina_List *implements = class->implements;
   EINA_LIST_FREE(implements, impl)
     {
        eina_stringshare_del(impl->class_name);
        eina_stringshare_del(impl->func_name);
        free(impl);
     }

   _Function_Id *fid;
   Eolian_Event ev;
   EINA_LIST_FREE(class->constructors, fid) _fid_del(fid);
   EINA_LIST_FREE(class->destructors, fid) _fid_del(fid);
   EINA_LIST_FREE(class->methods, fid) _fid_del(fid);
   EINA_LIST_FREE(class->properties, fid) _fid_del(fid);
   EINA_LIST_FREE(class->events, ev) database_event_free(ev);
   _fid_del((_Function_Id *)class->dflt_ctor);
   _fid_del((_Function_Id *)class->dflt_dtor);

   eina_stringshare_del(class->name);
   eina_stringshare_del(class->file);
   eina_stringshare_del(class->description);
   eina_stringshare_del(class->legacy_prefix);
   eina_stringshare_del(class->eo_prefix);
   eina_stringshare_del(class->data_type);
   free(class);
}

void _hash_free_cb(void *data)
{
   Class_desc *cl = data;
   _class_del(cl);
}

static Class_desc *
_class_get(const char *class_name)
{
   Eina_Stringshare *shr = eina_stringshare_add(class_name);
   Class_desc *cl = eina_hash_find(_classes, shr);
   eina_stringshare_del(shr);
   EINA_SAFETY_ON_NULL_RETURN_VAL(class_name, NULL);
   return cl;
}

int
database_init()
{
   if (_database_init_count > 0) return ++_database_init_count;
   eina_init();
   if (!_classes)
      _classes = eina_hash_stringshared_new(_hash_free_cb);
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
        eina_hash_free(_classes);
        eina_shutdown();
     }
   return _database_init_count;
}

Eina_Bool
database_class_add(const char *class_name, Eolian_Class_Type type)
{
   if (_classes)
     {
        Class_desc *desc = calloc(1, sizeof(*desc));
        desc->name = eina_stringshare_add(class_name);
        desc->type = type;
        eina_hash_set(_classes, desc->name, desc);
     }
   return EINA_TRUE;
}

Eina_Bool
database_class_file_set(const char *class_name, const char *file_name)
{
   Class_desc *cl = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->file = eina_stringshare_add(file_name);
   return EINA_TRUE;
}

EAPI const char *
eolian_class_find_by_file(const char *file_name)
{
   const Eina_List *names_list = eolian_class_names_list_get();
   const Eina_List *itr;
   const char *class_name;
   Eina_Stringshare *shr_file = eina_stringshare_add(file_name);
   EINA_LIST_FOREACH(names_list, itr, class_name)
     {
        Class_desc *cl = _class_get(class_name);
        if (cl->file == shr_file)
          {
             eina_stringshare_del(shr_file);
             return class_name;
          }
     }
   eina_stringshare_del(shr_file);
   return NULL;
}

EAPI Eolian_Class_Type
eolian_class_type_get(const char *class_name)
{
   Class_desc *cl = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EOLIAN_CLASS_UNKNOWN_TYPE);
   return cl->type;
}

Eina_Bool
database_class_del(const char *class_name)
{
   Class_desc *cl = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   eina_hash_del(_classes, class_name, NULL);
   _class_del(cl);
   return EINA_TRUE;
}

static Eina_Bool _class_name_get(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Class_desc *desc = data;
   Eina_List **list = fdata;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(desc && list , EINA_FALSE);
   *list = eina_list_append(*list, desc->name);
   return EINA_TRUE;
}

EAPI const Eina_List *
eolian_class_names_list_get(void)
{
   Eina_List *list = NULL;
   eina_hash_foreach(_classes, _class_name_get, &list);
   return list;
}

EAPI Eina_Bool
eolian_class_exists(const char *class_name)
{
   return !!_class_get(class_name);
}

Eina_Bool
database_class_inherit_add(const char *class_name, const char *inherit_class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   desc->inherits = eina_list_append(desc->inherits, eina_stringshare_add(inherit_class_name));
   return EINA_TRUE;
}

EAPI const char *
eolian_class_description_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   return desc->description;
}

void
database_class_description_set(const char *class_name, const char *description)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN(desc);
   desc->description = eina_stringshare_add(description);
}

EAPI const char*
eolian_class_legacy_prefix_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   return desc->legacy_prefix;
}

void
database_class_legacy_prefix_set(const char *class_name, const char *legacy_prefix)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN(desc);
   desc->legacy_prefix = eina_stringshare_add(legacy_prefix);
}

EAPI const char*
eolian_class_eo_prefix_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   return desc->eo_prefix;
}

void
database_class_eo_prefix_set(const char *class_name, const char *eo_prefix)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN(desc);
   desc->eo_prefix = eina_stringshare_add(eo_prefix);
}

EAPI const char*
eolian_class_data_type_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   return desc->data_type;
}

void
database_class_data_type_set(const char *class_name, const char *data_type)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN(desc);
   desc->data_type= eina_stringshare_add(data_type);
}

EAPI const Eina_List *
eolian_class_inherits_list_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   return desc->inherits;
}

EAPI const Eina_List*
eolian_class_implements_list_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   return desc->implements;
}

Eolian_Function
database_function_new(const char *function_name, Eolian_Function_Type foo_type)
{
   _Function_Id *fid = calloc(1, sizeof(*fid));
   fid->name = eina_stringshare_add(function_name);
   fid->type = foo_type;
   fid->data  = eina_hash_string_superfast_new(free);
   return (Eolian_Function) fid;
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

Eina_Bool database_class_function_add(const char *class_name, Eolian_Function foo_id)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(foo_id && desc, EINA_FALSE);
   _Function_Id *fid = (_Function_Id *) foo_id;
   switch (fid->type)
     {
      case EOLIAN_PROPERTY:
      case EOLIAN_PROP_SET:
      case EOLIAN_PROP_GET:
         desc->properties = eina_list_append(desc->properties, foo_id);
         break;
      case EOLIAN_METHOD:
         desc->methods = eina_list_append(desc->methods, foo_id);
         break;
      case EOLIAN_CTOR:
         desc->constructors = eina_list_append(desc->constructors, foo_id);
         break;
      case EOLIAN_DFLT_CTOR:
         desc->dflt_ctor = foo_id;
         break;
      case EOLIAN_DTOR:
         desc->destructors = eina_list_append(desc->destructors, foo_id);
         break;
      case EOLIAN_DFLT_DTOR:
         desc->dflt_dtor = foo_id;
         break;
      default:
         ERR("Bad function type %d.", fid->type);
         return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eolian_Implement
database_implement_new(const char *class_name, const char *func_name, Eolian_Function_Type type)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(class_name && func_name, NULL);
   _Implement_Desc *impl_desc = calloc(1, sizeof(_Implement_Desc));
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl_desc, NULL);
   impl_desc->class_name = eina_stringshare_add(class_name);
   impl_desc->func_name = eina_stringshare_add(func_name);
   impl_desc->type = type;
   return (Eolian_Implement) impl_desc;
}

Eina_Bool
database_class_implement_add(const char *class_name, Eolian_Implement impl_desc)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl_desc, EINA_FALSE);
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   desc->implements = eina_list_append(desc->implements, impl_desc);
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_implement_information_get(Eolian_Implement impl, const char **class_name, const char **func_name, Eolian_Function_Type *type)
{
   _Implement_Desc *_impl = (_Implement_Desc *)impl;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_impl, EINA_FALSE);
   if (class_name) *class_name = _impl->class_name;
   if (func_name) *func_name = _impl->func_name;
   if (type) *type = _impl->type;
   return EINA_TRUE;
}

Eolian_Implement_Legacy
database_implement_legacy_add(Eolian_Implement impl, const char *legacy_function_name)
{
   _Implement_Desc *_impl = (_Implement_Desc *)impl;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_impl, NULL);
   _impl->leg_desc = calloc(1, sizeof(_Implement_Legacy_Desc));
   _Implement_Legacy_Desc *leg_desc = (_Implement_Legacy_Desc *)_impl->leg_desc;
   if (leg_desc && legacy_function_name) leg_desc->legacy_function_name = legacy_function_name;
   return _impl->leg_desc;
}

Eolian_Implement_Legacy_Parameter
database_implement_legacy_param_add(Eolian_Implement_Legacy leg,
      Eina_Stringshare *eo_param, Eina_Stringshare *leg_param,
      Eina_Stringshare *comment)
{
   _Implement_Legacy_Desc *_leg = (_Implement_Legacy_Desc *)leg;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_leg, NULL);

   _Implement_Legacy_Param *param = calloc(1, sizeof(_Implement_Legacy_Param));
   param->eo_param = eo_param;
   param->leg_param = leg_param;
   param->comment = comment;

   _leg->params = eina_list_append(_leg->params, param);
   return (Eolian_Implement_Legacy_Parameter) param;
}

EAPI Eina_Bool
eolian_implement_legacy_param_info_get(Eolian_Implement_Legacy_Parameter param,
      Eina_Stringshare **eo_param, Eina_Stringshare **leg_param,
      Eina_Stringshare **comment)
{
   _Implement_Legacy_Param *_param = (_Implement_Legacy_Param *)param;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_param, EINA_FALSE);

   if (eo_param) *eo_param = _param->eo_param;
   if (leg_param) *leg_param = _param->leg_param;
   if (comment) *comment = _param->comment;
   return EINA_TRUE;
}

Eina_Bool
database_implement_legacy_return_add(Eolian_Implement_Legacy leg, Eina_Stringshare *ret_type, Eina_Stringshare *ret_value)
{
   _Implement_Legacy_Desc *_leg = (_Implement_Legacy_Desc *)leg;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_leg, EINA_FALSE);
   _leg->ret_type = ret_type;
   _leg->ret_value = ret_value;
   return EINA_TRUE;
}

EAPI Eolian_Implement_Legacy
eolian_implement_legacy_desc_get(Eolian_Implement impl)
{
   _Implement_Desc *_impl = (_Implement_Desc *)impl;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_impl, NULL);
   return (Eolian_Implement_Legacy) (_impl->leg_desc);
}

EAPI Eina_Bool
eolian_implement_legacy_information_get(const Eolian_Implement_Legacy leg_desc,
      Eina_Stringshare **leg_func_name, Eina_List **params,
      Eina_Stringshare **ret_type, Eina_Stringshare **ret_value)
{
   _Implement_Legacy_Desc *_leg = (_Implement_Legacy_Desc *)leg_desc;
   EINA_SAFETY_ON_NULL_RETURN_VAL(_leg, EINA_FALSE);
   if (leg_func_name) *leg_func_name = _leg->legacy_function_name;
   if (params) *params = _leg->params;
   if (ret_type) *ret_type = _leg->ret_type;
   if (ret_value) *ret_value = _leg->ret_value;
   return EINA_TRUE;
}

EAPI Eolian_Function
eolian_class_function_find_by_name(const char *class_name, const char *func_name, Eolian_Function_Type f_type)
{
   Eina_List *itr;
   Eolian_Function foo_id;
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);

   if (f_type == EOLIAN_UNRESOLVED || f_type == EOLIAN_METHOD)
      EINA_LIST_FOREACH(desc->methods, itr, foo_id)
        {
           _Function_Id *fid = (_Function_Id *) foo_id;
           if (!strcmp(fid->name, func_name))
              return foo_id;
        }

   if (f_type == EOLIAN_UNRESOLVED || f_type == EOLIAN_PROPERTY ||
         f_type == EOLIAN_PROP_SET || f_type == EOLIAN_PROP_GET)
     {
        EINA_LIST_FOREACH(desc->properties, itr, foo_id)
          {
             _Function_Id *fid = (_Function_Id *) foo_id;
             if (!strcmp(fid->name, func_name))
                return foo_id;
          }
     }

   if (f_type == EOLIAN_CTOR)
     {
        EINA_LIST_FOREACH(desc->constructors, itr, foo_id)
          {
             _Function_Id *fid = (_Function_Id *) foo_id;
             if (!strcmp(fid->name, func_name))
                return foo_id;
          }
     }

   if (f_type == EOLIAN_DTOR)
     {
        EINA_LIST_FOREACH(desc->destructors, itr, foo_id)
          {
             _Function_Id *fid = (_Function_Id *) foo_id;
             if (!strcmp(fid->name, func_name))
                return foo_id;
          }
     }

   return NULL;
}

EAPI const Eina_List *
eolian_class_functions_list_get(const char *class_name, Eolian_Function_Type foo_type)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   switch (foo_type)
     {
      case EOLIAN_PROPERTY:
         return desc->properties;
      case EOLIAN_METHOD:
         return desc->methods;
      case EOLIAN_CTOR:
         return desc->constructors;
      case EOLIAN_DTOR:
         return desc->destructors;
      default: return NULL;
     }
}

EAPI Eolian_Function
eolian_class_default_constructor_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   return desc->dflt_ctor;
}

EAPI Eolian_Function
eolian_class_default_destructor_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   return desc->dflt_dtor;
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
_parameter_new(const char *type, const char *name, const char *description)
{
   _Parameter_Desc *param = NULL;
   param = calloc(1, sizeof(*param));
   param->name = eina_stringshare_add(name);
   param->type = eina_stringshare_add(type);
   param->description = eina_stringshare_add(description);
   return param;
}

Eolian_Function_Parameter
database_property_key_add(Eolian_Function foo_id, const char *type, const char *name, const char *description)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   _Parameter_Desc *param = _parameter_new(type, name, description);
   fid->keys = eina_list_append(fid->keys, param);
   return (Eolian_Function_Parameter)param;
}

Eolian_Function_Parameter
database_property_value_add(Eolian_Function foo_id, const char *type, const char *name, const char *description)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   _Parameter_Desc *param = _parameter_new(type, name, description);
   fid->params = eina_list_append(fid->params, param);
   return (Eolian_Function_Parameter)param;
}

Eolian_Function_Parameter
database_method_parameter_add(Eolian_Function foo_id, Eolian_Parameter_Dir param_dir, const char *type, const char *name, const char *description)
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

EAPI Eina_Stringshare*
eolian_parameter_type_get(const Eolian_Function_Parameter param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   eina_stringshare_ref(((_Parameter_Desc*)param)->type);
   return ((_Parameter_Desc*)param)->type;
}

EAPI Eina_Stringshare*
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
eolian_parameter_information_get(Eolian_Function_Parameter param_desc, Eolian_Parameter_Dir *param_dir, const char **type, const char **name, const char **description)
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

void
database_parameter_own_set(Eolian_Function_Parameter param_desc, Eina_Bool own)
{
   _Parameter_Desc *param = (_Parameter_Desc *)param_desc;
   EINA_SAFETY_ON_NULL_RETURN(param);
   param->own = own;
}

EAPI Eina_Bool
eolian_parameter_is_own(Eolian_Function_Parameter param_desc)
{
   _Parameter_Desc *param = (_Parameter_Desc *)param_desc;
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EINA_FALSE);
   return param->own;
}

void database_function_return_type_set(Eolian_Function foo_id, Eolian_Function_Type ftype, const char *ret_type)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = EOLIAN_PROP_SET_RETURN_TYPE; break;
      case EOLIAN_PROP_GET: key = EOLIAN_PROP_GET_RETURN_TYPE; break;
      case EOLIAN_METHOD: key = EOLIAN_METHOD_RETURN_TYPE; break;
      default: return;
     }
   database_function_data_set(foo_id, key, ret_type);
}

EAPI const char *
eolian_function_return_type_get(Eolian_Function foo_id, Eolian_Function_Type ftype)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = EOLIAN_PROP_SET_RETURN_TYPE; break;
      case EOLIAN_PROP_GET: key = EOLIAN_PROP_GET_RETURN_TYPE; break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: key = EOLIAN_METHOD_RETURN_TYPE; break;
      default: return NULL;
     }
   return eolian_function_data_get(foo_id, key);
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
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: key = EOLIAN_RETURN_COMMENT; break;
      default: return NULL;
     }
   return eolian_function_data_get(foo_id, key);
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

void database_function_return_flag_set_own(Eolian_Function foo_id,
      Eolian_Function_Type ftype, Eina_Bool own)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN(fid);
   switch (ftype)
     {
      case EOLIAN_METHOD: case EOLIAN_PROP_GET: fid->get_return_own = own; break;
      case EOLIAN_PROP_SET: fid->set_return_own = own; break;
      default: return;
     }
}

EAPI Eina_Bool
eolian_function_return_own_get(Eolian_Function foo_id,
      Eolian_Function_Type ftype)
{
   _Function_Id *fid = (_Function_Id *)foo_id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_METHOD: case EOLIAN_PROP_GET: return fid->get_return_own;
      case EOLIAN_PROP_SET: return fid->set_return_own;
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
database_class_event_add(const char *class_name, Eolian_Event event_desc)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(event_desc && desc, EINA_FALSE);
   desc->events = eina_list_append(desc->events, event_desc);
   return EINA_TRUE;
}

EAPI const Eina_List*
eolian_class_events_list_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, NULL);
   return desc->events;
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
database_class_ctor_enable_set(const char *class_name, Eina_Bool enable)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   desc->class_ctor_enable = enable;
   return EINA_TRUE;
}

Eina_Bool
database_class_dtor_enable_set(const char *class_name, Eina_Bool enable)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   desc->class_dtor_enable = enable;
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_class_ctor_enable_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   return desc->class_ctor_enable;
}

EAPI Eina_Bool
eolian_class_dtor_enable_get(const char *class_name)
{
   Class_desc *desc = _class_get(class_name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   return desc->class_dtor_enable;
}

static void
_implements_print(Eolian_Implement impl, int nb_spaces)
{
   const char *t, *cl, *fn;
   Eolian_Function_Type ft;

   eolian_implement_information_get(impl, &cl, &fn, &ft);
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
   printf("%*s <%s :: %s> <%s>\n", nb_spaces + 5, "", cl, fn, t);
   Eolian_Implement_Legacy leg_desc = eolian_implement_legacy_desc_get(impl);
   if (leg_desc)
     {
        Eina_Stringshare *func_name, *ret_type, *ret_value;
        Eina_List *params = NULL;
        eolian_implement_legacy_information_get(leg_desc, &func_name, &params, &ret_type, &ret_value);
        printf("%*s Legacy\n", nb_spaces + 8, "");
        if (func_name) printf("%*s Function name: %s\n", nb_spaces + 11, "", func_name);
        if (ret_type && ret_value)
           printf("%*s Return %s::%s\n", nb_spaces + 11, "", ret_type, ret_value);
        if (params)
          {
             printf("%*s Params:\n", nb_spaces + 11, "");
             Eina_List *itr;
             Eolian_Implement_Legacy_Parameter p;
             EINA_LIST_FOREACH(params, itr, p)
               {
                  Eina_Stringshare *eo_param, *leg_param, *comment;
                  eolian_implement_legacy_param_info_get(p, &eo_param, &leg_param, &comment);
                  printf("%*s %s -> %s <%s>\n", nb_spaces + 14, "", eo_param, leg_param, comment);
               }
          }
     }
}

static void
_event_print(Eolian_Event ev, int nb_spaces)
{
   const char *name, *comment, *type;

   eolian_class_event_information_get(ev, &name, &type, &comment);
   printf("%*s <%s> <%s> <%s>\n", nb_spaces + 5, "", name, type, comment);
}

static Eina_Bool _function_print(const _Function_Id *fid, int nb_spaces)
{
   Eolian_Function foo_id = (Eolian_Function) fid;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   const char *ret_desc = eolian_function_description_get(foo_id, EOLIAN_RETURN_COMMENT);
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
      case EOLIAN_DTOR:
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
         printf("%*s%s <%s> <%s> <%s>\n", nb_spaces + 5, "", param_dir, param->name, param->type, (param->description?param->description:""));
     }
   return EINA_TRUE;
}

static Eina_Bool _class_print(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   Eina_List *itr;
   _Function_Id *function;
   const char *types[5] = {"", "Regular", "Regular Non Instantiable", "Mixin", "Interface"};

   Class_desc *desc = data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   printf("Class %s:\n", desc->name);
   if (desc->description)
      printf("  description: <%s>\n", desc->description);

   printf("  type: %s\n", types[desc->type]);

   // Inherits
   if (desc->inherits)
     {
        printf("  inherits: ");
        char *word;
        EINA_LIST_FOREACH(desc->inherits, itr, word)
          {
             printf("%s ", word);
          }
        printf("\n");
     }

   // Legacy prefix
   if (desc->legacy_prefix)
     {
        printf("  legacy prefix: <%s>\n", desc->legacy_prefix);
     }

   // Eo prefix
   if (desc->eo_prefix)
     {
        printf("  Eo prefix: <%s>\n", desc->eo_prefix);
     }

   // Data type
   if (desc->data_type)
     {
        printf("  Data type: <%s>\n", desc->data_type);
     }

   // Default constructor
   if (desc->dflt_ctor)
     {
        printf("  default constructor: present - description <%s>\n", eolian_function_description_get(desc->dflt_ctor, "comment"));
     }

   // Default destructor
   if (desc->dflt_dtor)
     {
        printf("  default destructor: present - description <%s>\n", eolian_function_description_get(desc->dflt_dtor, "comment"));
     }

   // Constructors
   printf("  constructors:\n");
   EINA_LIST_FOREACH(desc->constructors, itr, function)
     {
        _function_print(function, 4);
     }
   printf("\n");

   // Properties
   printf("  properties:\n");
   EINA_LIST_FOREACH(desc->properties, itr, function)
     {
        _function_print(function, 4);
     }
   printf("\n");

   // Methods
   printf("  methods:\n");
   EINA_LIST_FOREACH(desc->methods, itr, function)
     {
        _function_print(function, 4);
     }
   // Implement
   printf("  implements:\n");
   Eolian_Implement impl;
   EINA_LIST_FOREACH((Eina_List *) eolian_class_implements_list_get(desc->name), itr, impl)
     {
        _implements_print(impl, 4);
     }
   printf("\n");
   // Implement
   printf("  events:\n");
   Eolian_Event ev;
   EINA_LIST_FOREACH((Eina_List *) eolian_class_events_list_get(desc->name), itr, ev)
     {
        _event_print(ev, 4);
     }
   printf("\n");
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_show(const char *class_name)
{
   if (!class_name)
      eina_hash_foreach(_classes, _class_print, NULL);
   else
     {
        Class_desc *klass = _class_get(class_name);
        _class_print(NULL, NULL, klass, NULL);
     }
   return EINA_TRUE;
}
