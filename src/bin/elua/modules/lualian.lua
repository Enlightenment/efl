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

-- char not included - manual disambiguation needed
local isnum = {
    ["short"   ] = true, ["int"      ] = true, ["long"       ] = true,
    ["size_t"  ] = true, ["ptrdiff_t"] = true, ["int8_t"     ] = true,
    ["int16_t" ] = true, ["int32_t"  ] = true, ["int64_t"    ] = true,
    ["uint8_t" ] = true, ["uint16_t" ] = true, ["uint32_t"   ] = true,
    ["uint64_t"] = true, ["intptr_t" ] = true, ["uintptr_t"  ] = true,
    ["float"   ] = true, ["double"   ] = true, ["long double"] = true
}

local known_out = {
    ["Eina_Bool" ] = function(expr) return ("((%s) ~= 0)"):format(expr) end,
    ["Evas_Coord"] = function(expr) return ("tonumber(%s)"):format(expr) end
}

local known_in = {
    ["Eina_Bool" ] = function(expr) return expr end,
    ["Evas_Coord"] = function(expr) return expr end
}

local known_ptr_out = {
    ["const char"] = function(expr) return ("ffi.string(%s)"):format(expr) end
}

local known_ptr_in = {
    ["const char"] = function(expr) return expr end
}

local convfuncs = {}

