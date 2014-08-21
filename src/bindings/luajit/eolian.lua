-- EFL LuaJIT bindings: Eolian
-- For use with Elua

local ffi = require("ffi")
local bit = require("bit")

ffi.cdef [[
    void eina_stringshare_del(const char *str);
]]

ffi.cdef [[
    typedef unsigned char Eina_Bool;
    typedef struct _Eina_Iterator Eina_Iterator;

    typedef struct _Eolian_Class Eolian_Class;
    typedef struct _Eolian_Function Eolian_Function;
    typedef struct _Eolian_Type Eolian_Type;
    typedef struct _Eolian_Function_Parameter Eolian_Function_Parameter;
    typedef struct _Eolian_Implement Eolian_Implement;
    typedef struct _Eolian_Event Eolian_Event;
    typedef struct _Eolian_Expression Eolian_Expression;
    typedef struct _Eolian_Variable Eolian_Variable;
    typedef struct _Eolian_Value Eolian_Value;

    typedef enum
    {
        EOLIAN_UNRESOLVED,
        EOLIAN_PROPERTY,
        EOLIAN_PROP_SET,
        EOLIAN_PROP_GET,
        EOLIAN_METHOD,
        EOLIAN_CTOR
    } Eolian_Function_Type;

    typedef enum
    {
        EOLIAN_IN_PARAM,
        EOLIAN_OUT_PARAM,
        EOLIAN_INOUT_PARAM
    } Eolian_Parameter_Dir;

    typedef enum
    {
       EOLIAN_CLASS_UNKNOWN_TYPE,
        EOLIAN_CLASS_REGULAR,
        EOLIAN_CLASS_ABSTRACT,
        EOLIAN_CLASS_MIXIN,
        EOLIAN_CLASS_INTERFACE
    } Eolian_Class_Type;

    typedef enum
    {
        EOLIAN_SCOPE_PUBLIC,
        EOLIAN_SCOPE_PROTECTED
    } Eolian_Object_Scope;

    typedef enum
    {
        EOLIAN_TYPE_UNKNOWN_TYPE,
        EOLIAN_TYPE_VOID,
        EOLIAN_TYPE_REGULAR,
        EOLIAN_TYPE_REGULAR_STRUCT,
        EOLIAN_TYPE_REGULAR_ENUM,
        EOLIAN_TYPE_POINTER,
        EOLIAN_TYPE_FUNCTION,
        EOLIAN_TYPE_STRUCT,
        EOLIAN_TYPE_STRUCT_OPAQUE,
        EOLIAN_TYPE_ENUM,
        EOLIAN_TYPE_ALIAS,
        EOLIAN_TYPE_CLASS
    } Eolian_Type_Type;

    typedef enum {
        EOLIAN_EXPR_UNKNOWN = 0,
        EOLIAN_EXPR_INT,
        EOLIAN_EXPR_UINT,
        EOLIAN_EXPR_LONG,
        EOLIAN_EXPR_ULONG,
        EOLIAN_EXPR_LLONG,
        EOLIAN_EXPR_ULLONG,
        EOLIAN_EXPR_FLOAT,
        EOLIAN_EXPR_DOUBLE,
        EOLIAN_EXPR_STRING,
        EOLIAN_EXPR_CHAR,
        EOLIAN_EXPR_NULL,
        EOLIAN_EXPR_BOOL,
        EOLIAN_EXPR_NAME,
        EOLIAN_EXPR_ENUM,
        EOLIAN_EXPR_UNARY,
        EOLIAN_EXPR_BINARY
    } Eolian_Expression_Type;

    typedef enum {
        EOLIAN_MASK_SINT   = 1 << 0,
        EOLIAN_MASK_UINT   = 1 << 1,
        EOLIAN_MASK_INT    = EOLIAN_MASK_SINT | EOLIAN_MASK_UINT,
        EOLIAN_MASK_FLOAT  = 1 << 2,
        EOLIAN_MASK_BOOL   = 1 << 3,
        EOLIAN_MASK_STRING = 1 << 4,
        EOLIAN_MASK_CHAR   = 1 << 5,
        EOLIAN_MASK_NULL   = 1 << 6,
        EOLIAN_MASK_NUMBER = EOLIAN_MASK_INT    | EOLIAN_MASK_FLOAT,
        EOLIAN_MASK_ALL    = EOLIAN_MASK_NUMBER | EOLIAN_MASK_BOOL
                           | EOLIAN_MASK_STRING | EOLIAN_MASK_CHAR
                           | EOLIAN_MASK_NULL
    } Eolian_Expression_Mask;

    typedef enum {
        EOLIAN_VAR_UNKNOWN = 0,
        EOLIAN_VAR_CONSTANT,
        EOLIAN_VAR_GLOBAL
    } Eolian_Variable_Type;

    typedef union {
        char               c;
        Eina_Bool          b;
        const char        *s;
        signed int         i;
        unsigned int       u;
        signed long        l;
        unsigned long      ul;
        signed long long   ll;
        unsigned long long ull;
        float              f;
        double             d;
    } Eolian_Value_Union;

    typedef struct _Eolian_Value_t {
        Eolian_Expression_Type type;
        Eolian_Value_Union value;
    } Eolian_Value_t;

    Eina_Bool eolian_eo_file_parse(const char *filename);
    Eina_Bool eolian_eot_file_parse(const char *filepath);
    int eolian_init(void);
    int eolian_shutdown(void);
    Eina_Bool eolian_directory_scan(const char *dir);
    Eina_Bool eolian_system_directory_scan();
    Eina_Bool eolian_all_eo_files_parse();
    Eina_Bool eolian_all_eot_files_parse();
    Eina_Bool eolian_database_validate(void);
    Eina_Bool eolian_show_class(const Eolian_Class *klass);
    Eina_Bool eolian_show_typedef(const char *alias);
    Eina_Bool eolian_show_struct(const char *name);
    Eina_Bool eolian_show_enum(const char *name);
    Eina_Bool eolian_show_global(const char *name);
    Eina_Bool eolian_show_constant(const char *name);
    void eolian_show_all();
    const Eolian_Class *eolian_class_get_by_name(const char *class_name);
    const Eolian_Class *eolian_class_get_by_file(const char *file_name);
    const char *eolian_class_file_get(const Eolian_Class *klass);
    const char *eolian_class_full_name_get(const Eolian_Class *klass);
    const char *eolian_class_name_get(const Eolian_Class *klass);
    Eina_Iterator *eolian_class_namespaces_get(const Eolian_Class *klass);
    Eolian_Class_Type eolian_class_type_get(const Eolian_Class *klass);
    Eina_Iterator *eolian_all_classes_get(void);
    const char *eolian_class_description_get(const Eolian_Class *klass);
    const char *eolian_class_legacy_prefix_get(const Eolian_Class *klass);
    const char *eolian_class_eo_prefix_get(const Eolian_Class *klass);
    const char *eolian_class_data_type_get(const Eolian_Class *klass);
    Eina_Iterator *eolian_class_inherits_get(const Eolian_Class *klass);
    Eina_Iterator *eolian_class_functions_get(const Eolian_Class *klass, Eolian_Function_Type func_type);
    Eolian_Function_Type eolian_function_type_get(const Eolian_Function *function_id);
    Eolian_Object_Scope eolian_function_scope_get(const Eolian_Function *function_id);
    const char *eolian_function_name_get(const Eolian_Function *function_id);
    const char *eolian_function_full_c_name_get(const Eolian_Function *function_id, const char *prefix);
    const Eolian_Function *eolian_class_function_get_by_name(const Eolian_Class *klass, const char *func_name, Eolian_Function_Type f_type);
    const char *eolian_function_legacy_get(const Eolian_Function *function_id, Eolian_Function_Type f_type);
    const char *eolian_function_description_get(const Eolian_Function *function_id, Eolian_Function_Type f_type);
    Eina_Bool eolian_function_is_virtual_pure(const Eolian_Function *function_id, Eolian_Function_Type f_type);
    Eina_Bool eolian_function_is_legacy_only(const Eolian_Function *function_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_is_class(const Eolian_Function *function_id);
    Eina_Bool eolian_function_is_constructing(const Eolian_Function *function_id);
    const Eolian_Function_Parameter *eolian_function_parameter_get_by_name(const Eolian_Function *function_id, const char *param_name);
    Eina_Iterator *eolian_property_keys_get(const Eolian_Function *foo_id);
    Eina_Iterator *eolian_property_values_get(const Eolian_Function *foo_id);
    Eina_Iterator *eolian_function_parameters_get(const Eolian_Function *function_id);
    Eolian_Parameter_Dir eolian_parameter_direction_get(const Eolian_Function_Parameter *param);
    const Eolian_Type *eolian_parameter_type_get(const Eolian_Function_Parameter *param);
    const char *eolian_parameter_name_get(const Eolian_Function_Parameter *param);
    const char *eolian_parameter_description_get(const Eolian_Function_Parameter *param);
    Eina_Bool eolian_parameter_const_attribute_get(const Eolian_Function_Parameter *param_desc, Eina_Bool is_get);
    Eina_Bool eolian_parameter_is_nonull(const Eolian_Function_Parameter *param_desc);
    const Eolian_Type *eolian_function_return_type_get(const Eolian_Function *function_id, Eolian_Function_Type ftype);
    const Eolian_Expression *eolian_function_return_default_value_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    const char *eolian_function_return_comment_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_return_is_warn_unused(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_object_is_const(const Eolian_Function *function_id);
    Eina_Bool eolian_type_is_extern(const Eolian_Type *tp);
    const char *eolian_implement_full_name_get(const Eolian_Implement *impl);
    const Eolian_Class *eolian_implement_class_get(const Eolian_Implement *impl);
    const Eolian_Function *eolian_implement_function_get(const Eolian_Implement *impl, Eolian_Function_Type *func_type);
    Eina_Iterator *eolian_class_implements_get(const Eolian_Class *klass);
    Eina_Iterator *eolian_class_events_get(const Eolian_Class *klass);
    const char *eolian_event_name_get(const Eolian_Event *event);
    const Eolian_Type *eolian_event_type_get(const Eolian_Event *event);
    const char *eolian_event_description_get(const Eolian_Event *event);
    Eolian_Object_Scope eolian_event_scope_get(const Eolian_Event *event);
    Eina_Bool eolian_class_ctor_enable_get(const Eolian_Class *klass);
    Eina_Bool eolian_class_dtor_enable_get(const Eolian_Class *klass);
    const Eolian_Type *eolian_type_alias_get_by_name(const char *name);
    const Eolian_Type *eolian_type_struct_get_by_name(const char *name);
    const Eolian_Type *eolian_type_enum_get_by_name(const char *name);
    Eina_Iterator *eolian_type_aliases_get_by_file(const char *fname);
    Eina_Iterator *eolian_type_structs_get_by_file(const char *fname);
    Eina_Iterator *eolian_type_enums_get_by_file(const char *fname);
    Eolian_Type_Type eolian_type_type_get(const Eolian_Type *tp);
    Eina_Iterator *eolian_type_arguments_get(const Eolian_Type *tp);
    Eina_Iterator *eolian_type_subtypes_get(const Eolian_Type *tp);
    Eina_Iterator *eolian_type_struct_field_names_get(const Eolian_Type *tp);
    const Eolian_Type *eolian_type_struct_field_get(const Eolian_Type *tp, const char *field);
    const char *eolian_type_struct_field_description_get(const Eolian_Type *tp, const char *field);
    Eina_Iterator *eolian_type_enum_field_names_get(const Eolian_Type *tp);
    Eina_Bool eolian_type_enum_field_exists(const Eolian_Type *tp, const char *field);
    const Eolian_Expression *eolian_type_enum_field_get(const Eolian_Type *tp, const char *field);
    const char *eolian_type_enum_field_description_get(const Eolian_Type *tp, const char *field);
    const char *eolian_type_enum_legacy_prefix_get(const Eolian_Type *tp);
    const char *eolian_type_description_get(const Eolian_Type *tp);
    const char *eolian_type_file_get(const Eolian_Type *tp);
    const Eolian_Type *eolian_type_return_type_get(const Eolian_Type *tp);
    const Eolian_Type *eolian_type_base_type_get(const Eolian_Type *tp);
    const Eolian_Class *eolian_type_class_get(const Eolian_Type *tp);
    Eina_Bool eolian_type_is_own(const Eolian_Type *tp);
    Eina_Bool eolian_type_is_const(const Eolian_Type *tp);
    const char *eolian_type_c_type_named_get(const Eolian_Type *tp, const char *name);
    const char *eolian_type_c_type_get(const Eolian_Type *tp);
    const char *eolian_type_name_get(const Eolian_Type *tp);
    const char *eolian_type_full_name_get(const Eolian_Type *tp);
    Eina_Iterator *eolian_type_namespaces_get(const Eolian_Type *tp);
    const char *eolian_type_free_func_get(const Eolian_Type *tp);
    Eolian_Value_t eolian_expression_eval(const Eolian_Expression *expr, Eolian_Expression_Mask m);
    Eolian_Value_t eolian_expression_eval_type(const Eolian_Expression *expr, const Eolian_Type *type);
    const char *eolian_expression_value_to_literal(const Eolian_Value *v);
    const char *eolian_expression_serialize(const Eolian_Expression *expr);
    const Eolian_Variable *eolian_variable_global_get_by_name(const char *name);
    const Eolian_Variable *eolian_variable_constant_get_by_name(const char *name);
    Eina_Iterator *eolian_variable_globals_get_by_file(const char *fname);
    Eina_Iterator *eolian_variable_constants_get_by_file(const char *fname);
    Eolian_Variable_Type eolian_variable_type_get(const Eolian_Variable *var);
    const char *eolian_variable_description_get(const Eolian_Variable *var);
    const char *eolian_variable_file_get(const Eolian_Variable *var);
    const Eolian_Type *eolian_variable_base_type_get(const Eolian_Variable *var);
    const Eolian_Expression *eolian_variable_value_get(const Eolian_Variable *var);
    const char *eolian_variable_name_get(const Eolian_Variable *var);
    const char *eolian_variable_full_name_get(const Eolian_Variable *var);
    Eina_Iterator *eolian_variable_namespaces_get(const Eolian_Variable *var);
    Eina_Bool eolian_variable_is_extern(const Eolian_Variable *var);
]]

local cutil = require("cutil")
local util  = require("util")

local iterator = require("eina.iterator")

local Ptr_Iterator = iterator.Ptr_Iterator

local M = {}

local eolian
local eina

local init = function()
    eolian = util.lib_load("eolian")
    eina = util.lib_load("eina")
    eolian.eolian_init()
end

local shutdown = function()
    eolian.eolian_shutdown()
    util.lib_unload("eolian")
    util.lib_unload("eina")
end

local ffi_stringshare = function(s)
    local r = ffi.string(s)
    eina.eina_stringshare_del(s)
    return r
end

cutil.init_module(init, shutdown)

M.object_scope = {
    PUBLIC    = 0,
    PRIVATE   = 1,
    PROTECTED = 2
}

M.directory_scan = function(dir)
    return eolian.eolian_directory_scan(dir) ~= 0
end

M.system_directory_scan = function()
    return eolian.eolian_system_directory_scan() ~= 0
end

M.all_eo_files_parse = function()
    return eolian.eolian_all_eo_files_parse() ~= 0
end

M.eo_file_parse = function(fname)
    return eolian.eolian_eo_file_parse(fname) ~= 0
end

M.all_eot_files_parse = function()
    return eolian.eolian_all_eot_files_parse() ~= 0
end

M.database_validate = function()
    return eolian.eolian_databasE_validate() ~= 0
end

M.eot_file_parse = function(fname)
    return eolian.eolian_eot_file_parse(fname) ~= 0
end

M.show_class = function(klass)
    return eolian.eolian_show_class(klass) ~= 0
end

M.show_typedef = function(alias)
    return eolian.eolian_show_typedef(alias) ~= 0
end

M.show_struct = function(name)
    return eolian.eolian_show_typedef(name) ~= 0
end

M.show_enum = function(name)
    return eolian.eolian_show_enum(name) ~= 0
end

M.show_global = function(name)
    return eolian.eolian_show_global(name) ~= 0
end

M.show_consatnt = function(name)
    return eolian.eolian_show_constant(name) ~= 0
end

M.show_all = function()
    eolian.eolian_show_all()
end

M.type_type = {
    UNKNOWN        = 0,
    VOID           = 1,
    REGULAR        = 2,
    REGULAR_STRUCT = 3,
    REGULAR_ENUM   = 4,
    POINTER        = 5,
    FUNCTION       = 6,
    STRUCT         = 7,
    STRUCT_OPAQUE  = 8,
    ENUM           = 9,
    ALIAS          = 10,
    CLASS          = 11
}

M.Type = ffi.metatype("Eolian_Type", {
    __index = {
        type_get = function(self)
            return eolian.eolian_type_type_get(self)
        end,

        arguments_get = function(self)
            return Ptr_Iterator("const Eolian_Type*",
                eolian.eolian_type_arguments_get(self))
        end,

        subtypes_get = function(self)
            return Ptr_Iterator("const Eolian_Type*",
                eolian.eolian_type_subtypes_get(self))
        end,

        struct_field_names_get = function(self)
            return iterator.String_Iterator(
                eolian.eolian_type_struct_field_names_get(self))
        end,

        struct_field_get = function(self, name)
            local v = eolian.eolian_type_struct_field_get(self, name)
            if v == nil then return nil end
            return v
        end,

        struct_field_description_get = function(self, name)
            local v = eolian.eolian_type_struct_field_description_get(self, name)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        enum_field_names_get = function(self)
            return iterator.String_Iterator(
                eolian.eolian_type_enum_field_names_get(self))
        end,

        enum_field_exists = function(self, field)
            return eolian.eolian_type_enum_field_exists(self, field) ~= 0
        end,

        enum_field_get = function(self, field)
            local v = eolian.eolian_type_enum_field_get(self, field)
            if v == nil then return nil end
            return v
        end,

        enum_field_description_get = function(self, name)
            local v = eolian.eolian_type_enum_field_description_get(self, name)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        enum_legacy_prefix_get = function(self)
            local v = eolian.eolian_type_enum_legacy_prefix_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        description_get = function(self, name)
            local v = eolian.eolian_type_description_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        file_get = function(self, name)
            local v = eolian.eolian_type_file_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        return_type_get = function(self)
            local v = eolian.eolian_type_return_type_get(self)
            if v == nil then return nil end
            return v
        end,

        base_type_get = function(self)
            local v = eolian.eolian_type_base_type_get(self)
            if v == nil then return nil end
            return v
        end,

        class_get = function(self)
            local v = eolian.eolian_type_class_get(self)
            if v == nil then return nil end
            return v
        end,

        is_own = function(self)
            return eolian.eolian_type_is_own(self) ~= 0
        end,

        is_const = function(self)
            return eolian.eolian_type_is_const(self) ~= 0
        end,

        is_extern = function(self)
            return eolian.eolian_type_is_extern(self) ~= 0
        end,

        c_type_named_get = function(self, name)
            local v = eolian.eolian_type_c_type_named_get(self, name)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end,

        c_type_get = function(self)
            local v = eolian.eolian_type_c_type_get(self)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end,

        name_get = function(self)
            local v = eolian.eolian_type_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        full_name_get = function(self)
            local v = eolian.eolian_type_full_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        namespaces_get = function(self)
            return iterator.String_Iterator(
                eolian.eolian_type_namespaces_get(self))
        end,

        free_func_get = function(self)
            local v = eolian.eolian_type_free_func_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end
    }
})

M.function_type = {
    UNRESOLVED = 0,
    PROPERTY   = 1,
    PROP_SET   = 2,
    PROP_GET   = 3,
    METHOD     = 4,
    CTOR       = 5
}

M.Function = ffi.metatype("Eolian_Function", {
    __index = {
        type_get = function(self)
            return eolian.eolian_function_type_get(self)
        end,

        scope_get = function(self)
            return eolian.eolian_function_scope_get(self)
        end,

        name_get = function(self)
            local v = eolian.eolian_function_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        full_c_name_get = function(self, prefix)
            local v = eolian.eolian_function_full_c_name_get(self, prefix)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end,

        legacy_get = function(self, ftype)
            local v = eolian.eolian_function_legacy_get(self, ftype)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        description_get = function(self, ftype)
            local v = eolian.eolian_function_description_get(self, ftype)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        is_virtual_pure = function(self, ftype)
            return eolian.eolian_function_is_virtual_pure(self, ftype) ~= 0
        end,

        is_legacy_only = function(self, ftype)
            return eolian.eolian_function_is_legacy_only(self, ftype) ~= 0
        end,

        is_class = function(self)
            return eolian.eolian_function_is_class(self) ~= 0
        end,

        is_constructing = function(self)
            return eolian.eolian_function_is_constructing(self) ~= 0
        end,

        parameter_get_by_name = function(self, pname)
            local v = eolian.eolian_function_parameter_get_by_name(self, pname)
            if v == nil then return nil end
            return v
        end,

        property_keys_get = function(self)
            return Ptr_Iterator("const Eolian_Function_Parameter*",
                eolian.eolian_property_keys_get(self))
        end,

        property_values_get = function(self)
            return Ptr_Iterator("const Eolian_Function_Parameter*",
                eolian.eolian_property_values_get(self))
        end,

        parameters_get = function(self)
            return Ptr_Iterator("const Eolian_Function_Parameter*",
                eolian.eolian_function_parameters_get(self))
        end,

        return_type_get = function(self, ftype)
            local v = eolian.eolian_function_return_type_get(self, ftype)
            if v == nil then return nil end
            return v
        end,

        return_default_value_get = function(self, ftype)
            local v = eolian.eolian_function_return_default_value_get(self, ftype)
            if v == nil then return nil end
            return v
        end,

        return_comment_get = function(self, ftype)
            local v = eolian.eolian_function_return_comment_get(self, ftype)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        return_is_warn_unused = function(self, ftype)
            return eolian.eolian_function_return_is_warn_unused(self,
                ftype) ~= 0
        end,

        is_const = function(self)
            return eolian.eolian_function_object_is_const(self) ~= 0
        end
    }
})

M.parameter_dir = {
    IN    = 0,
    OUT   = 1,
    INOUT = 2
}

ffi.metatype("Eolian_Function_Parameter", {
    __index = {
        direction_get = function(self)
            return eolian.eolian_parameter_direction_get(self)
        end,

        type_get = function(self)
            local v = eolian.eolian_parameter_type_get(self)
            if v == nil then return nil end
            return v
        end,

        name_get = function(self)
            local v = eolian.eolian_parameter_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        description_get = function(self)
            local v = eolian.eolian_parameter_description_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        const_attribute_get = function(self, get)
            return eolian.eolian_parameter_const_attribute_get(self, get) ~= 0
        end,

        is_nonull = function(self)
            return eolian.eolian_parameter_is_nonull(self) ~= 0
        end
    }
})

ffi.metatype("Eolian_Implement", {
    __index = {
        full_name_get = function(self)
            local v = eolian.eolian_implement_full_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        class_get = function(self)
            local v = eolian.eolian_implement_class_get(self)
            if v == nil then return nil end
            return v
        end,

        function_get = function(self)
            local tp = ffi.new("Eolian_Function_Type[1]")
            local v = eolian.eolian_implement_class_get(self, tp)
            if v == nil then return nil end
            return v, tp[0]
        end
    }
})

ffi.metatype("Eolian_Event", {
    __index = {
        name_get = function(self)
            local v = eolian.eolian_event_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        type_get = function(self)
            local v = eolian.eolian_event_type_get(self)
            if v == nil then return nil end
            return v
        end,

        description_get = function(self)
            local v = eolian.eolian_event_description_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        scope_get = function(self)
            return eolian.eolian_event_scope_get(self)
        end
    }
})

M.class_get_by_name = function(cname)
    local v = eolian.eolian_class_get_by_name(cname)
    if v == nil then return nil end
    return v
end

M.class_get_by_file = function(fname)
    local v = eolian.eolian_class_get_by_file(fname)
    if v == nil then return nil end
    return v
end

M.all_classes_get = function()
    return Ptr_Iterator("const Eolian_Class*",
        eolian.eolain_all_classes_get())
end

M.class_type = {
    UNKNOWN   = 0,
    REGULAR   = 1,
    ABSTRACT  = 2,
    MIXIN     = 3,
    INTERFACE = 4
}

M.Class = ffi.metatype("Eolian_Class", {
    __index = {
        file_get = function(self)
            local v = eolian.eolian_class_file_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        full_name_get = function(self)
            local v = eolian.eolian_class_full_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        name_get = function(self)
            local v = eolian.eolian_class_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        namespaces_get = function(self)
            return iterator.String_Iterator(
                eolian.eolian_class_namespaces_get(self))
        end,

        type_get = function(self)
            return eolian.eolian_class_type_get(self)
        end,

        description_get = function(self)
            local v = eolian.eolian_class_description_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        legacy_prefix_get = function(self)
            local v = eolian.eolian_class_legacy_prefix_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        eo_prefix_get = function(self)
            local v = eolian.eolian_class_eo_prefix_get(self)
            if v == nil then
                local buf = self:namespaces_get()
                buf[#buf + 1] = self:name_get()
                return table.concat(buf, "_"):lower()
            end
            return ffi.string(v)
        end,

        data_type_get = function(self)
            local v = eolian.eolian_class_data_type_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        inherits_get = function(self)
            return iterator.String_Iterator(
                eolian.eolian_class_inherits_get(self))
        end,

        functions_get = function(self, func_type)
            return Ptr_Iterator("const Eolian_Function*",
                eolian.eolian_class_functions_get(self, func_type))
        end,

        function_get_by_name = function(self, fname, ftype)
            local v = eolian.eolian_class_function_get_by_name(self, fname,
                ftype)
            if v == nil then return nil end
            return v
        end,

        implements_get = function(self)
            return Ptr_Iterator("const Eolian_Implement*",
                eolian.eolian_class_implements_get(self))
        end,

        events_get = function(self)
            return Ptr_Iterator("const Eolian_Event*",
                eolian.eolian_class_events_get(self))
        end,

        ctor_enable_get = function(self)
            return eolian.eolian_class_ctor_enable_get(self) ~= 0
        end,

        dtor_enable_get = function(self)
            return eolian.eolian_class_dtor_enable_get(self) ~= 0
        end
    }
})

M.type_alias_get_by_name = function(name)
    local v = eolian.eolian_type_alias_get_by_name(name)
    if v == nil then return nil end
    return v
end

M.type_struct_get_by_name = function(name)
    local v = eolian.eolian_type_struct_get_by_name(name)
    if v == nil then return nil end
    return v
end

M.type_enum_get_by_name = function(name)
    local v = eolian.eolian_type_enum_get_by_name(name)
    if v == nil then return nil end
    return v
end

M.type_aliases_get_by_file = function(fname)
    return Ptr_Iterator("const Eolian_Type*",
        eolian.eolian_type_aliases_get_by_file(self))
end

M.type_structs_get_by_file = function(fname)
    return Ptr_Iterator("const Eolian_Type*",
        eolian.eolian_type_structs_get_by_file(self))
end

M.type_enums_get_by_file = function(fname)
    return Ptr_Iterator("const Eolian_Type*",
        eolian.eolian_type_enums_get_by_file(self))
end

M.expression_type = {
    UNKNOWN = 0,
    INT     = 1,
    UINT    = 2,
    LONG    = 3,
    ULONG   = 4,
    LLONG   = 5,
    ULLONG  = 6,
    FLOAT   = 7,
    DOUBLE  = 8,
    STRING  = 9,
    CHAR    = 10,
    NULL    = 11,
    BOOL    = 12,
    NAME    = 13,
    ENUM    = 14,
    UNARY   = 15,
    BINARY  = 16
}

local etype = M.expression_type

M.expression_mask = {
    SINT   = bit.lshift(1, 0),
    UINT   = bit.lshift(1, 1),
    FLOAT  = bit.lshift(1, 2),
    BOOL   = bit.lshift(1, 3),
    STRING = bit.lshift(1, 4),
    CHAR   = bit.lshift(1, 5),
    NULL   = bit.lshift(1, 6)
}

local emask = M.expression_mask

emask.INT    = bit.bor(emask.SINT  , emask.UINT )
emask.NUMBER = bit.bor(emask.INT   , emask.FLOAT)
emask.ALL    = bit.bor(emask.NUMBER, emask.BOOL,
                       emask.STRING, emask.CHAR, emask.NULL)

M.variable_type = {
    UNKNOWN  = 0,
    CONSTANT = 1,
    GLOBAL   = 2
}

local value_con = {
    [etype.INT   ] = function(v) return tonumber(v.value.i   ) end,
    [etype.UINT  ] = function(v) return tonumber(v.value.u   ) end,
    [etype.LONG  ] = function(v) return tonumber(v.value.l   ) end,
    [etype.ULONG ] = function(v) return tonumber(v.value.ul  ) end,
    [etype.LLONG ] = function(v) return tonumber(v.value.ll  ) end,
    [etype.ULLONG] = function(v) return tonumber(v.value.ull ) end,
    [etype.FLOAT ] = function(v) return tonumber(v.value.f   ) end,
    [etype.DOUBLE] = function(v) return tonumber(v.value.d   ) end,
    [etype.STRING] = function(v) return ffi.string(v.value.s ) end,
    [etype.CHAR  ] = function(v) return string.char(v.value.c) end,
    [etype.NULL  ] = function(v) return nil                    end,
    [etype.BOOL  ] = function(v) return v.value.b ~= 0         end
}

M.Value = ffi.metatype("Eolian_Value", {
    __index = {
        get_type = function(self)
            return ffi.cast("Eolian_Value_t*", self).type
        end,

        get_value = function(self)
            local   tp = self:get_type()
            local  fun = value_con[tonumber(tp)]
            if not fun then return nil end
            return fun()
        end,

        to_literal = function(self)
            local v = eolian.eolian_expression_value_to_literal(self)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end
    }
})

M.Expression = ffi.metatype("Eolian_Expression", {
    __index = {
        eval = function(self, mask)
            mask = mask or emask.ALL
            local v = eolian.eolian_expression_eval(self, mask)
            if v == nil then return nil end
            return ffi.cast("Eolian_Value*", v)
        end,

        eval_type = function(self, tp)
            local v = eolian.eolian_expression_eval_type(self, tp)
            if v == nil then return nil end
            return ffi.cast("Eolian_Value*", v)
        end,

        serialize = function(self)
            local v = eolian.eolian_expression_serialize(self)
            if v == nil then return nil end
            return ffi_stringshare(v)
        end
    }
})

M.variable_global_get_by_name = function(name)
    local v = eolian.eolian_variable_global_get_by_name(name)
    if v == nil then return nil end
    return v
end

M.variable_constant_get_by_name = function(name)
    local v = eolian.eolian_variable_constant_get_by_name(name)
    if v == nil then return nil end
    return v
end

M.variable_globals_get_by_file = function(fname)
    return Ptr_Iterator("const Eolian_Variable*",
        eolian.eolian_variable_globals_get_by_file(self))
end

M.variable_constants_get_by_file = function(fname)
    return Ptr_Iterator("const Eolian_Variable*",
        eolian.eolian_variable_constants_get_by_file(self))
end

M.Variable = ffi.metatype("Eolian_Variable", {
    __index = {
        type_get = function(self)
            return eolian.eolian_variable_type_get(self)
        end,

        description_get = function(self)
            local v = eolian.eolian_variable_description_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        file_get = function(self)
            local v = eolian.eolian_variable_file_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        base_type_get = function(self)
            local v = eolian.eolian_variable_base_type_get(self)
            if v == nil then return nil end
            return v
        end,

        value_get = function(self)
            local v = eolian.eolian_variable_value_get(self)
            if v == nil then return nil end
            return v
        end,

        name_get = function(self)
            local v = eolian.eolian_variable_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        full_name_get = function(self)
            local v = eolian.eolian_variable_full_name_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        namespaces_get = function(self)
            return iterator.String_Iterator(
                eolian.eolian_variable_namespaces_get(self))
        end,

        is_extern = function(self)
            return eolian.eolian_variable_is_extern(self) ~= 0
        end
    }
})

return M
