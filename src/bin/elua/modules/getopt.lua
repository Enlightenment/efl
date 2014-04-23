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

M.getopt = function(args, short, long)
    local ret, opts, args = pcall(getopt_u, args, short, long)
    if not ret then
        return nil, opts
    end
    return opts, args
end

return M