local util = require("util")

local eolian = require("eolian")

local dutil = require("docgen.util")
local writer = require("docgen.writer")
local eomap = require("docgen.mappings")

local M = {}

local Node = util.Object:clone {
}

local gen_doc_refd = function(str)
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
        return self.class:functions_get(ft):to_array()
    end,

    function_get_by_name = function(self, fn, ft)
        return self.class:function_get_by_name(fn, ft)
    end,

    events_get = function(self)
        return self.class:events_get():to_array()
    end,

    c_get_function_name_get = function(self)
        return self.class:c_get_function_name_get()
    end,

    nspaces_get = function(self, root)
        local tbl = self:namespaces_get()
        for i = 1, #tbl do
            tbl[i] = tbl[i]:lower()
        end
        table.insert(tbl, 1, eomap.classt_to_str[self.class:type_get()])
        tbl[#tbl + 1] = self:name_get():lower()
        if root then
            tbl[#tbl + 1] = true
        end
        return tbl
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

return M
