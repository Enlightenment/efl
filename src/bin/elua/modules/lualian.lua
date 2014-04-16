-- Elua lualian module

local cutil  = require("cutil")
local util   = require("util")
local log    = require("eina.log")
local eolian = require("eolian")

local M = {}

local dom

cutil.init_module(function()
    dom = log.Domain("lualian")
    if not dom:is_valid() then
        log.err("Could not register log domain: lualian")
        error("Could not register log domain: lualian")
    end
end, function()
    dom:unregister()
    dom = nil
end)

local Node = util.Object:clone {
    generate = function(self, s)
    end,

    gen_children = function(self, s)
        local len = #self.children
        for i, v in ipairs(self.children) do
            v.parent_node = self
            v:generate(s, i == len)
        end
    end
}

local Method = Node:clone {
    __ctor = function(self, meth)
        self.method = meth
    end,

    gen_ffi = function(self, s)
        s:write("    void butt();\n")
    end
}

local Property = Node:clone {
    __ctor = function(self, prop, ftype)
        self.property = prop
        self.isget    = (ftype == eolian.function_type.PROP_GET)
    end,

    gen_proto = function(self)
        local prop = self.property
        local keys = prop:property_keys_list_get()
        local vals = prop:property_values_list_get()
        local rett = prop:return_type_get()
    end,

    generate = function(self, s, last)
        local prop   = self.property
        local par    = self.parent_node
        local name   = prop:name_get()
        local isget  = self.isget
        local suffix = isget and "_get" or "_set"
        local ret    = isget and "local v = " or ""
        local retr   = isget and "\n        return v" or ""
        local comma  = last  and "" or ","

        local keys   = prop:property_keys_list_get()
        local vals   = prop:property_values_list_get()

        local argsf = { "self" }
        local argsv = {}
        if #keys > 0 then
            local argn = (#keys > 1) and "keys" or "key"
            argsf[#argsf + 1] = argn
            for i, v in ipairs(keys) do
                argsv[#argsv + 1] = argn .. "[" .. i .. "]"
            end
        end
        if not isget and #vals > 0 then
            local argn = (#vals > 1) and "vals" or "val"
            argsf[#argsf + 1] = argn
            for i, v in ipairs(vals) do
                argsv[#argsv + 1] = argn .. "[" .. i .. "]"
            end
        end

        s:write(([[
    %s%s = function(%s)
        self:__do_start()
        %s__lib.%s_%s%s(%s)
        self:__do_end()%s
    end%s

]]):format(name, suffix, table.concat(argsf, ", "), ret, par.prefix, name,
        suffix, table.concat(argsv, ", "), retr, comma))
    end,

    gen_ffi = function(self, s)
        s:write("    void butt();\n")
    end
}

local Constructor = Node:clone {
}

local Destructor = Node:clone {
}

local Mixin = Node:clone {
    __ctor = function(self, cname, ch)
        self.cname    = cname
        self.prefix   = eolian.class_eo_prefix_get(cname)
        self.children = ch
    end,

    generate = function(self, s)
        dom:log(log.level.INFO, "  Generating for interface/mixin: "
            .. self.cname)

        s:write("ffi.cdef [[\n")
        self:gen_ffi(s)
        s:write("]]\n\n")

        s:write(("M.%s = {\n"):format(self.cname))

        self:gen_children(s)

        s:write("\n}\n")
    end,

    gen_ffi = function(self, s)
        for i, v in ipairs(self.children) do
            v:gen_ffi(s)
        end
    end
}

local Class = Node:clone {
    __ctor = function(self, cname, parent, mixins, ch)
        self.cname      = cname
        self.parent     = parent
        self.interfaces = interfaces
        self.mixins     = mixins
        self.prefix     = eolian.class_eo_prefix_get(cname)
        self.children   = ch
    end,

    generate = function(self, s)
        dom:log(log.level.INFO, "  Generating for class: " .. self.cname)

        s:write("ffi.cdef [[\n")
        self:gen_ffi(s)
        s:write("]]\n\n")

        s:write(([[
local Parent = eo.class_get("%s")
M.%s = Parent:clone {
]]):format(self.parent, self.cname))

        self:gen_children(s)

        s:write("\n}\n")

        for i, v in ipairs(self.mixins) do
            s:write(("\nM.%s:mixin(eo.class_get(\"%s\"))\n")
                :format(self.cname, v))
        end
    end,

    gen_ffi = function(self, s)
        for i, v in ipairs(self.children) do
            v:gen_ffi(s)
        end
    end
}

local File = Node:clone {
    __ctor = function(self, fname, cname, libname, ch)
        self.fname    = fname:match(".+/(.+)") or fname
        self.cname    = cname
        self.libname  = libname
        self.children = ch
    end,

    generate = function(self, s)
        dom:log(log.level.INFO, "Generating for file: " .. self.fname)
        dom:log(log.level.INFO, "  Class            : " .. self.cname)
        s:write(([[
-- EFL LuaJIT bindings: %s (class %s)
-- For use with Elua; automatically generated, do not modify

local cutil = require("cutil")
local util  = require("util")
local eo    = require("eo")

local M     = {}

local __lib

local init = function()
    __lib = util.lib_load("%s")
end

local shutdown = function()
    util.lib_unload("%s")
end

cutil.init_module(init, shutdown)

]]):format(self.fname, self.cname, self.libname, self.libname))

        self:gen_children(s)

        s:write([[

return M
]])
    end
}

local gen_contents = function(classn)
    local cnt = {}
    local ft  = eolian.function_type
    -- first try properties
    local props = eolian.class_functions_list_get(classn, ft.PROPERTY)
    for i, v in ipairs(props) do
        local ftype  = v:type_get()
        local fread  = (ftype == ft.PROPERTY or ftype == ft.PROP_GET)
        local fwrite = (ftype == ft.PROPERTY or ftype == ft.PROP_SET)
        if fwrite then
            cnt[#cnt + 1] = Property(v, ft.PROP_SET)
        end
        if fread then
            cnt[#cnt + 1] = Property(v, ft.PROP_GET)
        end
    end
    -- then methods
    local meths = eolian.class_functions_list_get(classn, ft.METHOD)
    for i, v in ipairs(meths) do
        cnt[#cnt + 1] = Method(v)
    end
    return cnt
end

local gen_mixin = function(classn)
    return Mixin(classn, gen_contents(classn))
end

local gen_class = function(classn)
    local inherits = eolian.class_inherits_list_get(classn)
    local parent
    local mixins   = {}
    local ct = eolian.class_type
    for i, v in ipairs(inherits) do
        local tp = eolian.class_type_get(v)
        if tp == ct.REGULAR or tp == ct.ABSTRACT or v == "Eo_Base" then
            if parent then
                error(classn .. ": more than 1 parent!")
            end
            parent = v
        elseif tp == ct.MIXIN or tp == ct.INTERFACE then
            mixins[#mixins + 1] = v
        else
            error(classn .. ": unknown inherit " .. v)
        end
    end
    return Class(classn, parent, mixins, gen_contents(classn))
end

M.generate = function(files, include_files, fstream)
    for i, file in ipairs(include_files) do
        if not eolian.eo_file_parse(file) then
            error("Failed parsing include file: " .. file)
        end
    end
    for i, file in ipairs(files) do
        if not eolian.eo_file_parse(file[1]) then
            error("Failed parsing file: " .. file[1])
        end
    end
    for i, file in ipairs(files) do
        local fname = file[1]
        local classn = eolian.class_find_by_file(fname)
        local tp = eolian.class_type_get(classn)
        local ct = eolian.class_type
        local cl
        if tp == ct.MIXIN or tp == ct.INTERFACE then
            cl = gen_mixin(classn)
        elseif tp == ct.REGULAR or tp == ct.ABSTRACT then
            cl = gen_class(classn)
        else
            error(classn .. ": unknown type")
        end
        File(fname, classn, file[2], { cl }):generate(fstream)
    end
end

return M