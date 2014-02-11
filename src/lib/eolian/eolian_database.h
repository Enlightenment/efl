#ifndef __EOLIAN_DATABASE_H
#define __EOLIAN_DATABASE_H

#include <Eolian.h>

Eina_Bool database_init();
Eina_Bool database_shutdown();

/* Add a class in the database */
Eina_Bool database_class_add(const char *class_name, Eolian_Class_Type type);

/* Add a class from the database */
Eina_Bool
database_class_del(const char *class_name);

/* Add an inherit class name to a class */
Eina_Bool database_class_inherit_add(const char *class_name, const char *inherit_class_name);

/* Set description of class. */
void
database_class_description_set(const char *class_name, const char *description);

/* Set legacy of class. */
void
database_class_legacy_prefix_set(const char *class_name, const char *legacy_prefix);

/* Create a function */
Eolian_Function database_function_new(const char *function_name, Eolian_Function_Type foo_type);

/* Set a type to a function */
void database_function_type_set(Eolian_Function function_id, Eolian_Function_Type foo_type);

/* Add a function to a class */
Eina_Bool database_class_function_add(const char *classname, Eolian_Function foo_id);

void database_function_data_set(Eolian_Function function_id, const char *key, const char *description);

/* Add a description to a function */
#define database_function_description_set(foo_id, key, desc) database_function_data_set((foo_id), (key), (desc))

/* Add a parameter to a function */
Eolian_Function_Parameter database_function_parameter_add(Eolian_Function foo_id, Eolian_Parameter_Dir param_dir, const char *type, const char *name, const char *description);

void database_parameter_get_const_attribute_set(Eolian_Function_Parameter param_desc, Eina_Bool is_const);

void database_function_return_type_set(Eolian_Function foo_id, Eolian_Function_Type ftype, const char *ret_type);

void database_function_object_set_as_const(Eolian_Function foo_id, Eina_Bool is_const);

/* Need to add API for callbacks and implements */

Eolian_Implement
database_implement_new(const char *class_name, const char *func_name, Eolian_Function_Type type);

Eolian_Implement_Legacy
database_implement_legacy_add(Eolian_Implement impl, const char *legacy_func_name);

Eolian_Implement_Legacy_Parameter
database_implement_legacy_param_add(Eolian_Implement_Legacy leg,
      Eina_Stringshare *eo_param, Eina_Stringshare *leg_param,
      Eina_Stringshare *comment);

Eina_Bool
database_implement_legacy_return_add(Eolian_Implement_Legacy leg, Eina_Stringshare *ret_type, Eina_Stringshare *ret_value);

Eina_Bool
database_class_implement_add(const char *class_name, Eolian_Implement impl_id);

Eolian_Event
database_event_new(const char *event_name, const char *event_desc);

void
database_event_free(Eolian_Event event);

Eina_Bool
database_class_event_add(const char *class_name, Eolian_Event event_desc);

#endif