local build_calln = function(tps, expr, tp, isin)
    local nm, own
    local buf  = { "__convert", isin and "IN" or "OUT" }
    local owns = {}
    while tps do
        tps, nm, own = tps:information_get()
        owns[#owns + 1] = own and "true" or "false"
        buf[#buf + 1] = nm:gsub("%s", "_"):gsub("%*", "_ptr"):gsub("__+", "_")
    end
    local funcn = table.concat(buf, "_")
    convfuncs[funcn] = true
    return table.concat {
        funcn, "(", expr, ", ", table.concat(owns, ", "), ")"
    }
end

local build_tp = function(tps)
    return table.concat(buf, "_")
end

local typeconv_in = function(tps, tp, expr, isconst, isptr)
    if isptr then
        local passtp = (isconst and "const " or "") .. tp
        local f = known_ptr_in[passtp]
        if f then return f(expr) end
        return build_calln(tps, expr, true)
    end
    if isnum[tp] then
        return expr
    end

    local f = known_in[tp]
    if f then
        return f(expr)
    end

    return build_calln(tps, expr, true)
end

local typeconv = function(tps, expr, isin)
    local tp = select(2, tps:information_get())
    -- strip away type qualifiers
    local isconst, tpr = tp:match("^(const)[ ]+(.+)$")
    isconst = not not isconst
    if tpr then tp = tpr end

    -- check if it's a pointer
    local basetype = (tp:match("(.+)[ ]+%*$"))

    -- out val
    if isin then
        return typeconv_in(tps, basetype or tp, expr, isconst, not not basetype)
    end

    -- pointer type
    if basetype then
        local passtp = (isconst and "const " or "") .. basetype
        local f = known_ptr_out[passtp]
        if f then return f(expr) end
        return build_calln(tps, expr, false)
    end

    -- number?
    if isnum[tp] then
        return ("tonumber(%s)"):format(expr)
    end

    -- known primitive EFL type?
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
        local pars = meth:parameters_list_get()
        local rett = meth:return_types_list_get(eolian.function_type.METHOD)

        local proto = {
            name    = meth:name_get()
        }
        proto.ret_type = rett and (select(2, rett:information_get())) or "void"
        local args, cargs, vargs = { "self" }, {}, {}
        proto.args, proto.cargs, proto.vargs = args, cargs, vargs
        local rets = {}
        proto.rets = rets
        local allocs = {}
        proto.allocs = allocs

        proto.full_name = self.parent_node.prefix .. "_" .. proto.name

        local dirs = eolian.parameter_dir

        local fulln = proto.full_name

        if rett then
            rets[#rets + 1] = typeconv(rett, "v", false)
        end

        if #pars > 0 then
            for i, v in ipairs(pars) do
                local dir, tp, nm = v:information_get()
                local tps = v:types_list_get()
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
        end

        if #cargs == 0 then cargs[1] = "void" end

        self.cached_proto = proto

        return proto
    end,

    generate = function(self, s, last)
        local proto = self:gen_proto()
        s:write("    ", proto.name, proto.suffix or "", " = function(",
            table.concat(proto.args, ", "), ")\n")
        s:write( "        self:__do_start()\n")
        for i, v in ipairs(proto.allocs) do
            s:write("        local ", v[2], " = ffi.new(\"", v[1], "[1]\")\n")
        end
        local genv = (proto.ret_type ~= "void")
        s:write("        ", genv and "local v = " or "", "__lib.",
            proto.full_name, "(", table.concat(proto.vargs, ", "), ")\n")
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

        local fulln = proto.full_name
        if #keys > 0 then
            local argn = (#keys > 1) and "keys" or "key"
            for i, v in ipairs(keys) do
                local nm  = v:name_get()
                local tps = v:types_list_get()
                local tp  = select(2, tps:information_get())
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
                    local tps = vals[1]:types_list_get()
                    proto.ret_type = (select(2, tps:information_get()))
                    rets[#rets + 1] = typeconv(tps, "v", false)
                else
                    for i, v in ipairs(vals) do
                        local dir, tp, nm = v:information_get()
                        local tps = v:types_list_get()
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
                    local dir, tp, nm = v:information_get()
                    local tps = v:types_list_get()
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
            "_", self.parent_node.cname:upper(), "_EVENT_",
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
        s:write( "    ", defctor and "__ctor" or name, " = function(",
            table.concat(proto.args, ", "), ")\n")
        if not defctor then
            s:write("        self = self:clone()\n")
        end
        self.parent_node:gen_ctor(s)
        for i, v in ipairs(proto.allocs) do
            s:write("        local ", v[2], " = ffi.new(\"", v[1], "[1]\")\n")
        end
        local genv = (proto.ret_type ~= "void")
        s:write("        ", genv and "local v = " or "", "self:__ctor_common(",
            "__lib.", self.parent_node.prefix, "_class_get(), parent, __lib.",
            proto.full_name, ", ", table.concat(proto.vargs, ", "), ")\n")
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
        s:write( "    __ctor = function(self, parent)\n")
        self.parent_node:gen_ctor(s)
        s:write("        self:__ctor_common(__lib.", self.parent_node.prefix,
            "_class_get(), parent)\n")
        s:write("    end", last and "" or ",", last and "\n" or "\n\n")
    end,

    gen_ffi = function(self, s)
    end,

    gen_ctor = function(self)
    end
}

local Mixin = Node:clone {
    __ctor = function(self, cname, ch, evs)
        self.cname    = cname
        self.prefix   = eolian.class_eo_prefix_get(cname)
        self.children = ch
        self.events   = evs
    end,

    generate = function(self, s)
        dom:log(log.level.INFO, "  Generating for interface/mixin: "
            .. self.cname)

        s:write("ffi.cdef [[\n")
        self:gen_ffi(s)
        s:write("]]\n\n")

        s:write(("M.%s = eo.class_register(\"%s\", {\n"):format(
            strip_name(self, self.cname, self.parent_node.cprefix), self.cname))

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
    __ctor = function(self, cname, parent, mixins, ch, evs)
        self.cname      = cname
        self.parent     = parent
        self.interfaces = interfaces
        self.mixins     = mixins
        self.prefix     = eolian.class_eo_prefix_get(cname)
        self.children   = ch
        self.events     = evs
    end,

    generate = function(self, s)
        dom:log(log.level.INFO, "  Generating for class: " .. self.cname)

        s:write("ffi.cdef [[\n")
        self:gen_ffi(s)
        s:write("]]\n\n")

        local name_stripped = strip_name(self, self.cname,
            self.parent_node.cprefix)

        s:write(([[
local Parent = eo.class_get("%s")
M.%s = eo.class_register("%s", Parent:clone {
]]):format(self.parent, name_stripped, self.cname))

        self:gen_children(s)

        s:write("})")

        for i, v in ipairs(self.mixins) do
            s:write(("\nM.%s:mixin(eo.class_get(\"%s\"))\n")
                :format(name_stripped, v))
        end
    end,

    gen_ffi = Mixin.gen_ffi,
    gen_ctor = Mixin.gen_ctor
}

local File = Node:clone {
    __ctor = function(self, fname, cname, modname, libname, cprefix, ch)
        self.fname    = fname:match(".+/(.+)") or fname
        self.cname    = cname
        self.modname  = (modname and #modname > 0) and modname or nil
        self.libname  = libname
        self.cprefix  = cprefix
        self.children = ch
    end,

    generate = function(self, s)
        dom:log(log.level.INFO, "Generating for file: " .. self.fname)
        dom:log(log.level.INFO, "  Class            : " .. self.cname)

        local modn = self.modname
        if    modn then
              modn = ("require(\"%s\")"):format(modn)
        else
              modn = "{}"
        end

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

]]):format(self.fname, self.cname, modn, self.libname, self.libname))

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

local gen_contents = function(classn)
    local cnt = {}
    local ft  = eolian.function_type
    -- first try properties
    local props = eolian.class_functions_list_get(classn, ft.PROPERTY)
    for i, v in ipairs(props) do
        if v:scope_get() == eolian.function_scope.PUBLIC then
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
    local meths = eolian.class_functions_list_get(classn, ft.METHOD)
    for i, v in ipairs(meths) do
        if v:scope_get() == eolian.function_scope.PUBLIC then
            cnt[#cnt + 1] = Method(v)
        end
    end
    -- and constructors
    local ctors = eolian.class_functions_list_get(classn, ft.CTOR)
    for i, v in ipairs(ctors) do
        cnt[#cnt + 1] = Constructor(v)
    end
    if #ctors == 0 then
        cnt[#cnt + 1] = Default_Constructor()
    end
    -- events
    local evs = {}
    local events = eolian.class_events_list_get(classn)
    for i, v in ipairs(events) do
        local en, et, ed = v:information_get()
        evs[#evs + 1] = Event(en, et, ed)
    end
    return cnt, evs
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
        if tp == ct.REGULAR or tp == ct.ABSTRACT then
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

M.include_dir = function(dir)
    if not eolian.directory_scan(dir) then
        error("Failed including directory: " .. dir)
    end
end

M.generate = function(fname, modname, libname, cprefix, fstream)
    if not eolian.eo_file_parse(fname) then
        error("Failed parsing file: " .. fname)
    end
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
    File(fname, classn, modname, libname, cprefix, { cl })
        :generate(fstream or io.stdout)
end

return M