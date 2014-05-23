-- Elua xgettext: lexer

local yield = coroutine.yield
local tconc = table.concat

local source_to_msg = function(source)
    local c = source:sub(1, 1)
    local srclen = #source
    if c == "@" then
        if srclen <= (max_fname_len + 1) then
            return source:sub(2)
        else
            return "..." .. source:sub(srclen - max_fname_len + 1)
        end
    elseif c == "=" then
        return source:sub(2, max_custom_len + 1)
    else
        return '[string "' .. source:sub(1, max_str_len)
            .. ((srclen > max_str_len) and '..."]' or '"]')
    end
end

local lex_error = function(ls, msg, tok)
    msg = ("%s:%d: %s"):format(source_to_msg(ls.source), ls.line_number, msg)
    if tok then
        msg = msg .. " near '" .. tok .. "'"
    end
    error(msg, 0)
end

local syntax_error = function(ls, msg)
    lex_error(ls, msg, ls.token.value or ls.token.name)
end

local next_char = function(ls)
    local c = ls.reader()
    ls.current = c
    return c
end

local next_line = function(ls, cs)
    local old = ls.current
    assert(old == "\n" or old == "\r")
    local c = next_char(ls)
    if (c == "\n" or c == "\r") and c ~= old then
        c = next_char(ls)
    end
    ls.line_number = ls.line_number + 1
    return c
end

