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

    nspaces_get = function(self, root)
        local tbl = self:namespaces_get()
        -- temporary workaround
        if type(tbl) ~= "table" then
            tbl = tbl:to_array()
        end
        for i = 1, #tbl do
            tbl[i] = tbl[i]:lower()
        end

        tbl[#tbl + 1] = self:name_get():lower()
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
    __ctor = function(self, doc)
        self.doc = doc
    end,

    summary_get = function(self)
        if not self.doc then
            return nil
        end
        return self.doc:summary_get()
    end,

    description_get = function(self)
        if not self.doc then
            return nil
        end
        return self.doc:description_get()
    end,

    since_get = function(self)
        if not self.doc then
            return nil
        end
        return self.doc:since_get()
    end,

    brief_get = function(self, doc2)
        if not self.doc and (not doc2 or not doc2.doc) then
            return "No description supplied."
        end
        local doc1, doc2 = self.doc, doc2 and doc2.doc or nil
        if not doc1 then
            doc1, doc2 = doc2, doc1
        end
        return gen_doc_refd(doc1:summary_get())
    end,

    full_get = function(self, doc2, write_since)
        if not self.doc and (not doc2 or not doc2.doc) then
            return "No description supplied."
        end
        local doc1, doc2 = self.doc, doc2 and doc2.doc or nil
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
        return not not self.doc
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

    __ctor = function(self, cl)
        self.class = cl
        assert(self.class)
    end,

    full_name_get = function(self)
        return self.class:full_name_get()
    end,

    name_get = function(self)
        return self.class:name_get()
    end,

    namespaces_get = function(self)
        return self.class:namespaces_get():to_array()
    end,

    type_get = function(self)
        return self.class:type_get()
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
        return M.Doc(self.class:documentation_get())
    end,

    legacy_prefix_get = function(self)
        return self.class:legacy_prefix_get()
    end,

    eo_prefix_get = function(self)
        return self.class:eo_prefix_get()
    end,

    inherits_get = function(self)
        local ret = self._cache_inhc
        if ret then
            return ret
        end
        ret = {}
        for cl in self.class:inherits_get() do
            ret[#ret + 1] = M.Class(cl)
        end
        self._cache_inhc = ret
        return ret
    end,

    children_get = function(self)
        return revh[self:full_name_get()] or {}
    end,

    functions_get = function(self, ft)
        local ret = self._cache_funcs
        if ret then
            return ret
        end
        ret = {}
        self._cache_funcs = ret
        for fn in self.class:functions_get(ft) do
            ret[#ret + 1] = M.Function(fn)
        end
        return ret
    end,

    function_get_by_name = function(self, fn, ft)
        local fun = self._cache_func
        if fun then
            return fun
        end
        fun = M.Function(self.class:function_get_by_name(fn, ft))
        self._cache_func = fun
        return fun
    end,

    events_get = function(self)
        local ret = {}
        for ev in self.class:events_get() do
            ret[#ret + 1] = M.Event(ev)
        end
        return ret
    end,

    implements_get = function(self)
        local ret = {}
        for impl in self.class:implements_get() do
            ret[#ret + 1] = M.Implement(impl)
        end
        return ret
    end,

    c_get_function_name_get = function(self)
        return self.class:c_get_function_name_get()
    end,

    nspaces_get = function(self, root)
        return M.Node.nspaces_get(self, root)
    end,

    is_same = function(self, other)
        return self.class == other.class
    end,

    -- static getters

    by_name_get = function(name)
        local stor = get_cache(M.Class, "_cache_bn")
        local ret = stor[name]
        if ret then
            return ret
        end
        -- FIXME: unit
        local v = eolian.class_get_by_name(eos:unit_get(), name)
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
        -- FIXME: unit
        local v = eolian.class_get_by_file(eos:unit_get(), name)
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
            for cl in eolian.all_classes_get(eos:unit_get()) do
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

    __ctor = function(self, fn)
        self.func = fn
        assert(self.func)
    end,

    name_get = function(self)
        return self.func:name_get()
    end,

    type_get = function(self)
        return self.func:type_get()
    end,

    type_str_get = function(self)
        return func_type_str[self:type_get()]
    end,

    scope_get = function(self, ft)
        return self.func:scope_get(ft)
    end,

    full_c_name_get = function(self, ft, legacy)
        return self.func:full_c_name_get(ft, legacy)
    end,

    legacy_get = function(self, ft)
        return self.func:legacy_get(ft)
    end,

    implement_get = function(self)
        return M.Implement(self.func:implement_get())
    end,

    is_legacy_only = function(self, ft)
        return self.func:is_legacy_only(ft)
    end,

    is_class = function(self)
        return self.func:is_class()
    end,

    is_beta = function(self)
        return self.func:is_beta()
    end,

    is_constructor = function(self, klass)
        return self.func:is_constructor(klass.class)
    end,

    is_function_pointer = function(self)
        return self.func:is_function_pointer()
    end,

    property_keys_get = function(self, ft)
        local ret = {}
        for par in self.func:property_keys_get(ft) do
            ret[#ret + 1] = M.Parameter(par)
        end
        return ret
    end,

    property_values_get = function(self, ft)
        local ret = {}
        for par in self.func:property_values_get(ft) do
            ret[#ret + 1] = M.Parameter(par)
        end
        return ret
    end,

    parameters_get = function(self)
        local ret = {}
        for par in self.func:parameters_get() do
            ret[#ret + 1] = M.Parameter(par)
        end
        return ret
    end,

    return_type_get = function(self, ft)
        local v = self.func:return_type_get(ft)
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    return_default_value_get = function(self, ft)
        local v = self.func:return_default_value_get(ft)
        if not v then
            return nil
        end
        return M.Expression(v)
    end,

    return_doc_get = function(self, ft)
        return M.Doc(self.func:return_documentation_get(ft))
    end,

    return_is_warn_unused = function(self, ft)
        return self.func:return_is_warn_unused(ft)
    end,

    is_const = function(self)
        return self.func:is_const()
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
        return self.func == other.func
    end,

    id_get = function(self)
        return tonumber(ffi.cast("uintptr_t", self.func))
    end
}

M.Parameter = Node:clone {
    UNKNOWN = eolian.parameter_dir.UNKNOWN,
    IN = eolian.parameter_dir.IN,
    OUT = eolian.parameter_dir.OUT,
    INOUT = eolian.parameter_dir.INOUT,

    __ctor = function(self, par)
        self.param = par
        assert(self.param)
    end,

    direction_get = function(self)
        return self.param:direction_get()
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
        local v = self.param:type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    default_value_get = function(self)
        local v = self.param:default_value_get()
        if not v then
            return nil
        end
        return M.Expression(v)
    end,

    name_get = function(self)
        return self.param:name_get()
    end,

    doc_get = function(self)
        return M.Doc(self.param:documentation_get())
    end,

    is_nonull = function(self)
        return self.param:is_nonull()
    end,

    is_nullable = function(self)
        return self.param:is_nullable()
    end,

    is_optional = function(self)
        return self.param:is_optional()
    end,

    is_same = function(self, other)
        return self.param == other.param
    end
}

M.Event = Node:clone {
    __ctor = function(self, ev)
        self.event = ev
        assert(self.event)
    end,

    name_get = function(self)
        return self.event:name_get()
    end,

    type_get = function(self)
        local v = self.event:type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    doc_get = function(self)
        return M.Doc(self.event:documentation_get())
    end,

    scope_get = function(self)
        return self.event:scope_get()
    end,

    c_name_get = function(self)
        return self.event:c_name_get()
    end,

    is_beta = function(self)
        return self.event:is_beta()
    end,

    is_hot = function(self)
        return self.event:is_hot()
    end,

    is_restart = function(self)
        return self.event:is_restart()
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
    __ctor = function(self, fl)
        self.field = fl
        assert(self.field)
    end,

    name_get = function(self)
        return self.field:name_get()
    end,

    doc_get = function(self)
        return M.Doc(self.field:documentation_get())
    end,

    type_get = function(self)
        local v = self.field:type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end
}

M.EnumField = Node:clone {
    __ctor = function(self, fl)
        self.field = fl
        assert(self.field)
    end,

    name_get = function(self)
        return self.field:name_get()
    end,

    c_name_get = function(self)
        return self.field:c_name_get()
    end,

    doc_get = function(self)
        return M.Doc(self.field:documentation_get())
    end,

    value_get = function(self, force)
        local v = self.field:value_get(force)
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

    __ctor = function(self, tp)
        self.type = tp
        assert(self.type)
    end,

    type_get = function(self)
        return self.type:type_get()
    end,

    file_get = function(self)
        return self.type:file_get()
    end,

    base_type_get = function(self)
        local v = self.type:base_type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    next_type_get = function(self)
        local v = self.type:next_type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    typedecl_get = function(self)
        local v = self.type:typedecl_get()
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    aliased_base_get = function(self)
        local v = self.type:aliased_base_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    class_get = function(self)
        -- FIXME: unit
        return self.type:class_get(eos:unit_get())
    end,

    is_owned = function(self)
        return self.type:is_owned()
    end,

    is_const = function(self)
        return self.type:is_const()
    end,

    is_ptr = function(self)
        return self.type:is_ptr()
    end,

    c_type_get = function(self)
        return self.type:c_type_get(eolian.c_type_type.DEFAULT)
    end,

    name_get = function(self)
        return self.type:name_get()
    end,

    full_name_get = function(self)
        return self.type:full_name_get()
    end,

    namespaces_get = function(self)
        return self.type:namespaces_get()
    end,

    free_func_get = function(self)
        return self.type:free_func_get()
    end,

    -- utils

    serialize = function(self)
        local tpt = self:type_get()
        if tpt == self.UNKNOWN then
            error("unknown type: " .. self:full_name_get())
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
                return wrap_type_attrs(self, self:full_name_get() .. "<"
                    .. table.concat(stypes, ", ") .. ">")
            end
            return wrap_type_attrs(self, self:full_name_get())
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

    __ctor = function(self, tp)
        self.typedecl = tp
        assert(self.typedecl)
    end,

    type_get = function(self)
        return self.typedecl:type_get()
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
        for fl in self.typedecl:struct_fields_get() do
            ret[#ret + 1] = M.StructField(fl)
        end
        return ret
    end,

    struct_field_get = function(self, name)
        local v = self.typedecl:struct_field_get(name)
        if not v then
            return nil
        end
        return M.StructField(v)
    end,

    enum_fields_get = function(self)
        local ret = {}
        for fl in self.typedecl:enum_fields_get() do
            ret[#ret + 1] = M.EnumField(fl)
        end
        return ret
    end,

    enum_field_get = function(self, name)
        local v = self.typedecl:enum_field_get(name)
        if not v then
            return nil
        end
        return M.EnumField(v)
    end,

    enum_legacy_prefix_get = function(self)
        return self.typedecl:enum_legacy_prefix_get()
    end,

    doc_get = function(self)
        return M.Doc(self.typedecl:documentation_get())
    end,

    file_get = function(self)
        return self.typedecl:file_get()
    end,

    base_type_get = function(self)
        local v = self.typedecl:base_type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    aliased_base_get = function(self)
        local v = self.typedecl:aliased_base_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    is_extern = function(self)
        return self.typedecl:is_extern()
    end,

    c_type_get = function(self)
        return self.typedecl:c_type_get()
    end,

    name_get = function(self)
        return self.typedecl:name_get()
    end,

    full_name_get = function(self)
        return self.typedecl:full_name_get()
    end,

    namespaces_get = function(self)
        return self.typedecl:namespaces_get():to_array()
    end,

    free_func_get = function(self)
        return self.typedecl:free_func_get()
    end,

    function_pointer_get = function(self)
        local v = self.typedecl:function_pointer_get()
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
        -- FIXME: unit
        for tp in eolian.typedecl_all_aliases_get(eos:unit_get()) do
            local tpo = M.Typedecl(tp)
            if matches_filter(tpo) then
                ret[#ret + 1] = tpo
            end
        end
        return ret
    end,

    all_structs_get = function()
        local ret = {}
        -- FIXME: unit
        for tp in eolian.typedecl_all_structs_get(eos:unit_get()) do
            local tpo = M.Typedecl(tp)
            if matches_filter(tpo) then
                ret[#ret + 1] = tpo
            end
        end
        return ret
    end,

    all_enums_get = function()
        local ret = {}
        -- FIXME: unit
        for tp in eolian.typedecl_all_enums_get(eos:unit_get()) do
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
        -- FIXME: unit
        for tp in eolian.typedecl_aliases_get_by_file(eos:unit_get(), fn) do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    structs_by_file_get = function(fn)
        local ret = {}
        -- FIXME: unit
        for tp in eolian.typedecl_structs_get_by_file(eos:unit_get(), fn) do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    enums_by_file_get = function(fn)
        local ret = {}
        -- FIXME: unit
        for tp in eolian.typedecl_enums_get_by_file(eos:unit_get(), fn) do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    alias_by_name_get = function(tn)
        -- FIXME: unit
        local v = eolian.typedecl_alias_get_by_name(eos:unit_get(), tn)
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    struct_by_name_get = function(tn)
        -- FIXME: unit
        local v = eolian.typedecl_struct_get_by_name(eos:unit_get(), tn)
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    enum_by_name_get = function(tn)
        -- FIXME: unit
        local v = eolian.typedecl_enum_get_by_name(eos:unit_get(), tn)
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    -- utils

    serialize = function(self)
        local tpt = self:type_get()
        if tpt == self.UNKNOWN then
            error("unknown typedecl: " .. self:full_name_get())
        elseif tpt == self.STRUCT or
               tpt == self.STRUCT_OPAQUE then
            local buf = { "struct " }
            add_typedecl_attrs(self, buf)
            buf[#buf + 1] = self:full_name_get()
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
            buf[#buf + 1] = self:full_name_get()
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
            buf[#buf + 1] = self:full_name_get()
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
            error("unknown typedecl: " .. self:full_name_get())
        elseif tpt == self.STRUCT or
               tpt == self.STRUCT_OPAQUE then
            local buf = { "typedef struct " }
            local fulln = self:full_name_get():gsub("%.", "_");
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
            local fulln = self:full_name_get():gsub("%.", "_");
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
            local fulln = self:full_name_get():gsub("%.", "_");
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

    __ctor = function(self, var)
        self.variable = var
        assert(self.variable)
    end,

    type_get = function(self)
        return self.variable:type_get()
    end,

    type_str_get = function(self)
        local strs = {
            [eolian.variable_type.CONSTANT] = "constant",
            [eolian.variable_type.GLOBAL] = "global"
        }
        return strs[self:type_get()]
    end,

    doc_get = function(self)
        return M.Doc(self.variable:documentation_get())
    end,

    file_get = function(self)
        return self.variable:file_get()
    end,

    base_type_get = function(self)
        local v = self.variable:base_type_get()
        if not v then
            return nil
        end
        return M.Type(v)
    end,

    value_get = function(self)
        local v = self.variable:value_get()
        if not v then
            return nil
        end
        return M.Expression(v)
    end,

    name_get = function(self)
        return self.variable:name_get()
    end,

    full_name_get = function(self)
        return self.variable:full_name_get()
    end,

    namespaces_get = function(self)
        return self.variable:namespaces_get():to_array()
    end,

    is_extern = function(self)
        return self.variable:is_extern()
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
        buf[#buf + 1] = self:full_name_get()
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
        local fulln = self:full_name_get():gsub("%.", "_"):upper()
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
        -- FIXME: unit
        for v in eolian.variable_all_globals_get(eos:unit_get()) do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    all_constants_get = function()
        local ret = {}
        -- FIXME: unit
        for v in eolian.variable_all_constants_get(eos:unit_get()) do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    globals_by_file_get = function(fn)
        local ret = {}
        -- FIXME: unit
        for v in eolian.variable_globals_get_by_file(eos:unit_get(), fn) do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    constants_by_file_get = function(fn)
        local ret = {}
        -- FIXME: unit
        for v in eolian.variable_constants_get_by_file(eos:unit_get(), fn) do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    global_by_name_get = function(vn)
        -- FIXME: unit
        local v = eolian.variable_global_get_by_name(eos:unit_get(), vn)
        if not v then
            return nil
        end
        return M.Variable(v)
    end,

    constant_by_name_get = function(vn)
        -- FIXME: unit
        local v = eolian.variable_constant_get_by_name(eos:unit_get(), vn)
        if not v then
            return nil
        end
        return M.Variable(v)
    end
}

M.Expression = Node:clone {
    __ctor = function(self, expr)
        self.expr = expr
        assert(self.expr)
    end,

    eval_enum = function(self)
        return self.expr:eval(eolian.expression_mask.INT)
    end,

    eval_type = function(self, tp)
        return self.expr:eval_type(tp.type)
    end,

    serialize = function(self)
        return self.expr:serialize()
    end
}

M.Implement = Node:clone {
    __ctor = function(self, impl)
        self.impl = impl
        assert(self.impl)
    end,

    full_name_get = function(self)
        return self.impl:full_name_get()
    end,

    class_get = function(self)
        local ccl = self._cache_cl
        if ccl then
            return ccl
        end
        -- so that we don't re-instantiate, it gets cached over there too
        ccl = M.Class.by_name_get(self.impl:class_get():full_name_get())
        self._cache_cl = ccl
        return ccl
    end,

    function_get = function(self)
        local func, tp = self._cache_func, self._cache_tp
        if func then
            return func, tp
        end
        func, tp = self.impl:function_get()
        func = M.Function(func)
        self._cache_func, self._cache_tp = func, tp
        return func, tp
    end,

    doc_get = function(self, ftype, inh)
        return M.Doc(self.impl:documentation_get(ftype))
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
        return self.impl:is_auto(ftype)
    end,

    is_empty = function(self, ftype)
        return self.impl:is_empty(ftype)
    end,

    is_pure_virtual = function(self, ftype)
        return self.impl:is_pure_virtual(ftype)
    end,

    is_prop_get = function(self)
        return self.impl:is_prop_get()
    end,

    is_prop_set = function(self)
        return self.impl:is_prop_set()
    end,

    is_overridden = function(self, cl)
        return cl.class ~= self.impl:class_get()
    end
}

M.DocTokenizer = Node:clone {
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

    ref_get = function(self, root)
        -- FIXME: unit
        local tp, d1, d2 = self.tok:ref_get(eos:unit_get())
        local reft = eolian.doc_ref_type
        local ret = {}
        if tp == reft.CLASS or tp == reft.FUNC or tp == reft.EVENT then
            if not class_type_str[d1:type_get()] then
                error("unknown class type for class '"
                      .. d1:full_name_get() .. "'")
            end
        elseif tp == reft.ALIAS then
        elseif tp == reft.STRUCT or tp == reft.STRUCT_FIELD then
            -- TODO: point to field
        elseif tp == reft.ENUM or tp == reft.ENUM_FIELD  then
            -- TODO: point to field
        elseif tp == reft.VAR then
        else
            error("invalid reference '" .. self:text_get() .. "'")
        end
        for tok in d1:full_name_get():gmatch("[^%.]+") do
            ret[#ret + 1] = tok:lower()
        end
        if tp == reft.FUNC then
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
        if not eos:system_directory_scan() then
            error("failed scanning system directory")
        end
        return
    end
    if not eos:directory_scan(dir) then
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
    -- build reverse inheritance hierarchy, FIXME: unit
    for cl in eolian.all_classes_get(eos:unit_get()) do
        local cln = cl:full_name_get()
        for icl in cl:inherits_get() do
            local t = revh[icl]
            if not t then
                t = {}
                revh[icl] = t
            end
            t[#t + 1] = M.Class.by_name_get(cl:full_name_get())
        end
    end
end

return M
