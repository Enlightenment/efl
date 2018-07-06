#!/usr/bin/env python3
# encoding: utf-8

from ctypes import CDLL, c_bool, c_int, c_uint, c_char_p, c_void_p
from ctypes.util import find_library

import os


# load the eolian so/dll already built in source tree
script_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.abspath(os.path.join(script_path, '..', '..', '..'))

search_in = (
    os.path.join(root_path, 'src', 'lib', 'eolian', '.libs'),
    os.path.join(root_path, 'build', 'src', 'lib', 'eolian'),
)

search_names = ('libeolian.so', 'eolian.dll')

file_path = None
for lib_dir in search_in:
    for name in search_names:
        f = os.path.join(lib_dir, name)
        if os.path.exists(f):
            file_path = f

if not file_path:
    raise RuntimeError('Error: cannot find a built eolian library in source tree')

lib = CDLL(file_path)


# EAPI int eolian_init(void);
lib.eolian_init.argtypes = []
lib.eolian_init.restype = c_int

# EAPI int eolian_shutdown(void);
lib.eolian_shutdown.argtypes = []
lib.eolian_shutdown.restype = c_int

# EAPI Eolian *eolian_new(void);
lib.eolian_new.argtypes = []
lib.eolian_new.restype = c_void_p

# EAPI void eolian_free(Eolian *state);
lib.eolian_free.argtypes = [c_void_p,]
lib.eolian_free.restype = None

# EAPI const Eolian_Unit *eolian_file_parse(Eolian *state, const char *filepath);
lib.eolian_file_parse.argtypes = [c_void_p, c_char_p]
lib.eolian_file_parse.restype = c_void_p

# EAPI Eina_Iterator *eolian_all_eo_file_paths_get(const Eolian *state);
lib.eolian_all_eo_file_paths_get.argtypes = [c_void_p,]
lib.eolian_all_eo_file_paths_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_all_eot_file_paths_get(const Eolian *state);
lib.eolian_all_eot_file_paths_get.argtypes = [c_void_p,]
lib.eolian_all_eot_file_paths_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_all_eo_files_get(const Eolian *state);
lib.eolian_all_eo_files_get.argtypes = [c_void_p,]
lib.eolian_all_eo_files_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_all_eot_files_get(const Eolian *state);
lib.eolian_all_eot_files_get.argtypes = [c_void_p,]
lib.eolian_all_eot_files_get.restype = c_void_p

# EAPI Eina_Bool eolian_directory_scan(Eolian *state, const char *dir);
lib.eolian_directory_scan.argtypes = [c_void_p, c_char_p]
lib.eolian_directory_scan.restype = c_bool

# EAPI Eina_Bool eolian_system_directory_scan(Eolian *state);
lib.eolian_system_directory_scan.argtypes = [c_void_p,]
lib.eolian_system_directory_scan.restype = c_bool

# EAPI Eina_Bool eolian_all_eo_files_parse(Eolian *state);
lib.eolian_all_eo_files_parse.argtypes = [c_void_p,]
lib.eolian_all_eo_files_parse.restype = c_bool

# EAPI Eina_Bool eolian_all_eot_files_parse(Eolian *state);
lib.eolian_all_eot_files_parse.argtypes = [c_void_p,]
lib.eolian_all_eot_files_parse.restype = c_bool

# EAPI const Eolian_Class *eolian_class_get_by_name(const Eolian_Unit *unit, const char *class_name);
lib.eolian_class_get_by_name.argtypes = [c_void_p, c_char_p]
lib.eolian_class_get_by_name.restype = c_void_p

# EAPI const Eolian_Class *eolian_class_get_by_file(const Eolian_Unit *unit, const char *file_name);
lib.eolian_class_get_by_file.argtypes = [c_void_p, c_char_p]
lib.eolian_class_get_by_file.restype = c_void_p

# EAPI Eina_Stringshare *eolian_class_file_get(const Eolian_Class *klass);
lib.eolian_class_file_get.argtypes = [c_void_p,]
lib.eolian_class_file_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_class_full_name_get(const Eolian_Class *klass);
lib.eolian_class_full_name_get.argtypes = [c_void_p,]
lib.eolian_class_full_name_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_class_name_get(const Eolian_Class *klass);
lib.eolian_class_name_get.argtypes = [c_void_p,]
lib.eolian_class_name_get.restype = c_char_p

# EAPI Eina_Iterator *eolian_class_namespaces_get(const Eolian_Class *klass);
lib.eolian_class_namespaces_get.argtypes = [c_void_p,]
lib.eolian_class_namespaces_get.restype = c_void_p

# EAPI Eolian_Class_Type eolian_class_type_get(const Eolian_Class *klass);
lib.eolian_class_type_get.argtypes = [c_void_p,]
lib.eolian_class_type_get.restype = c_int

# EAPI Eina_Iterator *eolian_all_classes_get(const Eolian_Unit *unit);
lib.eolian_all_classes_get.argtypes = [c_void_p,]
lib.eolian_all_classes_get.restype = c_void_p

