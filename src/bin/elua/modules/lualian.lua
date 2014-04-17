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

local strip_name = function(self, cn, cp)
    if not cp or #cp == 0 then return cn end
    local  nm = cn:match("^" .. cp .. "_(.*)$")
    return nm or cp
end

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

    gen_proto = function(self)
        if self.cached_proto then return self.cached_proto end

        local meth = self.method
        local pars = meth:parameters_list_get()
        local rett = meth:return_type_get(eolian.function_type.METHOD)

        local proto = {
            name    = meth:name_get()
        }
        proto.ret_type = rett or "void"
        local args, cargs, vargs = { "self" }, {}, {}
        proto.args, proto.cargs, proto.vargs = args, cargs, vargs
        local rets = {}
        proto.rets = rets
        local allocs = {}
        proto.allocs = allocs

        proto.full_name = self.parent_node.prefix .. "_" .. proto.name

        local dirs = eolian.parameter_dir

        if #pars > 0 then
            for i, v in ipairs(pars) do
                local dir, tp, nm = v:information_get()
                if dir == dirs.OUT or dir == dirs.INOUT then
                    if dir == dirs.INOUT then
                        args[#args + 1] = nm
                    end
                    cargs [#cargs  + 1] = tp .. " *" .. nm
                    vargs [#vargs  + 1] = nm
                    allocs[#allocs + 1] = { tp, nm, (dir == dirs.INOUT)
                        and nm or nil }
                    rets  [#rets   + 1] = nm .. "[0]"
                else
                    args  [#args   + 1] = nm
                    cargs [#cargs  + 1] = tp .. " " .. nm
                    vargs [#vargs  + 1] = nm
                end
            end
        end

        if #cargs == 0 then cargs[1] = "void" end

        self.cached_proto = proto

        return proto
    end,

    generate = function(self, s, last)
        local proto = self:gen_proto()
        local lproto = {
            "    ", proto.name, proto.suffix or "", " = function(",
            table.concat(proto.args, ", "), ")\n"
        }
        s:write(table.concat(lproto))
        s:write( "        self:__do_start()\n")
        for i, v in ipairs(proto.allocs) do
            s:write("        local ", v[2], " = ffi.new(\"", v[1], "[1]\")\n")
        end
        local genv = (proto.ret_type ~= "void")
        local lcall = {
            "        ", genv and "local v = " or "", "__lib.", proto.full_name,
            "(", table.concat(proto.vargs, ", "), ")\n"
        }
        s:write(table.concat(lcall))
        s:write("        self:__do_end()\n")
        if #proto.rets > 0 then
            s:write("        return ", table.concat(proto.rets, ", "), "\n")
        end
        s:write("    end", last and "" or ",", last and "\n" or "\n\n")
    end,

    gen_ffi = function(self, s)
        local proto = self:gen_proto()
        local cproto = {
            "    ", proto.ret_type, " ", proto.full_name, "(",
            table.concat(proto.cargs, ", "), ");\n"
        }
        s:write(table.concat(cproto))
    end,

    gen_ctor = function(self, s)
    end
}

local Property = Method:clone {
    __ctor = function(self, prop, ftype)
        self.property = prop
        self.isget    = (ftype == eolian.function_type.PROP_GET)
        self.ftype    = ftype
    end,

    gen_proto = function(self)
        if self.cached_proto then return self.cached_proto end

        local prop = self.property
        local keys = prop:property_keys_list_get()
        local vals = prop:property_values_list_get()
        local rett = prop:return_type_get(self.ftype)

        local proto = {
            name    = prop:name_get(),
            suffix  = (self.isget and "_get" or "_set")
        }
        proto.ret_type = rett or "void"
        local args, cargs, vargs = { "self" }, {}, {}
        proto.args, proto.cargs, proto.vargs = args, cargs, vargs
        local rets = {}
        proto.rets = rets
        local allocs = {}
        proto.allocs = allocs

        proto.full_name = self.parent_node.prefix .. "_" .. proto.name
            .. proto.suffix

        local dirs = eolian.parameter_dir

        local kprop = false
        if #keys > 0 then
            local argn = (#keys > 1) and "keys" or "key"
            for i, v in ipairs(keys) do
                local dir, tp, nm = v:information_get()
                if dir == dirs.OUT or dir == dirs.INOUT then
                    if dir == dirs.INOUT then kprop = true end
                    cargs [#cargs  + 1] = tp .. " *" .. nm
                    vargs [#vargs  + 1] = nm
                    allocs[#allocs + 1] = { tp, nm, (dir == dirs.INOUT)
                        and (argn .. "[" .. i .. "]") or nil }
                    rets  [#rets   + 1] = nm .. "[0]"
                else
                    kprop = true
                    cargs [#cargs  + 1] = tp .. " " .. nm
                    vargs [#vargs  + 1] = argn .. "[" .. i .. "]"
                end
            end
            if kprop then args[#args + 1] = argn end
        end

        proto.kprop = kprop

        if #vals > 0 then
            if self.isget then
                if #vals == 1 and not rett then
                    rets[#rets + 1] = "v"
                    proto.ret_type = vals[1]:type_get()
                else
                    for i, v in ipairs(vals) do
                        local dir, tp, nm = v:information_get()
                        cargs [#cargs  + 1] = tp .. " *" .. nm
                        vargs [#vargs  + 1] = nm
                        allocs[#allocs + 1] = { tp, nm }
                        rets  [#rets   + 1] = nm .. "[0]"
                    end
                end
            else
                local argn = (#keys > 1) and "vals" or "val"
                args[#args + 1] = argn
                for i, v in ipairs(vals) do
                    local dir, tp, nm = v:information_get()
                    cargs[#cargs + 1] = tp .. " " .. nm
                    vargs[#vargs + 1] = argn .. "[" .. i .. "]"
                end
            end
        end

        if #cargs == 0 then cargs[1] = "void" end

        self.cached_proto = proto

        return proto
    end,

    gen_ctor = function(self, s)
        local proto = self:gen_proto()
        s:write("        ", "self:define_property",
            proto.kprop and "_key(" or "(", '"', proto.name, '", ')
        if self.isget then
            s:write("self.", proto.name, "_get, nil)\n")
        else
            s:write("nil, self.", proto.name, "_set)\n")
        end
    end
}

local Constructor = Node:clone {
    gen_ffi = function(self, s)
    end
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

        s:write(("M.%s = {\n"):format(strip_name(self, self.cname,
            self.parent_node.cprefix)))

        self:gen_children(s)

        s:write("}\n")
    end,

    gen_ffi = function(self, s)
        for i, v in ipairs(self.children) do
            v.parent_node = self
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
]]):format(self.parent, strip_name(self, self.cname,
            self.parent_node.cprefix)))

        self:gen_children(s)

        s:write("}\n")

        for i, v in ipairs(self.mixins) do
            s:write(("\nM.%s:mixin(eo.class_get(\"%s\"))\n")
                :format(self.cname, v))
        end
    end,

    gen_ffi = function(self, s)
        for i, v in ipairs(self.children) do
            v.parent_node = self
            v:gen_ffi(s)
        end
    end
}

local File = Node:clone {
    __ctor = function(self, fname, cname, libname, cprefix, ch)
        self.fname    = fname:match(".+/(.+)") or fname
        self.cname    = cname
        self.libname  = libname
        self.cprefix  = cprefix
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
    -- and constructors
    local dflt_ctor = eolian.class_default_constructor_get(classn)
    if dflt_ctor then
        cnt[#cnt + 1] = Constructor(dflt_ctor)
    end
    local ctors = eolian.class_functions_list_get(classn, ft.CTOR)
    for i, v in ipairs(ctors) do
        cnt[#cnt + 1] = Constructor(v)
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
        File(fname, classn, file[2], file[3], { cl }):generate(fstream)
    end
end

return M