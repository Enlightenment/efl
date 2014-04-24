-- Elua getopt module

local M = {}

local hasval_l = function(opt, long)
    for i, v in ipairs(long) do
        if v:sub(1, #opt) == opt then
            local rest = v:sub(#opt + 1)
            if #rest == 0 then
                return false, opt
            elseif rest == "=" then
                return true, opt
            end
        end
    end
    error("option --" .. opt .. " not recognized", 4)
end

local parse_l = function(opts, opt, long, args)
    local optval
    local i = opt:find("=")
    if i then
        opt, optval = opt:sub(1, i - 1), opt:sub(i + 1)
    end

    local has_val
    hasval, opt = hasval_l(opt, long)
    if hasval then
        if not optval then
            if #args == 0 then
                error("option --" .. opt .. " requires an argument", 3)
            end
            optval, args = args[1], { unpack(args, 2) }
        end
    elseif optval then
        error("option --" .. opt .. " cannot have an argument", 3)
    end
    opts[#opts + 1] = { "--" .. opt, optval }
    return opts, args
end

local hasval_s = function(opt, short)
    if short:find(opt, 1, true) then
        return not not short:find(opt .. ":", 1, true)
    end
    error("option -" .. opt .. " not recognized", 4)
end

local parse_s = function(opts, optstr, short, args)
    while optstr ~= "" do
        local optval
        local opt = optstr:sub(1, 1)
        optstr = optstr:sub(2)
        if hasval_s(opt, short) then
            if optstr == "" then
                if #args == 0 then
                    error("option -" .. opt .. " requires an argument", 3)
                end
                optstr, args = args[1], { unpack(args, 2) }
            end
            optval, optstr = optstr, ""
        end
        opts[#opts + 1] = { "-" .. opt, optval }
    end
    return opts, args
end

local getopt_u = function(args, short, long)
    local opts = {}
    if type(long) == "string" then
        long = { long }
    end
    while args and #args > 0 and args[1]:sub(1, 1) == "-" and args[1] ~= "-" do
        if args[1] == "--" then
            args = { unpack(args, 2) }
            break
        end
        if args[1]:sub(1, 2) == "--" then
            opts, args = parse_l(opts, args[1]:sub(3), long,
                { unpack(args, 2) })
        else
            opts, args = parse_s(opts, args[1]:sub(2), short,
                { unpack(args, 2) })
        end
    end
    return opts, args
end

M.parse = function(args, short, long)
    local ret, opts, args = pcall(getopt_u, args, short, long)
    if not ret then
        return nil, opts
    end
    return opts, args
end
local parse = M.parse

M.parse_desc = function(parser)
    local args = parser.args
    local short, long = {}, {}
    for i, desc in ipairs(parser.descs) do
        if desc[1] then short[#short + 1] = desc[1] end
        local buf
        if desc[2] then buf = desc[2] end
        if desc[3] then
            if buf then buf = buf .. "=" end
            short[#short + 1] = ":"
        end
        if buf then long[#long + 1] = buf end
    end
    short = table.concat(short)
    return parse(args, short, long)
end

M.help = function(parser, f)
    f = f or io.stderr
    local usage = parser.usage
    local progn = parser.prog or parser.args[0] or "program"
    if usage then
        usage = usage:gsub("%%prog", progn)
    else
        usage = ("Usage: %s [OPTIONS]"):format(progn)
    end
    f:write(usage, "\n")
    if parser.header then
        f:write("\n", parser.header:gsub("%%prog", progn), "\n")
    end
    if #parser.descs > 0 then
        local ohdr = parser.optheader
        f:write("\n", ohdr and ohdr:gsub("%%prog", progn)
            or "The following options are supported:", "\n\n")
        local lns = {}
        local lln = 0
        for i, desc in ipairs(parser.descs) do
            if desc[1] or desc[2] then
                local mv = desc.metavar
                if not mv and desc[3] then
                    mv = desc[2] and desc[2]:upper() or "VAL"
                end
                local ln = {}
                ln[#ln + 1] = "  "
                if desc[1] then
                    ln[#ln + 1] = "-" .. desc[1]
                    if mv then ln[#ln + 1] = "[" .. mv .. "]" end
                    if desc[2] then ln[#ln + 1] = ", " end
                end
                if desc[2] then
                    ln[#ln + 1] = "--" .. desc[2]
                    if mv then ln[#ln + 1] = "=[" .. mv .. "]" end
                end
                ln = table.concat(ln)
                lln = math.max(lln, #ln)
                lns[#lns + 1] = { ln, desc.help }
            end
        end
        for i, lnt in ipairs(lns) do
            local ln = lnt[1]
            local hp = lnt[2]
            f:write(ln)
            if hp then f:write((" "):rep(lln - #ln), "  ", hp) end
            f:write("\n")
        end
    end
    if parser.footer then
        f:write("\n", parser.footer:gsub("%%prog", progn), "\n")
    end
end

return M