# EAPI const Eolian_Documentation *eolian_class_documentation_get(const Eolian_Class *klass);
lib.eolian_class_documentation_get.argtypes = [c_void_p,]
lib.eolian_class_documentation_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_class_legacy_prefix_get(const Eolian_Class *klass);
lib.eolian_class_legacy_prefix_get.argtypes = [c_void_p,]
lib.eolian_class_legacy_prefix_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_class_eo_prefix_get(const Eolian_Class *klass);
lib.eolian_class_eo_prefix_get.argtypes = [c_void_p,]
lib.eolian_class_eo_prefix_get.restype = c_char_p

# EAPI Eina_Stringshare* eolian_class_event_prefix_get(const Eolian_Class *klass);
lib.eolian_class_event_prefix_get.argtypes = [c_void_p,]
lib.eolian_class_event_prefix_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_class_data_type_get(const Eolian_Class *klass);
lib.eolian_class_data_type_get.argtypes = [c_void_p,]
lib.eolian_class_data_type_get.restype = c_char_p

# EAPI Eina_Iterator *eolian_class_inherits_get(const Eolian_Class *klass);
lib.eolian_class_inherits_get.argtypes = [c_void_p,]
lib.eolian_class_inherits_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_class_functions_get(const Eolian_Class *klass, Eolian_Function_Type func_type);
lib.eolian_class_functions_get.argtypes = [c_void_p, c_int]
lib.eolian_class_functions_get.restype = c_void_p

# EAPI Eolian_Function_Type eolian_function_type_get(const Eolian_Function *function_id);
lib.eolian_function_type_get.argtypes = [c_void_p,]
lib.eolian_function_type_get.restype = c_int

# EAPI Eolian_Object_Scope eolian_function_scope_get(const Eolian_Function *function_id, Eolian_Function_Type ftype);
lib.eolian_function_scope_get.argtypes = [c_void_p, c_int]
lib.eolian_function_scope_get.restype = c_int

# EAPI Eina_Stringshare *eolian_function_name_get(const Eolian_Function *function_id);
lib.eolian_function_name_get.argtypes = [c_void_p,]
lib.eolian_function_name_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_function_full_c_name_get(const Eolian_Function *function_id, Eolian_Function_Type ftype, Eina_Bool use_legacy);
lib.eolian_function_full_c_name_get.argtypes = [c_void_p, c_int, c_bool]
lib.eolian_function_full_c_name_get.restype = c_void_p  # Stringshare TO BE FREED

# EAPI const Eolian_Function *eolian_class_function_get_by_name(const Eolian_Class *klass, const char *func_name, Eolian_Function_Type f_type);
lib.eolian_class_function_get_by_name.argtypes = [c_void_p, c_char_p, c_int]
lib.eolian_class_function_get_by_name.restype = c_void_p

# EAPI Eina_Stringshare *eolian_function_legacy_get(const Eolian_Function *function_id, Eolian_Function_Type f_type);
lib.eolian_function_legacy_get.argtypes = [c_void_p, c_int]
lib.eolian_function_legacy_get.restype = c_char_p

# EAPI const Eolian_Implement *eolian_function_implement_get(const Eolian_Function *function_id);
lib.eolian_function_implement_get.argtypes = [c_void_p,]
lib.eolian_function_implement_get.restype = c_void_p

# EAPI Eina_Bool eolian_function_is_legacy_only(const Eolian_Function *function_id, Eolian_Function_Type ftype);
lib.eolian_function_is_legacy_only.argtypes = [c_void_p, c_int]
lib.eolian_function_is_legacy_only.restype = c_bool

# EAPI Eina_Bool eolian_function_is_class(const Eolian_Function *function_id);
lib.eolian_function_is_class.argtypes = [c_void_p,]
lib.eolian_function_is_class.restype = c_bool

# EAPI Eina_Bool eolian_function_is_beta(const Eolian_Function *function_id);
lib.eolian_function_is_beta.argtypes = [c_void_p,]
lib.eolian_function_is_beta.restype = c_bool

# EAPI Eina_Bool eolian_function_is_constructor(const Eolian_Function *function_id, const Eolian_Class *klass);
lib.eolian_function_is_constructor.argtypes = [c_void_p,c_void_p,]
lib.eolian_function_is_constructor.restype = c_bool

# TODO FIXME
# EAPI Eina_Bool eolian_function_is_function_pointer(const Eolian_Function *function_id);
#  lib.eolian_function_is_function_pointer.argtypes = [c_void_p,]
#  lib.eolian_function_is_function_pointer.restype = c_bool

# EAPI Eina_Iterator *eolian_function_parameters_get(const Eolian_Function *function_id);
lib.eolian_function_parameters_get.argtypes = [c_void_p,]
lib.eolian_function_parameters_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_property_keys_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
lib.eolian_property_keys_get.argtypes = [c_void_p, c_int]
lib.eolian_property_keys_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_property_values_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
lib.eolian_property_values_get.argtypes = [c_void_p, c_int]
lib.eolian_property_values_get.restype = c_void_p

# EAPI Eolian_Parameter_Dir eolian_parameter_direction_get(const Eolian_Function_Parameter *param);
lib.eolian_parameter_direction_get.argtypes = [c_void_p,]
lib.eolian_parameter_direction_get.restype = c_int

# EAPI const Eolian_Type *eolian_parameter_type_get(const Eolian_Function_Parameter *param);
lib.eolian_parameter_type_get.argtypes = [c_void_p,]
lib.eolian_parameter_type_get.restype = c_void_p

