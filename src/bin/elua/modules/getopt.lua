-- Elua getopt module

local M = {}

local get_desc = function(opt, j, descs)
    for i, v in ipairs(descs) do
        if v[j] == opt then
            return v
        end
    end
    error("option --" .. opt .. " not recognized", 4)
end

local parse_l = function(opts, opt, descs, args)
    local optval
    local i = opt:find("=")
    if i then
        opt, optval = opt:sub(1, i - 1), opt:sub(i + 1)
    end

    local desc = get_desc(opt, 2, descs)
    if desc[3] then
        if not optval then
            if #args == 0 then
                error("option --" .. opt .. " requires an argument", 3)
            end
            optval, args = args[1], { unpack(args, 2) }
        end
    elseif optval then
        error("option --" .. opt .. " cannot have an argument", 3)
    end
    opts[#opts + 1] = { desc.alias or desc[1] or desc[2], optval }
    return opts, args
end

local parse_s = function(opts, optstr, descs, args)
    while optstr ~= "" do
        local optval
        local opt = optstr:sub(1, 1)
        optstr = optstr:sub(2)
        local desc = get_desc(opt, 1, descs)
        if desc[3] then
            if optstr == "" then
                if #args == 0 then
                    error("option -" .. opt .. " requires an argument", 3)
                end
                optstr, args = args[1], { unpack(args, 2) }
            end
            optval, optstr = optstr, ""
        end
        opts[#opts + 1] = { desc.alias or desc[1] or desc[2], optval }
    end
    return opts, args
end

local getopt_u  = function(parser)
    local args  = parser.args
    local descs = parser.descs
    local opts  = {}
    while args and #args > 0 and args[1]:sub(1, 1) == "-" and args[1] ~= "-" do
        if args[1] == "--" then
            args = { unpack(args, 2) }
            break
        end
        if args[1]:sub(1, 2) == "--" then
            opts, args = parse_l(opts, args[1]:sub(3), descs,
                { unpack(args, 2) })
        else
            opts, args = parse_s(opts, args[1]:sub(2), descs,
                { unpack(args, 2) })
        end
    end
    return opts, args
end

M.parse = function(parser)
    local ret, opts, args = pcall(getopt_u, parser)
    if not ret then
        return nil, opts
    end
    return opts, args
end
local parse = M.parse

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