
local writer = require("docgen.writer")
local dtree = require("docgen.doctree")

local M = {}

local propt_to_type = {
    [dtree.Function.PROPERTY] = "(get, set)",
    [dtree.Function.PROP_GET] = "(get)",
    [dtree.Function.PROP_SET] = "(set)",
}

local verbs = {
    "add",
    "get",
    "is",
    "del",
    "thaw",
    "freeze",
    "save",
    "wait",
    "eject",
    "raise",
    "lower",
    "load",
    "dup",
    "reset",
    "unload",
    "close",
    "set",
    "interpolate",
    "has",
    "grab",
    "check",
    "find",
    "ungrab",
    "unset",
    "clear",
    "pop",
    "new",
    "peek",
    "push",
    "update",
    "show",
    "move",
    "hide",
    "calculate",
    "resize",
    "attach",
    "pack",
    "unpack",
    "emit",
    "call",
    "append"
}

local not_verbs = {
    "below",
    "above",
    "name",
    "unfreezable",
    "value",
    "r",
    "g",
    "b",
    "a",
    "finalize",
    "destructor",
    "to",
    "circle",
    "rect",
    "path",
    "commands",
    "type",
    "colorspace",
    "op",
    "type",
    "properties",
    "status",
    "status",
    "relative",
    "ptr",
    "pair",
    "pos",
    "end"
}

