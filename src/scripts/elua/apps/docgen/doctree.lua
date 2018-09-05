local util = require("util")

local eolian = require("eolian")

local keyref = require("docgen.keyref")
local dutil = require("docgen.util")

-- writer has to be loaded late to prevent cycles
local writer

local M = {}
local eos = eolian:new()

local get_cache = function(o, nm)
    local ret = o[nm]
    if not ret then
        ret = {}
        o[nm] = ret
        return ret, false
    end
    return ret, true
end

local matches_filter = function(obj)
    local ns = obj:nspaces_get()
    if #ns and (ns[1] == "efl" or ns[1] == "eina") then
        return true
    end

    return false
end

M.Node = util.Object:clone {
    scope = {
        UNKNOWN = eolian.object_scope.UNKNOWN,
        PUBLIC = eolian.object_scope.PUBLIC,
        PRIVATE = eolian.object_scope.PRIVATE,
        PROTECTED = eolian.object_scope.PROTECTED
    },

    __ctor = function(self, obj)
        self._obj = obj
        assert(self._obj)
    end,

    name_get = function(self)
        return self._obj:name_get()
    end,

    short_name_get = function(self)
        return self._obj:short_name_get()
    end,

    namespaces_get = function(self)
        return self._obj:namespaces_get():to_array()
    end,

    nspaces_get = function(self, root)
        local tbl = self:namespaces_get()
        -- temporary workaround
        if type(tbl) ~= "table" then
            tbl = tbl:to_array()
        end
        for i = 1, #tbl do
            tbl[i] = tbl[i]:lower()
        end

        tbl[#tbl + 1] = self:short_name_get():lower()
        if root ~= nil then
            tbl[#tbl + 1] = not not root
        end
        return tbl
    end
}
local Node = M.Node

local gen_doc_refd = function(str)
    if not writer then
        writer = require("docgen.writer")
    end
    if not str then
        return nil
    end
    local pars = eolian.documentation_string_split(str)
    for i = 1, #pars do
        pars[i] = writer.Buffer():write_par(pars[i]):finish()
    end
    return table.concat(pars, "\n\n")
end

local add_since = function(str, since)
    if not writer then
        writer = require("docgen.writer")
    end
    if not since then
        return str
    end
    local buf = writer.Buffer()
    if not str then
        buf:write_i("Since " .. since)
        return buf:finish()
    end
    buf:write_raw(str)
    buf:write_nl(2)
    buf:write_i("Since " .. since)
    return buf:finish()
end

M.Doc = Node:clone {
    -- duplicate ctor to disable assertion
    __ctor = function(self, obj)
        self._obj = obj
    end,

    summary_get = function(self)
        if not self._obj then
            return nil
        end
        return self._obj:summary_get()
    end,

    description_get = function(self)
        if not self._obj then
            return nil
        end
        return self._obj:description_get()
    end,

    since_get = function(self)
        if not self._obj then
            return nil
        end
        return self._obj:since_get()
    end,

    brief_get = function(self, doc2)
        if not self._obj and (not doc2 or not doc2._obj) then
            return "No description supplied."
        end
        local doc1, doc2 = self._obj, doc2 and doc2._obj or nil
        if not doc1 then
            doc1, doc2 = doc2, doc1
        end
        return gen_doc_refd(doc1:summary_get())
    end,

    full_get = function(self, doc2, write_since)
        if not self._obj and (not doc2 or not doc2._obj) then
            return "No description supplied."
        end
        local doc1, doc2 = self._obj, doc2 and doc2._obj or nil
        if not doc1 then
            doc1, doc2 = doc2, doc1
        end
        local sum1 = doc1:summary_get()
        local desc1 = doc1:description_get()
        local edoc = ""
        local since
        if doc2 then
            local sum2 = doc2:summary_get()
            local desc2 = doc2:description_get()
            if not desc2 then
                if sum2 then edoc = "\n\n" .. sum2 end
            else
                edoc = "\n\n" .. sum2 .. "\n\n" .. desc2
            end
            if write_since then
                since = doc2:since_get()
            end
        end
        if not since and write_since then
            since = doc1:since_get()
        end
        if not desc1 then
            return add_since(gen_doc_refd(sum1 .. edoc), since)
        end
        return add_since(gen_doc_refd(sum1 .. "\n\n" .. desc1 .. edoc), since)
    end,

    exists = function(self)
        return not not self._obj
    end
}

local revh = {}

local class_type_str = {
    [eolian.class_type.REGULAR] = "class",
    [eolian.class_type.ABSTRACT] = "class",
    [eolian.class_type.MIXIN] = "mixin",
    [eolian.class_type.INTERFACE] = "interface"
}

M.Class = Node:clone {
    -- class types
    UNKNOWN = eolian.class_type.UNKNOWN,
    REGULAR = eolian.class_type.REGULAR,
    ABSTRACT = eolian.class_type.ABSTRACT,
    MIXIN = eolian.class_type.MIXIN,
    INTERFACE = eolian.class_type.INTERFACE,

    type_get = function(self)
        return self._obj:type_get()
    end,

    type_str_get = function(self)
        return class_type_str[self:type_get()]
    end,

    theme_str_get = function(self)
        return ({
            [eolian.class_type.REGULAR] = "regular",
            [eolian.class_type.ABSTRACT] = "abstract",
            [eolian.class_type.MIXIN] = "mixin",
            [eolian.class_type.INTERFACE] = "interface"
        })[self:type_get()]
    end,

    doc_get = function(self)
        return M.Doc(self._obj:documentation_get())
    end,

    legacy_prefix_get = function(self)
        return self._obj:legacy_prefix_get()
    end,

    eo_prefix_get = function(self)
        return self._obj:eo_prefix_get()
    end,

    inherits_get = function(self)
        local ret = self._cache_inhc
        if ret then
            return ret
        end
        ret = {}
        for cl in self._obj:inherits_get() do
            ret[#ret + 1] = M.Class(cl)
        end
        self._cache_inhc = ret
        return ret
    end,

    children_get = function(self)
        return revh[self:name_get()] or {}
    end,

    functions_get = function(self, ft)
        local ret = self._cache_funcs
        if ret then
            return ret
        end
        ret = {}
        self._cache_funcs = ret
        for fn in self._obj:functions_get(ft) do
            ret[#ret + 1] = M.Function(fn)
        end
        return ret
    end,

    function_by_name_get = function(self, fn, ft)
        local fun = self._cache_func
        if fun then
            return fun
        end
        fun = M.Function(self._obj:function_by_name_get(fn, ft))
        self._cache_func = fun
        return fun
    end,

    events_get = function(self)
        local ret = {}
        for ev in self._obj:events_get() do
            ret[#ret + 1] = M.Event(ev)
        end
        return ret
    end,

    implements_get = function(self)
        local ret = {}
        for impl in self._obj:implements_get() do
            ret[#ret + 1] = M.Implement(impl)
        end
        return ret
    end,

    c_get_function_name_get = function(self)
        return self._obj:c_get_function_name_get()
    end,

    nspaces_get = function(self, root)
        return M.Node.nspaces_get(self, root)
    end,

    is_same = function(self, other)
        return self._obj == other._obj
    end,

    -- static getters

    by_name_get = function(name)
        local stor = get_cache(M.Class, "_cache_bn")
        local ret = stor[name]
        if ret then
            return ret
        end
        local v = eos:class_by_name_get(name)
        if not v then
            return nil
        end
        ret = M.Class(v)
        stor[name] = ret
        return ret
    end,

    by_file_get = function(name)
        local stor = get_cache(M.Class, "_cache_bf")
        local ret = stor[name]
        if ret then
            return ret
        end
        local v = eos:class_by_file_get(name)
        if not v then
            return nil
        end
        ret = M.Class(v)
        stor[name] = ret
        return ret
    end,

    all_get = function()
        local ret, had = get_cache(M.Class, "_cache_all")
        if not had then
            -- FIXME: unit
            for cl in eos:classes_get() do
                local cls = M.Class(cl)
                if matches_filter(cls) then
                   ret[#ret + 1] = cls
                end
            end
        end
        return ret
    end
}

local func_type_str = {
    [eolian.function_type.PROPERTY] = "property",
    [eolian.function_type.PROP_GET] = "property",
    [eolian.function_type.PROP_SET] = "property",
    [eolian.function_type.METHOD] = "method"
}

local ffi = require("ffi")

M.Function = Node:clone {
    -- function types
    UNRESOLVED = eolian.function_type.UNRESOLVED,
    PROPERTY = eolian.function_type.PROPERTY,
    PROP_SET = eolian.function_type.PROP_SET,
    PROP_GET = eolian.function_type.PROP_GET,
    METHOD = eolian.function_type.METHOD,
    FUNCTION_POINTER = eolian.function_type.FUNCTION_POINTER,

    type_get = function(self)
        return self._obj:type_get()
    end,

    type_str_get = function(self)
        return func_type_str[self:type_get()]
    end,

    scope_get = function(self, ft)
        return self._obj:scope_get(ft)
    end,

    full_c_name_get = function(self, ft, legacy)
        return self._obj:full_c_name_get(ft, legacy)
    end,

    legacy_get = function(self, ft)
        return self._obj:legacy_get(ft)
    end,

    implement_get = function(self)
        return M.Implement(self._obj:implement_get())
    end,

    is_legacy_only = function(self, ft)
        return self._obj:is_legacy_only(ft)
    end,

    is_class = function(self)
        return self._obj:is_class()
    end,

    is_beta = function(self)
        return self._obj:is_beta()
    end,

    is_constructor = function(self, klass)
        return self._obj:is_constructor(klass.class)
    end,

    is_function_pointer = function(self)
        return self._obj:is_function_pointer()
    end,

    property_keys_get = function(self, ft)
        local ret = {}
        for par in self._obj:property_keys_get(ft) do
            ret[#ret + 1] = M.Parameter(par)
        end
        return ret
    end,

    property_values_get = function(self, ft)
        local ret = {}
        for par in self._obj:property_values_get(ft) do
            ret[#ret + 1] = M.Parameter(par)
        end
        return ret
    end,

    parameters_get = function(self)
        local ret = {}
        for par in self._obj:parameters_get() do
            ret[#ret + 1] = M.Parameter(par)
        end
        return ret
    end,

    return_type_get = function(self, ft)
        local v = self._obj:return_type_get(ft)
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    return_default_value_get = function(self, ft)
        local v = self._obj:return_default_value_get(ft)
        if not v then
            return nil
        end
        return M.Expression(v)
    end,

    return_doc_get = function(self, ft)
        return M.Doc(self._obj:return_documentation_get(ft))
    end,

    return_is_warn_unused = function(self, ft)
        return self._obj:return_is_warn_unused(ft)
    end,

    is_const = function(self)
        return self._obj:is_const()
    end,

    nspaces_get = function(self, cl, root)
        local tbl = cl:nspaces_get()
        tbl[#tbl + 1] = self:type_str_get()
        tbl[#tbl + 1] = self:name_get():lower()
        if root ~= nil then
            tbl[#tbl + 1] = not not root
        end
        return tbl
    end,

    is_same = function(self, other)
        return self._obj == other._obj
    end,

    id_get = function(self)
        return tonumber(ffi.cast("uintptr_t", self._obj))
    end
}

M.Parameter = Node:clone {
    UNKNOWN = eolian.parameter_dir.UNKNOWN,
    IN = eolian.parameter_dir.IN,
    OUT = eolian.parameter_dir.OUT,
    INOUT = eolian.parameter_dir.INOUT,

    direction_get = function(self)
        return self._obj:direction_get()
    end,

    direction_name_get = function(self)
        local dir_to_str = {
            [self.IN] = "in",
            [self.OUT] = "out",
            [self.INOUT] = "inout"
        }
        return assert(dir_to_str[self:direction_get()],
                      "unknown parameter direction")
    end,

    type_get = function(self)
        local v = self._obj:type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    default_value_get = function(self)
        local v = self._obj:default_value_get()
        if not v then
            return nil
        end
        return M.Expression(v)
    end,

    doc_get = function(self)
        return M.Doc(self._obj:documentation_get())
    end,

    is_nonull = function(self)
        return self._obj:is_nonull()
    end,

    is_nullable = function(self)
        return self._obj:is_nullable()
    end,

    is_optional = function(self)
        return self._obj:is_optional()
    end,

    is_same = function(self, other)
        return self._obj == other._obj
    end
}

M.Event = Node:clone {
    type_get = function(self)
        local v = self._obj:type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    doc_get = function(self)
        return M.Doc(self._obj:documentation_get())
    end,

    scope_get = function(self)
        return self._obj:scope_get()
    end,

    c_name_get = function(self)
        return self._obj:c_name_get()
    end,

    is_beta = function(self)
        return self._obj:is_beta()
    end,

    is_hot = function(self)
        return self._obj:is_hot()
    end,

    is_restart = function(self)
        return self._obj:is_restart()
    end,

    nspaces_get = function(self, cl, root)
        local tbl = cl:nspaces_get()
        tbl[#tbl + 1] = "event"
        tbl[#tbl + 1] = self:name_get():lower():gsub(",", "_")
        if root ~= nil then
            tbl[#tbl + 1] = not not root
        end
        return tbl
    end
}

M.StructField = Node:clone {
    doc_get = function(self)
        return M.Doc(self._obj:documentation_get())
    end,

    type_get = function(self)
        local v = self._obj:type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end
}

M.EnumField = Node:clone {
    c_name_get = function(self)
        return self._obj:c_name_get()
    end,

    doc_get = function(self)
        return M.Doc(self._obj:documentation_get())
    end,

    value_get = function(self, force)
        local v = self._obj:value_get(force)
        if not v then
            return nil
        end
        return M.Expression(v)
    end
}

local wrap_type_attrs = function(tp, str)
    if tp:is_const() then
        str = "const(" .. str .. ")"
    end
    -- TODO: implement new ownership system into docs
    --if tp:is_own() then
    --    str = "own(" .. str .. ")"
    --end
    local ffunc = tp:free_func_get()
    if ffunc then
        str = "free(" .. str .. ", " .. ffunc .. ")"
    end
    if tp:is_ptr() then
        str = "ptr(" .. str .. ")"
    end
    return str
end

M.Type = Node:clone {
    UNKNOWN = eolian.type_type.UNKNOWN,
    VOID = eolian.type_type.VOID,
    REGULAR = eolian.type_type.REGULAR,
    CLASS = eolian.type_type.CLASS,
    UNDEFINED = eolian.type_type.UNDEFINED,

    type_get = function(self)
        return self._obj:type_get()
    end,

    file_get = function(self)
        return self._obj:file_get()
    end,

    base_type_get = function(self)
        local v = self._obj:base_type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    next_type_get = function(self)
        local v = self._obj:next_type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    typedecl_get = function(self)
        local v = self._obj:typedecl_get()
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    aliased_base_get = function(self)
        local v = self._obj:aliased_base_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    class_get = function(self)
        return self._obj:class_get()
    end,

    is_owned = function(self)
        return self._obj:is_owned()
    end,

    is_const = function(self)
        return self._obj:is_const()
    end,

    is_ptr = function(self)
        return self._obj:is_ptr()
    end,

    c_type_get = function(self)
        return self._obj:c_type_get(eolian.c_type_type.DEFAULT)
    end,

    free_func_get = function(self)
        return self._obj:free_func_get()
    end,

    -- utils

    serialize = function(self)
        local tpt = self:type_get()
        if tpt == self.UNKNOWN then
            error("unknown type: " .. self:name_get())
        elseif tpt == self.VOID then
            return wrap_type_attrs(self, "void")
        elseif tpt == self.UNDEFINED then
            return wrap_type_attrs(self, "__undefined_type")
        elseif tpt == self.REGULAR or tpt == self.CLASS then
            local stp = self:base_type_get()
            if stp then
                local stypes = {}
                while stp do
                    stypes[#stypes + 1] = stp:serialize()
                    stp = stp:next_type_get()
                end
                return wrap_type_attrs(self, self:name_get() .. "<"
                    .. table.concat(stypes, ", ") .. ">")
            end
            return wrap_type_attrs(self, self:name_get())
        end
        error("unhandled type type: " .. tpt)
    end
}

M.type_cstr_get = function(tp, suffix)
    tp = tp or "void"
    local ct = (type(tp) == "string") and tp or tp:c_type_get()
    if not suffix then
        return ct
    end
    if ct:sub(#ct) == "*" then
        return ct .. suffix
    else
        return ct .. " " .. suffix
    end
end

local add_typedecl_attrs = function(tp, buf)
    if tp:is_extern() then
        buf[#buf + 1] = "@extern "
    end
    local ffunc = tp:free_func_get()
    if ffunc then
        buf[#buf + 1] = "@free("
        buf[#buf + 1] = ffunc
        buf[#buf + 1] = ") "
    end
end

M.Typedecl = Node:clone {
    UNKNOWN = eolian.typedecl_type.UNKNOWN,
    STRUCT = eolian.typedecl_type.STRUCT,
    STRUCT_OPAQUE = eolian.typedecl_type.STRUCT_OPAQUE,
    ENUM = eolian.typedecl_type.ENUM,
    ALIAS = eolian.typedecl_type.ALIAS,
    FUNCTION_POINTER = eolian.typedecl_type.FUNCTION_POINTER,

    type_get = function(self)
        return self._obj:type_get()
    end,

    type_str_get = function(self)
        local strs = {
            [eolian.typedecl_type.STRUCT] = "struct",
            [eolian.typedecl_type.STRUCT_OPAQUE] = "struct",
            [eolian.typedecl_type.ENUM] = "enum",
            [eolian.typedecl_type.ALIAS] = "alias"
        }
        return strs[self:type_get()]
    end,

    struct_fields_get = function(self)
        local ret = {}
        for fl in self._obj:struct_fields_get() do
            ret[#ret + 1] = M.StructField(fl)
        end
        return ret
    end,

    struct_field_get = function(self, name)
        local v = self._obj:struct_field_get(name)
        if not v then
            return nil
        end
        return M.StructField(v)
    end,

    enum_fields_get = function(self)
        local ret = {}
        for fl in self._obj:enum_fields_get() do
            ret[#ret + 1] = M.EnumField(fl)
        end
        return ret
    end,

    enum_field_get = function(self, name)
        local v = self._obj:enum_field_get(name)
        if not v then
            return nil
        end
        return M.EnumField(v)
    end,

    enum_legacy_prefix_get = function(self)
        return self._obj:enum_legacy_prefix_get()
    end,

    doc_get = function(self)
        return M.Doc(self._obj:documentation_get())
    end,

    file_get = function(self)
        return self._obj:file_get()
    end,

    base_type_get = function(self)
        local v = self._obj:base_type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    aliased_base_get = function(self)
        local v = self._obj:aliased_base_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    is_extern = function(self)
        return self._obj:is_extern()
    end,

    c_type_get = function(self)
        return self._obj:c_type_get()
    end,

    free_func_get = function(self)
        return self._obj:free_func_get()
    end,

    function_pointer_get = function(self)
        local v = self._obj:function_pointer_get()
        if not v then
            return nil
        end
        return M.Function(v)
    end,

    nspaces_get = function(self, root)
        return M.Node.nspaces_get(self, root)
    end,

    -- static getters

    all_aliases_get = function()
        local ret = {}
        for tp in eos:aliases_get() do
            local tpo = M.Typedecl(tp)
            if matches_filter(tpo) then
                ret[#ret + 1] = tpo
            end
        end
        return ret
    end,

    all_structs_get = function()
        local ret = {}
        for tp in eos:structs_get() do
            local tpo = M.Typedecl(tp)
            if matches_filter(tpo) then
                ret[#ret + 1] = tpo
            end
        end
        return ret
    end,

    all_enums_get = function()
        local ret = {}
        for tp in eos:enums_get() do
            local tpo = M.Typedecl(tp)
            local tpn = tpo:nspaces_get()
            if matches_filter(tpo) then
                ret[#ret + 1] = tpo
            end
        end
        return ret
    end,

    aliases_by_file_get = function(fn)
        local ret = {}
        for tp in eos:aliases_by_file_get(fn) do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    structs_by_file_get = function(fn)
        local ret = {}
        for tp in eos:struts_by_file_get(fn) do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    enums_by_file_get = function(fn)
        local ret = {}
        for tp in eeos:enums_by_file_get(fn) do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    alias_by_name_get = function(tn)
        local v = eos:alias_by_name_get(tn)
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    struct_by_name_get = function(tn)
        local v = eos:struct_by_name_get(tn)
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    enum_by_name_get = function(tn)
        local v = eos:enum_by_name_get(tn)
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    -- utils

    serialize = function(self)
        local tpt = self:type_get()
        if tpt == self.UNKNOWN then
            error("unknown typedecl: " .. self:name_get())
        elseif tpt == self.STRUCT or
               tpt == self.STRUCT_OPAQUE then
            local buf = { "struct " }
            add_typedecl_attrs(self, buf)
            buf[#buf + 1] = self:name_get()
            if tpt == self.STRUCT_OPAQUE then
                buf[#buf + 1] = ";"
                return table.concat(buf)
            end
            local fields = self:struct_fields_get()
            if #fields == 0 then
                buf[#buf + 1] = " {}"
                return table.concat(buf)
            end
            buf[#buf + 1] = " {\n"
            for i, fld in ipairs(fields) do
                buf[#buf + 1] = "    "
                buf[#buf + 1] = fld:name_get()
                buf[#buf + 1] = ": "
                buf[#buf + 1] = fld:type_get():serialize()
                buf[#buf + 1] = ";\n"
            end
            buf[#buf + 1] = "}"
            return table.concat(buf)
        elseif tpt == self.ENUM then
            local buf = { "enum " }
            add_typedecl_attrs(self, buf)
            buf[#buf + 1] = self:name_get()
            local fields = self:enum_fields_get()
            if #fields == 0 then
                buf[#buf + 1] = " {}"
                return table.concat(buf)
            end
            buf[#buf + 1] = " {\n"
            for i, fld in ipairs(fields) do
                buf[#buf + 1] = "    "
                buf[#buf + 1] = fld:name_get()
                local val = fld:value_get()
                if val then
                    buf[#buf + 1] = ": "
                    buf[#buf + 1] = val:serialize()
                end
                if i == #fields then
                    buf[#buf + 1] = "\n"
                else
                    buf[#buf + 1] = ",\n"
                end
            end
            buf[#buf + 1] = "}"
            return table.concat(buf)
        elseif tpt == self.ALIAS then
            local buf = { "type " }
            add_typedecl_attrs(self, buf)
            buf[#buf + 1] = self:name_get()
            buf[#buf + 1] = ": "
            buf[#buf + 1] = self:base_type_get():serialize()
            buf[#buf + 1] = ";"
            return table.concat(buf)
        elseif tpt == self.FUNCTION_POINTER then
            return "TODO"
        end
        error("unhandled typedecl type: " .. tpt)
    end,

    serialize_c = function(self, ns)
        local tpt = self:type_get()
        if tpt == self.UNKNOWN then
            error("unknown typedecl: " .. self:name_get())
        elseif tpt == self.STRUCT or
               tpt == self.STRUCT_OPAQUE then
            local buf = { "typedef struct " }
            local fulln = self:name_get():gsub("%.", "_");
            keyref.add(fulln, ns, "c")
            buf[#buf + 1] = "_" .. fulln;
            if tpt == self.STRUCT_OPAQUE then
                buf[#buf + 1] = " " .. fulln .. ";"
                return table.concat(buf)
            end
            local fields = self:struct_fields_get()
            if #fields == 0 then
                buf[#buf + 1] = " {} " .. fulln .. ";"
                return table.concat(buf)
            end
            buf[#buf + 1] = " {\n"
            for i, fld in ipairs(fields) do
                buf[#buf + 1] = "    "
                buf[#buf + 1] = M.type_cstr_get(fld:type_get(), fld:name_get())
                buf[#buf + 1] = ";\n"
            end
            buf[#buf + 1] = "} " .. fulln .. ";"
            return table.concat(buf)
        elseif tpt == self.ENUM then
            local buf = { "typedef enum" }
            local fulln = self:name_get():gsub("%.", "_");
            keyref.add(fulln, ns, "c")
            local fields = self:enum_fields_get()
            if #fields == 0 then
                buf[#buf + 1] = " {} " .. fulln .. ";"
                return table.concat(buf)
            end
            buf[#buf + 1] = " {\n"
            for i, fld in ipairs(fields) do
                buf[#buf + 1] = "    "
                local cn = fld:c_name_get()
                buf[#buf + 1] = cn
                keyref.add(cn, ns, "c")
                local val = fld:value_get()
                if val then
                    buf[#buf + 1] = " = "
                    local ev = val:eval_enum()
                    local lit = ev:to_literal()
                    buf[#buf + 1] = lit
                    local ser = val:serialize()
                    if ser and ser ~= lit then
                        buf[#buf + 1] = " /* " .. ser .. " */"
                    end
                end
                if i == #fields then
                    buf[#buf + 1] = "\n"
                else
                    buf[#buf + 1] = ",\n"
                end
            end
            buf[#buf + 1] = "} " .. fulln .. ";"
            return table.concat(buf)
        elseif tpt == self.ALIAS then
            local fulln = self:name_get():gsub("%.", "_");
            keyref.add(fulln, ns, "c")
            return "typedef "
                .. M.type_cstr_get(self:base_type_get(), fulln) .. ";"
        elseif tpt == self.FUNCTION_POINTER then
            return "TODO"
        end
        error("unhandled typedecl type: " .. tpt)
    end
}

M.Variable = Node:clone {
    UNKNOWN = eolian.variable_type.UNKNOWN,
    CONSTANT = eolian.variable_type.CONSTANT,
    GLOBAL = eolian.variable_type.GLOBAL,

    type_get = function(self)
        return self._obj:type_get()
    end,

    type_str_get = function(self)
        local strs = {
            [eolian.variable_type.CONSTANT] = "constant",
            [eolian.variable_type.GLOBAL] = "global"
        }
        return strs[self:type_get()]
    end,

    doc_get = function(self)
        return M.Doc(self._obj:documentation_get())
    end,

    file_get = function(self)
        return self._obj:file_get()
    end,

    base_type_get = function(self)
        local v = self._obj:base_type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    value_get = function(self)
        local v = self._obj:value_get()
        if not v then
            return nil
        end
        return M.Expression(v)
    end,

    is_extern = function(self)
        return self._obj:is_extern()
    end,

    nspaces_get = function(self, root)
        return M.Node.nspaces_get(self, root)
    end,

    serialize = function(self)
        local buf = {}
        if self:type_get() == self.GLOBAL then
            buf[#buf + 1] = "var "
        else
            buf[#buf + 1] = "const "
        end
        if self:is_extern() then
            buf[#buf + 1] = "@extern "
        end
        buf[#buf + 1] = self:name_get()
        buf[#buf + 1] = ": "
        buf[#buf + 1] = self:base_type_get():serialize()
        local val = self:value_get()
        if val then
            buf[#buf + 1] = " = "
            buf[#buf + 1] = val:serialize()
        end
        buf[#buf + 1] = ";"
        return table.concat(buf)
    end,

    serialize_c = function(self, ns)
        local buf = {}
        local bt = self:base_type_get()
        local fulln = self:name_get():gsub("%.", "_"):upper()
        keyref.add(fulln, ns, "c")
        if self:type_get() == self.GLOBAL then
            local ts = bt:c_type_get()
            buf[#buf + 1] = ts
            if ts:sub(#ts) ~= "*" then
                buf[#buf + 1] = " "
            end
            buf[#buf + 1] = fulln
            local val = self:value_get()
            if val then
                buf[#buf + 1] = " = "
                local vt = val:eval_type(bt)
                local lv = vt:to_literal()
                local sv = val:serialize()
                buf[#buf + 1] = lv
                if lv ~= sv then
                    buf[#buf + 1] = "/* " .. sv .. " */"
                end
            end
            buf[#buf + 1] = ";"
        else
            buf[#buf + 1] = "#define "
            buf[#buf + 1] = fulln
            buf[#buf + 1] = " "
            local val = self:value_get()
            local vt = val:eval_type(bt)
            local lv = vt:to_literal()
            local sv = val:serialize()
            buf[#buf + 1] = lv
            if lv ~= sv then
                buf[#buf + 1] = "/* " .. sv .. " */"
            end
        end
        return table.concat(buf)
    end,

    -- static getters

    all_globals_get = function()
        local ret = {}
        for v in eos:globals_get() do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    all_constants_get = function()
        local ret = {}
        for v in eos:constants_get() do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    globals_by_file_get = function(fn)
        local ret = {}
        for v in eos:globals_by_file_get(fn) do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    constants_by_file_get = function(fn)
        local ret = {}
        for v in eos:constants_by_file_get(fn) do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    global_by_name_get = function(vn)
        local v = eos:global_by_name_get(vn)
        if not v then
            return nil
        end
        return M.Variable(v)
    end,

    constant_by_name_get = function(vn)
        local v = eos:constant_by_name_get(vn)
        if not v then
            return nil
        end
        return M.Variable(v)
    end
}

M.Expression = Node:clone {
    eval_enum = function(self)
        return self._obj:eval(eolian.expression_mask.INT)
    end,

    eval_type = function(self, tp)
        return self._obj:eval_type(tp.type)
    end,

    serialize = function(self)
        return self._obj:serialize()
    end
}

M.Implement = Node:clone {
    class_get = function(self)
        local ccl = self._cache_cl
        if ccl then
            return ccl
        end
        -- so that we don't re-instantiate, it gets cached over there too
        ccl = M.Class.by_name_get(self._obj:class_get():name_get())
        self._cache_cl = ccl
        return ccl
    end,

    function_get = function(self)
        local func, tp = self._cache_func, self._cache_tp
        if func then
            return func, tp
        end
        func, tp = self._obj:function_get()
        func = M.Function(func)
        self._cache_func, self._cache_tp = func, tp
        return func, tp
    end,

    doc_get = function(self, ftype, inh)
        return M.Doc(self._obj:documentation_get(ftype))
    end,

    fallback_doc_get = function(self, inh)
        local ig, is = self:is_prop_get(), self:is_prop_set()
        if ig and not is then
            return self:doc_get(M.Function.PROP_GET, inh)
        elseif is and not ig then
            return self:doc_get(M.Function.PROP_SET, inh)
        end
        return nil
    end,

    is_auto = function(self, ftype)
        return self._obj:is_auto(ftype)
    end,

    is_empty = function(self, ftype)
        return self._obj:is_empty(ftype)
    end,

    is_pure_virtual = function(self, ftype)
        return self._obj:is_pure_virtual(ftype)
    end,

    is_prop_get = function(self)
        return self._obj:is_prop_get()
    end,

    is_prop_set = function(self)
        return self._obj:is_prop_set()
    end,

    is_overridden = function(self, cl)
        return cl.class ~= self._obj:class_get()
    end
}

M.DocTokenizer = util.Object:clone {
    UNKNOWN          = eolian.doc_token_type.UNKNOWN,
    TEXT             = eolian.doc_token_type.TEXT,
    REF              = eolian.doc_token_type.REF,
    MARK_NOTE        = eolian.doc_token_type.MARK_NOTE,
    MARK_WARNING     = eolian.doc_token_type.MARK_WARNING,
    MARK_REMARK      = eolian.doc_token_type.MARK_REMARK,
    MARK_TODO        = eolian.doc_token_type.MARK_TODO,
    MARKUP_MONOSPACE = eolian.doc_token_type.MARKUP_MONOSPACE,

    __ctor = function(self, str)
        self.tok = eolian.doc_token_init()
        self.str = str
        assert(self.str)
        assert(self.tok)
    end,

    tokenize = function(self)
       self.str = eolian.documentation_tokenize(self.str, self.tok)
       return not not self.str
    end,

    text_get = function(self)
        return self.tok:text_get()
    end,

    type_get = function(self)
        return self.tok:type_get()
    end,

    ref_resolve = function(self, root)
        -- FIXME: unit
        local tp, d1, d2 = self.tok:ref_resolve(eos)
        local reft = eolian.object_type
        local ret = {}
        if tp == reft.CLASS or tp == reft.FUNCTION or tp == reft.EVENT then
            if not class_type_str[d1:type_get()] then
                error("unknown class type for class '"
                      .. d1:name_get() .. "'")
            end
        elseif tp == reft.TYPEDECL then
        elseif tp == reft.ENUM_FIELD or tp == reft.STRUCT_FIELD then
            -- TODO: point to field
        elseif tp == reft.VARIABLE then
        else
            error("invalid reference '" .. self:text_get() .. "'")
        end
        for tok in d1:name_get():gmatch("[^%.]+") do
            ret[#ret + 1] = tok:lower()
        end
        if tp == reft.FUNCTION then
            ret[#ret + 1] = func_type_str[d2:type_get()]
            ret[#ret + 1] = d2:name_get():lower()
        elseif tp == reft.EVENT then
            ret[#ret + 1] = "event"
            ret[#ret + 1] = d2:name_get():lower()
        end
        if root ~= nil then
            ret[#ret + 1] = not not root
        end
        return ret
    end
}

M.scan_directory = function(dir)
    if not dir then
        if not eos:system_directory_add() then
            error("failed scanning system directory")
        end
        return
    end
    if not eos:directory_add(dir) then
        error("failed scanning directory: " .. dir)
    end
end

M.parse = function(st)
    if not eos:all_eot_files_parse() then
        error("failed parsing eo type files")
    end
    if st and st:match("%.") then
        if not eos:file_parse(st:gsub("%.", "_"):lower() .. ".eo") then
            error("failed parsing eo file")
        end
    else
        if not eos:all_eo_files_parse() then
            error("failed parsing eo files")
        end
    end
    -- build reverse inheritance hierarchy
    for cl in eos:classes_get() do
        local cln = cl:name_get()
        for icl in cl:inherits_get() do
            local t = revh[icl]
            if not t then
                t = {}
                revh[icl] = t
            end
            t[#t + 1] = M.Class.by_name_get(cl:name_get())
        end
    end
end

return M
