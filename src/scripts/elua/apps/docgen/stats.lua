local is_verbose = false

local M = {}

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

local get_percent = function(sv, svu)
    return (sv == 0) and 100 or math.floor(((sv - svu) / sv) * 100 + 0.5)
end

local print_stat = function(printname, statname)
    local sv = stats[statname] or 0
    local svu = stats[statname .. "_undoc"] or 0
    local percent = get_percent(sv, svu)
    local tb = (" "):rep(math.max(0, fcol - #printname - 1) + ncol - stats_pd(sv))
    local dtb = (" "):rep(ncol - stats_pd(sv - svu))
    local ptb = (" "):rep(3 - stats_pd(percent))
    print(("%s:%s%d (documented: %s%d or %s%d%%)")
        :format(printname, tb, sv, dtb, sv - svu, ptb, percent))
end

local get_secstats = function(...)
    local sv, svu = 0, 0
    for i, v in ipairs({ ... }) do
        sv = sv + (stats[v] or 0)
        svu = svu + (stats[v .. "_undoc"] or 0)
    end
    return sv - svu, sv, get_percent(sv, svu)
end

M.print = function()
    for k, v in pairs(stats) do
        ncol = math.max(ncol, stats_pd(v))
    end

    print(("=== CLASS SECTION: %d out of %d (%d%%) ===\n")
        :format(get_secstats("class", "interface", "mixin", "event")))
    print_stat("Classes", "class")
    print_stat("Interfaces", "interface")
    print_stat("Mixins", "mixin")
    print_stat("Events", "event")

    print(("\n=== FUNCTION SECTION: %d out of %d (%d%%) ===\n")
        :format(get_secstats("method", "param", "mret",
                             "getter", "gret", "gkey", "gvalue",
                             "setter", "sret", "skey", "svalue")))
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

    print(("\n=== TYPE SECTION: %d out of %d (%d%%) ===\n")
        :format(get_secstats("alias", "struct", "sfield", "enum", "efield")))
    print_stat("Aliases", "alias")
    print_stat("Structs", "struct")
    print_stat("Struct fields", "sfield")
    print_stat("Enums", "enum")
    print_stat("Enum fields", "efield")

    print(("\n=== VARIABLE SECTION: %d out of %d (%d%%) ===\n")
        :format(get_secstats("constant", "global")))
    print_stat("Constants", "constant")
    print_stat("Globals", "global")

    local sv, svu = 0, 0
    for k, v in pairs(stats) do
        if k:match(".*_undoc$") then
            svu = svu + v
        else
            sv = sv + v
        end
    end
    print(("\n=== TOTAL: %d out of %d (%d%%) ===")
        :format(sv - svu, sv, get_percent(sv, svu)))
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
    if not is_verbose then
        return
    end
    print(tp .. " '" .. name .. "'" .. " missing documentation")
end

M.check_class = function(cl)
    local ct = cl:type_str_get()
    if not ct then
        return
    end
    if not cl:doc_get():exists() then
        print_missing(cl:name_get(), ct)
        stat_incr(ct, true)
    else
        stat_incr(ct, false)
    end

    for i, ev in ipairs(cl:events_get()) do
        if not ev:doc_get():exists() then
            print_missing(cl:name_get() .. "." .. ev:name_get(), "event")
            stat_incr("event", true)
        else
            stat_incr("event", false)
        end
    end
end

M.check_method = function(fn, cl)
    local fulln = cl:name_get() .. "." .. fn:name_get()
    if fn:return_type_get(fn.METHOD) then
        if not fn:return_doc_get(fn.METHOD):exists() then
            print_missing(fulln, "method return")
            stat_incr("mret", true)
        else
            stat_incr("mret", false)
        end
    end
    if not fn:implement_get():doc_get(fn.METHOD):exists() then
        print_missing(fulln, "method")
        stat_incr("method", true)
    else
        stat_incr("method", false)
    end
    for i, p in ipairs(fn:parameters_get()) do
        if not p:doc_get():exists() then
            print_missing(fulln .. "." .. p:name_get(), "method parameter")
            stat_incr("param", true)
        else
            stat_incr("param", false)
        end
    end
end

M.check_property = function(fn, cl, ft)
    local pfxs = {
        [fn.PROP_GET] = "g",
        [fn.PROP_SET] = "s",
    }
    local pfx = pfxs[ft]

    local fulln = cl:name_get() .. "." .. fn:name_get()
    if fn:return_type_get(ft) then
        if not fn:return_doc_get(ft):exists() then
            print_missing(fulln, pfx .. "etter return")
            stat_incr(pfx .. "ret", true)
        else
            stat_incr(pfx .. "ret", false)
        end
    end

    local pimp = fn:implement_get()

    if not pimp:doc_get(fn.PROPERTY):exists() and not pimp:doc_get(ft):exists() then
        print_missing(fulln, pfx .. "etter")
        stat_incr(pfx .. "etter", true)
    else
        stat_incr(pfx .. "etter", false)
    end

    for i, p in ipairs(fn:property_keys_get(ft)) do
        if not p:doc_get():exists() then
            print_missing(fulln .. "." .. p:name_get(), pfx .. "etter key")
            stat_incr(pfx .. "key", true)
        else
            stat_incr(pfx .. "key", false)
        end
    end

    for i, p in ipairs(fn:property_values_get(ft)) do
        if not p:doc_get():exists() then
            print_missing(fulln .. "." .. p:name_get(), pfx .. "etter value")
            stat_incr(pfx .. "value", true)
        else
            stat_incr(pfx .. "value", false)
        end
    end
end

M.check_alias = function(v)
    if not v:doc_get():exists() then
        print_missing(v:name_get(), "alias")
        stat_incr("alias", true)
    else
        stat_incr("alias", false)
    end
end

M.check_struct = function(v)
    if not v:doc_get():exists() then
        print_missing(v:name_get(), "struct")
        stat_incr("struct", true)
    else
        stat_incr("struct", false)
    end
    for i, fl in ipairs(v:struct_fields_get()) do
        if not fl:doc_get():exists() then
            print_missing(v:name_get() .. "." .. fl:name_get(), "struct field")
            stat_incr("sfield", true)
        else
            stat_incr("sfield", false)
        end
    end
end

M.check_enum = function(v)
    if not v:doc_get():exists() then
        print_missing(v:name_get(), "enum")
        stat_incr("enum", true)
    else
        stat_incr("enum", false)
    end
    for i, fl in ipairs(v:enum_fields_get()) do
        if not fl:doc_get():exists() then
            print_missing(v:name_get() .. "." .. fl:name_get(), "enum field")
            stat_incr("efield", true)
        else
            stat_incr("efield", false)
        end
    end
end

M.check_constant = function(v)
    if not v:doc_get():exists() then
        print_missing(v:name_get(), "constant")
        stat_incr("constant", true)
    else
        stat_incr("constant", false)
    end
end

M.check_global = function(v)
    if not v:doc_get():exists() then
        print_missing(v:name_get(), "global")
        stat_incr("global", true)
    else
        stat_incr("global", false)
    end
end

M.init = function(verbose)
    is_verbose = verbose
end

return M
