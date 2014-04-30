-- Elua getopt module

local M = {}

local prefixes = { "-", "--" }

local get_desc = function(opt, j, descs)
    for i, v in ipairs(descs) do
        if v[j] == opt then
            return v
        end
    end
    error("option " .. prefixes[j] .. opt .. " not recognized", 4)
end

local is_arg = function(opt, j, descs)
    if opt == "--" then return true end
    for i, v in ipairs(descs) do
        if v[j] and opt == (prefixes[j] .. v[j]) then
            return true
        end
    end
    return false
end

local parse_l = function(opts, opt, descs, args, parser)
    local optval
    local i = opt:find("=")
    if i then
        opt, optval = opt:sub(1, i - 1), opt:sub(i + 1)
    end

    local desc = get_desc(opt, 2, descs)
    local argr = desc[3]
    if argr or argr == nil then
        if not optval then
            if #args == 0 then
                if argr then
                    error("option --" .. opt .. " requires an argument", 3)
                end
            elseif argr or not is_arg(args[1], 2, descs) then
                optval, args = args[1], { unpack(args, 2) }
            end
        end
    elseif optval then
        error("option --" .. opt .. " cannot have an argument", 3)
    end
    local rets
    if desc.callback then rets = { desc:callback(parser, optval) } end
    if not rets or #rets == 0 then rets = { optval } end
    local optn = desc.alias or desc[1] or desc[2]
    opts[#opts + 1] = { optn, unpack(rets) }
    opts[optn] = #rets > 1 and rets or rets[1]
    return opts, args
end

local parse_s = function(opts, optstr, descs, args, parser)
    while optstr ~= "" do
        local optval
        local opt = optstr:sub(1, 1)
        optstr = optstr:sub(2)
        local desc = get_desc(opt, 1, descs)
        local argr = desc[3]
        if argr or argr == nil then
            if optstr == "" then
                if #args == 0 then
                    if argr then
                        error("option -" .. opt .. " requires an argument", 3)
                    end
                elseif argr or not is_arg(args[1], 1, descs) then
                    optstr, args = args[1], { unpack(args, 2) }
                end
            end
            optval, optstr = optstr, ""
        end
        local rets
        if desc.callback then rets = { desc:callback(parser, optval) } end
        if not rets or #rets == 0 then rets = { optval } end
        local optn = desc.alias or desc[1] or desc[2]
        opts[#opts + 1] = { optn, unpack(rets) }
        opts[optn] = #rets > 1 and rets or rets[1]
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
                { unpack(args, 2) }, parser)
        else
            opts, args = parse_s(opts, args[1]:sub(2), descs,
                { unpack(args, 2) }, parser)
        end
    end
    return opts, args
end

M.parse = function(parser)
    local ret, opts, args = pcall(getopt_u, parser)
    if not ret then
        if  parser.error_cb then
            parser:error_cb(opts)
        end
        return nil, opts
    end
    if  parser.done_cb then
        parser:done_cb(opts, args)
    end
    return opts, args, parser
end
local parse = M.parse

local repl_prog = function(str, progn)
    return (str:gsub("%f[%%]%%prog", progn):gsub("%%%%prog", "%%prog"))
end

local buf_write = function(self, ...)
    local vs = { ... }
    for i, v in ipairs(vs) do self[#self + 1] = v end
end

local help = function(parser, f, category)
    local usage = parser.usage
    local progn = parser.prog or parser.args[0] or "program"
    if usage then
        usage = repl_prog(usage, progn)
    else
        usage = ("Usage: %s [OPTIONS]"):format(progn)
    end
    local buf = { write = buf_write }
    buf:write(usage, "\n")
    if parser.header then
        buf:write("\n", repl_prog(parser.header, progn), "\n")
    end
    if #parser.descs > 0 then
        local ohdr = parser.optheader
        buf:write("\n", ohdr and repl_prog(ohdr, progn)
            or "The following options are supported:", "\n\n")
        local lns = {}
        local lln = 0
        local iscat = false
        local wascat = false
        for i, desc in ipairs(parser.descs) do
            if (not category or iscat) and (desc[1] or desc[2]) then
                local mv = desc.metavar
                if not mv and (desc[3] or desc[3] == nil) then
                    mv = desc[2] and desc[2]:upper() or "VAL"
                elseif desc[3] == false then
                    mv = nil
                end
                local ln = {}
                ln[#ln + 1] = "  "
                if desc[1] then
                    ln[#ln + 1] = "-" .. desc[1]
                    if mv then ln[#ln + 1] = (desc[3] and "[" or "[?")
                        .. mv .. "]" end
                    if desc[2] then ln[#ln + 1] = ", " end
                end
                if desc[2] then
                    ln[#ln + 1] = "--" .. desc[2]
                    if mv then ln[#ln + 1] = (desc[3] and "=[" or "=[?")
                        .. mv .. "]" end
                end
                ln = table.concat(ln)
                lln = math.max(lln, #ln)
                lns[#lns + 1] = { ln, desc.help }
            elseif desc.category then
                local lcat  = category   and   category:lower() or nil
                local alias = desc.alias and desc.alias:lower() or nil
                iscat = (not category) or (alias                 == lcat)
                                       or (desc.category:lower() == lcat)
                if iscat then
                    wascat = true
                    lns[#lns + 1] = { false, desc.category }
                end
            end
        end
        if category and not wascat then
            error("no such category: '" .. category .. "'", 0)
        end
        local fcat = true
        for i, lnt in ipairs(lns) do
            local ln = lnt[1]
            local hp = lnt[2]
            if ln == false then
                if not fcat then
                    buf:write("\n")
                end
                buf:write(hp, ":\n")
                fcat = false
            else
                fcat = false
                buf:write(ln)
                if hp then buf:write((" "):rep(lln - #ln), "  ", hp) end
                buf:write("\n")
            end
        end
    end
    if parser.footer then
        buf:write("\n", repl_prog(parser.footer, progn), "\n")
    end
    f:write(table.concat(buf))
end

M.help = function(parser, category, f)
    if category and type(category) ~= "string" then
        f, category = category, f
    end
    f = f or io.stderr
    local ret, err = pcall(help, parser, f, category)
    if not ret then
        f:write(err, "\n\n")
        help(parser, f)
        return false, err
    end
    return true
end

M.geometry_parse_cb = function(desc, parser, v)
    return v:match("^(%d+):(%d+):(%d+):(%d+)$")
end

M.size_parse_cb = function(desc, parser, v)
    return v:match("^(%d+)x(%d+)$")
end

M.help_cb = function(fstream)
    return function(desc, parser, v)
        M.help(parser, v, fstream)
    end
end

return M