-- EFL LuaJIT bindings: Eolian
-- For use with Elua

local ffi = require("ffi")

ffi.cdef [[
    typedef unsigned char Eina_Bool;
    typedef struct _Eina_List Eina_List;
    typedef struct _Eina_Iterator Eina_Iterator;

    typedef struct _Eolian_Class Eolian_Class;
    typedef struct _Eolian_Function Eolian_Function;
    typedef struct _Eolian_Type Eolian_Type;
    typedef struct _Eolian_Function_Parameter Eolian_Function_Parameter;
    typedef struct _Eolian_Implement Eolian_Implement;
    typedef struct _Eolian_Event Eolian_Event;

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
    } Eolian_Function_Scope;

    typedef enum
    {
        EOLIAN_TYPE_UNKNOWN_TYPE,
        EOLIAN_TYPE_VOID,
        EOLIAN_TYPE_REGULAR,
        EOLIAN_TYPE_REGULAR_STRUCT,
        EOLIAN_TYPE_POINTER,
        EOLIAN_TYPE_FUNCTION,
        EOLIAN_TYPE_STRUCT
    } Eolian_Type_Type;

    Eina_Bool eolian_eo_file_parse(const char *filename);
    Eina_Bool eolian_eot_file_parse(const char *filepath);
    int eolian_init(void);
    int eolian_shutdown(void);
    Eina_Bool eolian_directory_scan(const char *dir);
    Eina_Bool eolian_system_directory_scan();
    Eina_Bool eolian_all_eo_files_parse();
    Eina_Bool eolian_all_eot_files_parse();
    Eina_Bool eolian_show_class(const Eolian_Class *klass);
    Eina_Bool eolian_show_typedef(const char *alias);
    Eina_Bool eolian_show_struct(const char *name);
    void eolian_show_all();
    const Eolian_Class *eolian_class_find_by_name(const char *class_name);
    const Eolian_Class *eolian_class_find_by_file(const char *file_name);
    const char *eolian_class_file_get(const Eolian_Class *klass);
    const char *eolian_class_full_name_get(const Eolian_Class *klass);
    const char *eolian_class_name_get(const Eolian_Class *klass);
    const Eina_List *eolian_class_namespaces_list_get(const Eolian_Class *klass);
    Eolian_Class_Type eolian_class_type_get(const Eolian_Class *klass);
    const Eina_List *eolian_all_classes_list_get(void);
    const char *eolian_class_description_get(const Eolian_Class *klass);
    const char *eolian_class_legacy_prefix_get(const Eolian_Class *klass);
    const char *eolian_class_eo_prefix_get(const Eolian_Class *klass);
    const char *eolian_class_data_type_get(const Eolian_Class *klass);
    const Eina_List *eolian_class_inherits_list_get(const Eolian_Class *klass);
    const Eina_List *eolian_class_functions_list_get(const Eolian_Class *klass, Eolian_Function_Type func_type);
    Eolian_Function_Type eolian_function_type_get(const Eolian_Function *function_id);
    Eolian_Function_Scope eolian_function_scope_get(const Eolian_Function *function_id);
    const char *eolian_function_name_get(const Eolian_Function *function_id);
    const char *eolian_function_full_c_name_get(const Eolian_Function *function_id, const char *prefix);
    const Eolian_Function *eolian_class_function_find_by_name(const Eolian_Class *klass, const char *func_name, Eolian_Function_Type f_type);
    const char *eolian_function_data_get(const Eolian_Function *function_id, const char *key);
    Eina_Bool eolian_function_is_virtual_pure(const Eolian_Function *function_id, Eolian_Function_Type f_type);
    const Eolian_Function_Parameter *eolian_function_parameter_get(const Eolian_Function *function_id, const char *param_name);
    const Eina_List *eolian_property_keys_list_get(const Eolian_Function *foo_id);
    const Eina_List *eolian_property_values_list_get(const Eolian_Function *foo_id);
    const Eina_List *eolian_parameters_list_get(const Eolian_Function *function_id);
    void eolian_parameter_information_get(const Eolian_Function_Parameter *param_desc, Eolian_Parameter_Dir *param_dir, const Eolian_Type **type, const char **name, const char **description);
    const Eolian_Type *eolian_parameter_type_get(const Eolian_Function_Parameter *param);
    const char *eolian_parameter_name_get(const Eolian_Function_Parameter *param);
    Eina_Bool eolian_parameter_const_attribute_get(const Eolian_Function_Parameter *param_desc, Eina_Bool is_get);
    Eina_Bool eolian_parameter_is_nonull(const Eolian_Function_Parameter *param_desc);
    const Eolian_Type *eolian_function_return_type_get(const Eolian_Function *function_id, Eolian_Function_Type ftype);
    const char *eolian_function_return_default_value_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    const char *eolian_function_return_comment_get(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_return_is_warn_unused(const Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_object_is_const(const Eolian_Function *function_id);
    const char *eolian_implement_full_name_get(const Eolian_Implement *impl);
    Eina_Bool eolian_implement_information_get(const Eolian_Implement *impl, const Eolian_Class *klass, const Eolian_Function *function, Eolian_Function_Type *type);
    const Eina_List *eolian_class_implements_list_get(const Eolian_Class *klass);
    const Eina_List *eolian_class_events_list_get(const Eolian_Class *klass);
    Eina_Bool eolian_class_event_information_get(const Eolian_Event *event, const char **event_name, const Eolian_Type **event_type, const char **event_desc);
    Eina_Bool eolian_class_ctor_enable_get(const Eolian_Class *klass);
    Eina_Bool eolian_class_dtor_enable_get(const Eolian_Class *klass);
    const Eolian_Type *eolian_type_find_by_alias(const char *alias);
    const Eolian_Type *eolian_type_struct_find_by_name(const char *name);
    Eolian_Type_Type eolian_type_type_get(const Eolian_Type *tp);
    Eina_Iterator *eolian_type_arguments_list_get(const Eolian_Type *tp);
    Eina_Iterator *eolian_type_subtypes_list_get(const Eolian_Type *tp);
    Eina_Iterator *eolian_type_struct_field_names_list_get(const Eolian_Type *tp);
    const Eolian_Type *eolian_type_struct_field_get(const Eolian_Type *tp, const char *field);
    const char *eolian_type_struct_field_description_get(const Eolian_Type *tp, const char *field);
    const char *eolian_type_struct_description_get(const Eolian_Type *tp);
    const Eolian_Type *eolian_type_return_type_get(const Eolian_Type *tp);
    const Eolian_Type *eolian_type_base_type_get(const Eolian_Type *tp);
    Eina_Bool eolian_type_is_own(const Eolian_Type *tp);
    Eina_Bool eolian_type_is_const(const Eolian_Type *tp);
    const char *eolian_type_c_type_named_get(const Eolian_Type *tp, const char *name);
    const char *eolian_type_c_type_get(const Eolian_Type *tp);
    const char *eolian_type_name_get(const Eolian_Type *tp);
]]

local cutil = require("cutil")
local util  = require("util")

local list = require("eina.list")
local iterator = require("eina.iterator")

local Ptr_List = list.Ptr_List
local Ptr_Iterator = iterator.Ptr_Iterator

local M = {}

local eolian

local init = function()
    eolian = util.lib_load("eolian")
    eolian.eolian_init()
end

local shutdown = function()
    eolian.eolian_shutdown()
    util.lib_unload("eolian")
end

cutil.init_module(init, shutdown)

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

M.show_all = function()
    eolian.eolian_show_all()
end

M.type_type = {
    UNKNOWN        = 0,
    VOID           = 1,
    REGULAR        = 2,
    REGULAR_STRUCT = 3,
    POINTER        = 4,
    FUNCTION       = 5,
    STRUCT         = 6
}

M.Type = ffi.metatype("Eolian_Type", {
    __index = {
        type_get = function(self)
            return eolian.eolian_type_type_get(self)
        end,

        arguments_list_get = function(self)
            return Ptr_Iterator("const Eolian_Type*",
                eolian.eolian_type_arguments_list_get(self))
        end,

        subtypes_list_get = function(self)
            return Ptr_Iterator("const Eolian_Type*",
                eolian.eolian_type_subtypes_list_get(self))
        end,

        struct_field_names_list_get = function(self)
            return iterator.String_Iterator(
                eolian.eolian_type_struct_field_names_list_get(self))
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

        struct_description_get = function(self, name)
            local v = eolian.eolian_type_struct_description_get(self)
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

        is_own = function(self)
            return eolian.eolian_type_is_own(self) ~= 0
        end,

        is_const = function(self)
            return eolian.eolian_type_is_const(self) ~= 0
        end,

        c_type_named_get = function(self, name)
            local v = eolian.eolian_type_c_type_named_get(self, name)
            if v == nil then return v end
            return ffi.string(v)
        end,

        c_type_get = function(self)
            local v = eolian.eolian_type_c_type_get(self)
            if v == nil then return v end
            return ffi.string(v)
        end,

        name_get = function(self)
            local v = eolian.eolian_type_name_get(self)
            if v == nil then return v end
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

M.function_scope = {
    PUBLIC    = 0,
    PROTECTED = 1
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
            return ffi.string(v)
        end,

        data_get = function(self, key)
            local v = eolian.eolian_function_data_get(self, key)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        description_get = function(self, key)
            return self:data_get(key)
        end,

        is_virtual_pure = function(self, ftype)
            return eolian.eolian_function_is_virtual_pure(self, ftype) ~= 0
        end,

        parameter_get = function(self, pname)
            local v = eolian.eolian_function_parameter_get(self, pname)
            if v == nil then return nil end
            return v
        end,

        property_keys_list_get = function(self)
            return Ptr_List("const Eolian_Function_Parameter*",
                eolian.eolian_property_keys_list_get(self)):to_array()
        end,

        property_values_list_get = function(self)
            return Ptr_List("const Eolian_Function_Parameter*",
                eolian.eolian_property_values_list_get(self)):to_array()
        end,

        parameters_list_get = function(self)
            return Ptr_List("const Eolian_Function_Parameter*",
                eolian.eolian_parameters_list_get(self)):to_array()
        end,

        return_type_get = function(self, ftype)
            local v = eolian.eolian_function_return_type_get(self, ftype)
            if v == nil then return nil end
            return v
        end,

        return_default_value_get = function(self, ftype)
            local v = eolian.eolian_function_return_default_value_get(self, ftype)
            if v == nil then return nil end
            return ffi.string(v)
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
        information_get = function(self)
            local dir = ffi.new("Eolian_Parameter_Dir[1]")
            local str = ffi.new("const char*[2]")
            local tp  = ffi.new("const Eolian_Type*[1]")
            eolian.eolian_parameter_information_get(self, dir, tp, str, str + 1)
            local tp, nm, dsc = tp[0], str[0], str[1]
            tp, nm, dsc = (tp  ~= nil) and tp or nil,
                          (nm  ~= nil) and ffi.string(nm ) or nil,
                          (dsc ~= nil) and ffi.string(dsc) or nil
            return dir[0], tp, nm, dsc
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

        information_get = function(self)
            local cl = ffi.new("const Eolian_Class*[1]")
            local fn = ffi.new("const Eolian_Function*[1]")
            local tp = ffi.new("Eolian_Function_Type[1]")
            eolian.eolian_implement_information_get(self, cl, fn, tp)
            return cl[0], fn[0], tp[0]
        end
    }
})

ffi.metatype("Eolian_Event", {
    __index = {
        information_get = function(self)
            local ev = ffi.new("const char*[2]")
            local tp = ffi.new("const Eolian_Type*[1]");
            eolian.eolian_class_event_information_get(self, ev, tp, ev + 1)
            local en, et, ed = ev[0], tp[0], ev[1]
            en, et, ed = (en ~= nil) and ffi.string(en) or nil,
                         (et ~= nil) and et or nil,
                         (ed ~= nil) and ffi.string(ed) or nil
            return en, et, ed
        end
    }
})

M.class_find_by_name = function(cname)
    local v = eolian.eolian_class_find_by_name(cname)
    if v == nil then return nil end
    return v
end

M.class_find_by_file = function(fname)
    local v = eolian.eolian_class_find_by_file(fname)
    if v == nil then return nil end
    return v
end

M.all_classes_list_get = function()
    return Ptr_List("const Eolian_Class*",
        eolian.eolain_all_classes_list_get()):to_array()
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

        namespaces_list_get = function(self)
            return list.String_List(eolian.eolian_class_namespaces_list_get(
                self)):to_array()
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
                local buf = self:namespaces_list_get()
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

        inherits_list_get = function(self)
            return list.String_List(eolian.eolian_class_inherits_list_get(
                self)):to_array()
        end,

        functions_list_get = function(self, func_type)
            return Ptr_List("const Eolian_Function*",
                eolian.eolian_class_functions_list_get(self, func_type))
                    :to_array()
        end,

        function_find_by_name = function(self, fname, ftype)
            local v = eolian.eolian_class_function_find_by_name(self, fname,
                ftype)
            if v == nil then return nil end
            return v
        end,

        implements_list_get = function(self)
            return Ptr_List("const Eolian_Implement*",
                eolian.eolian_class_implements_list_get(self)):to_array()
        end,

        events_list_get = function(self)
            return Ptr_List("const Eolian_Event*",
                eolian.eolian_class_events_list_get(self)):to_array()
        end,

        ctor_enable_get = function(self)
            return eolian.eolian_class_ctor_enable_get(self) ~= 0
        end,

        dtor_enable_get = function(self)
            return eolian.eolian_class_dtor_enable_get(self) ~= 0
        end
    }
})

M.type_find_by_alias = function(alias)
    local v = eolian.eolian_type_find_by_alias(alias)
    if v == nil then return nil end
    return v
end

M.type_struct_find_by_name = function(name)
    local v = eolian.eolian_type_struct_find_by_name(name)
    if v == nil then return nil end
    return v
end

return M
