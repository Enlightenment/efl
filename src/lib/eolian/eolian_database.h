#ifndef __EOLIAN_DATABASE_H
#define __EOLIAN_DATABASE_H

#include <Eolian.h>

extern int _eolian_log_dom;

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

int database_init();
int database_shutdown();

/* Add a type in the database */
Eina_Bool database_type_add(const char *alias, Eolian_Type type);

/* Add a class in the database */
Eolian_Class database_class_add(const char *class_name, Eolian_Class_Type type);

/* Add a class from the database */
Eina_Bool
database_class_del(Eolian_Class class);

/* Add an inherit class name to a class */
Eina_Bool database_class_inherit_add(Eolian_Class class, const char *inherit_class_name);

/* Set description of class. */
void
database_class_description_set(Eolian_Class class, const char *description);

/* Set legacy of class. */
void
database_class_legacy_prefix_set(Eolian_Class class, const char *legacy_prefix);

/* Set eo prefix of class. */
void
database_class_eo_prefix_set(Eolian_Class class, const char *eo_prefix);

/* Set data of class. */
void
database_class_data_type_set(Eolian_Class class, const char *data_type);

/* Set file of class */
Eina_Bool
database_class_file_set(Eolian_Class class, const char *file_name);

/* Create a function */
Eolian_Function database_function_new(const char *function_name, Eolian_Function_Type foo_type);

/* Set a type to a function */
void database_function_type_set(Eolian_Function function_id, Eolian_Function_Type foo_type);

/* Add a function to a class */
Eina_Bool database_class_function_add(Eolian_Class class, Eolian_Function foo_id);

void database_function_data_set(Eolian_Function function_id, const char *key, const char *description);

/* Add a description to a function */
#define database_function_description_set(foo_id, key, desc) database_function_data_set((foo_id), (key), (desc))

/* Add a key to a property */
Eolian_Function_Parameter database_property_key_add(Eolian_Function foo_id, Eolian_Type type, const char *name, const char *description);

/* Add a value to a property */
Eolian_Function_Parameter database_property_value_add(Eolian_Function foo_id, Eolian_Type type, const char *name, const char *description);

/* Add a parameter to a method */
Eolian_Function_Parameter database_method_parameter_add(Eolian_Function foo_id, Eolian_Parameter_Dir param_dir, Eolian_Type type, const char *name, const char *description);

Eolian_Type database_type_append(Eolian_Type type, const char *name, Eina_Bool own);

void database_type_del(Eolian_Type type);

void database_parameter_const_attribute_set(Eolian_Function_Parameter param_desc, Eina_Bool is_get, Eina_Bool is_const);

void database_parameter_nonull_set(Eolian_Function_Parameter, Eina_Bool nonull);

void database_function_return_type_set(Eolian_Function foo_id, Eolian_Function_Type ftype, Eolian_Type ret_type);

void database_function_return_comment_set(Eolian_Function foo_id, Eolian_Function_Type ftype, const char *ret_comment);

void database_function_return_dflt_val_set(Eolian_Function foo_id, Eolian_Function_Type ftype, const char *ret_dflt_value);

void database_function_return_flag_set_as_warn_unused(Eolian_Function foo_id,
      Eolian_Function_Type ftype, Eina_Bool warn_unused);

void database_function_object_set_as_const(Eolian_Function foo_id, Eina_Bool is_const);

Eina_Bool
database_function_set_as_virtual_pure(Eolian_Function function_id, Eolian_Function_Type type);

void
database_function_scope_set(Eolian_Function function_id, Eolian_Function_Scope scope);

/* Need to add API for callbacks and implements */

Eolian_Implement
database_implement_new(const char *impl_name);

Eina_Bool
database_class_implement_add(Eolian_Class class, Eolian_Implement impl_id);

Eina_Bool
database_class_ctor_enable_set(Eolian_Class class, Eina_Bool enable);

Eina_Bool
database_class_dtor_enable_set(Eolian_Class class, Eina_Bool enable);

Eolian_Event
database_event_new(const char *event_name, const char *event_type, const char *event_desc);

void
database_event_free(Eolian_Event event);

Eina_Bool
database_class_event_add(Eolian_Class class, Eolian_Event event_desc);

#endif
