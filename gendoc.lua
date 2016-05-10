local eolian = require("eolian")
local getopt = require("getopt")
local cutil = require("cutil")
local util = require("util")
local ffi = require("ffi")

local global_opts = {}

-- utils

local path_sep, rep_sep = "/", "\\"
if ffi.os == "Windows" then
    path_sep, rep_sep = rep_sep, path_sep
end

local path_join = function(...)
    return table.concat({ ... }, path_sep):gsub(rep_sep, path_sep)
end

local path_to_nspace = function(p)
    return p:gsub(rep_sep, ":"):gsub(path_sep, ":"):lower()
end

local nspace_to_path = function(ns)
    return ns:gsub(":", path_sep):gsub(rep_sep, path_sep):lower()
end

local make_page = function(path)
    return path_join(global_opts.doc_root, path .. ".txt")
end

local mkdir_r = function(dirn)
    local dr = global_opts.doc_root
    assert(cutil.file_mkpath(dirn and path_join(dr, dirn) or dr))
end

local mkdir_p = function(path)
    mkdir_r(path:match("(.+)" .. path_sep .. "([^" .. path_sep .. "]+)"))
end

local str_split = function(str, delim)
    if not str then
        return nil
    end
    local s, e = str:find(delim, 1, true)
    if not s then
        return { str }
    end
    local t = {}
    while s do
        t[#t + 1] = str:sub(1, s - 1)
        str = str:sub(e + 1)
        s, e = str:find(delim, 1, true)
        if not s then
            t[#t + 1] = str
        end
    end
    return t
end

-- translation tables and funcs

local classt_to_str = {
    [eolian.class_type.REGULAR] = "class",
    [eolian.class_type.ABSTRACT] = "class",
    [eolian.class_type.MIXIN] = "mixin",
    [eolian.class_type.INTERFACE] = "interface"
}

local funct_to_str = {
    [eolian.function_type.PROPERTY] = "property",
    [eolian.function_type.PROP_GET] = "property",
    [eolian.function_type.PROP_SET] = "property",
    [eolian.function_type.METHOD] = "method"
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
        local ret = classt_to_str[decl:class_get():type_get()]
        if not ret then
            error("unknown class type for class '" .. decl:name_get() .. "'")
        end
        return ret
    else
        error("unknown declaration type for declaration '"
            .. decl:name_get() .. "'")
    end
end

local gen_nsp_eo = function(eobj, subn, root)
    local tbl = eobj:namespaces_get():to_array()
    for i = 1, #tbl do
        tbl[i] = tbl[i]:lower()
    end
    table.insert(tbl, 1, subn)
    tbl[#tbl + 1] = eobj:name_get():lower()
    if root then
        tbl[#tbl + 1] = true
    end
    return tbl
end

local gen_nsp_class = function(cl, root)
    return gen_nsp_eo(cl, classt_to_str[cl:type_get()], root)
end

local gen_nsp_func = function(fn, cl, root)
    local tbl = gen_nsp_class(cl)
    tbl[#tbl + 1] = funct_to_str[fn:type_get()]
    tbl[#tbl + 1] = fn:name_get():lower()
    if root then
        tbl[#tbl + 1] = true
    end
    return tbl
end

local gen_nsp_ref
gen_nsp_ref = function(str, root)
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
            return gen_nsp_ref(bstr, root)
        end
    end

    local ftp = eolian.function_type

    local cl = eolian.class_get_by_name(bstr)
    local fn
    local ftype = ftp.UNRESOLVED
    if not cl then
        if sfx == ".get" then
            ftype = ftp.PROP_GET
        elseif sfx == ".set" then
            ftype = ftp.PROP_SET
        end
        local mname
        if ftype ~= ftp.UNRESOLVED then
            mname = bstr:match(".+%.([^.]+)")
            if not mname then
                error("invalid reference '" .. str .. "'")
            end
            bstr = bstr:match("(.+)%.[^.]+")
            cl = eolian.class_get_by_name(bstr)
            if cl then
                fn = cl:function_get_by_name(mname, ftype)
            end
        end
    else
        fn = cl:function_get_by_name(sfx:sub(2), ftype)
        if fn then ftype = fn:type_get() end
    end

    if not fn or not funct_to_str[ftype] then
        error("invalid reference '" .. str .. "'")
    end

    local ret = gen_nsp_ref(bstr)
    ret[#ret + 1] = funct_to_str[ftype]
    ret[#ret + 1] = fn:name_get():lower()
    if root then ret[#ret + 1] = true end
    return ret
end

-- statistics

local stats = {}

local stats_pd = function(n)
    local ret = 0
    if n == 0 then
        return 1
    end
    while (n ~= 0) do
        n = math.floor(n / 10)
        ret = ret + 1
    end
    return ret
end

local fcol = 30
local ncol = 0

local print_stat = function(printname, statname, sub)
    local sv = stats[statname] or 0
    local svu = stats[statname .. "_undoc"] or 0
    local percent = (sv == 0) and 100 or math.floor(((sv - svu) / sv) * 100 + 0.5)
    local tb = (" "):rep(math.max(0, fcol - #printname - 1) + ncol - stats_pd(sv))
    local dtb = (" "):rep(ncol - stats_pd(sv - svu))
    local ptb = (" "):rep(3 - stats_pd(percent))
    print(("%s:%s%d (documented: %s%d or %s%d%%)")
        :format(printname, tb, sv, dtb, sv - svu, ptb, percent))
end

local print_stats = function()
    for k, v in pairs(stats) do
        ncol = math.max(ncol, stats_pd(v))
    end

    print("=== CLASS SECTION ===\n")
    print_stat("Classes", "class")
    print_stat("Interfaces", "interface")
    print_stat("Mixins", "mixin")
    print_stat("Events", "event")

    print("\n=== FUNCTION SECTION ===\n")
    print_stat("Methods", "method")
    print_stat("  Method params", "param")
    print_stat("  Method returns", "mret")
    print_stat("Getters", "getter")
    print_stat("  Getter returns", "gret")
    print_stat("  Getter keys", "gkey")
    print_stat("  Getter values", "gvalue")
    print_stat("Setters", "setter")
    print_stat("  Setter returns", "sret")
    print_stat("  Setter keys", "skey")
    print_stat("  Setter values", "svalue")

    print("\n=== TYPE SECTION ===\n")
    print_stat("Aliases", "alias")
    print_stat("Structs", "struct")
    print_stat("Struct fields", "sfield")
    print_stat("Enums", "enum")
    print_stat("Enum fields", "efield")

    print("\n=== VARIABLE SECTION ===\n")
    print_stat("Constants", "constant")
    print_stat("Globals", "global")
end

local stat_incr = function(name, missing)
    if not stats[name] then
        stats[name], stats[name .. "_undoc"] = 0, 0
    end
    stats[name] = stats[name] + 1
    if missing then
        stats[name .. "_undoc"] = stats[name .. "_undoc"] + 1
    end
end

local print_missing = function(name, tp)
    if not global_opts.verbose then
        return
    end
    print(tp .. " '" .. name .. "'" .. " missing documentation")
end

local check_class = function(cl)
    local ct = classt_to_str[cl:type_get()]
    if not ct then
        return
    end
    if not cl:documentation_get() then
        print_missing(cl:full_name_get(), ct)
        stat_incr(ct, true)
    else
        stat_incr(ct, false)
    end

    for ev in cl:events_get() do
        if not ev:documentation_get() then
            print_missing(cl:full_name_get() .. "." .. ev:name_get(), "event")
            stat_incr("event", true)
        else
            stat_incr("event", false)
        end
    end
end

local check_method = function(fn, cl)
    local fts = eolian.function_type
    local fulln = cl:full_name_get() .. "." .. fn:name_get()
    if fn:return_type_get(fts.METHOD) then
        if not fn:return_documentation_get(fts.METHOD) then
            print_missing(fulln, "method return")
            stat_incr("mret", true)
        else
            stat_incr("mret", false)
        end
    end
    if not fn:documentation_get(fts.METHOD) then
        print_missing(fulln, "method")
        stat_incr("method", true)
    else
        stat_incr("method", false)
    end
    for p in fn:parameters_get() do
        if not p:documentation_get() then
            print_missing(fulln .. "." .. p:name_get(), "method parameter")
            stat_incr("param", true)
        else
            stat_incr("param", false)
        end
    end
end

local check_property = function(fn, cl, ft)
    local fts = eolian.function_type

    local pfxs = {
        [fts.PROP_GET] = "g",
        [fts.PROP_SET] = "s",
    }
    local pfx = pfxs[ft]

    local fulln = cl:full_name_get() .. "." .. fn:name_get()
    if fn:return_type_get(ft) then
        if not fn:return_documentation_get(ft) then
            print_missing(fulln, pfx .. "etter return")
            stat_incr(pfx .. "ret", true)
        else
            stat_incr(pfx .. "ret", false)
        end
    end

    if not fn:documentation_get(fts.PROPERTY) and not fn:documentation_get(ft) then
        print_missing(fulln, pfx .. "etter")
        stat_incr(pfx .. "etter", true)
    else
        stat_incr(pfx .. "etter", false)
    end

    for p in fn:property_keys_get(ft) do
        if not p:documentation_get() then
            print_missing(fulln .. "." .. p:name_get(), pfx .. "etter key")
            stat_incr(pfx .. "key", true)
        else
            stat_incr(pfx .. "key", false)
        end
    end

    for p in fn:property_values_get(ft) do
        if not p:documentation_get() then
            print_missing(fulln .. "." .. p:name_get(), pfx .. "etter value")
            stat_incr(pfx .. "value", true)
        else
            stat_incr(pfx .. "value", false)
        end
    end
end

local check_alias = function(v)
    if not v:documentation_get() then
        print_missing(v:full_name_get(), "alias")
        stat_incr("alias", true)
    else
        stat_incr("alias", false)
    end
end

local check_struct = function(v)
    if not v:documentation_get() then
        print_missing(v:full_name_get(), "struct")
        stat_incr("struct", true)
    else
        stat_incr("struct", false)
    end
    for fl in v:struct_fields_get() do
        if not fl:documentation_get() then
            print_missing(v:full_name_get() .. "." .. fl:name_get(), "struct field")
            stat_incr("sfield", true)
        else
            stat_incr("sfield", false)
        end
    end
end

local check_enum = function(v)
    if not v:documentation_get() then
        print_missing(v:full_name_get(), "enum")
        stat_incr("enum", true)
    else
        stat_incr("enum", false)
    end
    for fl in v:enum_fields_get() do
        if not fl:documentation_get() then
            print_missing(v:full_name_get() .. "." .. fl:name_get(), "enum field")
            stat_incr("efield", true)
        else
            stat_incr("efield", false)
        end
    end
end

local check_constant = function(v)
    if not v:documentation_get() then
        print_missing(v:full_name_get(), "constant")
        stat_incr("constant", true)
    else
        stat_incr("constant", false)
    end
end

local check_global = function(v)
    if not v:documentation_get() then
        print_missing(v:full_name_get(), "global")
        stat_incr("global", true)
    else
        stat_incr("global", false)
    end
end

-- generator

local Writer = util.Object:clone {
    __ctor = function(self, path)
        local subs
        if type(path) == "table" then
            subs = path_join(unpack(path))
        else
            subs = nspace_to_path(path)
        end
        mkdir_p(subs)
        self.file = assert(io.open(make_page(subs), "w"))
    end,

    write_raw = function(self, ...)
        self.file:write(...)
        return self
    end,

    write_nl = function(self, n)
        self:write_raw(("\n"):rep(n or 1))
        return self
    end,

    write_h = function(self, heading, level, nonl)
        local s = ("="):rep(7 - level)
        self:write_raw(s, " ", heading, " ", s, "\n")
        if not nonl then
            self:write_nl()
        end
        return self
    end,

    write_fmt = function(self, fmt1, fmt2, ...)
        self:write_raw(fmt1, ...)
        self:write_raw(fmt2)
        return self
    end,

    write_b = function(self, ...)
        self:write_fmt("**", "**", ...)
        return self
    end,

    write_i = function(self, ...)
        self:write_fmt("//", "//", ...)
        return self
    end,

    write_u = function(self, ...)
        self:write_fmt("__", "__", ...)
        return self
    end,

    write_s = function(self, ...)
        self:write_fmt("<del>", "</del>", ...)
        return self
    end,

    write_m = function(self, ...)
        self:write_fmt("''", "''", ...)
        return self
    end,

    write_sub = function(self, ...)
        self:write_fmt("<sub>", "</sub>", ...)
        return self
    end,

    write_sup = function(self, ...)
        self:write_fmt("<sup>", "</sup>", ...)
        return self
    end,

    write_br = function(self, nl)
        self:write_raw("\\\\", nl and "\n" or " ")
        return self
    end,

    write_pre_inline = function(self, ...)
        self:write_fmt("%%", "%%", ...)
        return self
    end,

    write_pre = function(self, ...)
        self:write_fmt("<nowiki>\n", "\n</nowiki>", ...)
        return self
    end,

    write_code = function(self, str, lang)
        lang = lang and (" " .. lang) or ""
        self:write_raw("<code" .. lang .. ">\n", str, "\n</code>\n")
    end,

    write_link = function(self, target, title)
        if type(target) == "table" then
            if target[#target] == true then
                target[#target] = nil
                target = ":" .. global_opts.root_nspace .. ":"
                             .. table.concat(target, ":")
            else
                target = table.concat(target, ":")
            end
        end
        if not title then
            self:write_raw("[[", target:lower(), "|", target, "]]")
            return
        end
        target = target:lower()
        if type(title) == "string" then
            self:write_raw("[[", target, "|", title, "]]")
            return self
        end
        self:write_raw("[[", target, "|")
        title(self)
        self:write_raw("]]")
        return self
    end,

    write_graph = function(self, tbl)
        self:write_raw("<graphviz>\n")
        self:write_raw("digraph ", tbl.type, " {\n")

        for k, v in pairs(tbl.attrs or {}) do
            self:write_raw("    ", k, " = \"", v, "\"\n")
        end

        local write_node = function(nname, attrs)
            self:write_raw("    ", nname, " [")
            local first = true
            for k, v in pairs(attrs) do
                if not first then
                    self:write_raw(", ")
                end
                self:write_raw(k, " = \"", v, "\"")
                first = false
            end
            self:write_raw("]\n")
        end

        if tbl.node then
            self:write_nl()
            write_node("node", tbl.node)
        end
        if tbl.edge then
            if not tbl.node then self:write_nl() end
            write_node("edge", tbl.edge)
        end

        self:write_nl()
        for i, v in ipairs(tbl.nodes) do
            local nname = v.name
            v.name = nil
            write_node(nname, v)
        end

        self:write_nl()
        for i, v in ipairs(tbl.connections) do
            local from, to, sep = v[1], v[2], (v[3] or "->")
            if type(from) == "table" then
                self:write_raw("    {", table.concat(from, ", "), "}")
            else
                self:write_raw("    ", from)
            end
            self:write_raw(" ", sep, " ")
            if type(to) == "table" then
                self:write_raw("{", table.concat(to, ", "), "}")
            else
                self:write_raw(to)
            end
            self:write_nl()
        end

        self:write_raw("}\n</graphviz>")
    end,

    write_table = function(self, titles, tbl)
        self:write_raw("^ ", table.concat(titles, " ^ "), " ^\n")
        for i, v in ipairs(tbl) do
            self:write_raw("| ", table.concat(v,  " | "), " |\n")
        end
        return self
    end,

    write_list = function(self, tbl, ord)
        local prec = ord and "-" or "*"
        for i, v in ipairs(tbl) do
            local lvl, str = 1, v
            if type(v) == "table" then
                lvl, str = v[1] + 1, v[2]
            end
            local pbeg, pend = str:match("([^\n]+)\n(.+)")
            if not pbeg then
                pbeg = str
            end
            self:write_raw(("  "):rep(lvl), prec, " ", str, "\n")
            if pend then
                self:write_raw(pend, "\n")
            end
        end
        return self
    end,

    write_par_markup = function(self, str)
        self:write_raw("%%")
        local f = str:gmatch(".")
        local c = f()
        while c do
            if c == "\\" then
                c = f()
                if c ~= "@" and c ~= "$" then
                    self:write_raw("\\")
                end
                self:write_raw(c)
                c = f()
            elseif c == "$" then
                c = f()
                if c and c:match("[a-zA-Z_]") then
                    local wbuf = { c }
                    c = f()
                    while c and c:match("[a-zA-Z0-9_]") do
                        wbuf[#wbuf + 1] = c
                        c = f()
                    end
                    self:write_raw("%%''" .. table.concat(wbuf) .. "''%%")
                else
                    self:write_raw("$")
                end
            elseif c == "@" then
                c = f()
                if c and c:match("[a-zA-Z_]") then
                    local rbuf = { c }
                    c = f()
                    while c and c:match("[a-zA-Z0-9_.]") do
                        rbuf[#rbuf + 1] = c
                        c = f()
                    end
                    local ldot = false
                    if rbuf[#rbuf] == "." then
                        ldot = true
                        rbuf[#rbuf] = nil
                    end
                    local title = table.concat(rbuf)
                    self:write_raw("%%")
                    self:write_link(gen_nsp_ref(title, true), title)
                    self:write_raw("%%")
                    if ldot then
                        self:write_raw(".")
                    end
                else
                    self:write_raw("@")
                end
            elseif c == "%" then
                c = f()
                if c == "%" then
                    c = f()
                    self:write_raw("%%<nowiki>%%</nowiki>%%")
                else
                    self:write_raw("%")
                end
            else
                self:write_raw(c)
                c = f()
            end
        end
        self:write_raw("%%")
        return self
    end,

    write_par = function(self, str)
        local notetypes = global_opts.use_notes and {
            ["Note: "] = "<note>\n",
            ["Warning: "] = "<note warning>\n",
            ["Remark: "] = "<note tip>\n",
            ["TODO: "] = "<note>\n**TODO:** "
        } or {}
        local tag
        for k, v in pairs(notetypes) do
            if str:match("^" .. k) then
                tag = v
                str = str:sub(#k + 1)
                break
            end
        end
        if tag then
            self:write_raw(tag)
            self:write_par_markup(str)
            self:write_raw("\n</note>")
        else
            self:write_par_markup(str)
        end
        return self
    end,

    finish = function(self)
        self.file:close()
    end
}

local Buffer = Writer:clone {
    __ctor = function(self)
        self.buf = {}
    end,

    write_raw = function(self, ...)
        for i, v in ipairs({ ... }) do
            self.buf[#self.buf + 1] = v
        end
        return self
    end,

    finish = function(self)
        self.result = table.concat(self.buf)
        self.buf = {}
        return self.result
    end
}

-- eolian to various doc elements conversions

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
    return str
end

local get_type_str
get_type_str = function(tp)
    local tps = eolian.type_type
    local tpt = tp:type_get()
    if tpt == tps.UNKNOWN then
        error("unknown type: " .. tp:full_name_get())
    elseif tpt == tps.VOID then
        return wrap_type_attrs(tp, "void")
    elseif tpt == tps.UNDEFINED then
        return wrap_type_attrs(tp, "__undefined_type")
    elseif tpt == tps.REGULAR or tpt == tps.CLASS then
        return wrap_type_attrs(tp, tp:full_name_get())
    elseif tpt == tps.COMPLEX then
        local stypes = {}
        for stp in tp:subtypes_get() do
            stypes[#stypes + 1] = get_type_str(stp)
        end
        return wrap_type_attrs(tp, tp:full_name_get() .. "<"
            .. table.concat(stypes, ", ") .. ">")
    elseif tpt == tps.POINTER then
        local btp = tp:base_type_get()
        local suffix = " *"
        if btp:type_get() == tps.POINTER then
            suffix = "*"
        end
        return wrap_type_attrs(tp, get_type_str(btp) .. suffix)
    end
    error("unhandled type type: " .. tpt)
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

local get_typedecl_str = function(tp)
    local tps = eolian.typedecl_type
    local tpt = tp:type_get()
    if tpt == tps.UNKNOWN then
        error("unknown typedecl: " .. tp:full_name_get())
    elseif tpt == tps.STRUCT or tpt == tps.STRUCT_OPAQUE then
        local buf = { "struct " }
        add_typedecl_attrs(tp, buf)
        buf[#buf + 1] = tp:full_name_get()
        if tpt == tps.STRUCT_OPAQUE then
            buf[#buf + 1] = ";"
            return table.concat(buf)
        end
        local fields = tp:struct_fields_get():to_array()
        if #fields == 0 then
            buf[#buf + 1] = " {}"
            return table.concat(buf)
        end
        buf[#buf + 1] = " {\n"
        for i, fld in ipairs(fields) do
            buf[#buf + 1] = "    "
            buf[#buf + 1] = fld:name_get()
            buf[#buf + 1] = ": "
            buf[#buf + 1] = get_type_str(fld:type_get())
            buf[#buf + 1] = ";\n"
        end
        buf[#buf + 1] = "}"
        return table.concat(buf)
    elseif tpt == tps.ENUM then
        local buf = { "enum " }
        add_typedecl_attrs(tp, buf)
        buf[#buf + 1] = tp:full_name_get()
        local fields = tp:enum_fields_get():to_array()
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
    elseif tpt == tps.ALIAS then
        local buf = { "type " }
        add_typedecl_attrs(tp, buf)
        buf[#buf + 1] = tp:full_name_get()
        buf[#buf + 1] = ": "
        buf[#buf + 1] = get_type_str(tp:base_type_get())
        buf[#buf + 1] = ";"
        return table.concat(buf)
    end
    error("unhandled typedecl type: " .. tpt)
end

local gen_doc_refd = function(str)
    if not str then
        return nil
    end
    local pars = str_split(str, "\n\n")
    for i = 1, #pars do
        pars[i] = Buffer():write_par(pars[i]):finish()
    end
    return table.concat(pars, "\n\n")
end

local get_fallback_fdoc = function(f, ftype)
    if not ftype then
        local ft = f:type_get()
        local ftt = eolian.function_type
        if ft == ftt.PROP_GET or ft == ftt.PROP_SET then
            ftype = ft
        end
    end
    if ftype then
        return f:documentation_get(ftype)
    end
    return nil
end

local get_brief_doc = function(doc1, doc2)
    if not doc1 and not doc2 then
        return "No description supplied."
    end
    if not doc1 then
        doc1, doc2 = doc2, doc1
    end
    return gen_doc_refd(doc1:summary_get())
end

local get_brief_fdoc = function(f, ftype)
    return get_brief_doc(f:documentation_get(eolian.function_type.METHOD),
                         get_fallback_fdoc(f, ftype))
end

local get_full_doc = function(doc1, doc2)
    if not doc1 and not doc2 then
        return "No description supplied."
    end
    if not doc1 then
        doc1, doc2 = doc2, doc1
    end
    local sum1 = doc1:summary_get()
    local desc1 = doc1:description_get()
    local edoc = ""
    if doc2 then
        local sum2 = doc2:summary_get()
        local desc2 = doc2:description_get()
        if not desc2 then
            if sum2 then edoc = "\n\n" .. sum2 end
        else
            edoc = "\n\n" .. sum2 .. "\n\n" .. desc2
        end
    end
    if not desc1 then
        return gen_doc_refd(sum1 .. edoc)
    end
    return gen_doc_refd(sum1 .. "\n\n" .. desc1 .. edoc)
end

local get_full_fdoc = function(f, ftype)
    return get_full_doc(f:documentation_get(eolian.function_type.METHOD),
                        get_fallback_fdoc(f, ftype))
end

local propt_to_type = {
    [eolian.function_type.PROPERTY] = "(get, set)",
    [eolian.function_type.PROP_GET] = "(get)",
    [eolian.function_type.PROP_SET] = "(set)",
}

local gen_func_sig = function(f, ftype)
    ftype = ftype or eolian.function_type.METHOD
end

local gen_cparam = function(par, out)
    local part = par:type_get()
    local tstr
    out = out or (par:direction_get() == eolian.parameter_dir.OUT)
    if part:type_get() == eolian.type_type.POINTER then
        tstr = part:c_type_get()
        if out then
            tstr = tstr .. "*"
        end
    elseif out then
        tstr = part:c_type_get() .. " *"
    else
        tstr = part:c_type_get() .. " "
    end
    return tstr .. par:name_get()
end

local gen_func_csig = function(f, ftype)
    ftype = ftype or eolian.function_type.METHOD
    assert(ftype ~= eolian.function_type.PROPERTY)

    local cn = f:full_c_name_get(ftype)
    local rtype = f:return_type_get(ftype)

    local fparam = "Eo *obj"
    if f:is_const() or f:is_class() or ftype == eolian.function_type.PROP_GET then
        fparam = "const Eo *obj"
    end

    if f:type_get() == eolian.function_type.METHOD then
        local pars = f:parameters_get():to_array()
        local cnrt = rtype and rtype:c_type_named_get(cn) or ("void " .. cn)
        for i = 1, #pars do
            pars[i] = gen_cparam(pars[i])
        end
        table.insert(pars, 1, fparam);
        return cnrt .. "(" .. table.concat(pars, ", ") .. ");"
    end

    local keys = f:property_keys_get(ftype):to_array()
    local vals = f:property_values_get(ftype):to_array()

    if ftype == eolian.function_type.PROP_SET then
        local cnrt = rtype and rtype:c_type_named_get(cn) or ("void " .. cn)
        local pars = {}
        for i, par in ipairs(keys) do
            pars[#pars + 1] = gen_cparam(par)
        end
        for i, par in ipairs(vals) do
            pars[#pars + 1] = gen_cparam(par)
        end
        table.insert(pars, 1, fparam);
        return cnrt .. "(" .. table.concat(pars, ", ") .. ");"
    end

    -- getters
    local cnrt
    if not rtype then
        if #vals == 1 then
            cnrt = vals[1]:type_get():c_type_named_get(cn)
            table.remove(vals, 1)
        else
            cnrt = "void " .. cn
        end
    else
        cnrt = rtype:c_type_named_get(cn)
    end
    local pars = {}
    for i, par in ipairs(keys) do
        pars[#pars + 1] = gen_cparam(par)
    end
    for i, par in ipairs(vals) do
        pars[#pars + 1] = gen_cparam(par, true)
    end
    table.insert(pars, 1, fparam);
    return cnrt .. "(" .. table.concat(pars, ", ") .. ");"
end

local gen_func_namesig = function(fn, cl, buf)
    if fn:type_get() ~= eolian.function_type.METHOD then
        buf[#buf + 1] = "@property "
    end
    buf[#buf + 1] = cl:full_name_get()
    buf[#buf + 1] = "."
    buf[#buf + 1] = fn:name_get()
    buf[#buf + 1] = " "
    if fn:scope_get() == eolian.object_scope.PROTECTED then
        buf[#buf + 1] = "@protected "
    end
    if fn:is_class() then
        buf[#buf + 1] = "@class "
    end
    if fn:is_const() then
        buf[#buf + 1] = "@const "
    end
    if fn:is_c_only() then
        buf[#buf + 1] = "@c_only "
    end
end

local gen_func_param = function(fp, buf, nodir)
    -- TODO: default value
    buf[#buf + 1] = "        "
    local dirs = {
        [eolian.parameter_dir.IN] = "@in ",
        [eolian.parameter_dir.OUT] = "@out ",
        [eolian.parameter_dir.INOUT] = "@inout ",
    }
    if not nodir then buf[#buf + 1] = dirs[fp:direction_get()] end
    buf[#buf + 1] = fp:name_get()
    buf[#buf + 1] = ": "
    buf[#buf + 1] = get_type_str(fp:type_get())
    local dval = fp:default_value_get()
    if dval then
        buf[#buf + 1] = " ("
        buf[#buf + 1] = dval:serialize()
        buf[#buf + 1] = ")"
    end
    if fp:is_nonull() then
        buf[#buf + 1] = " @nonull"
    end
    if fp:is_nullable() then
        buf[#buf + 1] = " @nullable"
    end
    if fp:is_optional() then
        buf[#buf + 1] = " @optional"
    end
    buf[#buf + 1] = ";\n"
end

local gen_func_return = function(fp, ftype, buf, indent)
    local rett = fp:return_type_get(ftype)
    if not rett then
        return
    end
    buf[#buf + 1] = indent and ("    "):rep(indent) or "    "
    buf[#buf + 1] = "return: "
    buf[#buf + 1] = get_type_str(rett)
    local dval = fp:return_default_value_get(ftype)
    if dval then
        buf[#buf + 1] = " ("
        buf[#buf + 1] = dval:serialize()
        buf[#buf + 1] = ")"
    end
    if fp:return_is_warn_unused(ftype) then
        buf[#buf + 1] = " @warn_unused"
    end
    buf[#buf + 1] = ";\n"
end

local gen_method_sig = function(fn, cl)
    local buf = {}
    gen_func_namesig(fn, cl, buf)
    if fn:is_virtual_pure(eolian.function_type.METHOD) then
        buf[#buf + 1] = "@virtual_pure "
    end
    buf[#buf + 1] = "{"
    local params = fn:parameters_get():to_array()
    local rtp = fn:return_type_get(eolian.function_type.METHOD)
    if #params == 0 and not rtp then
        buf[#buf + 1] = "}"
        return table.concat(buf)
    end
    buf[#buf + 1] = "\n"
    if #params > 0 then
        buf[#buf + 1] = "    params {\n"
        for i, fp in ipairs(params) do
            gen_func_param(fp, buf)
        end
        buf[#buf + 1] = "    }\n"
    end
    gen_func_return(fn, eolian.function_type.METHOD, buf)
    buf[#buf + 1] = "}"
    return table.concat(buf)
end

local eovals_check_same = function(a1, a2)
    if #a1 ~= #a2 then return false end
    for i, v in ipairs(a1) do
        if v ~= a2[i] then return false end
    end
    return true
end

local gen_prop_keyvals = function(tbl, kword, buf, indent)
    local ind = indent and ("    "):rep(indent) or "    "
    if #tbl == 0 then return end
    buf[#buf + 1] = "    "
    buf[#buf + 1] = ind
    buf[#buf + 1] = kword
    buf[#buf + 1] = " {\n"
    for i, v in ipairs(tbl) do
        buf[#buf + 1] = ind
        gen_func_param(v, buf, true)
    end
    buf[#buf + 1] = "    "
    buf[#buf + 1] = ind
    buf[#buf + 1] = "}\n"
end

local gen_prop_sig = function(fn, cl)
    local buf = {}
    gen_func_namesig(fn, cl, buf)
    local fnt = fn:type_get()
    local ftt = eolian.function_type
    local isget = (fnt == ftt.PROPERTY or fnt == ftt.PROP_GET)
    local isset = (fnt == ftt.PROPERTY or fnt == ftt.PROP_SET)

    local gvirt = fn:is_virtual_pure(ftt.PROP_GET)
    local svirt = fn:is_virtual_pure(ftt.PROP_SET)

    if (not isget or gvirt) and (not isset or svirt) then
        buf[#buf + 1] = "@virtual_pure "
    end

    local gkeys = isget and fn:property_keys_get(ftt.PROP_GET):to_array() or {}
    local skeys = isset and fn:property_keys_get(ftt.PROP_SET):to_array() or {}
    local gvals = isget and fn:property_values_get(ftt.PROP_GET):to_array() or {}
    local svals = isget and fn:property_values_get(ftt.PROP_SET):to_array() or {}
    local grtt = isget and fn:return_type_get(ftt.PROP_GET) or nil
    local srtt = isset and fn:return_type_get(ftt.PROP_SET) or nil

    local keys_same = eovals_check_same(gkeys, skeys)
    local vals_same = eovals_check_same(gvals, svals)

    buf[#buf + 1] = "{\n"

    if isget then
        buf[#buf + 1] = "    get {"
        if (#gkeys == 0 or keys_same) and (#gvals == 0 or vals_same) and
           (not grtt or grtt == srtt) then
            buf[#buf + 1] = "}\n"
        else
            buf[#buf + 1] = "\n"
            if not keys_same then gen_prop_keyvals(gkeys, "keys", buf) end
            if not vals_same then gen_prop_keyvals(gvals, "values", buf) end
            if grtt ~= srtt then
                gen_func_return(fn, ftt.PROP_GET, buf, 2)
            end
            buf[#buf + 1] = "    }\n"
        end
    end

    if isset then
        buf[#buf + 1] = "    set {"
        if (#skeys == 0 or keys_same) and (#svals == 0 or vals_same) and
           (not srtt or grtt == srtt) then
            buf[#buf + 1] = "}\n"
        else
            buf[#buf + 1] = "\n"
            if not keys_same then gen_prop_keyvals(skeys, "keys", buf) end
            if not vals_same then gen_prop_keyvals(svals, "values", buf) end
            if grtt ~= srtt then
                gen_func_return(fn, ftt.PROP_SET, buf, 2)
            end
            buf[#buf + 1] = "    }\n"
        end
    end

    if keys_same then gen_prop_keyvals(gkeys, "keys", buf, 0) end
    if vals_same then gen_prop_keyvals(gvals, "values", buf, 0) end

    buf[#buf + 1] = "}"
    return table.concat(buf)
end

local get_property_sig = function(fn, cl)
    local buf = {}
    gen_func_namesig(fn, cl, buf)
    buf[#buf + 1] = "{"
    buf[#buf + 1] = "}"
    return table.concat(buf)
end

-- builders

local build_method, build_property

local build_reftable = function(f, title, ctitle, ctype, t)
    if not t or #t == 0 then
        return
    end
    f:write_h(title, 2)
    local nt = {}
    for i, v in ipairs(t) do
        nt[#nt + 1] = {
            Buffer():write_link(gen_nsp_eo(v, ctype, true),
                                v:full_name_get()):finish(),
            get_brief_doc(v:documentation_get())
        }
    end
    table.sort(nt, function(v1, v2) return v1[1] < v2[1] end)
    f:write_table({ ctitle, "Brief description" }, nt)
    f:write_nl()
end

local build_functable = function(f, title, ctitle, cl, tp)
    local t = cl:functions_get(tp):to_array()
    if #t == 0 then
        return
    end
    f:write_h(title, 3)
    local nt = {}
    for i, v in ipairs(t) do
        local lbuf = Buffer()
        lbuf:write_link(gen_nsp_func(v, cl, true), v:name_get())
        local pt = propt_to_type[v:type_get()]
        if pt then
            lbuf:write_raw(" ")
            lbuf:write_i(pt)
        end
        nt[#nt + 1] = {
            lbuf:finish(), get_brief_fdoc(v)
        }
        if funct_to_str[v:type_get()] == "property" then
            build_property(v, cl, linkt)
        else
            build_method(v, cl, linkt)
        end
    end
    table.sort(nt, function(v1, v2) return v1[1] < v2[1] end)
    f:write_table({ ctitle, "Brief description" }, nt)
    f:write_nl()
end

local build_ref = function()
    local f = Writer("reference")
    f:write_h("EFL Reference", 2)

    local classes = {}
    local ifaces = {}
    local mixins = {}

    local clt = eolian.class_type

    for cl in eolian.all_classes_get() do
        local tp = cl:type_get()
        if tp == clt.REGULAR or tp == clt.ABSTRACT then
            classes[#classes + 1] = cl
        elseif tp == clt.MIXIN then
            mixins[#mixins + 1] = cl
        elseif tp == clt.INTERFACE then
            ifaces[#ifaces + 1] = cl
        else
            error("unknown class: " .. cl:full_name_get())
        end
    end

    build_reftable(f, "Classes", "Class name", "class", classes)
    build_reftable(f, "Interfaces", "Interface name", "interface", ifaces)
    build_reftable(f, "Mixins", "Mixin name", "mixin", mixins)

    build_reftable(f, "Aliases", "Alias name", "alias",
        eolian.typedecl_all_aliases_get():to_array())

    build_reftable(f, "Structures", "Struct name", "struct",
        eolian.typedecl_all_structs_get():to_array())

    build_reftable(f, "Enums", "Enum name", "enum",
        eolian.typedecl_all_enums_get():to_array())

    build_reftable(f, "Constants", "Constant name", "constant",
        eolian.variable_all_constants_get():to_array())

    build_reftable(f, "Globals", "Global name", "global",
        eolian.variable_all_globals_get():to_array())

    f:finish()
end

local write_full_doc = function(f, doc1, doc2)
    f:write_raw(get_full_doc(doc1, doc2))
    local since
    if doc2 then
        since = doc2:since_get()
    end
    if not since and doc1 then
        since = doc1:since_get()
    end
    if since then
        f:write_nl(2)
        f:write_i("Since " .. since)
    end
end

local write_full_fdoc = function(f, fn, ftype)
    write_full_doc(f, fn:documentation_get(eolian.function_type.METHOD),
                   get_fallback_fdoc(fn, ftype))
end

local build_inherits
build_inherits = function(cl, t, lvl)
    t = t or {}
    lvl = lvl or 0
    local lbuf = Buffer()
    lbuf:write_link(gen_nsp_class(cl, true), cl:full_name_get())
    lbuf:write_raw(" ")
    lbuf:write_i("(" .. classt_to_str[cl:type_get()] .. ")")
    if lvl == 0 then
        lbuf:write_b(lbuf:finish())
    end
    t[#t + 1] = { lvl, lbuf:finish() }
    for cln in cl:inherits_get() do
        local acl = eolian.class_get_by_name(cln)
        if not acl then
            error("error retrieving inherited class " .. cln)
        end
        build_inherits(acl, t, lvl + 1)
    end
    return t
end


local class_to_color = function(cl)
    local classt_to_color = {
        [eolian.class_type.REGULAR] = { "black", "gray" },
        [eolian.class_type.ABSTRACT] = { "black", "gray" },
        [eolian.class_type.MIXIN] = { "blue", "skyblue" },
        [eolian.class_type.INTERFACE] = { "cornflowerblue", "azure" }
    }
    return classt_to_color[cl:type_get()]
end

local class_to_node = function(cl, main)
    local ret = {}

    ret.label = cl:full_name_get()
    ret.name = ret.label:lower():gsub("%.", "_")

    local clr = class_to_color(cl)
    ret.style = "filled"
    ret.color = clr[1]
    ret.fillcolor = main and clr[2] or "white"

    -- FIXME: need a dokuwiki graphviz plugin with proper URL support
    -- the existing one only supports raw URLs (no dokuwikí namespaces)
    --ret.URL = ":" .. global_opts.root_nspace .. ":"
    --              .. table.concat(gen_nsp_class(cl), ":")

    return ret
end

local build_igraph_r
build_igraph_r = function(cl, nbuf, ibuf)
    local sn = cl:full_name_get():lower():gsub("%.", "_")
    for cln in cl:inherits_get() do
        local acl = eolian.class_get_by_name(cln)
        if not acl then
            error("error retrieving inherited class " .. cln)
        end
        nbuf[#nbuf + 1] = class_to_node(acl)
        ibuf[#ibuf + 1] = { sn, (cln:lower():gsub("%.", "_")) }
        build_igraph_r(acl, nbuf, ibuf)
    end
end

local build_igraph = function(cl)
    local graph = {
        type = "hierarchy",
        attrs = {
            rankdir = "TB",
            size = "6",
            bgcolor = "transparent"
        },
        node = { shape = "box" }
    }

    local nbuf = {}
    local ibuf = {}
    nbuf[#nbuf + 1] = class_to_node(cl, true)
    build_igraph_r(cl, nbuf, ibuf)

    graph.nodes = nbuf
    graph.connections = ibuf

    return graph
end

local build_class = function(cl)
    local f = Writer(gen_nsp_class(cl))
    check_class(cl)

    f:write_h(cl:full_name_get(), 2)

    f:write_h("Inheritance hierarchy", 3)
    f:write_list(build_inherits(cl))
    f:write_nl()
    if global_opts.use_dot then
        f:write_graph(build_igraph(cl))
        f:write_nl(2)
    end

    f:write_h("Description", 3)
    write_full_doc(f, cl:documentation_get())
    f:write_nl(2)

    build_functable(f, "Methods", "Method name", cl, eolian.function_type.METHOD)
    build_functable(f, "Properties", "Property name",
        cl, eolian.function_type.PROPERTY)

    f:write_h("Events", 3)
    local evs = cl:events_get():to_array()
    if #evs == 0 then
        f:write_raw("This class does not define any events.\n")
    else
        local first = true
        for i, ev in ipairs(evs) do
            if not first then
                f:write_nl(2)
            end
            f:write_h(ev:name_get(), 4)
            write_full_doc(f, ev:documentation_get())
            first = false
        end
    end

    f:finish()
end

local build_classes = function()
    for cl in eolian.all_classes_get() do
        local ct = cl:type_get()
        if not classt_to_str[ct] then
            error("unknown class: " .. cl:full_name_get())
        end
        build_class(cl)
    end
end

local write_tsigs = function(f, tp)
    f:write_h(tp:full_name_get(), 2)

    f:write_h("Signature", 3)
    f:write_code(get_typedecl_str(tp))
    f:write_nl()

    f:write_h("C signature", 3)
    f:write_code(tp:c_type_get() .. ";")
    f:write_nl()
end

local build_alias = function(tp)
    local f = Writer(gen_nsp_eo(tp, "alias"))
    check_alias(tp)

    write_tsigs(f, tp)

    f:finish()
end

local build_struct = function(tp)
    local f = Writer(gen_nsp_eo(tp, "struct"))
    check_struct(tp)

    write_tsigs(f, tp)

    f:finish()
end

local build_enum = function(tp)
    local f = Writer(gen_nsp_eo(tp, "enum"))
    check_enum(tp)

    write_tsigs(f, tp)

    f:finish()
end

local build_variable = function(v, constant)
    local f = Writer(gen_nsp_eo(v, constant and "constant" or "global"))
    if constant then
        check_constant(v)
    else
        check_global(v)
    end

    f:finish()
end

local build_typedecls = function()
    for tp in eolian.typedecl_all_aliases_get() do
        build_alias(tp)
    end

    for tp in eolian.typedecl_all_structs_get() do
        build_struct(tp)
    end

    for tp in eolian.typedecl_all_enums_get() do
        build_enum(tp)
    end
end

local build_variables = function()
    for v in eolian.variable_all_constants_get() do
        build_variable(v, true)
    end

    for v in eolian.variable_all_globals_get() do
        build_variable(v, false)
    end
end

local pdir_to_str = {
    [eolian.parameter_dir.IN] = "(in)",
    [eolian.parameter_dir.OUT] = "(out)",
    [eolian.parameter_dir.INOUT] = "(inout)"
}

local build_parlist = function(f, pl, nodir)
    local params = {}
    for i, p in ipairs(pl) do
        local buf = Buffer()
        buf:write_b(p:name_get())
        if not nodir then
            buf:write_raw(" ")
            buf:write_i(pdir_to_str[p:direction_get()])
        end
        buf:write_raw(" - ", get_full_doc(p:documentation_get()))
        params[#params + 1] = buf:finish()
    end
    f:write_list(params)
end

local build_vallist = function(f, pg, ps, title)
    if #pg == #ps then
        local same = true
        for i = 1, #pg do
            if pg[i] ~= ps[i] then
                same = false
                break
            end
        end
        if same then ps = {} end
    end
    if #pg > 0 or #ps > 0 then
        f:write_h(title, 3)
        if #pg > 0 then
            if #ps > 0 then
                f:write_h("Getter", 4)
            end
            build_parlist(f, pg, true)
        end
        if #ps > 0 then
            if #pg > 0 then
                f:write_h("Setter", 4)
            end
            build_parlist(f, ps, true)
        end
    end
end

build_method = function(fn, cl)
    local f = Writer(gen_nsp_func(fn, cl))
    check_method(fn, cl)

    f:write_h(cl:full_name_get() .. "." .. fn:name_get(), 2)

    f:write_h("Signature", 3)
    f:write_code(gen_method_sig(fn, cl))
    f:write_nl()

    f:write_h("C signature", 3)
    f:write_code(gen_func_csig(fn), "c")
    f:write_nl()

    local pars = fn:parameters_get():to_array()
    if #pars > 0 then
        f:write_h("Parameters", 3)
        build_parlist(f, pars)
        f:write_nl()
    end

    f:write_h("Description", 3)
    write_full_doc(f, fn:documentation_get(eolian.function_type.METHOD))
    f:write_nl()

    f:finish()
end

build_property = function(fn, cl)
    local f = Writer(gen_nsp_func(fn, cl))

    local fts = eolian.function_type
    local ft = fn:type_get()
    local isget = (ft == fts.PROP_GET or ft == fts.PROPERTY)
    local isset = (ft == fts.PROP_SET or ft == fts.PROPERTY)

    if isget then check_property(fn, cl, fts.PROP_GET) end
    if isset then check_property(fn, cl, fts.PROP_SET) end

    local doc = fn:documentation_get(fts.PROPERTY)
    local gdoc = fn:documentation_get(fts.PROP_GET)
    local sdoc = fn:documentation_get(fts.PROP_SET)

    f:write_h(cl:full_name_get() .. "." .. fn:name_get(), 2)

    f:write_h("Signature", 3)
    f:write_code(gen_prop_sig(fn, cl))
    f:write_nl()

    f:write_h("C signature", 3)
    local codes = {}
    if isget then
        codes[#codes + 1] = gen_func_csig(fn, fts.PROP_GET)
    end
    if isset then
        codes[#codes + 1] = gen_func_csig(fn, fts.PROP_SET)
    end
    f:write_code(table.concat(codes, "\n"), "c")
    f:write_nl()

    local pgkeys = isget and fn:property_keys_get(fts.PROP_GET):to_array() or {}
    local pskeys = isset and fn:property_keys_get(fts.PROP_SET):to_array() or {}
    build_vallist(f, pgkeys, pskeys, "Keys")

    local pgvals = isget and fn:property_values_get(fts.PROP_GET):to_array() or {}
    local psvals = isset and fn:property_values_get(fts.PROP_SET):to_array() or {}
    build_vallist(f, pgvals, psvals, "Values")

    if isget and isset then
        f:write_h("Description", 3)
        if doc or (not gdoc and not sdoc) then
            write_full_doc(f, doc)
        end
        if (isget and gdoc) or (isset and sdoc) then
            f:write_nl(2)
        end
    end

    if isget and gdoc then
        if isset then
            f:write_h("Getter", 4)
        else
            f:write_h("Description", 3)
        end
        write_full_doc(f, gdoc)
        if isset and sdoc then
            f:write_nl(2)
        end
    end

    if isset and sdoc then
        if isget then
            f:write_h("Setter", 4)
        else
            f:write_h("Description", 3)
        end
        write_full_doc(f, sdoc)
    end

    f:write_nl()
    f:finish()
end

getopt.parse {
    args = arg,
    descs = {
        { category = "General" },
        { "h", "help", nil, help = "Show this message.", metavar = "CATEGORY",
            callback = getopt.help_cb(io.stdout)
        },
        { "v", "verbose", false, help = "Be verbose." },

        { category = "Generator" },
        { "r", "root", true, help = "Root path of the docs." },
        { "n", "namespace", true, help = "Root namespace of the docs." },
        { nil, "disable-graphviz", false, help = "Disable graphviz usage." },
        { nil, "disable-notes", false, help = "Disable notes plugin usage." }
    },
    error_cb = function(parser, msg)
        io.stderr:write(msg, "\n")
        getopt.help(parser, io.stderr)
    end,
    done_cb = function(parser, opts, args)
        if opts["h"] then
            return
        end
        global_opts.verbose = not not opts["v"]
        global_opts.use_dot = not opts["disable-graphviz"]
        global_opts.use_notes = not opts["disable-notes"]
        global_opts.root_nspace = (not opts["n"] or opts["n"] == "")
            and "efl" or opts["n"]
        if not opts["r"] or opts["r"] == "" then
            opts["r"] = "dokuwiki/data/pages"
        end
        global_opts.doc_root = path_join(opts["r"],
            nspace_to_path(global_opts.root_nspace))
        if not args[1] then
            if not eolian.system_directory_scan() then
                error("failed scanning system directory")
            end
        else
            if not eolian.directory_scan(args[1]) then
                error("failed scanning directory: " .. args[1])
            end
        end
        if not eolian.all_eot_files_parse() then
            error("failed parsing eo type files")
        end
        if not eolian.all_eo_files_parse() then
            error("failed parsing eo files")
        end
        cutil.file_rmrf(path_join(global_opts.doc_root))
        mkdir_r(nil)
        build_ref()
        build_classes()
        build_typedecls()
        build_variables()
        print_stats()
    end
}

return true