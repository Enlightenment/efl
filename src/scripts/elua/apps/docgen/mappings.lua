local eolian = require("eolian")
local dtree = require("docgen.doctree")

local M = {}

M.pdir_to_str = {
    [eolian.parameter_dir.IN] = "(in)",
    [eolian.parameter_dir.OUT] = "(out)",
    [eolian.parameter_dir.INOUT] = "(inout)"
}

M.scope_to_str = {
    [eolian.object_scope.PUBLIC] = "public",
    [eolian.object_scope.PRIVATE] = "private",
    [eolian.object_scope.PROTECTED] = "protected"
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
        local ret = dtree.Class(decl:class_get()):type_str_get()
        if not ret then
            error("unknown class type for class '" .. decl:name_get() .. "'")
        end
        return ret
    else
        error("unknown declaration type for declaration '"
            .. decl:name_get() .. "'")
    end
end

M.gen_nsp_eo = function(eobj, subn, root)
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

M.gen_nsp_ref = function(str, root)
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
            return M.gen_nsp_ref(bstr, root)
        end
    end

    local ftp = eolian.function_type

    local cl = dtree.Class.by_name_get(bstr)
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
            cl = dtree.Class.by_name_get(bstr)
            if cl then
                fn = cl:function_get_by_name(mname, ftype)
            end
        end
    else
        fn = cl:function_get_by_name(sfx:sub(2), ftype)
        if fn then ftype = fn:type_get() end
    end

    if not fn or not fn:type_str_get() then
        error("invalid reference '" .. str .. "'")
    end

    local ret = M.gen_nsp_ref(bstr)
    ret[#ret + 1] = fn:type_str_get()
    ret[#ret + 1] = fn:name_get():lower()
    if root then ret[#ret + 1] = true end
    return ret
end

return M
