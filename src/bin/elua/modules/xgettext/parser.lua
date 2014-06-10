-- Elua xgettext: parser

local util = require("util")
local lexer = require("xgettext.lexer")

local syntax_error = lexer.syntax_error

local yield = coroutine.yield
local tconc = table.concat

local String = util.Object:clone {
    __ctor = function(self, sing, plu, ctx, cmt, xcmt, flags, line)
        self.singular = sing
        self.plural   = plu
        self.context  = ctx
        self.comment  = cmt
        self.xcomment = xcmt
        self.flags    = flags
        self.line     = line
    end,

    guess_flags = function(self, flags)
    end,

    gen_flags = function(self)
        local flags = {}
        for i, v in ipairs(self.flags) do flags[i] = v end
        if self.parent then
            self.parent:add_flags(self, flags)
        end
        self:guess_flags(flags)
        return flags
    end,

    generate = function(self)
        yield {
            self.singular, self.plural, context = self.context,
            comment = self.xcomment, comment = self.comment, line = self.line,
            flags = self:gen_flags()
        }
    end
}

local Call = util.Object:clone {
    __ctor = function(self, flags, args)
        self.flags = flags
        self.args  = args
        for i, v in ipairs(args) do
            v.parent = self
        end
    end,

    add_flags = function(self, argo, flags, flagstr)
        local argn
        for i, a in ipairs(self.args) do
            if a == argo then
                argn = i
                break
            end
        end
        for i, flag in ipairs(self.flags) do
            if flag[1] == argn then
                local pass = flag[2]:match("^pass%-(.+)$")
                if not flagstr or flagstr == pass or flagstr == flag[2] then
                    if pass then
                        self.parent:add_flags(self, flags, pass)
                    else
                        flags[#flags + 1] = flag[2]
                    end
                end
            end
        end
    end,

    generate = function(self)
        for i, v in ipairs(self.args) do
            v:generate()
        end
    end
}

local saved_flagcomments = {}
local saved_comments     = {}

local check_match = function(ls, a, b, line)
    if ls.token.name ~= a then
        if line == ls.line_number then
            syntax_error(ls, "'" .. a .. "' expected")
        else
            syntax_error(ls, "'" .. a .. "' expected (to close '" .. b
                .. "' at line " .. line .. ")")
        end
    end
end

local parse_simple_expr = function(ls)
    local tok = ls.token
    local tn  = tok.name
    if tn == "(" then
        local line = ls.line_number
        ls:get()
        local v, tn = parse_expr(ls)
        check_match(ls, ")", "(", line)
        ls:get()
        return v, tn
    elseif tn == "<string>" or tn == "<number>" or tn == "<name>" then
        local v = tok.value
        ls:get()
        return v, tn
    else
        syntax_error(ls, "unexpected symbol")
    end
end

local parse_expr
parse_expr = function(ls)
    local tok     = ls.token
    local line    = ls.line_number
    local lhs, tn = parse_simple_expr(ls)
    while true do
        if tok.name ~= ".." then break end
        if tn ~= "<string>" and tn ~= "<number>" then
            syntax_error(ls, "invalid lhs for concat")
        end
        tn = "<string>"
        ls:get()
        local rhs, rtn = parse_expr(ls)
        if rtn ~= "<string>" and rtn ~= "<number>" then
            syntax_error(ls, "invalid rhs for concat")
        end
        lhs = lhs .. rhs
    end
    return lhs, tn
end

local parse_arglist = function(ls)
    local tok  = ls.token
    local rets = {}
    while true do
        rets[#rets + 1] = { parse_expr(ls) }
        if tok.name == "," then
            ls:get()
        else
            break
        end
    end
    return rets
end

local parse_kwcall = function(ls)
    local tok = ls.token
    if tok.name == "(" then
        local line = ls.line_number
        ls:get()
        if tok.name == ")" then
            ls:get()
            return {}
        end
        local al = parse_arglist(ls)
        check_match(ls, ")", "(", line)
        ls:get()
        return al
    elseif tok.name == "<string>" then
        local v = tok.value
        ls:get()
        return { { v, "<string>" } }
    else
        return nil
    end
end

local parse_kw = function(ls, keywords)
    local tok  = ls.token
    local line = ls.line_number
    local kw   = keywords[tok.value]
    ls:get()
    local args = parse_kwcall(ls)
    local n1, n2, cx, an = kw[1], kw[2], kw.context, kw.argnum
    local n1arg, n2arg, cxarg = args[n1], args[n2], args[cx]
    local n1argt, n2argt, cxargt = n1arg and (n1arg[2] ~= "<name>"),
                                   n2arg and (n2arg[2] ~= "<name>"),
                                   cxarg and (cxarg[2] ~= "<name>")
    if not args           then goto skip end
    if an and #args ~= an then goto skip end
    if        #args  < n1 then goto skip end
    if n2 and #args  < n2 then goto skip end
    if cx and #args  < cx then goto skip end
    if        not n1argt  then goto skip end
    if n2 and not n2argt  then goto skip end
    if cx and not cxargt  then goto skip end
    local sc = saved_comments
    saved_comments = {}
    sc = tconc(sc, "\n")
    local fsc = saved_flagcomments
    saved_flagcomments = {}
    return String(n1arg[1], n2 and n2arg[1] or nil, cx and cxarg[1] or nil,
        sc, kw.xcomment, fsc, line)
end

local parse_fg = function(ls, flags, keywords)
    error("NYI")
end

local parse = function(ls, keywords, flags)
    yield()
    local tok = ls.token
    while tok.name ~= "<eof>" do
        if tok.name == "<comment>" then
            saved_comments[#saved_comments + 1] = tok.value
            ls:get()
        elseif tok.name == "<flagcomment>" then
            saved_flagcomments[#saved_flagcomments + 1] = tok.value
            ls:get()
        elseif tok.name == "<name>" then
            if keywords[tok.value] then
                local status, str = pcall(parse_kw, keywords)
                if status then
                    str:generate()
                end
            elseif flags[tok.value] then
                local status, call = pcall(parse_fg, flags, keywords)
                if status then
                    call:generate()
                end
            else
                ls:get()
            end
        else
            ls:get()
        end
        ::skip::
    end
end

local parse_all = function(ls)
    yield()
    local tok = ls.token
    while tok.name ~= "<eof>" do
        if tok.name == "<comment>" then
            saved_comments[#saved_comments + 1] = tok.value
            ls:get()
        elseif tok.name == "<flagcomment>" then
            saved_flagcomments[#saved_flagcomments + 1] = tok.value
            ls:get()
        elseif tok.name == "<string>" then
            local line = ls.line_number
            local val = tok.value
            local sc = saved_comments
            saved_comments = {}
            sc = tconc(sc, "\n")
            local fsc = saved_flagcomments
            saved_flagcomments = {}
            ls:get()
            String(val, nil, nil, sc, nil, fsc, line):generate()
        else
            ls:get()
        end
    end
end

return { init = function (chunkname, input, keywords, flags, opts)
    local ls = lexer.init(chunkname, input, flags, opts)
    ls:get()
    local coro = coroutine.wrap(opts["a"] and parse_all or parse, ls,
        keywords, flags)
    coro(ls, keywords)
    return coro
end }
