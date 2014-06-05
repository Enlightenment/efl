-- EFL LuaJIT bindings: Eolian
-- For use with Elua

local ffi = require("ffi")

ffi.cdef [[
    typedef unsigned char Eina_Bool;
    typedef struct _Eina_List Eina_List;

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

    Eina_Bool eolian_eo_file_parse(const char *filename);
    int eolian_init(void);
    int eolian_shutdown(void);
    Eina_Bool eolian_directory_scan(const char *dir);
    Eina_Bool eolian_all_eo_files_parse();
    Eina_Bool eolian_show(const Eolian_Class *klass);
    Eolian_Class *eolian_class_find_by_name(const char *class_name);
    Eolian_Class *eolian_class_find_by_file(const char *file_name);
    const char *eolian_class_file_get(const Eolian_Class *klass);
    const char *eolian_class_full_name_get(const Eolian_Class *klass);
    const char *eolian_class_name_get(const Eolian_Class *klass);
    const Eina_List *eolian_class_namespaces_list_get(const Eolian_Class *klass);
    Eolian_Class_Type eolian_class_type_get(const Eolian_Class *klass);
    const Eina_List *eolian_class_names_list_get(void);
    const char *eolian_class_description_get(const Eolian_Class *klass);
    const char *eolian_class_legacy_prefix_get(const Eolian_Class *klass);
    const char *eolian_class_eo_prefix_get(const Eolian_Class *klass);
    const char *eolian_class_data_type_get(const Eolian_Class *klass);
    const Eina_List *eolian_class_inherits_list_get(const Eolian_Class *klass);
    const Eina_List *eolian_class_functions_list_get(const Eolian_Class *klass, Eolian_Function_Type func_type);
    Eolian_Function_Type eolian_function_type_get(Eolian_Function *function_id);
    Eolian_Function_Scope eolian_function_scope_get(Eolian_Function *function_id);
    const char *eolian_function_name_get(Eolian_Function *function_id);
    Eolian_Function *eolian_class_function_find_by_name(const Eolian_Class *klass, const char *func_name, Eolian_Function_Type f_type);
    const char *eolian_function_data_get(Eolian_Function *function_id, const char *key);
    Eina_Bool eolian_function_is_virtual_pure(Eolian_Function *function_id, Eolian_Function_Type f_type);
    Eolian_Function_Parameter *eolian_function_parameter_get(const Eolian_Function *function_id, const char *param_name);
    const Eina_List *eolian_property_keys_list_get(Eolian_Function *foo_id);
    const Eina_List *eolian_property_values_list_get(Eolian_Function *foo_id);
    const Eina_List *eolian_parameters_list_get(Eolian_Function *function_id);
    void eolian_parameter_information_get(const Eolian_Function_Parameter *param_desc, Eolian_Parameter_Dir *param_dir, const char **type, const char **name, const char **description);
    Eolian_Type *eolian_type_information_get(Eolian_Type *etype, const char **type, Eina_Bool *own);
    const char *eolian_parameter_type_get(const Eolian_Function_Parameter *param);
    Eolian_Type *eolian_parameter_types_list_get(const Eolian_Function_Parameter *param);
    const char *eolian_parameter_name_get(const Eolian_Function_Parameter *param);
    Eina_Bool eolian_parameter_const_attribute_get(Eolian_Function_Parameter *param_desc, Eina_Bool is_get);
    Eina_Bool eolian_parameter_is_nonull(Eolian_Function_Parameter *param_desc);
    const char *eolian_function_return_type_get(Eolian_Function *function_id, Eolian_Function_Type ftype);
    Eolian_Type *eolian_function_return_types_list_get(Eolian_Function *foo_id, Eolian_Function_Type ftype);
    const char *eolian_function_return_dflt_value_get(Eolian_Function *foo_id, Eolian_Function_Type ftype);
    const char *eolian_function_return_comment_get(Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_return_is_warn_unused(Eolian_Function *foo_id, Eolian_Function_Type ftype);
    Eina_Bool eolian_function_object_is_const(Eolian_Function *function_id);
    const char *eolian_implement_full_name_get(const Eolian_Implement *impl);
    Eina_Bool eolian_implement_information_get(const Eolian_Implement *impl, Eolian_Class *klass, Eolian_Function *function, Eolian_Function_Type *type);
    const Eina_List *eolian_class_implements_list_get(const Eolian_Class *klass);
    const Eina_List *eolian_class_events_list_get(const Eolian_Class *klass);
    Eina_Bool eolian_class_event_information_get(Eolian_Event *event, const char **event_name, const char **event_type, const char **event_desc);
    Eina_Bool eolian_class_ctor_enable_get(const Eolian_Class *klass);
    Eina_Bool eolian_class_dtor_enable_get(const Eolian_Class *klass);
    Eolian_Type *eolian_type_find_by_alias(const char *alias);
]]

local cutil = require("cutil")
local util  = require("util")

local list = require("eina.list")

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

M.all_eo_files_parse = function()
    return eolian.eolian_all_eo_files_parse() ~= 0
end

M.eo_file_parse = function(fname)
    return eolian.eolian_eo_file_parse(fname) ~= 0
end

M.show = function(klass)
    return eolian.eolian_show(klass) ~= 0
end

M.Type = ffi.metatype("Eolian_Type", {
    __index = {
        information_get = function(self)
            local tp = ffi.new("const char*[1]")
            local on = ffi.new("Eina_Bool[1]")
            local nx = eolian.eolian_type_information_get(self, tp, on)
            if nx == nil then nx = nil end
            return nx, (tp[0] ~= nil) and ffi.string(tp[0]) or nil, on[0] ~= 0
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

local List_Base = list.List_Base

local Eolian_Parameters_List = List_Base:clone {
    data_get = function(self, ptr)
        ptr = List_Base.data_get(self, ptr)
        return ffi.cast("Eolian_Function_Parameter*", ptr)
    end
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
            return Eolian_Parameters_List(
                eolian.eolian_property_keys_list_get(self)):to_array()
        end,

        property_values_list_get = function(self)
            return Eolian_Parameters_List(
                eolian.eolian_property_values_list_get(self)):to_array()
        end,

        parameters_list_get = function(self)
            return Eolian_Parameters_List(
                eolian.eolian_parameters_list_get(self)):to_array()
        end,

        return_type_get = function(self, ftype)
            local v = eolian.eolian_function_return_type_get(self, ftype)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        return_types_list_get = function(self, ftype)
            local v = eolian.eolian_function_return_types_list_get(self, ftype)
            if v == nil then return nil end
            return v
        end,

        return_dflt_value_get = function(self, ftype)
            local v = eolian.eolian_function_return_dflt_value_get(self, ftype)
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
            local str = ffi.new("const char*[3]")
            eolian.eolian_parameter_information_get(self, dir, str, str + 1,
                str + 2)
            local tp, nm, dsc = str[0], str[1], str[2]
            tp, nm, dsc = (tp  ~= nil) and ffi.string(tp ) or nil,
                          (nm  ~= nil) and ffi.string(nm ) or nil,
                          (dsc ~= nil) and ffi.string(dsc) or nil
            return dir[0], tp, nm, dsc
        end,

        type_get = function(self)
            local v = eolian.eolian_parameter_type_get(self)
            if v == nil then return nil end
            return ffi.string(v)
        end,

        types_list_get = function(self)
            local v = eolian.eolian_parameter_types_list_get(self)
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
            local cl = ffi.new("Eolian_Class[1]")
            local fn = ffi.new("Eolian_Function[1]")
            local tp = ffi.new("Eolian_Function_Type[1]")
            eolian.eolian_implement_information_get(self, cl, fn, tp)
            return cl[0], fn[0], tp[0]
        end
    }
})

ffi.metatype("Eolian_Event", {
    __index = {
        information_get = function(self)
            local ev = ffi.new("const char*[3]")
            eolian.eolian_class_event_information_get(self, ev, ev + 1, ev + 2)
            local en, et, ed = ev[0], ev[1], ev[2]
            en, et, ed = (en ~= nil) and ffi.string(en) or nil,
                         (et ~= nil) and ffi.string(et) or nil,
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

M.class_names_list_get = function()
    return list.String_List(eolian.eolian_class_names_list_get()):to_array()
end

M.class_type = {
    UNKNOWN   = 0,
    REGULAR   = 1,
    ABSTRACT  = 2,
    MIXIN     = 3,
    INTERFACE = 4
}

local Eolian_Functions_List = List_Base:clone {
    data_get = function(self, ptr)
        ptr = List_Base.data_get(self, ptr)
        return ffi.cast("Eolian_Function*", ptr)
    end
}

local Eolian_Implements_List = List_Base:clone {
    data_get = function(self, ptr)
        ptr = List_Base.data_get(self, ptr)
        return ffi.cast("Eolian_Implement*", ptr)
    end
}

local Eolian_Events_List = List_Base:clone {
    data_get = function(self, ptr)
        ptr = List_Base.data_get(self, ptr)
        return ffi.cast("Eolian_Event*", ptr)
    end
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
            if v == nil then return nil end
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
            return Eolian_Functions_List(
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
            return Eolian_Implements_List(
                eolian.eolian_class_implements_list_get(self)):to_array()
        end,

        events_list_get = function(self)
            return Eolian_Events_List(
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
    return eolian.eolian_type_find_by_alias(alias)
end

return M