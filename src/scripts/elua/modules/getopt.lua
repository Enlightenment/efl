--[[ getopt.lua: An argument parsing library for Lua 5.1 and later.

    This module is provided as a self-contained implementation with builtin
    documentation.

    TODO:
        - mutually exclusive groups
        - i18n support

    Copyright (c) 2014 Daniel "q66" Kolesa <quaker66@gmail.com>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
]]

local arg = _G.arg -- Capture global 'arg'

local M = {}

local prefixes = { "-", "--" }

local ssub, sfind, sgsub, sformat, smatch = string.sub,  string.find,
                                            string.gsub, string.format,
                                            string.match

local slower, supper, srep = string.lower, string.upper, string.rep

local unpack = table.unpack or unpack

local ac_process_name = function(np, nm)
    if not nm or #nm < 2 then
        return
    end
    local t = np[nm]
    if t then
        t[#t + 1] = nm
    else
        np[nm] = { nm }
    end
    for i = 1, #nm do
        local pnm = ssub(nm, 1, i - 1) .. ssub(nm, i + 1)
        local t = np[pnm]
        if not t then
            t = {}
            np[pnm] = t
        end
        t[#t + 1] = nm
    end
end

local get_autocorrect = function(descs, wrong, vi)
    if #wrong < 2 then
        return nil
    end

    local np = {}
    local pn = {}

    for i, v in ipairs(descs) do
        ac_process_name(np, v[vi])
    end

    for i = 1, #wrong do
        local nm = ssub(wrong, 1, i - 1) .. ssub(wrong, i + 1)
        local inp = np[nm]
        if inp then
            if inp == true then
                pn[nm] = true
            else
                for i, pnm in ipairs(inp) do
                    pn[pnm] = true
                end
            end
        end
    end

    local try = next(pn)
    if not try then
        return nil
    end
    if not next(pn, try) then
        return "\nmaybe you meant '" .. prefixes[vi] .. try .. "'?"
    end
    local narr = {}
    for k, v in pairs(pn) do
        narr[#narr + 1] = "'" .. prefixes[vi] .. k .. "'"
    end
    return "\nmaybe you meant one of: " .. table.concat(narr, ", ") .. "?"
end

local get_desc = function(opt, j, descs)
    for i, v in ipairs(descs) do
        if v[j] == opt then
            return v
        end
    end
    local ac = get_autocorrect(descs, opt, j) or ""
    error("option " .. prefixes[j] .. opt .. " not recognized" .. ac, 0)
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

local write_arg = function(desc, j, opts, opt, optval, parser, argcounts)
    local rets
    if desc.callback then
        rets = { desc:callback(parser, optval, opts) }
    end
    if not rets or #rets == 0 then rets = { optval } end
    local optn = desc.alias or desc[1] or desc[2]
    local cnt = desc.max_count or (desc.list and -1 or 1)
    local acnt = argcounts[optn]
    if acnt then
        if cnt >= 0 and acnt >= cnt then
            error("option " .. prefixes[j] .. opt
                .. " can be specified at most " .. cnt .. " times", 0)
        end
        argcounts[optn] = acnt + 1
    else
        argcounts[optn] = 1
    end
    opts[#opts + 1] = { optn, short = desc[1], long = desc[2],
        alias = desc.alias, val = optval, unpack(rets) }
    local optret = #rets > 1 and rets or rets[1]
    if desc.list then
        desc.list[#desc.list + 1] = optret
        opts[optn] = desc.list
    elseif optret ~= nil then
        opts[optn] = optret
    else
        opts[optn] = true
    end
    local dopts = desc.opts
    if    dopts then
          dopts[#dopts + 1] = opts[#opts]
          dopts[optn]       = opts[optn ]
    end
end

local parse_l = function(opts, opt, descs, args, parser, argcounts)
    local optval
    local i = sfind(opt, "=")
    if i then
        opt, optval = ssub(opt, 1, i - 1), ssub(opt, i + 1)
    end

    local desc = get_desc(opt, 2, descs)
    local argr = desc[3]
    if argr or argr == nil then
        if not optval then
            if #args == 0 then
                if argr then
                    error("option --" .. opt .. " requires an argument", 0)
                end
            elseif argr or not is_arg(args[1], 2, descs) then
                optval = table.remove(args, 1)
            end
        end
    elseif optval then
        error("option --" .. opt .. " cannot have an argument", 0)
    end
    write_arg(desc, 2, opts, opt, optval, parser, argcounts)
end

local parse_s = function(opts, optstr, descs, args, parser, argcounts)
    local opt = ssub(optstr, 1, 1)
    local optval = ssub(optstr, 2)
    local desc = get_desc(opt, 1, descs)
    local argr = desc[3]
    if argr or argr == nil then
        if optval == "" then
            optval = nil
            if #args == 0 then
                if argr then
                    error("option -" .. opt .. " requires an argument", 0)
                end
            elseif argr or not is_arg(args[1], 1, descs) then
                optval = table.remove(args, 1)
            end
        end
    elseif optval ~= "" then
        error("option -" .. opt .. " cannot have an argument", 0)
    else
        optval = nil
    end
    write_arg(desc, 1, opts, opt, optval, parser, argcounts)
end

local getopt_u  = function(parser)
    local argcounts = {}
    local args  = { unpack(parser.args or arg) }
    local descs = parser.descs
    local opts  = {}
    while #args > 0 and ssub(args[1], 1, 1) == "-" and args[1] ~= "-" do
        local v = table.remove(args, 1)
        if v == "--" then break end
        if ssub(v, 1, 2) == "--" then
            parse_l(opts, ssub(v, 3), descs, args, parser, argcounts)
        else
            parse_s(opts, ssub(v, 2), descs, args, parser, argcounts)
        end
    end
    return opts, args
end

--[[
    Given a parser definition, parse the arguments and return all optional
    arguments, all positional arguments and the parser itself.

    It takes exactly one argument, a parser.

    The parser is a dictionary. It can contain these fields:
        - usage - a usage string.
        - prog - a program name.
        - error_cb - a callback that is called when parsing fails.
        - done_cb - a callback that is called when it's done parsing.
        - args - the arguments.
        - descs - argument descriptions.

    In case of errors, this function returns nil and an error message.
    You can also handle any error from the error callback of course.

    Usage string is an arbitrary string that can contain a sequence "%prog".
    This sequence is replaced with program name.

    Program name can be explicitly specified here. If it's not, it's retrieved
    from "args" as zeroth index. If that is nil, "program" is used.

    Error callback is called on errors right before this function returns. It
    returns no values. It takes the parser and an error message as arguments.

    Done callack is called on success right before this function returns. It
    takes the parser, optionala rguments and positional arguments.

    Arguments ("args") is an array with zeroth index optionally specifying
    program name. It contains strings, similarly to "argv" in other languages.

    Descriptions (descs) is an array of tables, each table being an argument
    description.

    -- RETURN VALUES --

    --- OPTIONAL ARGS ---

    The returned optional arguments is a mixed table. It contains mappings
    from argument names (without prefix) to values. The argument name here
    is in this order: alias, short name, long name. The meaning of aliases
    is described below. This also means that any given argument has one key
    only. If a value is not given (optional or doesn't take it) it's the
    boolean value "true" instead. If it is given, it's either the string
    value or whatever a callback returns (see below).

    It also contains array elements as the order of given arguments goes.
    Those array elements have this layout:

    { optn, short = shortn, long = longn, alias = aliasn, val = stringval, ... }

    "optn" refers to the same name as above (in order alias, short, long).
    "shortn" refers to the short name given in the description. "longn"
    refers to the long name given in the description. "alias" refers to
    the optional alias. "val" is the string value that was given, if given.
    This is then followed by zero or more values which are return values
    of either option callback (see below) or the string value or nothing.

    --- POSITIONAL ARGS ---

    The returned positional arguments is a simple array of strings.

    -- DESCRIPTIONS --

    The most important part of the parser is descriptions. It describes
    what kind of arguments can be given and also describes categories
    for the help listing.

    A description is represented by a table. The table has this layout:

    { shortn, longn, optional, help = helpmsg, metavar = metavar,
      alias = alias, callback = retcb, list = list, max_count = max_count
    }

    "shortn" refers to the short name. For example if you want your argument
    to be specifeable as "-x", you use "x". "longn" here refers to the long
    name. For "--help", it's "help".

    "optional" refers to whether it is required to specify value of the
    argument. The boolean value "true" means that a value is always needed.
    The value "false" means that the value can never be given.
    The value "nil" means that the value is optional.

    "help" refers to the description of the parameter in help listing.
    The field "metavar" specifies the string under which the value field
    will be displayed in help listing (see the documentation for "help").

    The field "alias" can be used to specify an alias under which the
    value/argument will be known in the returned optional arguments (i.e.
    opts[alias]). It's fully optional, see above in "optional args".

    The field "callback" can be used to specify a function that takes the
    description, the parser and the string value and returns one or more
    values. Those values will then be present in optional args. When multiple
    values are returned from such callback, the mapping opts[n] will get you
    an array of these values.

    The field "list" can be used to specify a value into which values will
    be appended. When you pass such parameter to your application multiple
    times, the list will contain all the values provided. The mapping opts[n]
    will refer to the list rather than the last value given like without list.

    The field "max_count" can be used to specify a limit on how many arguments
    can be provided. Its implicit value is 1, unless a list is provided,
    in which case it's -1 (which is a value for infinity here).

    A description can also be used to specify a category, purely for help
    listing purposes:

    { category = "catname", alias = alias }

    The alias here refers to a name by which the category can be referred
    to when printing help. Useful when your category name is long and contains
    spaces and you want a simple "--help=mycat".
]]
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
    return (sgsub(sgsub(str, "%f[%%]%%prog", progn), "%%%%prog", "%%prog"))
end

local buf_write = function(self, ...)
    local vs = { ... }
    for i, v in ipairs(vs) do self[#self + 1] = v end
end

local get_metavar = function(desc)
    local mv = desc.metavar
    if not mv and (desc[3] or desc[3] == nil) then
        mv = desc[2] and supper(desc[2]) or "VAL"
    elseif desc[3] == false then
        mv = nil
    end
    return mv
end

local help = function(parser, f, category)
    local usage = parser.usage
    local progn = parser.prog or (parser.args or arg)[0] or "program"
    if usage then
        usage = repl_prog(usage, progn)
    else
        usage = sformat("Usage: %s [OPTIONS]", progn)
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
                        ln[#ln + 1] = srep(" ", sdf)
                    end
                elseif not desc[2] and mv then
                    ln[#ln + 1] = mv
                else
                    ln[#ln + 1] = srep(" ", lls + 2)
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
                local lcat  = category   and   slower(category) or nil
                local alias = desc.alias and slower(desc.alias) or nil
                iscat = (not category) or (alias                 == lcat)
                                       or (slower(desc.category) == lcat)
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

--[[
    Given a parser, print help. The parser is the very same parser as given
    to a normal parsing function.

    Arguments:
        - parser - the parser.
        - category - optional, allows you to specify a category to print,
          in which case only the given category will print (normally all
          categories will print).
        - f - optional file stream, defaults to io.stderr.

    Keep in mind that if the second argument is given and it's not a string,
    it's considered to be the file stream (without category being specified).

    The help uses this format:

    --------------

    <USAGE STRING>

    <HEADER>

    The following options are supported:

    <CATEGORYNAME>:
      -x,             --long               Description for no argument.
      -h[?<METAVAR>], --help=[?<METAVAR>]  Description for optional argument.
      -f[<METAVAR>],  --foo=[<METAVAR>]    Description for mandatory argument.

    <ANOTHERCATEGORYNAME>:
      <MOREARGS>

    <FOOTER>

    --------------

    The usage string can be either custom (specified within the parser) or
    default, which is "Usage: <progname> [OPTIONS]" where "<progname>" is
    replaced by the program name (either specified in the parser explicitly
    or zeroth argument in the given args or "program" as a fallback).

    The header is printed only when given as part of the parser.

    The "The following options are supported:" line is only printed when there
    are options to print.

    Same goes for the footer as for the header.

    A metavar can be specified explicitly as part of the parameter description
    in the parser. If not specified, it will check whether a "long" option is
    given; if it is, it will use an uppercase version of it (for example a
    default metavar for "--help" would be "HELP"). If it's not, it will
    fallback to simply "VAL".

    Please refer to parser documentation for more information.
]]
M.help = function(parser, category, f)
    if category and type(category) ~= "string" then
        f, category = category, f
    end
    return pcall(help, parser, f or io.stderr, category)
end

-- A utility callback to parse a number
-- If a 'base' field is present in the description, uses that.
M.number_cb = function(desc, parser, v)
    local n = tonumber(v, desc.base)
    if not n then
        error("bad number value: " .. v, 0)
    end
    return n
end

-- A utility callback for geometry parsing (--foo=x:y:w:h).
M.geometry_parse_cb = function(desc, parser, v)
    local x, y, w, h = smatch(v, "^(%d+):(%d+):(%d+):(%d+)$")
    if not x then
        error("bad geometry value (X:Y:W:H expected): " .. v, 0)
    end
    return tonumber(x), tonumber(y), tonumber(w), tonumber(h)
end

-- A utility callback for size parsing (--foo=WxH).
M.size_parse_cb = function(desc, parser, v)
    local w, h = smatch(v, "^(%d+)x(%d+)$")
    if not w then
        error("bad size value (WxH expected): " .. v, 0)
    end
    return tonumber(w), tonumber(h)
end

-- A utility callback generator for help. Returns a utility callback when
-- called with file stream as an argument (optional, defaults to stderr).
-- If the second argument is true, exits the program with successful exit code.
-- For help args that take a value, the value will be used as a category name.
M.help_cb = function(fstream, exit)
    return function(desc, parser, v)
        local succ, err = M.help(parser, v, fstream)
        if not succ then
            error(err, 0)
        end
        if exit then
            os.exit(0, true) -- need 0 for lua 5.1
        end
    end
end

return M
