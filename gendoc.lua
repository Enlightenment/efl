local eolian = require("eolian")
local getopt = require("getopt")
local cutil = require("cutil")
local util = require("util")

local doc_root
local root_nspace
local verbose = false

-- utils

local make_page = function(path)
    return doc_root .. "/" .. path .. ".txt"
end

local mkdir_r = function(dirn)
    local fullp = dirn and (doc_root .. "/" .. dirn) or doc_root
    local prev
    for x in fullp:gmatch("[^/]+") do
        local p
        if prev then
            p = prev .. "/" .. x
        else
            p = x
        end
        prev = p
        if cutil.file_exists(p) then
            assert(cutil.file_is_dir(p))
        else
            assert(cutil.file_mkdir(p))
        end
    end
end

local mkdir_p = function(path)
    mkdir_r(path:match("(.+)/([^/]+)"))
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

local gen_link_target = function(tbl, root)
    if tbl[#tbl] == true or root then
        if not root then tbl[#tbl] = nil end
        return ":" .. root_nspace .. ":" .. table.concat(tbl, ":")
    end
    return table.concat(tbl, ":")
end

-- generator

local Writer = util.Object:clone {
    __ctor = function(self, path)
        local subs
        if type(path) == "table" then
            subs = table.concat(path, "/")
        else
            subs = path:gsub(":", "/"):lower()
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
            target = gen_link_target(target)
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
            self:write_raw(("  "):rep(lvl), prec, " ", str, "\n")
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

local gen_ref_link
gen_ref_link = function(str)
    local decl = eolian.declaration_get_by_name(str)
    if decl then
        local t = { decl_to_nspace(decl) }
        for tok in str:gmatch("[^%.]+") do
            t[#t + 1] = tok:lower()
        end
        t[#t + 1] = true
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
            return gen_ref_link(bstr)
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

    local ret = gen_ref_link(bstr)
    ret[#ret] = funct_to_str[ftype]
    ret[#ret + 1] = fn:name_get():lower()
    ret[#ret + 1] = true
    return ret
end

local gen_doc_markup = function(str)
    local f = str:gmatch(".")
    local c = f()
    local buf = {}
    while c do
        if c == "\\" then
            c = f()
            if c ~= "@" and c ~= "$" then
                buf[#buf + 1] = "\\"
            end
            buf[#buf + 1] = c
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
                buf[#buf + 1] = "%%''" .. table.concat(wbuf) .. "''%%"
            else
                buf[#buf + 1] = "$"
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
                buf[#buf + 1] = "%%"
                buf[#buf + 1] = Buffer():write_link(gen_ref_link(title),
                    title):finish()
                buf[#buf + 1] = "%%"
                if ldot then
                    buf[#buf + 1] = "."
                end
            else
                buf[#buf + 1] = "@"
            end
        elseif c == "%" then
            c = f()
            if c == "%" then
                c = f()
                buf[#buf + 1] = "%%<nowiki>%%</nowiki>%%"
            else
                buf[#buf + 1] = "%"
            end
        else
            buf[#buf + 1] = c
            c = f()
        end
    end
    return "%%" .. table.concat(buf) .. "%%"
end

local gen_doc_par = function(str)
    local notetypes = {
        ["Note: "] = "<note>\n",
        ["Warning: "] = "<note warning>\n",
        ["Remark: "] = "<note tip>\n",
        ["TODO: "] = "<note>\n**TODO:** "
    }
    local tag
    for k, v in pairs(notetypes) do
        if str:match("^" .. k) then
            tag = v
            str = str:sub(#k + 1)
            break
        end
    end
    if tag then
        return tag .. gen_doc_markup(str) .. "\n</note>"
    end
    return gen_doc_markup(str)
end

local gen_doc_refd = function(str)
    if not str then
        return nil
    end
    local pars = str_split(str, "\n\n")
    for i = 1, #pars do
        pars[i] = gen_doc_par(pars[i])
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

    if f:type_get() == eolian.function_type.METHOD then
        local pars = f:parameters_get():to_array()
        local cnrt = rtype and rtype:c_type_named_get(cn) or ("void " .. cn)
        for i = 1, #pars do
            pars[i] = gen_cparam(pars[i])
        end
        if #pars == 0 then
            pars = { "void" }
        end
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
    if #pars == 0 then
        pars = { "void" }
    end
    return cnrt .. "(" .. table.concat(pars, ", ") .. ");"
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
    f:write_nl(2)
    local since
    if doc2 then
        since = doc2:since_get()
    end
    if not since and doc then
        since = doc:since_get()
    end
    if since then
        f:write_i(since)
        f:write_nl(2)
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

local build_class = function(cl)
    local f = Writer(gen_nsp_class(cl))

    f:write_h(cl:full_name_get(), 2)

    f:write_h("Inheritance hierarchy", 3)
    f:write_list(build_inherits(cl))

    f:write_h("Description", 3)
    write_full_doc(f, cl:documentation_get())

    build_functable(f, "Methods", "Method name", cl, eolian.function_type.METHOD)
    build_functable(f, "Properties", "Property name",
        cl, eolian.function_type.PROPERTY)

    f:write_h("Events", 3)
    local evs = cl:events_get():to_array()
    if #evs == 0 then
        f:write_raw("This class does not define any events.\n")
    else
        for i, ev in ipairs(evs) do
            f:write_h(ev:name_get(), 4)
            write_full_doc(f, ev:documentation_get())
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

build_method = function(fn, cl)
    local f = Writer(gen_nsp_func(fn, cl))

    f:write_h(fn:name_get(), 2)

    f:write_h("C signature", 3)
    f:write_code(gen_func_csig(fn), "c")
    f:write_nl()

    f:write_h("Description", 3)
    write_full_doc(f, fn:documentation_get(eolian.function_type.METHOD))

    f:finish()
end

build_property = function(fn, cl)
    local f = Writer(gen_nsp_func(fn, cl))

    local fts = eolian.function_type
    local ft = fn:type_get()
    local isget = (ft == fts.PROP_GET or ft == fts.PROPERTY)
    local isset = (ft == fts.PROP_SET or ft == fts.PROPERTY)

    local doc = fn:documentation_get(fts.PROPERTY)
    local gdoc = fn:documentation_get(fts.PROP_GET)
    local sdoc = fn:documentation_get(fts.PROP_SET)

    f:write_h(fn:name_get(), 2)

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

    if isget and isset then
        f:write_h("Description", 3)
        if doc or (not gdoc and not sdoc) then
            write_full_doc(f, doc)
        end
    end

    if isget and gdoc then
        if isset then
            f:write_h("Getter", 4)
        else
            f:write_h("Description", 3)
        end
        write_full_doc(f, gdoc)
    end

    if isset and sdoc then
        if isget then
            f:write_h("Setter", 4)
        else
            f:write_h("Description", 3)
        end
        write_full_doc(f, sdoc)
    end

    f:finish()
end

getopt.parse {
    args = arg,
    descs = {
        { category = "General" },
        { "h", "help", nil, help = "Show this message.", metavar = "CATEGORY",
            callback = getopt.help_cb(io.stdout)
        },
        -- TODO: implement verbose mode
        { "v", "verbose", false, help = "Be verbose." },

        { category = "Generator" },
        { "r", "root", true, help = "Root path of the docs." },
        { "n", "namespace", true, help = "Root namespace of the docs." }
    },
    error_cb = function(parser, msg)
        io.stderr:write(msg, "\n")
        getopt.help(parser, io.stderr)
    end,
    done_cb = function(parser, opts, args)
        if opts["v"] then
            verbose = true
        end
        root_nspace = opts["n"] or "efl"
        if not opts["r"] then
            error("no documentation root supplied")
        end
        doc_root = opts["r"] .. "/" .. root_nspace:gsub(":", "/")
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
        mkdir_r(nil)
        build_ref()
        build_classes()
    end
}

return true