# EAPI const Eolian_Expression *eolian_parameter_default_value_get(const Eolian_Function_Parameter *param);
lib.eolian_parameter_default_value_get.argtypes = [c_void_p,]
lib.eolian_parameter_default_value_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_parameter_name_get(const Eolian_Function_Parameter *param);
lib.eolian_parameter_name_get.argtypes = [c_void_p,]
lib.eolian_parameter_name_get.restype = c_char_p

# EAPI const Eolian_Documentation *eolian_parameter_documentation_get(const Eolian_Function_Parameter *param);
lib.eolian_parameter_documentation_get.argtypes = [c_void_p,]
lib.eolian_parameter_documentation_get.restype = c_void_p

# EAPI Eina_Bool eolian_parameter_is_nonull(const Eolian_Function_Parameter *param_desc);
lib.eolian_parameter_is_nonull.argtypes = [c_void_p,]
lib.eolian_parameter_is_nonull.restype = c_bool

# EAPI Eina_Bool eolian_parameter_is_nullable(const Eolian_Function_Parameter *param_desc);
lib.eolian_parameter_is_nullable.argtypes = [c_void_p,]
lib.eolian_parameter_is_nullable.restype = c_bool

# EAPI Eina_Bool eolian_parameter_is_optional(const Eolian_Function_Parameter *param_desc);
lib.eolian_parameter_is_optional.argtypes = [c_void_p,]
lib.eolian_parameter_is_optional.restype = c_bool

# EAPI const Eolian_Type *eolian_function_return_type_get(const Eolian_Function *function_id, Eolian_Function_Type ftype);
lib.eolian_function_return_type_get.argtypes = [c_void_p, c_int]
lib.eolian_function_return_type_get.restype = c_void_p

# EAPI const Eolian_Expression * eolian_function_return_default_value_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
lib.eolian_function_return_default_value_get.argtypes = [c_void_p, c_int]
lib.eolian_function_return_default_value_get.restype = c_void_p

# EAPI const Eolian_Documentation *eolian_function_return_documentation_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
lib.eolian_function_return_documentation_get.argtypes = [c_void_p, c_int]
lib.eolian_function_return_documentation_get.restype = c_void_p

# EAPI Eina_Bool eolian_function_return_is_warn_unused(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
lib.eolian_function_return_is_warn_unused.argtypes = [c_void_p, c_int]
lib.eolian_function_return_is_warn_unused.restype = c_bool

# EAPI Eina_Bool eolian_function_object_is_const(const Eolian_Function *function_id);
lib.eolian_function_object_is_const.argtypes = [c_void_p,]
lib.eolian_function_object_is_const.restype = c_bool

# EAPI const Eolian_Class *eolian_function_class_get(const Eolian_Function *function_id);
lib.eolian_function_class_get.argtypes = [c_void_p,]
lib.eolian_function_class_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_implement_full_name_get(const Eolian_Implement *impl);
lib.eolian_implement_full_name_get.argtypes = [c_void_p,]
lib.eolian_implement_full_name_get.restype = c_char_p

# EAPI const Eolian_Class *eolian_implement_class_get(const Eolian_Implement *impl);
lib.eolian_implement_class_get.argtypes = [c_void_p,]
lib.eolian_implement_class_get.restype = c_void_p

# EAPI const Eolian_Function *eolian_implement_function_get(const Eolian_Implement *impl, Eolian_Function_Type *func_type);
lib.eolian_implement_function_get.argtypes = [c_void_p, c_void_p]
lib.eolian_implement_function_get.restype = c_void_p

# EAPI const Eolian_Documentation *eolian_implement_documentation_get(const Eolian_Implement *impl, Eolian_Function_Type f_type);
lib.eolian_implement_documentation_get.argtypes = [c_void_p, c_int]
lib.eolian_implement_documentation_get.restype = c_void_p

# EAPI Eina_Bool eolian_implement_is_auto(const Eolian_Implement *impl, Eolian_Function_Type f_type);
lib.eolian_implement_is_auto.argtypes = [c_void_p, c_int]
lib.eolian_implement_is_auto.restype = c_bool

# EAPI Eina_Bool eolian_implement_is_empty(const Eolian_Implement *impl, Eolian_Function_Type f_type);
lib.eolian_implement_is_empty.argtypes = [c_void_p, c_int]
lib.eolian_implement_is_empty.restype = c_bool

# EAPI Eina_Bool eolian_implement_is_pure_virtual(const Eolian_Implement *impl, Eolian_Function_Type f_type);
lib.eolian_implement_is_pure_virtual.argtypes = [c_void_p, c_int]
lib.eolian_implement_is_pure_virtual.restype = c_bool

# EAPI Eina_Bool eolian_implement_is_prop_get(const Eolian_Implement *impl);
lib.eolian_implement_is_prop_get.argtypes = [c_void_p,]
lib.eolian_implement_is_prop_get.restype = c_bool

# EAPI Eina_Bool eolian_implement_is_prop_set(const Eolian_Implement *impl);
lib.eolian_implement_is_prop_set.argtypes = [c_void_p,]
lib.eolian_implement_is_prop_set.restype = c_bool

