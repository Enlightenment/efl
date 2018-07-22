local getopt = require("getopt")

local serializer = require("serializer")

local stats = require("docgen.stats")
local dutil = require("docgen.util")
local writer = require("docgen.writer")
local keyref = require("docgen.keyref")
local dtree = require("docgen.doctree")
local mono = require("docgen.mono")

local printgen = function() end

local propt_to_type = {
    [dtree.Function.PROPERTY] = "(get, set)",
    [dtree.Function.PROP_GET] = "(get)",
    [dtree.Function.PROP_SET] = "(set)",
}

local gen_cparam = function(par, out)
    local part = par:type_get()
    out = out or (par:direction_get() == par.OUT)
    local tstr = part:c_type_get()
    if out then
        tstr = dtree.type_cstr_get(tstr, "*")
    end
    return dtree.type_cstr_get(tstr, par:name_get())
end

local get_func_csig_part = function(cn, tp)
    if not tp then
        return "void " .. cn
    end
    return dtree.type_cstr_get(tp, cn)
end

local gen_func_csig = function(f, ftype)
    ftype = ftype or f.METHOD
    assert(ftype ~= f.PROPERTY)

    local cn = f:full_c_name_get(ftype)
    local rtype = f:return_type_get(ftype)

    local fparam = "Eo *obj"
    if f:is_class() then
        fparam = "Efl_Class *klass"
    elseif f:is_const() or ftype == f.PROP_GET then
        fparam = "const Eo *obj"
    end

    if f:type_get() == f.METHOD then
        local pars = f:parameters_get()
        local cnrt = get_func_csig_part(cn, rtype)
        for i = 1, #pars do
            pars[i] = gen_cparam(pars[i])
        end
        table.insert(pars, 1, fparam);
        return cnrt .. "(" .. table.concat(pars, ", ") .. ");"
    end

    local keys = f:property_keys_get(ftype)
    local vals = f:property_values_get(ftype)

    if ftype == f.PROP_SET then
        local cnrt = get_func_csig_part(cn, rtype)
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
            cnrt = get_func_csig_part(cn, vals[1]:type_get())
            table.remove(vals, 1)
        else
            cnrt = get_func_csig_part(cn)
        end
    else
        cnrt = get_func_csig_part(cn, rtype)
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

