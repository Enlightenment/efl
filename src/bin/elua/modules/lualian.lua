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

local int_builtin = {
    ["byte" ] = true, ["short"] = true, ["int"] = true, ["long"] = true,
    ["llong"] = true,

    ["int8"  ] = true, ["int16"] = true, ["int32"] = true, ["int64"] = true,
    ["int128"] = true,

    ["intptr"] = true
}

local num_others = {
    ["size" ] = true, ["ssize" ] = true, ["ptrdiff"] = true,
    ["float"] = true, ["double"] = true, ["ldouble"] = true
}

local is_num = function(x)
    if num_others [x       ] then return true end
    if int_builtin[x       ] then return true end
    if int_builtin["u" .. x] then return true end
    return false
end

local known_out = {
    ["Evas_Coord"] = function(expr) return ("tonumber(%s)"):format(expr) end,
    ["bool"] = function(expr) return ("((%s) ~= 0)"):format(expr) end,
    ["char"] = function(expr) return ("string.char(%s)"):format(expr) end
}

local known_in = {
    ["Evas_Coord"] = function(expr) return expr end,
    ["bool"] = function(expr) return expr end
}

local known_ptr_out = {
    ["const char"] = function(expr) return ("ffi.string(%s)"):format(expr) end
}

local known_ptr_in = {
    ["const char"] = function(expr) return expr end
}

local convfuncs = {}

local build_calln = function(tps, expr, isin)
    local funcn = "__convert_type_wip"
    convfuncs[funcn] = true
    return table.concat {
        funcn, "(", expr, ")"
    }
end

local typeconv_in = function(tps, expr)
    if tps:type_get() == eolian.type_type.POINTER then
        local base = tps:base_type_get()
        local f = known_ptr_in[base:c_type_get()]
        if f then return f(expr) end
        return build_calln(tps, expr, true)
    end

    local tp = tps:name_get()

    if is_num(tp) then return expr end

    local f = known_in[tp]
    if f then
        return f(expr)
    end

    return build_calln(tps, expr, true)
end

local typeconv = function(tps, expr, isin)
    if isin then
        return typeconv_in(tps, expr)
    end

    if tps:type_get() == eolian.type_type.POINTER then
        local base = tps:base_type_get()
        local f = known_ptr_out[base:c_type_get()]
        if f then return f(expr) end
        return build_calln(tps, expr, false)
    end

    local tp = tps:name_get()

    if is_num(tp) then
        return ("tonumber(%s)"):format(expr)
    end

    local f = known_out[tp]
    if f then
        return f(expr)
    end

    return build_calln(tps, expr, false)
end