# EAPI Eina_Iterator *eolian_class_implements_get(const Eolian_Class *klass);
lib.eolian_class_implements_get.argtypes = [c_void_p,]
lib.eolian_class_implements_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_constructor_full_name_get(const Eolian_Constructor *ctor);
lib.eolian_constructor_full_name_get.argtypes = [c_void_p,]
lib.eolian_constructor_full_name_get.restype = c_char_p

# EAPI const Eolian_Class *eolian_constructor_class_get(const Eolian_Constructor *ctor);
lib.eolian_constructor_class_get.argtypes = [c_void_p,]
lib.eolian_constructor_class_get.restype = c_void_p

# EAPI const Eolian_Function *eolian_constructor_function_get(const Eolian_Constructor *ctor);
lib.eolian_constructor_function_get.argtypes = [c_void_p,]
lib.eolian_constructor_function_get.restype = c_void_p

# EAPI Eina_Bool eolian_constructor_is_optional(const Eolian_Constructor *ctor);
lib.eolian_constructor_is_optional.argtypes = [c_void_p,]
lib.eolian_constructor_is_optional.restype = c_bool

# EAPI Eina_Iterator *eolian_class_constructors_get(const Eolian_Class *klass);
lib.eolian_class_constructors_get.argtypes = [c_void_p,]
lib.eolian_class_constructors_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_class_events_get(const Eolian_Class *klass);
lib.eolian_class_events_get.argtypes = [c_void_p,]
lib.eolian_class_events_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_event_name_get(const Eolian_Event *event);
lib.eolian_event_name_get.argtypes = [c_void_p,]
lib.eolian_event_name_get.restype = c_char_p

# EAPI const Eolian_Type *eolian_event_type_get(const Eolian_Event *event);
lib.eolian_event_type_get.argtypes = [c_void_p,]
lib.eolian_event_type_get.restype = c_void_p

# EAPI const Eolian_Documentation *eolian_event_documentation_get(const Eolian_Event *event);
lib.eolian_event_documentation_get.argtypes = [c_void_p,]
lib.eolian_event_documentation_get.restype = c_void_p

# EAPI Eolian_Object_Scope eolian_event_scope_get(const Eolian_Event *event);
lib.eolian_event_scope_get.argtypes = [c_void_p,]
lib.eolian_event_scope_get.restype = c_int

# EAPI Eina_Bool eolian_event_is_beta(const Eolian_Event *event);
lib.eolian_event_is_beta.argtypes = [c_void_p,]
lib.eolian_event_is_beta.restype = c_bool

# EAPI Eina_Bool eolian_event_is_hot(const Eolian_Event *event);
lib.eolian_event_is_hot.argtypes = [c_void_p,]
lib.eolian_event_is_hot.restype = c_bool

# EAPI Eina_Bool eolian_event_is_restart(const Eolian_Event *event);
lib.eolian_event_is_restart.argtypes = [c_void_p,]
lib.eolian_event_is_restart.restype = c_bool

# EAPI Eina_Iterator *eolian_class_parts_get(const Eolian_Class *klass);
lib.eolian_class_parts_get.argtypes = [c_void_p,]
lib.eolian_class_parts_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_part_name_get(const Eolian_Part *part);
lib.eolian_part_name_get.argtypes = [c_void_p,]
lib.eolian_part_name_get.restype = c_char_p

# EAPI const Eolian_Class *eolian_part_class_get(const Eolian_Part *part);
lib.eolian_part_class_get.argtypes = [c_void_p,]
lib.eolian_part_class_get.restype = c_void_p

# EAPI const Eolian_Documentation *eolian_part_documentation_get(const Eolian_Part *part);
lib.eolian_part_documentation_get.argtypes = [c_void_p,]
lib.eolian_part_documentation_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_event_c_name_get(const Eolian_Event *event);
lib.eolian_event_c_name_get.argtypes = [c_void_p,]
lib.eolian_event_c_name_get.restype = c_void_p  # Stringshare TO BE FREED

# EAPI const Eolian_Event *eolian_class_event_get_by_name(const Eolian_Class *klass, const char *event_name);
lib.eolian_class_event_get_by_name.argtypes = [c_void_p, c_char_p]
lib.eolian_class_event_get_by_name.restype = c_void_p

# EAPI Eina_Bool eolian_class_ctor_enable_get(const Eolian_Class *klass);
lib.eolian_class_ctor_enable_get.argtypes = [c_void_p,]
lib.eolian_class_ctor_enable_get.restype = c_bool

# EAPI Eina_Bool eolian_class_dtor_enable_get(const Eolian_Class *klass);
lib.eolian_class_dtor_enable_get.argtypes = [c_void_p,]
lib.eolian_class_dtor_enable_get.restype = c_bool

# EAPI Eina_Stringshare *eolian_class_c_get_function_name_get(const Eolian_Class *klass);
lib.eolian_class_c_get_function_name_get.argtypes = [c_void_p,]
lib.eolian_class_c_get_function_name_get.restype = c_void_p  # Stringshare TO BE FREED

# EAPI Eina_Stringshare *eolian_class_c_name_get(const Eolian_Class *klass);
lib.eolian_class_c_name_get.argtypes = [c_void_p,]
lib.eolian_class_c_name_get.restype = c_void_p  # Stringshare TO BE FREED

# EAPI Eina_Stringshare *eolian_class_c_data_type_get(const Eolian_Class *klass);
lib.eolian_class_c_data_type_get.argtypes = [c_void_p,]
lib.eolian_class_c_data_type_get.restype = c_void_p  # Stringshare TO BE FREED

