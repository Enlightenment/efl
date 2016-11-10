local util = require("util")

local eolian = require("eolian")

local keyref = require("docgen.keyref")
local dutil = require("docgen.util")

-- writer has to be loaded late to prevent cycles
local writer

local M = {}

M.Node = util.Object:clone {
    scope = {
        PUBLIC = eolian.object_scope.PUBLIC,
        PRIVATE = eolian.object_scope.PRIVATE,
        PROTECTED = eolian.object_scope.PROTECTED
    },

    nspaces_get = function(self, subn, root)
        local tbl = self:namespaces_get()
        -- temporary workaround
        if type(tbl) ~= "table" then
            tbl = tbl:to_array()
        end
        for i = 1, #tbl do
            tbl[i] = tbl[i]:lower()
        end
        table.insert(tbl, 1, subn)
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
    local pars = dutil.str_split(str, "\n\n")
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
        return ({
            [eolian.class_type.REGULAR] = "class",
            [eolian.class_type.ABSTRACT] = "class",
            [eolian.class_type.MIXIN] = "mixin",
            [eolian.class_type.INTERFACE] = "interface"
        })[self:type_get()]
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
        return self.class:inherits_get():to_array()
    end,

    functions_get = function(self, ft)
        local ret = {}
        for fn in self.class:functions_get(ft) do
            ret[#ret + 1] = M.Function(fn)
        end
        return ret
    end,

    function_get_by_name = function(self, fn, ft)
        return M.Function(self.class:function_get_by_name(fn, ft))
    end,

    events_get = function(self)
        local ret = {}
        for ev in self.class:events_get() do
            ret[#ret + 1] = M.Event(ev)
        end
        return ret
    end,

    c_get_function_name_get = function(self)
        return self.class:c_get_function_name_get()
    end,

    nspaces_get = function(self, root)
        return M.Node.nspaces_get(self, self:type_str_get(), root)
    end,

    -- static getters

    by_name_get = function(name)
        local v = eolian.class_get_by_name(name)
        if not v then
            return nil
        end
        return M.Class(v)
    end,

    by_file_get = function(name)
        local v = eolian.class_get_by_file(name)
        if not v then
            return nil
        end
        return M.Class(v)
    end,

    all_get = function()
        local ret = {}
        for cl in eolian.all_classes_get() do
            ret[#ret + 1] = M.Class(cl)
        end
        return ret
    end
}

M.Function = Node:clone {
    -- function types
    UNRESOLVED = eolian.function_type.UNRESOLVED,
    PROPERTY = eolian.function_type.PROPERTY,
    PROP_SET = eolian.function_type.PROP_SET,
    PROP_GET = eolian.function_type.PROP_GET,
    METHOD = eolian.function_type.METHOD,

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
        return ({
            [eolian.function_type.PROPERTY] = "property",
            [eolian.function_type.PROP_GET] = "property",
            [eolian.function_type.PROP_SET] = "property",
            [eolian.function_type.METHOD] = "method"
        })[self:type_get()]
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

    doc_get = function(self, ft)
        return M.Doc(self.func:documentation_get(ft))
    end,

    fallback_doc_get = function(self, ft)
        if not ft then
            local fft = self:type_get()
            if fft == self.PROP_GET or fft == self.PROP_SET then
                ft = fft
            end
        end
        if ft then
            return self:doc_get(ft)
        end
        return nil
    end,

    is_virtual_pure = function(self, ft)
        return self.func:is_virtual_pure(ft)
    end,

    is_auto = function(self, ft)
        return self.func:is_auto(ft)
    end,

    is_empty = function(self, ft)
        return self.func:is_empty(ft)
    end,

    is_legacy_only = function(self, ft)
        return self.func:is_legacy_only(ft)
    end,

    is_class = function(self)
        return self.func:is_class()
    end,

    is_c_only = function(self)
        return self.func:is_c_only()
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
    end
}

M.Parameter = Node:clone {
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
        return dir_to_str[self:direction_get()]
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
    if tp:is_own() then
        str = "own(" .. str .. ")"
    end
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
    COMPLEX = eolian.type_type.COMPLEX,
    POINTER = eolian.type_type.POINTER,
    CLASS = eolian.type_type.CLASS,
    STATIC_ARRAY = eolian.type_type.STATIC_ARRAY,
    TERMINATED_ARRAY = eolian.type_type.TERMINATED_ARRAY,
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
        return self.type:class_get()
    end,

    array_size_get = function(self)
        return self.type_array_size_get()
    end,

    is_own = function(self)
        return self.type:is_own()
    end,

    is_const = function(self)
        return self.type:is_const()
    end,

    is_ptr = function(self)
        return self.type:is_ptr()
    end,

    c_type_get = function(self)
        return self.type:c_type_get()
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
            return wrap_type_attrs(self, self:full_name_get())
        elseif tpt == self.COMPLEX then
            local stypes = {}
            local stp = self:base_type_get()
            while stp do
                stypes[#stypes + 1] = stp:serialize()
                stp = stp:next_type_get()
            end
            return wrap_type_attrs(self, self:full_name_get() .. "<"
                .. table.concat(stypes, ", ") .. ">")
        elseif tpt == self.POINTER then
            local btp = self:base_type_get()
            local suffix = " *"
            if btp:type_get() == self.POINTER then
                suffix = "*"
            end
            return wrap_type_attrs(self, btp:serialize() .. suffix)
        elseif tpt == self.STATIC_ARRAY then
            return wrap_type_attrs(self, "static_array<"
                .. self:base_type_get():serialize() .. ", "
                .. self:array_size_get() .. ">")
        elseif tpt == self.TERMINATED_ARRAY then
            return wrap_type_attrs(self, "terminated_array<"
                .. self:base_type_get():serialize() .. ">")
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

    nspaces_get = function(self, root)
        return M.Node.nspaces_get(self, self:type_str_get(), root)
    end,

    -- static getters

    all_aliases_get = function()
        local ret = {}
        for tp in eolian.typedecl_all_aliases_get() do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    all_structs_get = function()
        local ret = {}
        for tp in eolian.typedecl_all_structs_get() do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    all_enums_get = function()
        local ret = {}
        for tp in eolian.typedecl_all_enums_get() do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    aliases_by_file_get = function(fn)
        local ret = {}
        for tp in eolian.typedecl_aliases_get_by_file(fn) do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    structs_by_file_get = function(fn)
        local ret = {}
        for tp in eolian.typedecl_structs_get_by_file(fn) do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    enums_by_file_get = function(fn)
        local ret = {}
        for tp in eolian.typedecl_enums_get_by_file(fn) do
            ret[#ret + 1] = M.Typedecl(tp)
        end
        return ret
    end,

    alias_by_name_get = function(tn)
        local v = eolian.typedecl_alias_get_by_name(tn)
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    struct_by_name_get = function(tn)
        local v = eolian.typedecl_struct_get_by_name(tn)
        if not v then
            return nil
        end
        return M.Typedecl(v)
    end,

    enum_by_name_get = function(tn)
        local v = eolian.typedecl_enum_get_by_name(tn)
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
        return M.Node.nspaces_get(self, self:type_str_get(), root)
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
        for v in eolian.variable_all_globals_get() do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    all_constants_get = function()
        local ret = {}
        for v in eolian.variable_all_constants_get() do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    globals_by_file_get = function(fn)
        local ret = {}
        for v in eolian.variable_globals_get_by_file(fn) do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    constants_by_file_get = function(fn)
        local ret = {}
        for v in eolian.variable_constants_get_by_file(fn) do
            ret[#ret + 1] = M.Variable(v)
        end
        return ret
    end,

    global_by_name_get = function(vn)
        local v = eolian.variable_global_get_by_name(vn)
        if not v then
            return nil
        end
        return M.Variable(v)
    end,

    constant_by_name_get = function(vn)
        local v = eolian.variable_constant_get_by_name(vn)
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

local decl_to_nspace = function(decl)
    local dt = eolian.declaration_type
    local decltypes = {
        [dt.ALIAS] = "alias",
        [dt.STRUCT] = "struct",
        [dt.ENUM] = "enum",
        [dt.VAR] = "var"
    }
    local ns = decltypes[decl:type_get()]
    if ns then
        return ns
    elseif decl:type_get() == dt.CLASS then
        local ret = M.Class(decl:class_get()):type_str_get()
        if not ret then
            error("unknown class type for class '" .. decl:name_get() .. "'")
        end
        return ret
    else
        error("unknown declaration type for declaration '"
            .. decl:name_get() .. "'")
    end
end

M.ref_get = function(str, root)
    local decl = eolian.declaration_get_by_name(str)
    if decl then
        local t = { decl_to_nspace(decl) }
        for tok in str:gmatch("[^%.]+") do
            t[#t + 1] = tok:lower()
        end
        if root ~= nil then t[#t + 1] = not not root end
        return t
    end

    -- field or func
    local bstr = str:match("(.+)%.[^.]+")
    if not bstr then
        error("invalid reference '" .. str .. "'")
    end

    local sfx = str:sub(#bstr + 1)

    decl = eolian.declaration_get_by_name(bstr)
    if decl then
        local dt = eolian.declaration_type
        local tp = decl:type_get()
        if tp == dt.STRUCT or tp == dt.ENUM then
            -- TODO: point to the actual item
            return M.ref_get(bstr, root)
        end
    end

    local cl = M.Class.by_name_get(bstr)
    local fn
    local ftype = M.Function.UNRESOLVED
    if not cl then
        if sfx == ".get" then
            ftype = M.Function.PROP_GET
        elseif sfx == ".set" then
            ftype = M.Function.PROP_SET
        end
        local mname
        if ftype ~= M.Function.UNRESOLVED then
            mname = bstr:match(".+%.([^.]+)")
            if not mname then
                error("invalid reference '" .. str .. "'")
            end
            bstr = bstr:match("(.+)%.[^.]+")
            cl = M.Class.by_name_get(bstr)
            if cl then
                fn = cl:function_get_by_name(mname, ftype)
            end
        end
    else
        fn = cl:function_get_by_name(sfx:sub(2), ftype)
        if fn then ftype = fn:type_get() end
    end

    if not fn or not fn:type_str_get() then
        error("invalid reference '" .. str .. "'")
    end

    local ret = M.ref_get(bstr)
    ret[#ret + 1] = fn:type_str_get()
    ret[#ret + 1] = fn:name_get():lower()
    if root ~= nil then ret[#ret + 1] = not not root end
    return ret
end

M.scan_directory = function(dir)
    if not dir then
        if not eolian.system_directory_scan() then
            error("failed scanning system directory")
        end
        return
    end
    if not eolian.directory_scan(dir) then
        error("failed scanning directory: " .. dir)
    end
end

M.parse = function()
    if not eolian.all_eot_files_parse() then
        error("failed parsing eo type files")
    end
    if not eolian.all_eo_files_parse() then
        error("failed parsing eo files")
    end
end

return M
