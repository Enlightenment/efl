#ifndef __EOLIAN_DATABASE_H
#define __EOLIAN_DATABASE_H

#include <Eolian.h>

extern int _eolian_log_dom;
extern Eina_Prefix *_eolian_prefix;

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_eolian_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eolian_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eolian_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eolian_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eolian_log_dom, __VA_ARGS__)

#define PROP_GET_RETURN_DEFAULT_VAL "property_get_return_default_val"
#define PROP_SET_RETURN_DEFAULT_VAL "property_set_return_default_val"
#define METHOD_RETURN_DEFAULT_VAL "method_return_default_val"

#define EOLIAN_METHOD_RETURN_COMMENT "method_return_comment"
#define EOLIAN_PROP_GET_RETURN_COMMENT "property_get_return_comment"
#define EOLIAN_PROP_SET_RETURN_COMMENT "property_set_return_comment"

extern Eina_List *_classes;
extern Eina_Hash *_aliases;
extern Eina_Hash *_structs;
extern Eina_Hash *_filenames; /* Hash: filename without extension -> full path */
extern Eina_Hash *_tfilenames;

struct _Eolian_Class
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
   Eina_List *properties; /* List prop_name -> Eolian_Function */
   Eina_List *methods; /* List meth_name -> Eolian_Function */
   Eina_List *constructors; /* List constructor_name -> Eolian_Function */
   Eina_List *implements; /* List implements name -> Eolian_Implement */
   Eina_List *events; /* List event_name -> Eolian_Event */
   Eina_Bool class_ctor_enable:1;
   Eina_Bool class_dtor_enable:1;
};

struct _Eolian_Function
{
   Eina_Stringshare *name;
   Eina_List *keys; /* list of Eolian_Function_Parameter */
   Eina_List *params; /* list of Eolian_Function_Parameter */
   Eolian_Function_Type type;
   Eolian_Function_Scope scope;
   Eolian_Type *get_ret_type;
   Eolian_Type *set_ret_type;
   Eina_Hash *data;
   Eina_Bool obj_is_const :1; /* True if the object has to be const. Useful for a few methods. */
   Eina_Bool get_virtual_pure :1;
   Eina_Bool set_virtual_pure :1;
   Eina_Bool get_return_warn_unused :1; /* also used for methods */
   Eina_Bool set_return_warn_unused :1;
};

struct _Eolian_Function_Parameter
{
   Eina_Stringshare *name;
   Eolian_Type *type;
   Eina_Stringshare *description;
   Eolian_Parameter_Dir param_dir;
   Eina_Bool is_const_on_get :1; /* True if const in this the get property */
   Eina_Bool is_const_on_set :1; /* True if const in this the set property */
   Eina_Bool nonull :1; /* True if this argument cannot be NULL */
};

/* maps directly to Eo_Type_Def */

struct _Eolian_Type
{
   Eolian_Type_Type type;
   union {
      /* functions */
      struct {
         Eina_List   *arguments;
         Eolian_Type *ret_type;
      };
      /* everything else */
      struct {
         Eina_List   *subtypes;
         Eolian_Type *base_type;
         Eina_Stringshare *name;
         Eina_Stringshare *full_name;
         Eina_List        *namespaces;
         Eina_Hash        *fields;
         Eina_Stringshare *comment;
         Eina_Stringshare *file;
      };
   };
   Eina_Bool is_const  :1;
   Eina_Bool is_own    :1;
   Eina_Bool is_extern :1;
};

struct _Eolian_Implement
{
   Eina_Stringshare *full_name;
};

struct _Eolian_Event
{
   Eina_Stringshare *name;
   Eina_Stringshare *comment;
   Eolian_Type *type;
};

typedef struct _Eolian_Struct_Field
{
   Eolian_Type      *type;
   Eina_Stringshare *comment;
} Eolian_Struct_Field;

int database_init();
int database_shutdown();

/* types */