# EAPI const Eolian_Typedecl *eolian_typedecl_alias_get_by_name(const Eolian_Unit *unit, const char *name);
lib.eolian_typedecl_alias_get_by_name.argtypes = [c_void_p, c_char_p]
lib.eolian_typedecl_alias_get_by_name.restype = c_void_p

# EAPI const Eolian_Typedecl *eolian_typedecl_struct_get_by_name(const Eolian_Unit *unit, const char *name);
lib.eolian_typedecl_struct_get_by_name.argtypes = [c_void_p, c_char_p]
lib.eolian_typedecl_struct_get_by_name.restype = c_void_p

# EAPI const Eolian_Typedecl *eolian_typedecl_enum_get_by_name(const Eolian_Unit *unit, const char *name);
lib.eolian_typedecl_enum_get_by_name.argtypes = [c_void_p, c_char_p]
lib.eolian_typedecl_enum_get_by_name.restype = c_void_p

# EAPI Eina_Iterator *eolian_typedecl_aliases_get_by_file(const Eolian_Unit *unit, const char *fname);
lib.eolian_typedecl_aliases_get_by_file.argtypes = [c_void_p, c_char_p]
lib.eolian_typedecl_aliases_get_by_file.restype = c_void_p

# EAPI Eina_Iterator *eolian_typedecl_structs_get_by_file(const Eolian_Unit *unit, const char *fname);
lib.eolian_typedecl_structs_get_by_file.argtypes = [c_void_p, c_char_p]
lib.eolian_typedecl_structs_get_by_file.restype = c_void_p

# EAPI Eina_Iterator *eolian_typedecl_enums_get_by_file(const Eolian_Unit *unit, const char *fname);
lib.eolian_typedecl_enums_get_by_file.argtypes = [c_void_p, c_char_p]
lib.eolian_typedecl_enums_get_by_file.restype = c_void_p

# EAPI Eina_Iterator *eolian_typedecl_all_aliases_get(const Eolian_Unit *unit);
lib.eolian_typedecl_all_aliases_get.argtypes = [c_void_p,]
lib.eolian_typedecl_all_aliases_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_typedecl_all_structs_get(const Eolian_Unit *unit);
lib.eolian_typedecl_all_structs_get.argtypes = [c_void_p,]
lib.eolian_typedecl_all_structs_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_typedecl_all_enums_get(const Eolian_Unit *unit);
lib.eolian_typedecl_all_enums_get.argtypes = [c_void_p,]
lib.eolian_typedecl_all_enums_get.restype = c_void_p

# EAPI Eolian_Typedecl_Type eolian_typedecl_type_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_type_get.argtypes = [c_void_p,]
lib.eolian_typedecl_type_get.restype = c_int

# EAPI Eina_Iterator *eolian_typedecl_struct_fields_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_struct_fields_get.argtypes = [c_void_p,]
lib.eolian_typedecl_struct_fields_get.restype = c_void_p

# EAPI const Eolian_Struct_Type_Field *eolian_typedecl_struct_field_get(const Eolian_Typedecl *tp, const char *field);
lib.eolian_typedecl_struct_field_get.argtypes = [c_void_p, c_char_p]
lib.eolian_typedecl_struct_field_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_typedecl_struct_field_name_get(const Eolian_Struct_Type_Field *fl);
lib.eolian_typedecl_struct_field_name_get.argtypes = [c_void_p,]
lib.eolian_typedecl_struct_field_name_get.restype = c_char_p

# EAPI const Eolian_Documentation *eolian_typedecl_struct_field_documentation_get(const Eolian_Struct_Type_Field *fl);
lib.eolian_typedecl_struct_field_documentation_get.argtypes = [c_void_p,]
lib.eolian_typedecl_struct_field_documentation_get.restype = c_void_p

# EAPI const Eolian_Type *eolian_typedecl_struct_field_type_get(const Eolian_Struct_Type_Field *fl);
lib.eolian_typedecl_struct_field_type_get.argtypes = [c_void_p,]
lib.eolian_typedecl_struct_field_type_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_typedecl_enum_fields_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_enum_fields_get.argtypes = [c_void_p,]
lib.eolian_typedecl_enum_fields_get.restype = c_void_p

# EAPI const Eolian_Enum_Type_Field *eolian_typedecl_enum_field_get(const Eolian_Typedecl *tp, const char *field);
lib.eolian_typedecl_enum_field_get.argtypes = [c_void_p, c_char_p]
lib.eolian_typedecl_enum_field_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_typedecl_enum_field_name_get(const Eolian_Enum_Type_Field *fl);
lib.eolian_typedecl_enum_field_name_get.argtypes = [c_void_p,]
lib.eolian_typedecl_enum_field_name_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_typedecl_enum_field_c_name_get(const Eolian_Enum_Type_Field *fl);
lib.eolian_typedecl_enum_field_c_name_get.argtypes = [c_void_p,]
lib.eolian_typedecl_enum_field_c_name_get.restype = c_void_p  # Stringshare TO BE FREED

# EAPI const Eolian_Documentation *eolian_typedecl_enum_field_documentation_get(const Eolian_Enum_Type_Field *fl);
lib.eolian_typedecl_enum_field_documentation_get.argtypes = [c_void_p,]
lib.eolian_typedecl_enum_field_documentation_get.restype = c_void_p