local get_class_name = function(cls)
   local words = {}
   local klass = cls:full_name_get()
   for word in string.gmatch(klass, "%a+") do
      words[#words+1] = word
   end
   for i = 1, #words -1 do
      words[i] = string.lower(words[i])
   end
   return table.concat(words, '.')
end

local get_mono_type
get_mono_type = function(tp)
    if not tp then
        return "void "
    end

    tpt = tp:type_get()
    tpdecl = tp:typedecl_get()

    if tpt == tp.REGULAR then
       if tp:full_name_get() == "string" then
          return "System.String"
       elseif tp:full_name_get() == "list" then
          ntp = tp:base_type_get()
          --assert(btp ~= nil)
          --ntp = btp:next_type_get()
          return "eina.List<" .. get_mono_type(ntp) .. ">"
       elseif tpdecl then
          --print("typedecl type is ", tp:full_name_get())
          tpt = tpdecl:type_get()
          return get_class_name(tp) --tp:full_name_get()
       else
          --print("regular type is ", tp:full_name_get())
          return tp:full_name_get()
       end
    elseif tpt == tp.CLASS then
       return get_class_name(tp)
    else
       return "unknown"
    end
end


local is_verb = function(word)
   for i = 1, #verbs do
      if verbs[i] == word then
         return true
      end
   end
   return false
end

local mono_method_name_get = function(f, ftype)
   local cn = f:name_get(ftype)

   local words = {}

   for word in string.gmatch(cn, "%a+") do
      words[#words+1] = word
   end

   if #words > 1 and is_verb(words[#words]) then
      local tmp = words[#words]
      words[#words] = words[1]
      words[1] = tmp
   end

   for i = 1, #words do
      words[i] = words[i]:gsub("^%l", string.upper)
   end

   if ftype == f.PROP_GET then
      table.insert(words, 1, "Get")
   elseif ftype == f.PROP_SET then
      table.insert(words, 1, "Set")
   end

   return table.concat(words)
end

local gen_mono_param = function(par, out)
    local part = par:type_get()
    out = out or (par:direction_get() == par.OUT)
    if out then
       out = "out "
    else
       out = ""
    end

    return out .. get_mono_type(par:type_get()) .. ' ' .. par:name_get()
    --local tstr = part:c_type_get()
    --return out .. dtree.type_cstr_get(tstr, par:name_get())
end

local get_func_mono_sig_part = function(cn, tp)
   return get_mono_type(tp) .. " " .. cn
end

local find_parent_impl
find_parent_impl = function(fulln, cl)
    for i, pcl in ipairs(cl:inherits_get()) do
        for j, impl in ipairs(pcl:implements_get()) do
            if impl:full_name_get() == fulln then
            --if get_class_name(impl) == fulln then
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


local write_description = function(f, impl, func, cl)
    local over = impl:is_overridden(cl)
    local bdoc

    local doc = impl:doc_get(func.METHOD, true)
    local docf = impl:fallback_doc_get(true)
    if over and (not doc:exists() and (not docf or not docf:exists())) then
        bdoc = find_parent_briefdoc(impl:full_name_get(), cl)
    else
        bdoc = doc:brief_get(docf)
    end
    if bdoc ~= "No description supplied." then
        f:write_raw(bdoc)
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

local gen_func_mono_sig = function(f, ftype)
    ftype = ftype or f.METHOD
    assert(ftype ~= f.PROPERTY)

    local cn = mono_method_name_get(f, ftype)
    local rtype = f:return_type_get(ftype)
    local prefix = ""
    local suffix = ""

    if f:is_class() then
       prefix = "static "
    elseif f:is_const() or ftype == f.PROP_GET then
       suffix = " const"
    end

    if f:type_get() == f.METHOD then
        local pars = f:parameters_get()
        local cnrt = get_func_mono_sig_part(cn, rtype)
        for i = 1, #pars do
            pars[i] = gen_mono_param(pars[i])
        end
        return prefix .. cnrt .. "(" .. table.concat(pars, ", ") .. ")" .. suffix .. ";"
    end

    local keys = f:property_keys_get(ftype)
    local vals = f:property_values_get(ftype)

    if ftype == f.PROP_SET then
        local cnrt = get_func_mono_sig_part(cn, rtype)
        local pars = {}
        for i, par in ipairs(keys) do
            pars[#pars + 1] = gen_mono_param(par)
        end
        for i, par in ipairs(vals) do
            pars[#pars + 1] = gen_mono_param(par)
        end
        return cnrt .. "(" .. table.concat(pars, ", ") .. ");"
    end

    -- getters
    local cnrt
    if not rtype then
        if #vals == 1 then
            cnrt = get_func_mono_sig_part(cn, vals[1]:type_get())
            table.remove(vals, 1)
        else
            cnrt = get_func_mono_sig_part(cn)
        end
    else
        cnrt = get_func_mono_sig_part(cn, rtype)
    end
    local pars = {}
    for i, par in ipairs(keys) do
        pars[#pars + 1] = gen_mono_param(par)
    end
    for i, par in ipairs(vals) do
        print('parameter is value for get, so out')
        pars[#pars + 1] = gen_mono_param(par, true)
    end

    return cnrt .. "(" .. table.concat(pars, ", ") .. ");"
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
            return cl1:full_name_get() < cl2:full_name_get()
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

M.build_inherits = function(cl, t, lvl)
    t = t or {}
    lvl = lvl or 0
    local lbuf = writer.Buffer()
    if lvl > 0 then
        local cln = cl:nspaces_get(true)
        cln[#cln] = nil
        cln[#cln] = cln[#cln] .. "_mono"
        cln = ":" .. 'develop:api' .. ":"
           .. table.concat(cln, ":")
        lbuf:write_raw("[[", cln, "|", get_class_name(cl), "]]")
        --lbuf:write_link(cl:nspaces_get(true), cl:full_name_get())
        lbuf:write_raw(" ")
        lbuf:write_i("(" .. cl:type_str_get() .. ")")
 
        t[#t + 1] = { lvl - 1, lbuf:finish() }
    end

    for i, acl in ipairs(cl:inherits_get()) do
        M.build_inherits(acl, t, lvl + 1)
    end
    return t
end

M.build_inherit_summary = function(cl, buf)
    buf = buf or writer.Buffer()
    buf:write_raw(" => ")

    local cln = cl:nspaces_get(true)
    cln[#cln] = nil
    cln[#cln] = cln[#cln] .. "_mono"
    cln = ":" .. 'develop:api' .. ":"
       .. table.concat(cln, ":")
    buf:write_raw("[[", cln, "|", get_class_name(cl), "]]")
    buf:write_raw(" ")
    buf:write_i("(" .. cl:type_str_get() .. ")")

    local inherits = cl:inherits_get()
    if #inherits ~= 0 then
        M.build_inherit_summary(inherits[1], buf)
    end
    return buf
end

M.write_inherit_functable = function(f, tcl, tbl)
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
            f:write_link(cl:nspaces_get(true), cl:full_name_get())
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

M.write_functable = function(f, tcl, tbl)
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
            llbuf:write_link(ocl:nspaces_get(true), ocl:full_name_get())
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
            codes[#codes + 1] = gen_func_mono_sig(func, func:type_get())
        else
            codes[#codes + 1] = gen_func_mono_sig(func, dtree.Function.PROP_GET)
            codes[#codes + 1] = gen_func_mono_sig(func, dtree.Function.PROP_SET)
        end
        f:write_code(table.concat(codes, "\n"), "c")
        f:write_br(true)
    end
    f:write_nl()
end

M.build_class = function(cl)
    local cln = cl:nspaces_get()
    local fulln = cl:full_name_get()
    --table.insert(cln, "mono")
    cln[#cln] = cln[#cln] .. "_mono"
    --printgen("Generating (MONO) class: " .. fulln .. " in ns ", unpack(cln))
    local f = writer.Writer(cln, fulln .. " (mono)")
    f:write_h(cl:full_name_get() .. " (" .. cl:type_str_get() .. ")", 1)

    f:write_h("Description", 2)
    f:write_raw(cl:doc_get():full_get(nil, true))
    f:write_nl(2)

    f:write_editable(cln, "description")
    f:write_nl()

    local inherits = cl:inherits_get()
    if #inherits ~= 0 then
        f:write_h("Inheritance", 2)

        f:write_raw(M.build_inherit_summary(inherits[1]):finish())
	f:write_nl()

        f:write_folded("Full hierarchy", function()
            f:write_list(M.build_inherits(cl))
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
    M.write_functable(f, cl, meths, true)
    if #omeths ~= 0 then
        f:write_h("Inherited", 3)
    end
    M.write_inherit_functable(f, cl, omeths, false)
    
    f:finish()
end

return M