local Node = util.Object:clone {
    generate = function(self, s)
    end,

    gen_children = function(self, s)
        local len = #self.children
        local evs =  self.events
        local evslen
        if evs then evslen = #evs end
        local hasevs = evs and evslen > 0
        for i, v in ipairs(self.children) do
            v.parent_node = self
            v:generate(s, (not hasevs) and (i == len))
        end
        if hasevs then
            s:write("    events = {\n")
            for i, v in ipairs(evs) do
                v.parent_node = self
                v:generate(s, i == evslen)
            end
            s:write("    }\n")
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
        local pars = meth:parameters_get()
        local rett = meth:return_type_get(eolian.function_type.METHOD)

        local proto = {
            name    = meth:name_get()
        }
        proto.ret_type = rett and rett:c_type_get() or "void"
        local args, cargs, vargs = { "self" }, {}, {}
        proto.args, proto.cargs, proto.vargs = args, cargs, vargs
        local rets = {}
        proto.rets = rets
        local allocs = {}
        proto.allocs = allocs

        proto.full_name = meth:full_c_name_get(self.parent_node.prefix)

        local dirs = eolian.parameter_dir

        local fulln = proto.full_name

        if rett then
            rets[#rets + 1] = typeconv(rett, "v", false)
        end

        for v in pars do
            local dir, tps, nm = v:direction_get(), v:type_get(), v:name_get()
            local tp = tps:c_type_get()
            if dir == dirs.OUT or dir == dirs.INOUT then
                if dir == dirs.INOUT then
                    args[#args + 1] = nm
                end
                cargs [#cargs  + 1] = tp .. " *" .. nm
                vargs [#vargs  + 1] = nm
                allocs[#allocs + 1] = { tp, nm, (dir == dirs.INOUT)
                    and typeconv(tps, nm, true) or nil }
                rets  [#rets   + 1] = typeconv(tps, nm .. "[0]", false)
            else
                args  [#args   + 1] = nm
                cargs [#cargs  + 1] = tp .. " " .. nm
                vargs [#vargs  + 1] = typeconv(tps, nm, true)
            end
        end

        if #cargs == 0 then cargs[1] = "void" end

        self.cached_proto = proto

        return proto
    end,

    generate = function(self, s, last)
        local proto = self:gen_proto()
        s:write("    ", proto.name, proto.suffix or "", " = function(",
            table.concat(proto.args, ", "), ")\n")
        s:write( "        eo.__do_start(self, __class)\n")
        for i, v in ipairs(proto.allocs) do
            s:write("        local ", v[2], " = ffi.new(\"", v[1], "[1]\")\n")
        end
        local genv = (proto.ret_type ~= "void")
        s:write("        ", genv and "local v = " or "", "__lib.",
            proto.full_name, "(", table.concat(proto.vargs, ", "), ")\n")
        s:write("        eo.__do_end()\n")
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
        local keys = prop:property_keys_get():to_array()
        local vals = prop:property_values_get():to_array()
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

        proto.full_name = prop:full_c_name_get(self.parent_node.prefix)
            .. proto.suffix

        local dirs = eolian.parameter_dir

        local fulln = proto.full_name
        if #keys > 0 then
            local argn = (#keys > 1) and "keys" or "key"
            for i, v in ipairs(keys) do
                local nm  = v:name_get()
                local tps = v:type_get()
                local tp  = tps:c_type_get()
                cargs [#cargs  + 1] = tp .. " " .. nm
                vargs [#vargs  + 1] = typeconv(tps, argn .. "[" .. i
                    .. "]", true)
            end
            args[#args + 1] = argn
        end
        proto.kprop = #keys > 0

        if #vals > 0 then
            if self.isget then
                if #vals == 1 and not rett then
                    local tps = vals[1]:type_get()
                    proto.ret_type = tps:c_type_get()
                    rets[#rets + 1] = typeconv(tps, "v", false)
                else
                    for i, v in ipairs(vals) do
                        local dir, tps, nm = v:direction_get(), v:type_get(),
                            v:name_get()
                        local tp = tps:c_type_get()
                        cargs [#cargs  + 1] = tp .. " *" .. nm
                        vargs [#vargs  + 1] = nm
                        allocs[#allocs + 1] = { tp, nm }
                        rets  [#rets   + 1] = typeconv(tps, nm .. "[0]", false)
                    end
                end
            else
                local argn = (#keys > 1) and "vals" or "val"
                args[#args + 1] = argn
                for i, v in ipairs(vals) do
                    local dir, tps, nm = v:direction_get(), v:type_get(),
                        v:name_get()
                    local tp = tps:c_type_get()
                    cargs[#cargs + 1] = tp .. " " .. nm
                    vargs[#vargs + 1] = typeconv(tps, argn .. "[" .. i .. "]",
                        true)
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

local Event = Node:clone {
    __ctor = function(self, ename, etype, edesc)
        self.ename = ename
        self.etype = etype
        self.edesc = edesc
    end,

    gen_ffi_name = function(self)
        local ffin = self.cached_ffi_name
        if ffin then return ffin end
        ffin = table.concat {
            "_", self.parent_node.klass:name_get():upper(), "_EVENT_",
            self.ename:gsub("%W", "_"):upper()
        }
        self.cached_ffi_name = ffin
        return ffin
    end,

    generate = function(self, s, last)
        s:write("        [\"", self.ename, "\"] = __lib.",
            self:gen_ffi_name(), last and "\n" or ",\n")
    end,

    gen_ffi = function(self, s)
        s:write("    extern const Eo_Event_Description ",
            self:gen_ffi_name(), ";\n")
    end,

    gen_ctor = function(self, s)
    end
}

local Constructor = Method:clone {
    generate = function(self, s, last)
        local proto   = self:gen_proto()
        local name    = proto.name
        local defctor = name == "constructor"
        table.insert(proto.args, 2, "parent")

        s:write("    __define_properties = function(self, proto)\n")
        self.parent_node:gen_ctor(s)
        s:write("        proto.__define_properties(self, proto.__proto)\n")
        s:write("    end,\n\n")

        s:write("    ", defctor and "__ctor" or name, " = function(",
            table.concat(proto.args, ", "), ")\n")
        if not defctor then
            s:write("        self = self:clone()\n")
        end
        s:write("        self:__define_properties(self.__proto.__proto)\n")
        for i, v in ipairs(proto.allocs) do
            s:write("        local ", v[2], " = ffi.new(\"", v[1], "[1]\")\n")
        end
        local genv = (proto.ret_type ~= "void")
        local cvargs = table.concat(proto.vargs, ", ")
        if cvargs ~= "" then cvargs = ", " .. cvargs end
        s:write("        ", genv and "local v = " or "", "eo.__ctor_common(",
            "self, __class, parent, __lib.", proto.full_name, cvargs, ")\n")
        if not defctor then
            table.insert(proto.rets, 1, "self")
        end
        if #proto.rets > 0 then
            s:write("        return ", table.concat(proto.rets, ", "), "\n")
        end
        s:write("    end", last and "" or ",", last and "\n" or "\n\n")
    end
}

local Default_Constructor = Node:clone {
    generate = function(self, s, last)
        s:write("    __define_properties = function(self, proto)\n")
        self.parent_node:gen_ctor(s)
        s:write("        proto.__define_properties(self, proto.__proto)\n")
        s:write("    end,\n\n")

        s:write( "    __ctor = function(self, parent)\n")
        s:write("        self:__define_properties(self.__proto.__proto)\n")
        s:write("        eo.__ctor_common(self, __class, parent)\n")
        s:write("    end", last and "" or ",", last and "\n" or "\n\n")
    end,

    gen_ffi = function(self, s)
    end,

    gen_ctor = function(self)
    end
}

local Mixin = Node:clone {
    __ctor = function(self, klass, ch, evs)
        self.klass    = klass
        self.prefix   = klass:eo_prefix_get()
        self.children = ch
        self.events   = evs
    end,

    generate = function(self, s)
        dom:log(log.level.INFO, "  Generating for interface/mixin: "
            .. self.klass:full_name_get())

        s:write("ffi.cdef [[\n")
        self:gen_ffi(s)
        s:write("]]\n\n")

        local nspaces = self.klass:namespaces_get():to_array()
        local mname
        if #nspaces > 1 then
            local lnspaces = {}
            for i = 2, #nspaces do
                lnspaces[i] = '"' .. nspaces[i]:lower() .. '"'
            end
            s:write("local __M = util.get_namespace(M, { ",
                table.concat(lnspaces, ", "), " })\n")
            mname = "__M"
        else
            mname = "M"
        end

        s:write(([[
local __class = __lib.%s_class_get()
%s.%s = eo.class_register("%s", {
]]):format(self.prefix, mname, self.klass:name_get(),
        self.klass:full_name_get()))

        self:gen_children(s)

        s:write("})\n")
    end,

    gen_ffi = function(self, s)
        s:write("    const Eo_Class *", self.prefix, "_class_get(void);\n")
        for i, v in ipairs(self.children) do
            v.parent_node = self
            v:gen_ffi(s)
        end
        if self.events then
            for i, v in ipairs(self.events) do
                v.parent_node = self
                v:gen_ffi(s)
            end
        end
    end,

    gen_ctor = function(self, s)
        for i, v in ipairs(self.children) do
            v.parent_node = self
            v:gen_ctor(s)
        end
    end
}

local Class = Node:clone {
    __ctor = function(self, klass, parent, mixins, ch, evs)
        self.klass      = klass
        self.parent     = parent
        self.interfaces = interfaces
        self.mixins     = mixins
        self.prefix     = klass:eo_prefix_get()
        self.children   = ch
        self.events     = evs
    end,

    generate = function(self, s)
        dom:log(log.level.INFO, "  Generating for class: "
            .. self.klass:full_name_get())

        s:write("ffi.cdef [[\n")
        self:gen_ffi(s)
        s:write("]]\n\n")

        local nspaces = self.klass:namespaces_get():to_array()
        local mname
        if #nspaces > 1 then
            local lnspaces = {}
            for i = 2, #nspaces do
                lnspaces[i] = '"' .. nspaces[i]:lower() .. '"'
            end
            s:write("local __M = util.get_namespace(M, { ",
                table.concat(lnspaces, ", "), " })\n")
            mname = "__M"
        else
            mname = "M"
        end

        s:write(([[
local __class = __lib.%s_class_get()
local Parent  = eo.class_get("%s")
%s.%s = eo.class_register("%s", Parent:clone {
]]):format(self.prefix, self.parent, mname, self.klass:name_get(),
        self.klass:full_name_get()))

        self:gen_children(s)

        s:write("})")

        for i, v in ipairs(self.mixins) do
            s:write(("\nM.%s:mixin(eo.class_get(\"%s\"))\n")
                :format(ename, v))
        end
    end,

    gen_ffi = Mixin.gen_ffi,
    gen_ctor = Mixin.gen_ctor
}

local File = Node:clone {
    __ctor = function(self, fname, klass, libname, ch)
        self.fname    = fname:match(".+/(.+)") or fname
        self.klass    = klass
        self.libname  = libname
        self.children = ch
    end,

    generate = function(self, s)
        dom:log(log.level.INFO, "Generating for file: " .. self.fname)
        dom:log(log.level.INFO, "  Class            : "
            .. self.klass:full_name_get())

        local modn = ("require(\"%s\")"):format(self.klass:namespaces_get()()
            :lower())

        s:write(([[
-- EFL LuaJIT bindings: %s (class %s)
-- For use with Elua; automatically generated, do not modify

local cutil = require("cutil")
local util  = require("util")
local eo    = require("eo")

local M     = %s

local __lib

local init = function()
    __lib = util.lib_load("%s")
end

local shutdown = function()
    util.lib_unload("%s")
end

cutil.init_module(init, shutdown)

]]):format(self.fname, self.klass:full_name_get(), modn, self.libname,
        self.libname))

        self:gen_children(s)

        s:write([[

return M
]])

        local first = true
        for name, v in pairs(convfuncs) do
            if first then
                print("\nRequired conversion functions:")
                first = false
            end
            print("    " .. name)
        end
    end
}

local gen_contents = function(klass)
    local cnt = {}
    local ft  = eolian.function_type
    -- first try properties
    local props = klass:functions_get(ft.PROPERTY):to_array()
    for i, v in ipairs(props) do
        if v:scope_get() == eolian.object_scope.PUBLIC then
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
    end
    -- then methods
    local meths = klass:functions_get(ft.METHOD):to_array()
    for i, v in ipairs(meths) do
        if v:scope_get() == eolian.object_scope.PUBLIC then
            cnt[#cnt + 1] = Method(v)
        end
    end
    -- and constructors
    local ctors = klass:functions_get(ft.CTOR):to_array()
    for i, v in ipairs(ctors) do
        cnt[#cnt + 1] = Constructor(v)
    end
    if #ctors == 0 then
        cnt[#cnt + 1] = Default_Constructor()
    end
    -- events
    local evs = {}
    local events = klass:events_get():to_array()
    for i, v in ipairs(events) do
        evs[#evs + 1] = Event(v:name_get(), v:type_get(), v:description_get())
    end
    return cnt, evs
end

local gen_mixin = function(kliass)
    return Mixin(klass, gen_contents(klass))
end

local gen_class = function(klass)
    local inherits = klass:inherits_get()
    local parent
    local mixins   = {}
    local ct = eolian.class_type
    for v in inherits do
        local tp = eolian.class_get_by_name(v):type_get()
        if tp == ct.REGULAR or tp == ct.ABSTRACT then
            if parent then
                error(klass:full_name_get() .. ": more than 1 parent!")
            end
            parent = v
        elseif tp == ct.MIXIN or tp == ct.INTERFACE then
            mixins[#mixins + 1] = v
        else
            error(klass:full_name_get() .. ": unknown inherit " .. v)
        end
    end
    return Class(klass, parent, mixins, gen_contents(klass))
end

M.include_dir = function(dir)
    if not eolian.directory_scan(dir) then
        error("Failed including directory: " .. dir)
    end
end

M.load_eot_files = function()
    return eolian.all_eot_files_parse()
end

M.system_directory_scan = function()
    return eolian.system_directory_scan()
end

M.generate = function(fname, libname, fstream)
    if not eolian.eo_file_parse(fname) then
        error("Failed parsing file: " .. fname)
    end
    local sfn = fname:match(".*[\\/](.+)$") or fname
    local klass = eolian.class_get_by_file(sfn)
    local tp = klass:type_get()
    local ct = eolian.class_type
    local cl
    if tp == ct.MIXIN or tp == ct.INTERFACE then
        cl = gen_mixin(klass)
    elseif tp == ct.REGULAR or tp == ct.ABSTRACT then
        cl = gen_class(klass)
    else
        error(klass:full_name_get() .. ": unknown type")
    end
    File(fname, klass, libname, { cl }):generate(fstream or io.stdout)
end

return M