# EAPI const Eolian_Expression *eolian_typedecl_enum_field_value_get(const Eolian_Enum_Type_Field *fl, Eina_Bool force);
lib.eolian_typedecl_enum_field_value_get.argtypes = [c_void_p, c_bool]
lib.eolian_typedecl_enum_field_value_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_typedecl_enum_legacy_prefix_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_enum_legacy_prefix_get.argtypes = [c_void_p,]
lib.eolian_typedecl_enum_legacy_prefix_get.restype = c_char_p

# EAPI const Eolian_Documentation *eolian_typedecl_documentation_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_documentation_get.argtypes = [c_void_p,]
lib.eolian_typedecl_documentation_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_typedecl_file_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_file_get.argtypes = [c_void_p,]
lib.eolian_typedecl_file_get.restype = c_char_p

# EAPI const Eolian_Type *eolian_typedecl_base_type_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_base_type_get.argtypes = [c_void_p,]
lib.eolian_typedecl_base_type_get.restype = c_void_p

# EAPI const Eolian_Type *eolian_typedecl_aliased_base_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_aliased_base_get.argtypes = [c_void_p,]
lib.eolian_typedecl_aliased_base_get.restype = c_void_p

# EAPI Eina_Bool eolian_typedecl_is_extern(const Eolian_Typedecl *tp);
lib.eolian_typedecl_is_extern.argtypes = [c_void_p,]
lib.eolian_typedecl_is_extern.restype = c_bool

# EAPI Eina_Stringshare *eolian_typedecl_c_type_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_c_type_get.argtypes = [c_void_p,]
lib.eolian_typedecl_c_type_get.restype = c_void_p  # Stringshare TO BE FREED

# EAPI Eina_Stringshare *eolian_typedecl_name_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_name_get.argtypes = [c_void_p,]
lib.eolian_typedecl_name_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_typedecl_full_name_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_full_name_get.argtypes = [c_void_p,]
lib.eolian_typedecl_full_name_get.restype = c_char_p

# EAPI Eina_Iterator *eolian_typedecl_namespaces_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_namespaces_get.argtypes = [c_void_p,]
lib.eolian_typedecl_namespaces_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_typedecl_free_func_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_free_func_get.argtypes = [c_void_p,]
lib.eolian_typedecl_free_func_get.restype = c_char_p

# EAPI const Eolian_Function *eolian_typedecl_function_pointer_get(const Eolian_Typedecl *tp);
lib.eolian_typedecl_function_pointer_get.argtypes = [c_void_p,]
lib.eolian_typedecl_function_pointer_get.restype = c_void_p

# EAPI Eolian_Type_Type eolian_type_type_get(const Eolian_Type *tp);
lib.eolian_type_type_get.argtypes = [c_void_p,]
lib.eolian_type_type_get.restype = c_int

# EAPI Eolian_Type_Builtin_Type eolian_type_builtin_type_get(const Eolian_Type *tp);
lib.eolian_type_builtin_type_get.argtypes = [c_void_p,]
lib.eolian_type_builtin_type_get.restype = c_int

# EAPI Eina_Stringshare *eolian_type_file_get(const Eolian_Type *tp);
lib.eolian_type_file_get.argtypes = [c_void_p,]
lib.eolian_type_file_get.restype = c_char_p

# EAPI const Eolian_Type *eolian_type_base_type_get(const Eolian_Type *tp);
lib.eolian_type_base_type_get.argtypes = [c_void_p,]
lib.eolian_type_base_type_get.restype = c_void_p

# EAPI const Eolian_Type *eolian_type_next_type_get(const Eolian_Type *tp);
lib.eolian_type_next_type_get.argtypes = [c_void_p,]
lib.eolian_type_next_type_get.restype = c_void_p

# EAPI const Eolian_Typedecl *eolian_type_typedecl_get(const Eolian_Type *tp);
lib.eolian_type_typedecl_get.argtypes = [c_void_p,]
lib.eolian_type_typedecl_get.restype = c_void_p

# EAPI const Eolian_Type *eolian_type_aliased_base_get(const Eolian_Type *tp);
lib.eolian_type_aliased_base_get.argtypes = [c_void_p,]
lib.eolian_type_aliased_base_get.restype = c_void_p

# EAPI const Eolian_Class *eolian_type_class_get(const Eolian_Type *tp);
lib.eolian_type_class_get.argtypes = [c_void_p,]
lib.eolian_type_class_get.restype = c_void_p

# EAPI Eina_Bool eolian_type_is_owned(const Eolian_Type *tp);
lib.eolian_type_is_owned.argtypes = [c_void_p,]
lib.eolian_type_is_owned.restype = c_bool

# EAPI Eina_Bool eolian_type_is_const(const Eolian_Type *tp);
lib.eolian_type_is_const.argtypes = [c_void_p,]
lib.eolian_type_is_const.restype = c_bool

# EAPI Eina_Bool eolian_type_is_ptr(const Eolian_Type *tp);
lib.eolian_type_is_ptr.argtypes = [c_void_p,]
lib.eolian_type_is_ptr.restype = c_bool

