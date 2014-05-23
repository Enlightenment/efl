-- Elua xgettext: parser

local lexer = require("xgettext.lexer")

local syntax_error = lexer.syntax_error

local yield = coroutine.yield

local saved_comments = {}

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

local parse_arg = function(ls)
    local plevel = 0
end

local parse_arglist = function(ls)
    local tok  = ls.token
    local rets = {}
    while true do
        rets[#rets + 1] = parse_arg(ls)
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
        return { v }
    else
        return nil
    end
end

local parse = function(ls, keywords)
    yield()
    local tok = ls.token
    while tok.name do
        if tok.name == "<comment>" then
            saved_comments[#saved_comments + 1] = tok.value
            ls:get()
        elseif tok.name == "<name>" and keywords[tok.value] then
            local kw = keywords[tok.value]
            ls:get()

            local cmt = saved_comments[#saved_comments]
            saved_comments = {}
        else
            ls:get()
        end
    end
end

return { init = function (chunkname, input, keywords)
    local ls = lexer.init(chunkname, input)
    ls:get()
end }