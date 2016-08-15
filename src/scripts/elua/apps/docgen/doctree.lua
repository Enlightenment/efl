local util = require("util")

local eolian = require("eolian")

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
        if root then
            tbl[#tbl + 1] = true
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
        return self.func:return_type_get(ft)
    end,

    return_default_value_get = function(self, ft)
        return self.func:return_default_value_get(ft)
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
        if root then
            tbl[#tbl + 1] = true
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
        return self.param:type_get()
    end,

    default_value_get = function(self)
        return self.param:default_value_get()
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
        return self.event:type_get()
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
        if root then
            tbl[#tbl + 1] = true
        end
        return tbl
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
        if root then t[#t + 1] = true end
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
    if root then ret[#ret + 1] = true end
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