local gen_func_namesig = function(fn, cl, buf, isprop, isget, isset)
    if isprop then
        buf[#buf + 1] = "@property "
    end
    buf[#buf + 1] = fn:name_get()
    buf[#buf + 1] = " "
    if fn:is_beta() then
        buf[#buf + 1] = "@beta "
    end
    if not isprop then
        if fn:scope_get(fn.METHOD) == fn.scope.PROTECTED then
            buf[#buf + 1] = "@protected "
        end
    elseif isget and isset then
        if fn:scope_get(fn.PROP_GET) == fn.scope.PROTECTED and
           fn:scope_get(fn.PROP_SET) == fn.scope.PROTECTED then
            buf[#buf + 1] = "@protected "
        end
    end
    if fn:is_class() then
        buf[#buf + 1] = "@class "
    end
    if fn:is_const() then
        buf[#buf + 1] = "@const "
    end
end

local gen_func_param = function(fp, buf, nodir)
    -- TODO: default value
    buf[#buf + 1] = "        "
    local dirs = {
        [dtree.Parameter.IN] = "@in ",
        [dtree.Parameter.OUT] = "@out ",
        [dtree.Parameter.INOUT] = "@inout ",
    }
    if not nodir then buf[#buf + 1] = dirs[fp:direction_get()] end
    buf[#buf + 1] = fp:name_get()
    buf[#buf + 1] = ": "
    buf[#buf + 1] = fp:type_get():serialize()
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
    buf[#buf + 1] = rett:serialize()
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
    gen_func_namesig(fn, cl, buf, false, false, false)

    local fimp = fn:implement_get()

    if fimp:is_pure_virtual(fn.METHOD) then
        buf[#buf + 1] = "@pure_virtual "
    end
    buf[#buf + 1] = "{"
    local params = fn:parameters_get()
    local rtp = fn:return_type_get(fn.METHOD)
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
    gen_func_return(fn, fn.METHOD, buf)
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
    local fnt = fn:type_get()
    local isget = (fnt == fn.PROPERTY or fnt == fn.PROP_GET)
    local isset = (fnt == fn.PROPERTY or fnt == fn.PROP_SET)
    gen_func_namesig(fn, cl, buf, true, isget, isset)

    local pimp = fn:implement_get()

    local gvirt = pimp:is_pure_virtual(fn.PROP_GET)
    local svirt = pimp:is_pure_virtual(fn.PROP_SET)

    if (not isget or gvirt) and (not isset or svirt) then
        buf[#buf + 1] = "@pure_virtual "
    end

    local gkeys = isget and fn:property_keys_get(fn.PROP_GET) or {}
    local skeys = isset and fn:property_keys_get(fn.PROP_SET) or {}
    local gvals = isget and fn:property_values_get(fn.PROP_GET) or {}
    local svals = isget and fn:property_values_get(fn.PROP_SET) or {}
    local grtt = isget and fn:return_type_get(fn.PROP_GET) or nil
    local srtt = isset and fn:return_type_get(fn.PROP_SET) or nil

    local keys_same = eovals_check_same(gkeys, skeys)
    local vals_same = eovals_check_same(gvals, svals)

    buf[#buf + 1] = "{\n"

    if isget then
        buf[#buf + 1] = "    get "
        if fn:scope_get(fn.PROP_GET) == fn.scope.PROTECTED and
           fn:scope_get(fn.PROP_SET) ~= fn.scope.PROTECTED then
            buf[#buf + 1] = "@protected "
        end
        buf[#buf + 1] = "{"
        if (#gkeys == 0 or keys_same) and (#gvals == 0 or vals_same) and
           (not grtt or grtt == srtt) then
            buf[#buf + 1] = "}\n"
        else
            buf[#buf + 1] = "\n"
            if not keys_same then gen_prop_keyvals(gkeys, "keys", buf) end
            if not vals_same then gen_prop_keyvals(gvals, "values", buf) end
            if grtt ~= srtt then
                gen_func_return(fn, fn.PROP_GET, buf, 2)
            end
            buf[#buf + 1] = "    }\n"
        end
    end

    if isset then
        buf[#buf + 1] = "    set "
        if fn:scope_get(fn.PROP_SET) == fn.scope.PROTECTED and
           fn:scope_get(fn.PROP_GET) ~= fn.scope.PROTECTED then
            buf[#buf + 1] = "@protected "
        end
        buf[#buf + 1] = "{"
        if (#skeys == 0 or keys_same) and (#svals == 0 or vals_same) and
           (not srtt or grtt == srtt) then
            buf[#buf + 1] = "}\n"
        else
            buf[#buf + 1] = "\n"
            if not keys_same then gen_prop_keyvals(skeys, "keys", buf) end
            if not vals_same then gen_prop_keyvals(svals, "values", buf) end
            if grtt ~= srtt then
                gen_func_return(fn, fn.PROP_SET, buf, 2)
            end
            buf[#buf + 1] = "    }\n"
        end
    end

    if keys_same then gen_prop_keyvals(gkeys, "keys", buf, 0) end
    if vals_same then gen_prop_keyvals(gvals, "values", buf, 0) end

    buf[#buf + 1] = "}"
    return table.concat(buf)
end

-- builders

local nspaces_group = function(ns)
    if #ns <= 2 then
        return ns[1]
    end

    if ns[1] == "efl" and (ns[2] == "class" or ns[2] == "interface" or
                           ns[2] == "object" or ns[2] == "promise") then
        return ns[1]
    end

    return ns[1] .. "." .. ns[2]
end

local nspaces_filter = function(items, ns)
    local out = {}

    for _, item in ipairs(items) do
        local group = nspaces_group(item:nspaces_get())
        if group == ns then out[#out + 1] = item end
    end

    return out
end

local build_method, build_property, build_event

local build_reftable = function(f, title, ctype, t, iscl)
    if not t or #t == 0 then
        return
    end

    local nt = {}
    for i, v in ipairs(t) do
        nt[#nt + 1] = {
            writer.Buffer():write_link(
                iscl and v:nspaces_get(true)
                      or dtree.Node.nspaces_get(v, true),
                v:name_get()
            ):finish(),
            v:doc_get():brief_get()
        }
    end
    table.sort(nt, function(v1, v2) return v1[1] < v2[1] end)
    f:write_table({ title, "Brief description" }, nt)
end

local build_ref_group = function(f, ns, classlist, aliases, structs, enums, consts, globals)
    local classes = {}
    local ifaces = {}
    local mixins = {}

    for i, cl in ipairs(classlist) do
        local tp = cl:type_get()
        if tp == dtree.Class.REGULAR or tp == dtree.Class.ABSTRACT then
            classes[#classes + 1] = cl
        elseif tp == dtree.Class.MIXIN then
            mixins[#mixins + 1] = cl
        elseif tp == dtree.Class.INTERFACE then
            ifaces[#ifaces + 1] = cl
        end
    end

    local title = ns:gsub("(%l)(%w*)", function(a,b) return a:upper()..b end) --string.sub(ns, 1, 1):upper() .. string.sub(ns, 2):lower()
    f:write_h(title, 2)

    build_reftable(f, "Classes", "class", classes, true)
    build_reftable(f, "Interfaces", "interface", ifaces, true)
    build_reftable(f, "Mixins", "mixin", mixins, true)

    build_reftable(f, "Aliases", "alias", aliases)
    build_reftable(f, "Structures", "struct", structs)
    build_reftable(f, "Enums", "enum", enums)
    build_reftable(f, "Constants", "constant", consts)
    build_reftable(f, "Globals", "global", globals)

    f:write_nl()
end

local build_ref = function()
    local f = writer.Writer("start", "EFL Reference")
    printgen("Generating reference...")

    f:write_editable({ "reference" }, "general")
    f:write_nl()

    local classlist = dtree.Class.all_get()
    local aliases = dtree.Typedecl.all_aliases_get()
    local structs = dtree.Typedecl.all_structs_get()
    local enums = dtree.Typedecl.all_enums_get()
    local consts = dtree.Variable.all_constants_get()
    local globals = dtree.Variable.all_globals_get()

    grouped = {}
    groups = {}
    for i, cl in ipairs(classlist) do
        local ns = cl:nspaces_get()
        local name = nspaces_group(cl:nspaces_get())

        local group = grouped[name]
        if not group then
            group = {}
            grouped[name] = group
            groups[#groups + 1] = name
        end

        group[#group + 1] = cl
    end
    table.sort(groups)
 
    for _, ns in ipairs(groups) do
        build_ref_group(f, ns, grouped[ns], nspaces_filter(aliases, ns), nspaces_filter(structs, ns),
                        nspaces_filter(enums, ns), nspaces_filter(consts, ns), nspaces_filter(globals, ns))
    end

    f:finish()
end

local build_inherits
build_inherits = function(cl, t, lvl)
    t = t or {}
    lvl = lvl or 0
    local lbuf = writer.Buffer()
    if lvl > 0 then
        lbuf:write_link(cl:nspaces_get(true), cl:name_get())
        lbuf:write_raw(" ")
        lbuf:write_i("(" .. cl:type_str_get() .. ")")
 
        t[#t + 1] = { lvl - 1, lbuf:finish() }
    end

    for i, acl in ipairs(cl:inherits_get()) do
        build_inherits(acl, t, lvl + 1)
    end
    return t
end

local build_inherit_summary
build_inherit_summary = function(cl, buf)
    buf = buf or writer.Buffer()
    buf:write_raw(" => ")

    buf:write_link(cl:nspaces_get(true), cl:name_get())
    buf:write_raw(" ")
    buf:write_i("(" .. cl:type_str_get() .. ")")

    local inherits = cl:inherits_get()
    if #inherits ~= 0 then
        build_inherit_summary(inherits[1], buf)
    end
    return buf
end

local default_theme_light = {
    classes = {
        regular = {
            style = "filled",
            color = "black",
            fill_color = "white",
            font_color = "black",
            primary_color = "black",
            primary_fill_color = "gray",
            primary_font_color = "black"
        },
        abstract = {
            style = "filled",
            color = "black",
            fill_color = "white",
            font_color = "black",
            primary_color = "black",
            primary_fill_color = "gray",
            primary_font_color = "black"
        },
        mixin = {
            style = "filled",
            color = "blue",
            fill_color = "white",
            font_color = "black",
            primary_color = "blue",
            primary_fill_color = "skyblue",
            primary_font_color = "black"
        },
        interface = {
            style = "filled",
            color = "cornflowerblue",
            fill_color = "white",
            font_color = "black",
            primary_color = "cornflowerblue",
            primary_fill_color = "azure",
            primary_font_color = "black"
        }
    },
    node = {
        shape = "box"
    },
    edge = {
        color = "black"
    },
    bg_color = "transparent",
    rank_dir = "TB",
    size = "6"
}

local default_theme_dark = {
    classes = {
        regular = {
            style = "filled",
            color = "gray15",
            fill_color = "gray15",
            font_color = "white",
            primary_color = "gray15",
            primary_fill_color = "black",
            primary_font_color = "white"
        },
        abstract = {
            style = "filled",
            color = "gray15",
            fill_color = "gray15",
            font_color = "white",
            primary_color = "gray15",
            primary_fill_color = "black",
            primary_font_color = "white"
        },
        mixin = {
            style = "filled",
            color = "deepskyblue",
            fill_color = "gray15",
            font_color = "white",
            primary_color = "deepskyblue",
            primary_fill_color = "deepskyblue4",
            primary_font_color = "white"
        },
        interface = {
            style = "filled",
            color = "cornflowerblue",
            fill_color = "gray15",
            font_color = "white",
            primary_color = "cornflowerblue",
            primary_fill_color = "dodgerblue4",
            primary_font_color = "white"
        }
    },
    node = {
        shape = "box"
    },
    edge = {
        color = "gray35"
    },
    bg_color = "transparent",
    rank_dir = "TB",
    size = "6"
}

local current_theme = default_theme_dark

local validate_ctheme = function(tb, name)
    if type(tb.classes[name]) ~= "table" then
        return false
    end
    local t = tb.classes[name]
    if type(t.style) ~= "string" then
        return false
    end
    if type(t.color) ~= "string" then
        return false
    end
    if type(t.fill_color) ~= "string" then
        return false
    end
    if type(t.font_color) ~= "string" then
        return false
    end
    if not t.primary_color then
        t.primary_color = t.color
    end
    if not t.primary_fill_color then
        t.primary_fill_color = t.fill_color
    end
    if not t.primary_font_color then
        t.primary_font_color = t.font_color
    end
    if type(t.primary_color) ~= "string" then
        return false
    end
    if type(t.primary_fill_color) ~= "string" then
        return false
    end
    if type(t.primary_font_color) ~= "string" then
        return false
    end
    return true
end

local validate_theme = function(tb)
    if type(tb) ~= "table" then
        return false
    end
    if type(tb.classes) ~= "table" then
        return false
    end
    if not tb.node then
        tb.node = current_theme.node
    end
    if not tb.edge then
        tb.edge = current_theme.edge
    end
    if not tb.bg_color then
        tb.bg_color = current_theme.bg_color
    end
    if not tb.rank_dir then
        tb.rank_dir = current_theme.rank_dir
    end
    if not tb.size then
        tb.size = current_theme.size
    end
    if type(tb.node) ~= "table" then
        return false
    end
    if type(tb.edge) ~= "table" then
        return false
    end
    if type(tb.bg_color) ~= "string" then
        return false
    end
    if type(tb.rank_dir) ~= "string" then
        return false
    end
    if type(tb.size) ~= "string" then
        return false
    end
    if not validate_ctheme(tb, "regular") then
        return false
    end
    if not validate_ctheme(tb, "abstract") then
        return false
    end
    if not validate_ctheme(tb, "mixin") then
        return false
    end
    if not validate_ctheme(tb, "interface") then
        return false
    end
    return true
end

local set_theme = function(tname)
    local tf = io.open(tname)
    if tf then
        local cont = tf:read("*all")
        tf:close()
        local tb, err = serializer.deserialize(cont)
        if not tb then
            error("error parsing theme '" .. tname .. "': " .. err)
        end
        if not validate_theme(tb) then
            error("invalid theme '" .. tname .. "'")
        end
        current_theme = tb
    else
        error("theme '" .. tname .. "' does not exist")
    end
end

local find_parent_impl
find_parent_impl = function(fulln, cl)
    for i, pcl in ipairs(cl:inherits_get()) do
        for j, impl in ipairs(pcl:implements_get()) do
            if impl:name_get() == fulln then
                return impl, pcl
            end
        end
        local pimpl, pcl = find_parent_impl(fulln, pcl)
        if pimpl then
            return pimpl, pcl
        end
    end
    return nil, cl
end

local find_parent_briefdoc
find_parent_briefdoc = function(fulln, cl)
    local pimpl, pcl = find_parent_impl(fulln, cl)
    if not pimpl then
        return dtree.Doc():brief_get()
    end
    local pdoc = pimpl:doc_get(dtree.Function.METHOD, true)
    local pdocf = pimpl:fallback_doc_get(true)
    if not pdoc:exists() and (not pdocf or not pdocf:exists()) then
        return find_parent_briefdoc(fulln, pcl)
    end
    return pdoc:brief_get(pdocf)
end

local build_functable = function(f, tcl, tbl)
    if #tbl == 0 then
        return
    end
    local nt = {}
    for i, implt in ipairs(tbl) do
        local lbuf = writer.Buffer()

        local cl, impl = unpack(implt)
        local func = impl:function_get()

        local wt = {}
        wt[0] = cl
        wt[1] = func
        wt[2] = impl

        nt[#nt + 1] = wt
    end

    local get_best_scope = function(f)
        local ft = f:type_get()
        if ft == f.PROPERTY then
            local fs1, fs2 = f:scope_get(f.PROP_GET), f:scope_get(f.PROP_SET)
            if fs1 == f.scope.PUBLIC or fs2 == f.scope.PUBLIC then
                return f.scope.PUBLIC
            elseif fs1 == f.scope.PROTECTED or fs2 == f.scope.PROTECTED then
                return f.scope.PROTECTED
            else
                return f.scope.PRIVATE
            end
        else
            return f:scope_get(ft)
        end
    end
    table.sort(nt, function(v1, v2)
        local cl1, cl2 = v1[0], v2[0]
        if cl1 ~= cl2 then
            return cl1:name_get() < cl2:name_get()
        end

        local f1, f2 = v1[1], v2[1]
        local f1s, f2s = get_best_scope(f1), get_best_scope(f2)
        if f1s ~= f2s then
            if f1s ~= f1.scope.PROTECED then
                -- public funcs go first, private funcs go last
                return f1s == f1.scope.PUBLIC
            else
                -- protected funcs go second
                return f2s == f2.scope.PRIVATE
            end
        end
        return f1:name_get() < f2:name_get()
    end)

    return nt
end

local write_description = function(f, impl, func, cl)
    local over = impl:is_overridden(cl)
    local bdoc

    local doc = impl:doc_get(func.METHOD, true)
    local docf = impl:fallback_doc_get(true)
    if over and (not doc:exists() and (not docf or not docf:exists())) then
        bdoc = find_parent_briefdoc(impl:name_get(), cl)
    else
        bdoc = doc:brief_get(docf)
    end
    if bdoc ~= "No description supplied." then
        f:write_raw(bdoc)
    end
end

local write_function = function(f, func, cl)
    local llbuf = writer.Buffer()
    llbuf:write_link(func:nspaces_get(cl, true), func:name_get())
    f:write_b(llbuf:finish())

    local pt = propt_to_type[func:type_get()]
    if pt then
        f:write_raw(" ")
        local llbuf = writer.Buffer()
        llbuf:write_b(pt)
        f:write_i(llbuf:finish())
    end
end

local write_scope = function(f, func)
    local ftt = {
        [func.scope.PROTECTED] = "protected",
        [func.scope.PRIVATE] = "private"
    }
    if func:is_class() then
        f:write_raw(" ")
        f:write_m("class")
    end
    if func:type_get() == func.PROPERTY then
        local ft1, ft2 = ftt[func:scope_get(func.PROP_GET)],
                         ftt[func:scope_get(func.PROP_SET)]
        if ft1 and ft1 == ft2 then
            f:write_raw(" ")
            f:write_m(ft1)
        elseif ft1 or ft2 then
            local s = ""
            if ft1 then
                s = s .. ft1 .. " get" .. (ft2 and ", " or "")
            end
            if ft2 then
                s = s .. ft2 .. " set"
            end
            f:write_raw(" ")
            f:write_m(s)
        end
    else
        local ft = ftt[func:scope_get(func:type_get())]
        if ft then
            f:write_raw(" ")
            f:write_m(ft)
        end
    end
end

local write_functable = function(f, tcl, tbl)
    if #tbl == 0 then
        return
    end
    local nt = build_functable(t, tcl, tbl)

    local wrote = false
    for i, wt in ipairs(nt) do
        local cl = wt[0]
        local func = wt[1]
        local impl = wt[2]

        local ocl = impl:class_get()
        local func = impl:function_get()
        local over = impl:is_overridden(cl)

        -- function
        write_function(f, func, cl)
        -- scope
        write_scope(f, func)

        -- overrides
        if over then
            -- TODO: possibly also mention which part of a property was
            -- overridden and where, get/set override point might differ!
            -- but we get latest doc every time so it's ok for now
            local llbuf = writer.Buffer()
            llbuf:write_raw(" [Overridden from ")
            llbuf:write_link(ocl:nspaces_get(true), ocl:name_get())
            llbuf:write_raw("]")
            f:write_i(llbuf:finish())
        end

        -- description
        f:write_br(true)
        f:write_raw("> ")
        write_description(f, impl, func, cl)
 
        -- code snippets
        f:write_nl()
        local codes = {}
        if func:type_get() ~= dtree.Function.PROPERTY then
            codes[#codes + 1] = gen_func_csig(func, func:type_get())
        else
            codes[#codes + 1] = gen_func_csig(func, dtree.Function.PROP_GET)
            codes[#codes + 1] = gen_func_csig(func, dtree.Function.PROP_SET)
        end
        f:write_code(table.concat(codes, "\n"), "c")
        f:write_br(true)

        if cl == tcl then
            if impl:is_prop_get() or impl:is_prop_set() then
                build_property(impl, cl)
            else
                build_method(impl, cl)
            end
        end
    end
    f:write_nl()
end

local write_inherit_functable = function(f, tcl, tbl)
    if #tbl == 0 then
        return
    end
    local nt = build_functable(t, tcl, tbl)

    local prevcl = tcl
    for i, wt in ipairs(nt) do
        local cl = wt[0]
        local func = wt[1]
        local impl = wt[2]

        local ocl = impl:class_get()
        local func = impl:function_get()

	-- class grouping for inheritance
        if cl ~= prevcl then
            prevcl = cl
            f:write_raw("^ ")
            f:write_link(cl:nspaces_get(true), cl:name_get())
            f:write_raw(" ^^^")
            f:write_nl()
        end

        -- scope
        f:write_raw("| ")
        write_scope(f, func)
        f:write_raw(" | ")
        -- function
        write_function(f, func, cl)
        f:write_raw(" | ")
        -- description
	write_description(f, impl, func, cl)
        f:write_raw(" |")
        f:write_nl()
    end
    f:write_nl()
end

-- finds all stuff that is callable on a class, respecting
-- overrides and not duplicating, does a depth-first search
local find_callables
find_callables = function(cl, omeths, events, written)
    for i, pcl in ipairs(cl:inherits_get()) do
        for j, impl in ipairs(pcl:implements_get()) do
            local func = impl:function_get()
            local fid = func:id_get()
            if not written[fid] then
                omeths[#omeths + 1] = { pcl, impl }
                written[fid] = true
            end
        end
        for i, ev in ipairs(pcl:events_get()) do
            local evid = ev:name_get()
            if not written[evid] then
                events[#events + 1] = { pcl, ev }
                written[evid] = true
            end
        end
        find_callables(pcl, omeths, events, written)
    end
end

local build_evcsig = function(ev)
    local csbuf = { ev:c_name_get(), "(" }
    csbuf[#csbuf + 1] = dtree.type_cstr_get(ev:type_get())
    if ev:is_beta() then
        csbuf[#csbuf + 1] = ", @beta"
    end
    if ev:is_hot() then
        csbuf[#csbuf + 1] = ", @hot"
    end
    if ev:is_restart() then
        csbuf[#csbuf + 1] = ", @restart"
    end
    csbuf[#csbuf + 1] = ")";
    return table.concat(csbuf)
end

local build_evtable = function(f, tcl, tbl, newm)
    if #tbl == 0 then
        return
    end
    local nt = {}
    for i, evt in ipairs(tbl) do
        local lbuf = writer.Buffer()
        local evn
        local cl, ev
        if not newm then
            cl, ev = evt[1], evt[2]
        else
            cl, ev = tcl, evt
        end

        local wt = {}
        wt[0] = cl
        wt[1] = ev
        wt[2] = ev:name_get()

	nt[#nt + 1] = wt
    end

    table.sort(nt, function(v1, v2)
        if v1[0] ~= v2[0] then
            return v1[0]:name_get() < v2[0]:name_get()
        end

        return v1[2] < v2[2]
    end)

    return nt
end

local write_event_scope = function(f, ev)
    local ett = {
        [ev.scope.PROTECTED] = "protected",
        [ev.scope.PRIVATE] = "private"
    }
    local ets = ett[ev:scope_get()]
    if ets then
        f:write_raw(" ")
        f:write_m(ets)
    end
end

local write_evtable = function(f, tcl, tbl)
    if #tbl == 0 then
        return
    end
    local nt = build_evtable(f, tcl, tbl, true)
    for i, wt in ipairs(nt) do
        local evn
        local cl, ev = wt[0], wt[1]

        local llbuf = writer.Buffer()
        llbuf:write_link(ev:nspaces_get(cl, true), wt[2])
        f:write_b(llbuf:finish())

        -- scope
        write_event_scope(f, ev)
 
        -- description
        local bdoc = ev:doc_get():brief_get()
        if bdoc ~= "No description supplied." then
            f:write_br(true)
            f:write_raw("> ")
            f:write_raw(bdoc)
        end

        f:write_nl()
        f:write_code(build_evcsig(ev), "c");
        f:write_br()

        if cl == tcl then
            build_event(ev, cl)
        end
    end
end


local write_inherit_evtable = function(f, tcl, tbl)
    if #tbl == 0 then
        return
    end
    local nt = build_evtable(f, tcl, tbl, false)
    local prevcl
    for i, wt in ipairs(nt) do
        local evn
        local cl, ev = wt[0], wt[1]

        if cl ~= prevcl then
            prevcl = cl
            f:write_raw("^ ")
            f:write_link(cl:nspaces_get(true), cl:name_get())
            f:write_raw(" ^^^")
            f:write_nl()
        end
 
        f:write_raw("| ")
        -- scope
        write_event_scope(f, ev)
        f:write_raw(" | ")

        local llbuf = writer.Buffer()
        llbuf:write_link(ev:nspaces_get(cl, true), wt[2])
        f:write_b(llbuf:finish())

        f:write_raw(" | ")
	local bdoc = ev:doc_get():brief_get()
        if bdoc ~= "No description supplied." then
            f:write_raw(bdoc)
        end

        f:write_raw(" |")
        f:write_nl()
    end
end

local build_class = function(cl)
    local cln = cl:nspaces_get()
    local fulln = cl:name_get()
    local f = writer.Writer(cln, fulln)
    printgen("Generating class: " .. fulln)

    mono.build_class(cl)

    f:write_h(cl:name_get() .. " (" .. cl:type_str_get() .. ")", 1)
 
    f:write_h("Description", 2)
    f:write_raw(cl:doc_get():full_get(nil, true))
    f:write_nl(2)

    f:write_editable(cln, "description")
    f:write_nl()

    local inherits = cl:inherits_get()
    if #inherits ~= 0 then
        f:write_h("Inheritance", 2)

        f:write_raw(build_inherit_summary(inherits[1]):finish())
	f:write_nl()

        f:write_folded("Full hierarchy", function()
            f:write_list(build_inherits(cl))
        end)
        f:write_nl()
    end

    local written = {}
    local ievs = {}
    local meths, omeths = {}, {}
    for i, impl in ipairs(cl:implements_get()) do
        local func = impl:function_get()
        written[func:id_get()] = true
        meths[#meths + 1] = { cl, impl }
    end
    find_callables(cl, omeths, ievs, written)

    f:write_h("Members", 2)
    write_functable(f, cl, meths, true)
    if #omeths ~= 0 then
        f:write_h("Inherited", 3)
    end
    write_inherit_functable(f, cl, omeths, false)

    f:write_h("Events", 2)
    write_evtable(f, cl, cl:events_get(), true)
    if #ievs ~= 0 then
        f:write_h("Inherited", 3)
    end
    write_inherit_evtable(f, cl, ievs, false)

    f:finish()
end

local build_classes = function()
    for i, cl in ipairs(dtree.Class.all_get()) do
        build_class(cl)
    end
end

local write_tsigs = function(f, tp, ns)
    f:write_h("Signature", 2)
    f:write_code(tp:serialize())
    f:write_nl()

    f:write_h("C signature", 2)
    f:write_code(tp:serialize_c(ns), "c")
    f:write_nl()
end

local build_alias = function(tp)
    local ns = dtree.Node.nspaces_get(tp)
    local fulln = tp:name_get()
    local f = writer.Writer(ns, fulln)
    printgen("Generating alias: " .. fulln)

    f:write_h("Description", 2)
    f:write_raw(tp:doc_get():full_get(nil, true))
    f:write_nl(2)

    f:write_editable(ns, "description")
    f:write_nl()

    write_tsigs(f, tp, ns)

    f:finish()
end

local build_struct = function(tp)
    local ns = dtree.Node.nspaces_get(tp)
    local fulln = tp:name_get()
    local f = writer.Writer(ns, fulln)
    printgen("Generating struct: " .. fulln)

    f:write_h("Description", 2)
    f:write_raw(tp:doc_get():full_get(nil, true))
    f:write_nl(2)

    f:write_editable(ns, "description")
    f:write_nl()

    f:write_h("Fields", 2)

    f:write_editable(ns, "fields")
    f:write_nl()

    local arr = {}
    for i, fl in ipairs(tp:struct_fields_get()) do
        local buf = writer.Buffer()
        buf:write_b(fl:name_get())
        buf:write_raw(" - ", fl:doc_get():full_get())
        arr[#arr + 1] = buf:finish()
    end
    f:write_list(arr)
    f:write_nl()

    write_tsigs(f, tp, ns)

    f:finish()
end

local build_enum = function(tp)
    local ns = dtree.Node.nspaces_get(tp)
    local fulln = tp:name_get()
    local f = writer.Writer(ns, fulln)
    printgen("Generating enum: " .. fulln)

    f:write_h("Description", 2)
    f:write_raw(tp:doc_get():full_get(nil, true))
    f:write_nl(2)

    f:write_editable(ns, "description")
    f:write_nl()

    f:write_h("Fields", 2)

    f:write_editable(ns, "fields")
    f:write_nl()

    local arr = {}
    for i, fl in ipairs(tp:enum_fields_get()) do
        local buf = writer.Buffer()
        buf:write_b(fl:name_get())
        buf:write_raw(" - ", fl:doc_get():full_get())
        arr[#arr + 1] = buf:finish()
    end
    f:write_list(arr)
    f:write_nl()

    write_tsigs(f, tp, ns)

    f:finish()
end

local build_variable = function(v, constant)
    local ns = v:nspaces_get()
    local fulln = v:name_get()
    local f = writer.Writer(ns, fulln)
    printgen("Generating variable: " .. fulln)

    f:write_h("Description", 2)
    f:write_raw(v:doc_get():full_get(nil, true))
    f:write_nl(2)

    f:write_editable(ns, "description")
    f:write_nl()

    write_tsigs(f, v, ns)

    f:finish()
end

local build_typedecls = function()
    for i, tp in ipairs(dtree.Typedecl.all_aliases_get()) do
        build_alias(tp)
    end

    for i, tp in ipairs(dtree.Typedecl.all_structs_get()) do
        build_struct(tp)
    end

    for i, tp in ipairs(dtree.Typedecl.all_enums_get()) do
        build_enum(tp)
    end
end

local build_variables = function()
    for i, v in ipairs(dtree.Variable.all_constants_get()) do
        build_variable(v, true)
    end

    for i, v in ipairs(dtree.Variable.all_globals_get()) do
        build_variable(v, false)
    end
end

local build_parlist = function(f, pl, nodir)
    local params = {}
    for i, p in ipairs(pl) do
        local buf = writer.Buffer()
        buf:write_b(p:name_get())
        if not nodir then
            buf:write_raw(" ")
            buf:write_i("(", p:direction_name_get(), ")")
        end
        buf:write_raw(" - ", p:doc_get():full_get())
        params[#params + 1] = buf:finish()
    end
    f:write_list(params)
end

local build_vallist = function(f, pg, ps, title)
    if #pg == #ps then
        local same = true
        for i = 1, #pg do
            if not pg[i]:is_same(ps[i]) then
                same = false
                break
            end
        end
        if same then ps = {} end
    end
    if #pg > 0 or #ps > 0 then
        f:write_h(title, 2)
        if #pg > 0 then
            if #ps > 0 then
                f:write_h("Getter", 3)
            end
            build_parlist(f, pg, true)
        end
        if #ps > 0 then
            if #pg > 0 then
                f:write_h("Setter", 3)
            end
            build_parlist(f, ps, true)
        end
    end
end

local find_parent_doc
find_parent_doc = function(fulln, cl, ftype)
    local pimpl, pcl = find_parent_impl(fulln, cl)
    if not pimpl then
        return dtree.Doc()
    end
    local pdoc = pimpl:doc_get(ftype)
    if not pdoc:exists() then
        return find_parent_doc(fulln, pcl, ftype)
    end
    return pdoc
end

local write_inherited_from = function(f, impl, cl, over, prop)
    if not over then
        return
    end
    local buf = writer.Buffer()
    buf:write_raw("Overridden from ")
    local pimpl, pcl = find_parent_impl(impl:name_get(), cl)
    buf:write_link(
        impl:function_get():nspaces_get(pcl, true), impl:name_get()
    )
    if prop then
        buf:write_raw(" ")
        local lbuf = writer.Buffer()
        lbuf:write_raw("(")
        if impl:is_prop_get() then
            lbuf:write_raw("get")
            if impl:is_prop_set() then
                lbuf:write_raw(", ")
            end
        end
        if impl:is_prop_set() then
            lbuf:write_raw("set")
        end
        lbuf:write_raw(")")
        buf:write_b(lbuf:finish())
    end
    buf:write_raw(".")
    f:write_i(buf:finish())
end

local impls_of = {}

local get_all_impls_of
get_all_impls_of = function(tbl, cl, fn, got)
    local cfn = cl:name_get()
    if got[cfn] then
        return
    end
    got[cfn] = true
    for i, imp in ipairs(cl:implements_get()) do
        local ofn = imp:function_get()
        if ofn:is_same(fn) then
            tbl[#tbl + 1] = cl
            break
        end
    end
    for i, icl in ipairs(cl:children_get()) do
        get_all_impls_of(tbl, icl, fn, got)
    end
end

local write_ilist = function(f, impl, cl)
    local fn = impl:function_get()
    local fnn = fn:name_get()
    local ocl = fn:implement_get():class_get()
    local onm = ocl:name_get() .. "." .. fnn
    local imps = impls_of[onm]
    if not imps then
        imps = {}
        impls_of[onm] = imps
        get_all_impls_of(imps, ocl, fn, {})
    end

    f:write_h("Implemented by", 2)
    local t = {}
    for i, icl in ipairs(imps) do
        local buf = writer.Buffer()
        local cfn = icl:name_get() .. "." .. fnn
        if icl:is_same(cl) then
            buf:write_b(cfn)
        else
            buf:write_link(fn:nspaces_get(icl, true), cfn)
        end
        t[#t + 1] = buf:finish()
    end
    f:write_list(t)
end

build_method = function(impl, cl)
    local over = impl:is_overridden(cl)
    local fn = impl:function_get()
    local mns = fn:nspaces_get(cl)
    local methn = cl:name_get() .. "." .. fn:name_get()
    local f = writer.Writer(mns, methn)
    printgen("Generating method: " .. methn)

    local doc = impl:doc_get(fn.METHOD)
    if over and not doc:exists() then
        doc = find_parent_doc(impl:name_get(), cl, fn.METHOD)
    end

    f:write_h("Description", 2)
    f:write_raw(doc:full_get(nil, true))
    f:write_nl()

    f:write_editable(mns, "description")
    f:write_nl()

    write_inherited_from(f, impl, cl, over, false)

    f:write_h("Signature", 2)
    f:write_code(gen_method_sig(fn, cl))
    f:write_nl()

    f:write_h("C signature", 2)
    f:write_code(gen_func_csig(fn, nil), "c")
    f:write_nl()

    local pars = fn:parameters_get()
    if #pars > 0 then
        f:write_h("Parameters", 2)
        build_parlist(f, pars)
        f:write_nl()
    end

    write_ilist(f, impl, cl)
    f:write_nl()

    f:finish()
end

build_property = function(impl, cl)
    local over = impl:is_overridden(cl)
    local fn = impl:function_get()
    local pns = fn:nspaces_get(cl)
    local propn = cl:name_get() .. "." .. fn:name_get()
    local f = writer.Writer(pns, propn)
    printgen("Generating property: " .. propn)

    local pimp = fn:implement_get()

    local isget = pimp:is_prop_get()
    local isset = pimp:is_prop_set()

    local doc = impl:doc_get(fn.PROPERTY)
    local gdoc = impl:doc_get(fn.PROP_GET)
    local sdoc = impl:doc_get(fn.PROP_SET)

    if over then
        if not doc:exists() then
            doc = find_parent_doc(impl:name_get(), cl, fn.PROPERTY)
        end
        if isget and not gdoc:exists() then
            gdoc = find_parent_doc(impl:name_get(), cl, fn.PROP_GET)
        end
        if isset and not sdoc:exists() then
            sdoc = find_parent_doc(impl:name_get(), cl, fn.PROP_SET)
        end
    end

    if isget and isset then
        f:write_h("Description", 2)
        if doc:exists() or (not gdoc:exists() and not sdoc:exists()) then
            f:write_raw(doc:full_get(nil, true))
        end
        if (isget and gdoc:exists()) or (isset and sdoc:exists()) then
            f:write_nl(2)
        end
        f:write_nl()
        f:write_editable(pns, "description")
        f:write_nl()
    end

    local pgkeys = isget and fn:property_keys_get(fn.PROP_GET) or {}
    local pskeys = isset and fn:property_keys_get(fn.PROP_SET) or {}
    build_vallist(f, pgkeys, pskeys, "Keys")

    local pgvals = isget and fn:property_values_get(fn.PROP_GET) or {}
    local psvals = isset and fn:property_values_get(fn.PROP_SET) or {}
    build_vallist(f, pgvals, psvals, "Values")

    if isget and gdoc:exists() then
        if isset then
            f:write_h("Getter", 3)
        else
            f:write_h("Description", 2)
        end
        f:write_raw(gdoc:full_get(nil, true))
        if isset and sdoc:exists() then
            f:write_nl(2)
        end
        if isset then
            f:write_nl()
            f:write_editable(pns, "getter_description")
            f:write_nl()
        end
    end

    if isset and sdoc:exists() then
        if isget then
            f:write_h("Setter", 3)
        else
            f:write_h("Description", 2)
        end
        f:write_raw(sdoc:full_get(nil, true))
        if isget then
            f:write_nl()
            f:write_editable(pns, "getter_description")
            f:write_nl()
        end
    end

    f:write_nl()
    if not isget or not isset then
        f:write_nl()
        f:write_br()
        f:write_editable(pns, "description")
        f:write_nl()
    end

    write_inherited_from(f, impl, cl, over, true)

    f:write_h("Signature", 2)
    f:write_code(gen_prop_sig(fn, cl))
    f:write_nl()

    f:write_h("C signature", 2)
    local codes = {}
    if isget then
        codes[#codes + 1] = gen_func_csig(fn, fn.PROP_GET)
    end
    if isset then
        codes[#codes + 1] = gen_func_csig(fn, fn.PROP_SET)
    end
    f:write_code(table.concat(codes, "\n"), "c")
    f:write_nl()

    write_ilist(f, impl, cl)
    f:write_nl()

    f:finish()
end

local build_event_example = function(ev)
    local evcn = ev:c_name_get()
    local evcnl = evcn:lower()

    local dtype = "Data *"

    local tbl = { "static void\n" }
    tbl[#tbl + 1] = "on_"
    tbl[#tbl + 1] = evcnl
    tbl[#tbl + 1] = "(void *data, const Efl_Event *event)\n{\n    "
    tbl[#tbl + 1] = dtree.type_cstr_get(ev:type_get(), "info = event->info;\n")
    tbl[#tbl + 1] = "    Eo *obj = event->object;\n    "
    tbl[#tbl + 1] = dtree.type_cstr_get(dtype, "d = data;\n\n")
    tbl[#tbl + 1] = "    /* event hander code */\n}\n\n"
    tbl[#tbl + 1] = "static void\nsetup_event_handler(Eo *obj, "
    tbl[#tbl + 1] = dtree.type_cstr_get(dtype, "d")
    tbl[#tbl + 1] = ")\n{\n"
    tbl[#tbl + 1] = "    efl_event_callback_add(obj, "
    tbl[#tbl + 1] = evcn
    tbl[#tbl + 1] = ", on_"
    tbl[#tbl + 1] = evcnl
    tbl[#tbl + 1] = ", d);\n}\n"

    return table.concat(tbl)
end

build_event = function(ev, cl)
    local evn = ev:nspaces_get(cl)
    local evnm = cl:name_get() .. ": " .. ev:name_get()
    local f = writer.Writer(evn, evnm)
    printgen("Generating event: " .. evnm)

    f:write_h("Description", 2)
    f:write_raw(ev:doc_get():full_get(nil, true))
    f:write_nl()

    f:write_editable(evn, "description")
    f:write_nl()

    f:write_h("Signature", 2)
    local buf = { ev:name_get() }

    if ev:scope_get() == ev.scope.PRIVATE then
        buf[#buf + 1] = " @private"
    elseif ev:scope_get() == ev.scope.PROTECTED then
        buf[#buf + 1] = " @protected"
    end

    if ev:is_beta() then
        buf[#buf + 1] = " @beta"
    end
    if ev:is_hot() then
        buf[#buf + 1] = " @hot"
    end
    if ev:is_restart() then
        buf[#buf + 1] = " @restart"
    end

    local etp = ev:type_get()
    if etp then
        buf[#buf + 1] = ": "
        buf[#buf + 1] = etp:serialize()
    end

    buf[#buf + 1] = ";"
    f:write_code(table.concat(buf))
    f:write_nl()

    f:write_h("C information", 2)
    f:write_code(build_evcsig(ev), "c")
    f:write_nl()

    f:write_h("C usage", 2)
    f:write_code(build_event_example(ev), "c")
    f:write_nl()

    f:finish()
end

local build_stats_keyref = function()
    for i, cl in ipairs(dtree.Class.all_get()) do
        stats.check_class(cl)
        keyref.add(cl:name_get():gsub("%.", "_"), cl:nspaces_get(), "c")
        for i, imp in ipairs(cl:implements_get()) do
            -- TODO: handle doc overrides in stats system
            if not imp:is_overridden(cl) then
                local func = imp:function_get()
                local fns = func:nspaces_get(cl)
                if imp:is_prop_get() or imp:is_prop_set() then
                    if imp:is_prop_get() then
                        stats.check_property(func, cl, func.PROP_GET)
                        keyref.add(func:full_c_name_get(func.PROP_GET), fns, "c")
                    end
                    if imp:is_prop_set() then
                        stats.check_property(func, cl, func.PROP_SET)
                        keyref.add(func:full_c_name_get(func.PROP_SET), fns, "c")
                    end
                else
                    stats.check_method(func, cl)
                    keyref.add(func:full_c_name_get(func.METHOD), fns, "c")
                end
            end
        end
        for i, ev in ipairs(cl:events_get()) do
            keyref.add(ev:c_name_get(), ev:nspaces_get(cl), "c")
        end
    end
    for i, tp in ipairs(dtree.Typedecl.all_aliases_get()) do
        stats.check_alias(tp)
    end
    for i, tp in ipairs(dtree.Typedecl.all_structs_get()) do
        stats.check_struct(tp)
    end
    for i, tp in ipairs(dtree.Typedecl.all_enums_get()) do
        stats.check_enum(tp)
    end
    for i, v in ipairs(dtree.Variable.all_constants_get()) do
        stats.check_constant(v)
    end
    for i, v in ipairs(dtree.Variable.all_globals_get()) do
        stats.check_global(v)
    end
end

getopt.parse {
    args = arg,
    descs = {
        { category = "General" },
        { "h", "help", nil, help = "Show this message.", metavar = "CATEGORY",
            callback = getopt.help_cb(io.stdout)
        },
        { "v", "verbose", false, help = "Be verbose." },
        { "p", "print-gen", false, help = "Print what is being generated." },

        { category = "Generator" },
        { "r", "root", true, help = "Root path of the docs." },
        { "n", "namespace", true, help = "Root namespace of the docs." },
        { nil, "graph-theme", true, help = "Optional graph theme." },
        { nil, "graph-theme-light", false, help = "Use light builtin graph theme." },
        { nil, "disable-notes", false, help = "Disable notes plugin usage." },
        { nil, "disable-folded", false, help = "Disable folded plugin usage." },
        { nil, "disable-title", false, help = "Disable title plugin usage." },
        { "m", "use-markdown", false,
            help = "Generate Markdown instead of DokuWiki syntax." },
        { nil, "pass", true, help = "The pass to run (optional) "
            .. "(rm, ref, clist, classes, types, vars, stats or class name)." }
    },
    error_cb = function(parser, msg)
        io.stderr:write(msg, "\n")
        getopt.help(parser, io.stderr)
    end,
    done_cb = function(parser, opts, args)
        if opts["h"] then
            return
        end
        if opts["p"] then
            printgen = function(...) print(...) end
        end
        if opts["graph-theme-dark"] then
            current_theme = default_theme_light
        end
        if opts["graph-theme"] then
            set_theme(opts["graph-theme"])
        end
        local rootns = (not opts["n"] or opts["n"] == "")
            and "develop:api" or opts["n"]
        local dr
        if not opts["r"] or opts["r"] == "" then
            dr = "dokuwiki/data/pages"
        else
            dr = opts["r"]
        end
        dr = dutil.path_join(dr, dutil.nspace_to_path(rootns))
        dutil.init(dr, rootns)
        writer.set_backend("dokuwiki")
        if #args == 0 then
            dtree.scan_directory()
        else
            for i, p in ipairs(args) do
                dtree.scan_directory(p)
            end
        end

        local st = opts["pass"]

        dtree.parse(st)

        if st == "clist" then
            for i, cl in ipairs(dtree.Class.all_get()) do
                print(cl:name_get())
            end
            return
        end

        local wfeatures = {
            notes = not opts["disable-notes"],
            folds = not opts["disable-folded"],
            title = not opts["disable-title"]
        }
        writer.init(rootns, wfeatures)
        if not st or st == "rm" then
            dutil.rm_root()
            dutil.mkdir_r(nil)
        end
        if not st or st == "ref" then
            build_ref()
        end
        if not st or st == "classes" then
            build_classes()
        end
        if st and st:match("%.") then
            local cl = dtree.Class.by_name_get(st)
            if cl then
                build_class(cl)
            end
        end
        if not st or st == "types" then
            build_typedecls()
        end
        if not st or st == "vars" then
            build_variables()
        end

        if not st or st == "stats" then
            stats.init(not not opts["v"])
            build_stats_keyref()
            keyref.build()
            -- newline if printing what's being generated
            printgen()
            stats.print()
        end
    end
}

return true