# EAPI Eina_Stringshare *eolian_type_c_type_get(const Eolian_Type *tp, Eolian_C_Type_Type ctype);
lib.eolian_type_c_type_get.argtypes = [c_void_p, c_int]
lib.eolian_type_c_type_get.restype = c_void_p  # Stringshare TO BE FREED

# EAPI Eina_Stringshare *eolian_type_name_get(const Eolian_Type *tp);
lib.eolian_type_name_get.argtypes = [c_void_p,]
lib.eolian_type_name_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_type_full_name_get(const Eolian_Type *tp);
lib.eolian_type_full_name_get.argtypes = [c_void_p,]
lib.eolian_type_full_name_get.restype = c_char_p

# EAPI Eina_Iterator *eolian_type_namespaces_get(const Eolian_Type *tp);
lib.eolian_type_namespaces_get.argtypes = [c_void_p,]
lib.eolian_type_namespaces_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_type_free_func_get(const Eolian_Type *tp);
lib.eolian_type_free_func_get.argtypes = [c_void_p,]
lib.eolian_type_free_func_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_expression_serialize(const Eolian_Expression *expr);
lib.eolian_expression_serialize.argtypes = [c_void_p,]
lib.eolian_expression_serialize.restype = c_void_p  # Stringshare TO BE FREED

# EAPI Eolian_Expression_Type eolian_expression_type_get(const Eolian_Expression *expr);
lib.eolian_expression_type_get.argtypes = [c_void_p,]
lib.eolian_expression_type_get.restype = c_int

# EAPI Eolian_Binary_Operator eolian_expression_binary_operator_get(const Eolian_Expression *expr);
lib.eolian_expression_binary_operator_get.argtypes = [c_void_p,]
lib.eolian_expression_binary_operator_get.restype = c_int

# EAPI const Eolian_Expression *eolian_expression_binary_lhs_get(const Eolian_Expression *expr);
lib.eolian_expression_binary_lhs_get.argtypes = [c_void_p,]
lib.eolian_expression_binary_lhs_get.restype = c_void_p

# EAPI const Eolian_Expression *eolian_expression_binary_rhs_get(const Eolian_Expression *expr);
lib.eolian_expression_binary_rhs_get.argtypes = [c_void_p,]
lib.eolian_expression_binary_rhs_get.restype = c_void_p

# EAPI Eolian_Unary_Operator eolian_expression_unary_operator_get(const Eolian_Expression *expr);
lib.eolian_expression_unary_operator_get.argtypes = [c_void_p,]
lib.eolian_expression_unary_operator_get.restype = c_int

# EAPI const Eolian_Expression *eolian_expression_unary_expression_get(const Eolian_Expression *expr);
lib.eolian_expression_unary_expression_get.argtypes = [c_void_p,]
lib.eolian_expression_unary_expression_get.restype = c_void_p

# EAPI const Eolian_Variable *eolian_variable_global_get_by_name(const Eolian_Unit *unit, const char *name);
lib.eolian_variable_global_get_by_name.argtypes = [c_void_p, c_char_p]
lib.eolian_variable_global_get_by_name.restype = c_void_p

# EAPI const Eolian_Variable *eolian_variable_constant_get_by_name(const Eolian_Unit *unit, const char *name);
lib.eolian_variable_constant_get_by_name.argtypes = [c_void_p, c_char_p]
lib.eolian_variable_constant_get_by_name.restype = c_void_p

# EAPI Eina_Iterator *eolian_variable_globals_get_by_file(const Eolian_Unit *unit, const char *fname);
lib.eolian_variable_globals_get_by_file.argtypes = [c_void_p, c_char_p]
lib.eolian_variable_globals_get_by_file.restype = c_void_p

# EAPI Eina_Iterator *eolian_variable_constants_get_by_file(const Eolian_Unit *unit, const char *fname);
lib.eolian_variable_constants_get_by_file.argtypes = [c_void_p, c_char_p]
lib.eolian_variable_constants_get_by_file.restype = c_void_p

# EAPI Eina_Iterator *eolian_variable_all_constants_get(const Eolian_Unit *unit);
lib.eolian_variable_all_constants_get.argtypes = [c_void_p,]
lib.eolian_variable_all_constants_get.restype = c_void_p

# EAPI Eina_Iterator *eolian_variable_all_globals_get(const Eolian_Unit *unit);
lib.eolian_variable_all_globals_get.argtypes = [c_void_p,]
lib.eolian_variable_all_globals_get.restype = c_void_p

# EAPI Eolian_Variable_Type eolian_variable_type_get(const Eolian_Variable *var);
lib.eolian_variable_type_get.argtypes = [c_void_p,]
lib.eolian_variable_type_get.restype = c_int

# EAPI const Eolian_Documentation *eolian_variable_documentation_get(const Eolian_Variable *var);
lib.eolian_variable_documentation_get.argtypes = [c_void_p,]
lib.eolian_variable_documentation_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_variable_file_get(const Eolian_Variable *var);
lib.eolian_variable_file_get.argtypes = [c_void_p,]
lib.eolian_variable_file_get.restype = c_char_p

# EAPI const Eolian_Type *eolian_variable_base_type_get(const Eolian_Variable *var);
lib.eolian_variable_base_type_get.argtypes = [c_void_p,]
lib.eolian_variable_base_type_get.restype = c_void_p

