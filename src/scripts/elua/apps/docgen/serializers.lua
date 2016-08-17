local keyref = require("docgen.keyref")
local dtree = require("docgen.doctree")

local M = {}

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

M.get_typedecl_str = function(tp)
    local tpt = tp:type_get()
    if tpt == dtree.Typedecl.UNKNOWN then
        error("unknown typedecl: " .. tp:full_name_get())
    elseif tpt == dtree.Typedecl.STRUCT or
           tpt == dtree.Typedecl.STRUCT_OPAQUE then
        local buf = { "struct " }
        add_typedecl_attrs(tp, buf)
        buf[#buf + 1] = tp:full_name_get()
        if tpt == dtree.Typedecl.STRUCT_OPAQUE then
            buf[#buf + 1] = ";"
            return table.concat(buf)
        end
        local fields = tp:struct_fields_get()
        if #fields == 0 then
            buf[#buf + 1] = " {}"
            return table.concat(buf)
        end
        buf[#buf + 1] = " {\n"
        for i, fld in ipairs(fields) do
            buf[#buf + 1] = "    "
            buf[#buf + 1] = fld:name_get()
            buf[#buf + 1] = ": "
            buf[#buf + 1] = fld:type_get():serialize()
            buf[#buf + 1] = ";\n"
        end
        buf[#buf + 1] = "}"
        return table.concat(buf)
    elseif tpt == dtree.Typedecl.ENUM then
        local buf = { "enum " }
        add_typedecl_attrs(tp, buf)
        buf[#buf + 1] = tp:full_name_get()
        local fields = tp:enum_fields_get()
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
    elseif tpt == dtree.Typedecl.ALIAS then
        local buf = { "type " }
        add_typedecl_attrs(tp, buf)
        buf[#buf + 1] = tp:full_name_get()
        buf[#buf + 1] = ": "
        buf[#buf + 1] = tp:base_type_get():serialize()
        buf[#buf + 1] = ";"
        return table.concat(buf)
    end
    error("unhandled typedecl type: " .. tpt)
end

M.get_typedecl_cstr = function(tp)
    local tpt = tp:type_get()
    if tpt == dtree.Typedecl.UNKNOWN then
        error("unknown typedecl: " .. tp:full_name_get())
    elseif tpt == dtree.Typedecl.STRUCT or
           tpt == dtree.Typedecl.STRUCT_OPAQUE then
        local buf = { "typedef struct " }
        local fulln = tp:full_name_get():gsub("%.", "_");
        keyref.add(fulln, "c")
        buf[#buf + 1] = "_" .. fulln;
        if tpt == dtree.Typedecl.STRUCT_OPAQUE then
            buf[#buf + 1] = " " .. fulln .. ";"
            return table.concat(buf)
        end
        local fields = tp:struct_fields_get()
        if #fields == 0 then
            buf[#buf + 1] = " {} " .. fulln .. ";"
            return table.concat(buf)
        end
        buf[#buf + 1] = " {\n"
        for i, fld in ipairs(fields) do
            buf[#buf + 1] = "    "
            buf[#buf + 1] = dtree.type_cstr_get(fld:type_get(), fld:name_get())
            buf[#buf + 1] = ";\n"
        end
        buf[#buf + 1] = "} " .. fulln .. ";"
        return table.concat(buf)
    elseif tpt == dtree.Typedecl.ENUM then
        local buf = { "typedef enum" }
        local fulln = tp:full_name_get():gsub("%.", "_");
        keyref.add(fulln, "c")
        local fields = tp:enum_fields_get()
        if #fields == 0 then
            buf[#buf + 1] = " {} " .. fulln .. ";"
            return table.concat(buf)
        end
        buf[#buf + 1] = " {\n"
        for i, fld in ipairs(fields) do
            buf[#buf + 1] = "    "
            local cn = fld:c_name_get()
            buf[#buf + 1] = cn
            keyref.add(cn, "c")
            local val = fld:value_get()
            if val then
                buf[#buf + 1] = " = "
                local ev = val:eval_enum()
                local lit = ev:to_literal()
                buf[#buf + 1] = lit
                local ser = val:serialize()
                if ser and ser ~= lit then
                    buf[#buf + 1] = " /* " .. ser .. " */"
                end
            end
            if i == #fields then
                buf[#buf + 1] = "\n"
            else
                buf[#buf + 1] = ",\n"
            end
        end
        buf[#buf + 1] = "} " .. fulln .. ";"
        return table.concat(buf)
    elseif tpt == dtree.Typedecl.ALIAS then
        local fulln = tp:full_name_get():gsub("%.", "_");
        keyref.add(fulln, "c")
        return "typedef " .. dtree.type_cstr_get(tp:base_type_get(), fulln) .. ";"
    end
    error("unhandled typedecl type: " .. tpt)
end

return M
