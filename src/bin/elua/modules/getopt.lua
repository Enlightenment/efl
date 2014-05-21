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
                optval = table.remove(args, 1)
            end
        end
    elseif optval then
        error("option --" .. opt .. " cannot have an argument", 3)
    end
    local rets
    if desc.callback then
        rets = { desc:callback(parser, optval, opts) }
    end
    if not rets or #rets == 0 then rets = { optval } end
    local optn = desc.alias or desc[1] or desc[2]
    opts[#opts + 1] = { optn, short = desc[1], long = desc[2],
        alias = desc.alias, val = optval, unpack(rets) }
    local optret = #rets > 1 and rets or rets[1]
    if desc.list then
        desc.list[#desc.list + 1] = optret
        opts[optn] = desc.list
    else
        opts[optn] = optret or true
    end
    local dopts = desc.opts
    if    dopts then
          dopts[#dopts + 1] = opts[#opts]
          dopts[optn]       = opts[optn ]
    end
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
                    optstr = table.remove(args, 1)
                end
            end
            optval, optstr = optstr, ""
        end
        local rets
        if desc.callback then
            rets = { desc:callback(parser, optval, opts) }
        end
        if not rets or #rets == 0 then rets = { optval } end
        local optn = desc.alias or desc[1] or desc[2]
        opts[#opts + 1] = { optn, short = desc[1], long = desc[2],
            alias = desc.alias, val = optval, unpack(rets) }
        local optret = #rets > 1 and rets or rets[1]
        if desc.list then
            desc.list[#desc.list + 1] = optret
            opts[optn] = desc.list
        else
            opts[optn] = optret or true
        end
        local dopts = desc.opts
        if    dopts then
              dopts[#dopts + 1] = opts[#opts]
              dopts[optn]       = opts[optn ]
        end
    end
end

local getopt_u  = function(parser)
    local args  = { unpack(parser.args) }
    local descs = parser.descs
    local opts  = {}
    while #args > 0 and args[1]:sub(1, 1) == "-" and args[1] ~= "-" do
        local v = table.remove(args, 1)
        if v == "--" then break end
        if v:sub(1, 2) == "--" then
            parse_l(opts, v:sub(3), descs, args, parser)
        else
            parse_s(opts, v:sub(2), descs, args, parser)
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

local get_metavar = function(desc)
    local mv = desc.metavar
    if not mv and (desc[3] or desc[3] == nil) then
        mv = desc[2] and desc[2]:upper() or "VAL"
    elseif desc[3] == false then
        mv = nil
    end
    return mv
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
    local nignore = 0
    for i, desc in ipairs(parser.descs) do
        if desc.help == false then nignore = nignore + 1 end
    end
    if #parser.descs > nignore then
        local ohdr = parser.optheader
        buf:write("\n", ohdr and repl_prog(ohdr, progn)
            or "The following options are supported:", "\n\n")
        local lls = 0
        for i, desc in ipairs(parser.descs) do
            if desc.help ~= false and desc[1] then
                local mv = get_metavar(desc)
                if mv then
                    lls = math.max(lls, #mv + ((desc[3] == nil) and 5 or 4))
                else
                    lls = math.max(lls, 2)
                end
            end
        end
        local lns = {}
        local lln = 0
        local iscat = false
        local wascat = false
        for i, desc in ipairs(parser.descs) do
            local nign = desc.help ~= false
            if nign and (not category or iscat)
            and (desc[1] or desc[2] or desc.metavar) then
                local mv = get_metavar(desc)
                local ln = {}
                ln[#ln + 1] = "  "
                if desc[1] then
                    ln[#ln + 1] = "-" .. desc[1]
                    if mv then ln[#ln + 1] = (desc[3] and "[" or "[?")
                        .. mv .. "]" end
                    local sln = #table.concat(ln) - 2
                    local sdf = lls - sln
                    if desc[2] then ln[#ln + 1] = ", " end
                    if sdf > 0 then
                        ln[#ln + 1] = (" "):rep(sdf)
                    end
                elseif not desc[2] and mv then
                    ln[#ln + 1] = mv
                else
                    ln[#ln + 1] = (" "):rep(lls + 2)
                end
                if desc[2] then
                    ln[#ln + 1] = "--" .. desc[2]
                    if mv then ln[#ln + 1] = (desc[3] and "=[" or "=[?")
                        .. mv .. "]" end
                end
                ln = table.concat(ln)
                lln = math.max(lln, #ln)
                lns[#lns + 1] = { ln, desc.help }
            elseif nign and desc.category then
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