# EAPI const Eolian_Expression *eolian_variable_value_get(const Eolian_Variable *var);
lib.eolian_variable_value_get.argtypes = [c_void_p,]
lib.eolian_variable_value_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_variable_name_get(const Eolian_Variable *var);
lib.eolian_variable_name_get.argtypes = [c_void_p,]
lib.eolian_variable_name_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_variable_full_name_get(const Eolian_Variable *var);
lib.eolian_variable_full_name_get.argtypes = [c_void_p,]
lib.eolian_variable_full_name_get.restype = c_char_p

# EAPI Eina_Iterator *eolian_variable_namespaces_get(const Eolian_Variable *var);
lib.eolian_variable_namespaces_get.argtypes = [c_void_p,]
lib.eolian_variable_namespaces_get.restype = c_void_p

# EAPI Eina_Bool eolian_variable_is_extern(const Eolian_Variable *var);
lib.eolian_variable_is_extern.argtypes = [c_void_p,]
lib.eolian_variable_is_extern.restype = c_bool

# EAPI const Eolian_Declaration *eolian_declaration_get_by_name(const Eolian_Unit *unit, const char *name);
lib.eolian_declaration_get_by_name.argtypes = [c_void_p, c_char_p]
lib.eolian_declaration_get_by_name.restype = c_void_p

# EAPI Eina_Iterator *eolian_declarations_get_by_file(const Eolian *state, const char *fname);
lib.eolian_declarations_get_by_file.argtypes = [c_void_p, c_char_p]
lib.eolian_declarations_get_by_file.restype = c_void_p

# EAPI Eina_Iterator *eolian_all_declarations_get(const Eolian_Unit *unit);
lib.eolian_all_declarations_get.argtypes = [c_void_p,]
lib.eolian_all_declarations_get.restype = c_void_p

# EAPI Eolian_Declaration_Type eolian_declaration_type_get(const Eolian_Declaration *decl);
lib.eolian_declaration_type_get.argtypes = [c_void_p,]
lib.eolian_declaration_type_get.restype = c_int

# EAPI Eina_Stringshare *eolian_declaration_name_get(const Eolian_Declaration *decl);
lib.eolian_declaration_name_get.argtypes = [c_void_p,]
lib.eolian_declaration_name_get.restype = c_char_p

# EAPI const Eolian_Class *eolian_declaration_class_get(const Eolian_Declaration *decl);
lib.eolian_declaration_class_get.argtypes = [c_void_p,]
lib.eolian_declaration_class_get.restype = c_void_p

# EAPI const Eolian_Typedecl *eolian_declaration_data_type_get(const Eolian_Declaration *decl);
lib.eolian_declaration_data_type_get.argtypes = [c_void_p,]
lib.eolian_declaration_data_type_get.restype = c_void_p

# EAPI const Eolian_Variable *eolian_declaration_variable_get(const Eolian_Declaration *decl);
lib.eolian_declaration_variable_get.argtypes = [c_void_p,]
lib.eolian_declaration_variable_get.restype = c_void_p

# EAPI Eina_Stringshare *eolian_documentation_summary_get(const Eolian_Documentation *doc);
lib.eolian_documentation_summary_get.argtypes = [c_void_p,]
lib.eolian_documentation_summary_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_documentation_description_get(const Eolian_Documentation *doc);
lib.eolian_documentation_description_get.argtypes = [c_void_p,]
lib.eolian_documentation_description_get.restype = c_char_p

# EAPI Eina_Stringshare *eolian_documentation_since_get(const Eolian_Documentation *doc);
lib.eolian_documentation_since_get.argtypes = [c_void_p,]
lib.eolian_documentation_since_get.restype = c_char_p

# NOT IMPLEMENTED
# EAPI Eina_List *eolian_documentation_string_split(const char *doc);
#  lib.eolian_documentation_string_split.argtypes = [c_char_p,]
#  lib.eolian_documentation_string_split.restype = c_void_p

#  # EAPI const char *eolian_documentation_tokenize(const char *doc, Eolian_Doc_Token *ret);
lib.eolian_documentation_tokenize.argtypes = [c_char_p, c_void_p]
lib.eolian_documentation_tokenize.restype = c_void_p  # this need to be passed back as char*

# EAPI void eolian_doc_token_init(Eolian_Doc_Token *tok);
lib.eolian_doc_token_init.argtypes = [c_void_p,]
lib.eolian_doc_token_init.restype = None

# EAPI Eolian_Doc_Token_Type eolian_doc_token_type_get(const Eolian_Doc_Token *tok);
lib.eolian_doc_token_type_get.argtypes = [c_void_p,]
lib.eolian_doc_token_type_get.restype = c_int

# EAPI char *eolian_doc_token_text_get(const Eolian_Doc_Token *tok);
lib.eolian_doc_token_text_get.argtypes = [c_void_p,]
lib.eolian_doc_token_text_get.restype = c_void_p  # char* TO BE FREED

# EAPI Eolian_Doc_Ref_Type eolian_doc_token_ref_get(const Eolian_Unit *unit, const Eolian_Doc_Token *tok, const void **data, const void **data2);
#  lib.eolian_doc_token_ref_get.argtypes = [c_void_p, c_void_p, ???, ???]
#  lib.eolian_doc_token_ref_get.restype = c_int

