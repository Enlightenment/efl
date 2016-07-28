local eolian = require("eolian")
local getopt = require("getopt")

local eomap = require("docgen.mappings")
local stats = require("docgen.stats")
local dutil = require("docgen.util")
local writer = require("docgen.writer")
local keyref = require("docgen.keyref")
local ser = require("docgen.serializers")

local use_dot

-- eolian to various doc elements conversions

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
    out = out or (par:direction_get() == eolian.parameter_dir.OUT)
    local tstr = part:c_type_get()
    if out then
        tstr = ser.get_ctype_str(tstr, "*")
    end
    return ser.get_ctype_str(tstr, par:name_get())
end

local get_func_csig_part = function(cn, tp)
    if not tp then
        return "void " .. cn
    end
    return ser.get_ctype_str(tp, cn)
end

local gen_func_csig = function(f, ftype)
    ftype = ftype or eolian.function_type.METHOD
    assert(ftype ~= eolian.function_type.PROPERTY)

    local cn = f:full_c_name_get(ftype)
    keyref.add(cn, "c")
    local rtype = f:return_type_get(ftype)

    local fparam = "Eo *obj"
    if f:is_const() or f:is_class() or ftype == eolian.function_type.PROP_GET then
        fparam = "const Eo *obj"
    end

    if f:type_get() == eolian.function_type.METHOD then
        local pars = f:parameters_get():to_array()
        local cnrt = get_func_csig_part(cn, rtype)
        for i = 1, #pars do
            pars[i] = gen_cparam(pars[i])
        end
        table.insert(pars, 1, fparam);
        return cnrt .. "(" .. table.concat(pars, ", ") .. ");"
    end

    local keys = f:property_keys_get(ftype):to_array()
    local vals = f:property_values_get(ftype):to_array()

    if ftype == eolian.function_type.PROP_SET then
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
    buf[#buf + 1] = cl:full_name_get()
    buf[#buf + 1] = "."
    buf[#buf + 1] = fn:name_get()
    buf[#buf + 1] = " "
    local ftt = eolian.function_type
    local obs = eolian.object_scope
    if not isprop then
        if fn:scope_get(ftt.METHOD) == obs.PROTECTED then
            buf[#buf + 1] = "@protected "
        end
    elseif isget and isset then
        if fn:scope_get(ftt.PROP_GET) == obs.PROTECTED and
           fn:scope_get(ftt.PROP_SET) == obs.PROTECTED then
            buf[#buf + 1] = "@protected "
        end
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
    buf[#buf + 1] = ser.get_type_str(fp:type_get())
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
    buf[#buf + 1] = ser.get_type_str(rett)
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
    local fnt = fn:type_get()
    local ftt = eolian.function_type
    local obs = eolian.object_scope
    local isget = (fnt == ftt.PROPERTY or fnt == ftt.PROP_GET)
    local isset = (fnt == ftt.PROPERTY or fnt == ftt.PROP_SET)
    gen_func_namesig(fn, cl, buf, true, isget, isset)

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
        buf[#buf + 1] = "    get "
        if fn:scope_get(ftt.PROP_GET) == obs.PROTECTED and
           fn:scope_get(ftt.PROP_SET) ~= obs.PROTECTED then
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
                gen_func_return(fn, ftt.PROP_GET, buf, 2)
            end
            buf[#buf + 1] = "    }\n"
        end
    end

    if isset then
        buf[#buf + 1] = "    set "
        if fn:scope_get(ftt.PROP_SET) == obs.PROTECTED and
           fn:scope_get(ftt.PROP_GET) ~= obs.PROTECTED then
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

-- builders

local build_method, build_property, build_event

local build_reftable = function(f, title, ctitle, ctype, t)
    if not t or #t == 0 then
        return
    end
    f:write_h(title, 2)
    local nt = {}
    for i, v in ipairs(t) do
        nt[#nt + 1] = {
            writer.Buffer():write_link(eomap.gen_nsp_eo(v, ctype, true),
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
        local lbuf = writer.Buffer()
        lbuf:write_link(eomap.gen_nsp_func(v, cl, true), v:name_get())
        local pt = propt_to_type[v:type_get()]
        if pt then
            lbuf:write_raw(" ")
            lbuf:write_i(pt)
        end
        nt[#nt + 1] = {
            lbuf:finish(), get_brief_fdoc(v)
        }
        if eomap.funct_to_str[v:type_get()] == "property" then
            build_property(v, cl)
        else
            build_method(v, cl)
        end
    end
    table.sort(nt, function(v1, v2) return v1[1] < v2[1] end)
    f:write_table({ ctitle, "Brief description" }, nt)
    f:write_nl()
end

local build_ref = function()
    local f = writer.Writer("reference")
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
    local lbuf = writer.Buffer()
    lbuf:write_link(eomap.gen_nsp_class(cl, true), cl:full_name_get())
    lbuf:write_raw(" ")
    lbuf:write_i("(" .. eomap.classt_to_str[cl:type_get()] .. ")")
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

local default_theme = {
    classes = {
        regular = {
            style = "filled",
            color = "black",
            fill_color = "white",
            primary_color = "black",
            primary_fill_color = "gray"
        },
        abstract = {
            style = "filled",
            color = "black",
            fill_color = "white",
            primary_color = "black",
            primary_fill_color = "gray"
        },
        mixin = {
            style = "filled",
            color = "blue",
            fill_color = "white",
            primary_color = "blue",
            primary_fill_color = "skyblue"
        },
        interface = {
            style = "filled",
            color = "cornflowerblue",
            fill_color = "white",
            primary_color = "cornflowerblue",
            primary_fill_color = "azure"
        }
    },
    node = {
        shape = "box"
    },
    edge = {
        color = "black"
    },
    bg_color = "transparent"
}

local classt_to_theme = {
    [eolian.class_type.REGULAR] = "regular",
    [eolian.class_type.ABSTRACT] = "abstract",
    [eolian.class_type.MIXIN] = "mixin",
    [eolian.class_type.INTERFACE] = "interface"
}

local class_to_node = function(cl, main)
    local ret = {}

    ret.label = cl:full_name_get()
    ret.name = ret.label:lower():gsub("%.", "_")

    local clr = classt_to_theme[cl:type_get()]

    ret.style = default_theme.classes[clr].style
    ret.color = default_theme.classes[clr][main and "primary_color" or "color"]
    ret.fillcolor = default_theme.classes[clr][main and "primary_fill_color"
                                                     or "fill_color"]

    -- FIXME: need a dokuwiki graphviz plugin with proper URL support
    -- the existing one only supports raw URLs (no dokuwikí namespaces)
    --ret.URL = ":" .. global_opts.root_nspace .. ":"
    --              .. table.concat(eomap.gen_nsp_class(cl), ":")

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
            bgcolor = default_theme.bg_color
        },
        node = default_theme.node,
        edge = default_theme.edge
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
    local f = writer.Writer(eomap.gen_nsp_class(cl))
    stats.check_class(cl)

    f:write_h(cl:full_name_get(), 2)
    keyref.add(cl:full_name_get():gsub("%.", "_"), "c")

    f:write_h("Inheritance hierarchy", 3)
    f:write_list(build_inherits(cl))
    f:write_nl()
    if use_dot then
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
        local nt = {}
        for i, ev in ipairs(evs) do
            local lbuf = writer.Buffer()
            lbuf:write_link(eomap.gen_nsp_ev(ev, cl, true), ev:name_get())
            nt[#nt + 1] = {
                lbuf:finish(), get_brief_doc(ev:documentation_get())
            }
            build_event(ev, cl)
        end
        table.sort(nt, function(v1, v2) return v1[1] < v2[1] end)
        f:write_table({ "Event name", "Brief description" }, nt)
    end

    f:finish()
end

local build_classes = function()
    for cl in eolian.all_classes_get() do
        local ct = cl:type_get()
        if not eomap.classt_to_str[ct] then
            error("unknown class: " .. cl:full_name_get())
        end
        build_class(cl)
    end
end

local write_tsigs = function(f, tp)
    f:write_h(tp:full_name_get(), 2)

    f:write_h("Signature", 3)
    f:write_code(ser.get_typedecl_str(tp))
    f:write_nl()

    f:write_h("C signature", 3)
    f:write_code(ser.get_typedecl_cstr(tp), "c")
    f:write_nl()
end

local build_alias = function(tp)
    local f = writer.Writer(eomap.gen_nsp_eo(tp, "alias"))
    stats.check_alias(tp)

    write_tsigs(f, tp)

    f:write_h("Description", 3)
    write_full_doc(f, tp:documentation_get())
    f:write_nl(2)

    f:finish()
end

local build_struct = function(tp)
    local f = writer.Writer(eomap.gen_nsp_eo(tp, "struct"))
    stats.check_struct(tp)

    write_tsigs(f, tp)

    f:write_h("Description", 3)
    write_full_doc(f, tp:documentation_get())
    f:write_nl(2)

    f:write_h("Fields", 3)

    local arr = {}
    for fl in tp:struct_fields_get() do
        local buf = writer.Buffer()
        buf:write_b(fl:name_get())
        buf:write_raw(" - ", get_full_doc(fl:documentation_get()))
        arr[#arr + 1] = buf:finish()
    end
    f:write_list(arr)
    f:write_nl()

    f:finish()
end

local build_enum = function(tp)
    local f = writer.Writer(eomap.gen_nsp_eo(tp, "enum"))
    stats.check_enum(tp)

    write_tsigs(f, tp)

    f:write_h("Description", 3)
    write_full_doc(f, tp:documentation_get())
    f:write_nl(2)

    f:write_h("Fields", 3)

    local arr = {}
    for fl in tp:enum_fields_get() do
        local buf = writer.Buffer()
        buf:write_b(fl:name_get())
        buf:write_raw(" - ", get_full_doc(fl:documentation_get()))
        arr[#arr + 1] = buf:finish()
    end
    f:write_list(arr)
    f:write_nl()

    f:finish()
end

local build_variable = function(v, constant)
    local f = writer.Writer(eomap.gen_nsp_eo(v, constant and "constant" or "global"))
    if constant then
        stats.check_constant(v)
    else
        stats.check_global(v)
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

local build_parlist = function(f, pl, nodir)
    local params = {}
    for i, p in ipairs(pl) do
        local buf = writer.Buffer()
        buf:write_b(p:name_get())
        if not nodir then
            buf:write_raw(" ")
            buf:write_i(eomap.pdir_to_str[p:direction_get()])
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
    local f = writer.Writer(eomap.gen_nsp_func(fn, cl))
    stats.check_method(fn, cl)

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
    local f = writer.Writer(eomap.gen_nsp_func(fn, cl))

    local fts = eolian.function_type
    local ft = fn:type_get()
    local isget = (ft == fts.PROP_GET or ft == fts.PROPERTY)
    local isset = (ft == fts.PROP_SET or ft == fts.PROPERTY)

    if isget then stats.check_property(fn, cl, fts.PROP_GET) end
    if isset then stats.check_property(fn, cl, fts.PROP_SET) end

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

build_event = function(ev, cl)
    local f = writer.Writer(eomap.gen_nsp_ev(ev, cl))

    f:write_h(cl:full_name_get() .. ": " .. ev:name_get(), 2)

    f:write_h("Signature", 3)
    local buf = { ev:name_get() }

    if ev:scope_get() == eolian.object_scope.PRIVATE then
        buf[#buf + 1] = " @private"
    elseif ev:scope_get() == eolian.object_scope.PROTECTED then
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
        buf[#buf + 1] = ser.get_type_str(etp)
    end

    buf[#buf + 1] = ";"
    f:write_code(table.concat(buf))
    f:write_nl()

    f:write_h("C signature", 3)
    local cn = ev:c_name_get()
    keyref.add(cn, "c")
    f:write_code(ser.get_ctype_str(etp, cn) .. ";", "c")
    f:write_nl()

    f:write_h("Description", 3)
    write_full_doc(f, ev:documentation_get())
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
        use_dot = not opts["disable-graphviz"]
        local rootns = (not opts["n"] or opts["n"] == "")
            and "efl" or opts["n"]
        local dr
        if not opts["r"] or opts["r"] == "" then
            dr = "dokuwiki/data/pages"
        else
            dr = opts["r"]
        end
        dr = dutil.path_join(dr, dutil.nspace_to_path(rootns))
        dutil.init(dr)
        if #args == 0 then
            if not eolian.system_directory_scan() then
                error("failed scanning system directory")
            end
        else
            for i, p in ipairs(args) do
                if not eolian.directory_scan(p) then
                    error("failed scanning directory: " .. p)
                end
            end
        end
        if not eolian.all_eot_files_parse() then
            error("failed parsing eo type files")
        end
        if not eolian.all_eo_files_parse() then
            error("failed parsing eo files")
        end
        stats.init(not not opts["v"])
        writer.init(rootns, not opts["disable-notes"])
        dutil.rm_root()
        dutil.mkdir_r(nil)
        build_ref()
        build_classes()
        build_typedecls()
        build_variables()
        keyref.build()
        stats.print()
    end
}

return true
