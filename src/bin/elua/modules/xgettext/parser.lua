-- Elua xgettext: parser

local lexer = require("xgettext.lexer")

local syntax_error = lexer.syntax_error

local yield = coroutine.yield

local saved_comment

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
end

local parse_call = function(ls)
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

local parse = function(ls, keywords)
    yield()
    local tok = ls.token
    while tok.name do
        if tok.name == "<comment>" then
            saved_comment = tok.value
            ls:get()
        elseif tok.name == "<name>" and keywords[tok.value] then
            local line = ls.line_number
            local kw   = keywords[tok.value]
            ls:get()
            local  args = parse_call(ls)
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
            local sc = saved_comment
            saved_comment = nil
            yield {
                n1arg[1], n2 and n2arg[1], context = cx and cxarg[1],
                xcomment = kw.xcomment, comment = sc, line = line
            }
        else
            ls:get()
        end
        ::skip::
    end
end

local parse_all = function(ls)
    yield()
    local tok = ls.token
    while tok.name do
        if tok.name == "<comment>" then
            saved_comment = tok.value
            ls:get()
        elseif tok.name == "<string>" then
            local line    = ls.line_number
            local val     = tok.value
            local sc      = saved_comment
            saved_comment = nil
            ls:get()
            yield { val, comment = sc, line = line }
        else
            ls:get()
        end
    end
end

return { init = function (chunkname, input, keywords, flags, opts)
    local ls = lexer.init(chunkname, input, opts)
    ls:get()
    local coro = coroutine.wrap(opts["a"] and parse_all or parse, ls, keywords)
    coro(ls, keywords)
    return coro
end }