Eina_Bool database_type_add(Eolian_Type *def);
Eina_Bool database_struct_add(Eolian_Type *tp);
void database_type_del(Eolian_Type *tp);
void database_typedef_del(Eolian_Type *tp);

void database_type_print(Eolian_Type *type);
void database_type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name);

/* classes */

Eolian_Class *database_class_add(const char *class_name, Eolian_Class_Type type);
void database_class_del(Eolian_Class *cl);

Eina_Bool database_class_inherit_add(Eolian_Class *cl, const char *inherit_class_name);
Eina_Bool database_class_function_add(Eolian_Class *cl, Eolian_Function *foo_id);
Eina_Bool database_class_implement_add(Eolian_Class *cl, Eolian_Implement *impl_id);
Eina_Bool database_class_event_add(Eolian_Class *cl, Eolian_Event *event_desc);

void database_class_description_set(Eolian_Class *cl, const char *description);
void database_class_legacy_prefix_set(Eolian_Class *cl, const char *legacy_prefix);
void database_class_eo_prefix_set(Eolian_Class *cl, const char *eo_prefix);
void database_class_data_type_set(Eolian_Class *cl, const char *data_type);
void database_class_file_set(Eolian_Class *cl, const char *file_name);
Eina_Bool database_class_ctor_enable_set(Eolian_Class *cl, Eina_Bool enable);
Eina_Bool database_class_dtor_enable_set(Eolian_Class *cl, Eina_Bool enable);

Eina_Bool database_class_name_validate(const char *class_name, const Eolian_Class **cl);

/* functions */

Eolian_Function *database_function_new(const char *function_name, Eolian_Function_Type foo_type);
void database_function_del(Eolian_Function *fid);

void database_function_type_set(Eolian_Function *function_id, Eolian_Function_Type foo_type);
void database_function_data_set(Eolian_Function *function_id, const char *key, const char *description);
#define database_function_description_set(foo_id, key, desc) database_function_data_set((foo_id), (key), (desc))

void database_function_return_type_set(Eolian_Function *foo_id, Eolian_Function_Type ftype, Eolian_Type *ret_type);
void database_function_return_comment_set(Eolian_Function *foo_id, Eolian_Function_Type ftype, const char *ret_comment);
void database_function_return_default_val_set(Eolian_Function *foo_id, Eolian_Function_Type ftype, const char *ret_default_value);
void database_function_return_flag_set_as_warn_unused(Eolian_Function *foo_id, Eolian_Function_Type ftype, Eina_Bool warn_unused);

void database_function_object_set_as_const(Eolian_Function *foo_id, Eina_Bool is_const);
Eina_Bool database_function_set_as_virtual_pure(Eolian_Function *function_id, Eolian_Function_Type type);
void database_function_scope_set(Eolian_Function *function_id, Eolian_Function_Scope scope);

Eolian_Function_Parameter *database_property_key_add(Eolian_Function *foo_id, Eolian_Type *type, const char *name, const char *description);
Eolian_Function_Parameter *database_property_value_add(Eolian_Function *foo_id, Eolian_Type *type, const char *name, const char *description);
Eolian_Function_Parameter *database_method_parameter_add(Eolian_Function *foo_id, Eolian_Parameter_Dir param_dir, Eolian_Type *type, const char *name, const char *description);

/* func parameters */

Eolian_Function_Parameter *database_parameter_add(Eolian_Type *type, const char *name, const char *description);
void database_parameter_del(Eolian_Function_Parameter *pdesc);

void database_parameter_const_attribute_set(Eolian_Function_Parameter *param, Eina_Bool is_get, Eina_Bool is_const);
void database_parameter_nonull_set(Eolian_Function_Parameter *param, Eina_Bool nonull);

/* implements */

void database_implement_del(Eolian_Implement *impl);

/* events */

Eolian_Event *database_event_new(const char *event_name, const char *event_type, const char *event_desc);
void database_event_del(Eolian_Event *event);


#endif