local read_number = function(ls, beg)
    local buf = {}
    if beg then buf[1] = beg end
    local c = ls.current
    while c == "." or c:match("%d") do
        buf[#buf + 1] = c
        c = next_char(ls)
    end
    if c == "e" or c == "E" then
        buf[#buf + 1] = c
        c = next_char(ls)
        if c == "+" or c == "-" then
            buf[#buf + 1] = c
            c = next_char(ls)
        end
    end
    while c:match("%w") do
        buf[#buf + 1] = c
        c = next_char(ls)
    end
    local str = tconc(buf)
    if not tonumber(str) then
        lex_error(ls, "malformed number", str)
    end
    return str
end

local skip_sep = function(ls, buf)
    local cnt = 0
    local s = ls.current
    assert(s == "[" or s == "]")
    buf[#buf + 1] = s
    local c = next_char(ls)
    while c == "=" do
        buf[#buf + 1] = c
        c = next_char(ls)
        cnt = cnt + 1
    end
    return c == s and cnt or ((-cnt) - 1)
end

local read_long_string = function(ls, sep, cmt)
    local buf = {}
    local c = next_char(ls)
    if c == "\n" or c == "\r" then c = next_line(ls) end
    while true do
        if not c then
            lex_error(ls, tok and cmt and "unfinished long comment"
                or "unfinished long string", "<eof>")
        elseif c == "]" then
            local tbuf = {}
            if skip_sep(ls, tbuf) == sep then
                c = next_char(ls)
                break
            else
                buf[#buf + 1] = tconc(tbuf)
            end
            c = ls.current
        else
            buf[#buf + 1] = c
            c = next_char(ls)
        end
    end
    return tconc(buf)
end

local read_string = function(ls)
    local delim = ls.current
    local buf = { delim }
    local c = next_char(ls)
    while c ~= delim do
        if not c then lex_error(ls, "unfinished string", "<eof>")
        elseif c == "\n" or c == "\r" then
            lex_error(ls, "unfinished string", tconc(buf))
        elseif c == "\\" then
            buf[#buf + 1] = c
            buf[#buf + 1] = next_char(ls)
            c = next_char(ls)
        else
            buf[#buf + 1] = c
            c = next_char(ls)
        end
    end
    buf[#buf + 1] = c
    next_char(ls)
    return tconc(buf)
end

local lex_tbl = {
    ["\n"] = function(ls) next_line(ls) end,
    [" " ] = function(ls) next_char(ls) end,
    ["-" ] = function(ls)
        local c = next_char(ls)
        if c ~= "-" then
            return "-"
        end
        c = next_char(ls)
        if c == "[" then
            local sep = skip_sep(ls, {})
            if sep >= 0 then
                return "<comment>", read_long_string(ls, sep, true)
            end
        end
        local buf = {}
        while ls.current and ls.current ~= "\n" and ls.current ~= "\r" do
            buf[#buf + 1] = ls.current
            next_char(ls)
        end
        return "<comment>", tconc(buf)
    end,
    ["[" ] = function(ls)
        local buf = {}
        local sep = skip_sep(ls, {})
        if sep >= 0 then
            return "<string>", read_long_string(ls, sep)
        elseif sep == -1 then
            return "["
        else
            lex_error(ls, "invalid long string delimiter", tconc(buf))
        end
    end,
    ['"' ] = function(ls)
        return "<string>", read_string(ls)
    end,
    ["." ] = function(ls)
        local c = next_char(ls)
        if c:match("%d") then
            return "<number>", read_number(ls, ".")
        elseif c ~= "." then
            return "."
        end
        next_char(ls)
        return ".."
    end,
    ["0" ] = function(ls)
        return "<number>", read_number(ls)
    end
}
lex_tbl["\r"] = lex_tbl["\n"]
lex_tbl["\f"] = lex_tbl[" " ]
lex_tbl["\t"] = lex_tbl[" " ]
lex_tbl["\v"] = lex_tbl[" " ]
lex_tbl["'" ] = lex_tbl['"' ]
lex_tbl["1" ] = lex_tbl["0" ]
lex_tbl["2" ] = lex_tbl["0" ]
lex_tbl["3" ] = lex_tbl["0" ]
lex_tbl["4" ] = lex_tbl["0" ]
lex_tbl["5" ] = lex_tbl["0" ]
lex_tbl["6" ] = lex_tbl["0" ]
lex_tbl["7" ] = lex_tbl["0" ]
lex_tbl["8" ] = lex_tbl["0" ]
lex_tbl["9" ] = lex_tbl["0" ]

local lex_default = function(ls)
    local c = ls.current
    if c == "_" or c:match("%a") then
        local buf = {}
        repeat
            buf[#buf + 1] = c
            c = next_char(ls)
            if not c then break end
        until not (c == "_" or c:match("%w"))
        local str = tconc(buf)
        return "<name>", str
    else
        next_char(ls)
        return c
    end
end

local lex_main = function(ls)
    yield()
    while true do
        local c = ls.current
        if c == nil then
            return "<eof>"
        end
        local opt = lex_tbl[c]
        if opt then
            local t, v = opt(ls)
            if t then yield(t, v) end
        else
            yield(lex_default(ls))
        end
    end
end

local strstream = function(str)
    return str:gmatch(".")
end

local skip_bom = function(rdr)
    local c = rdr()
    if c ~= 0xEF then return c end
    c = rdr()
    if c ~= 0xBB then return c end
    c = rdr()
    if c ~= 0xBF then return c end
    return rdr()
end

local skip_shebang = function(rdr)
    local c = skip_bom(rdr)
    if c == 35 then -- #
        repeat
            c = rdr()
        until not c or is_newline(c)
        local e = c
        c = rdr()
        if (e == 10 and c == 13) or (e == 13 and c == 10) then -- LF, CR
            c = rdr()
        end
    end
    return c
end

local ls_get = function(self)
    local tok = self.token
    tok.name, tok.value = self.coro()
    return tok
end

return { init = function(chunkname, input)
    local reader = type(input) == "string" and strstream(input) or input
    local current = skip_shebang(reader)
    local ls = {
        reader      = reader,
        token       = {},
        source      = chunkname,
        current     = current,
        line_number = 1,
        get         = ls_get
    }
    local coro = coroutine.wrap(lex_main, ls)
    ls.coro = coro
    coro(ls)
    return coro
end, syntax_error = syntax_error }