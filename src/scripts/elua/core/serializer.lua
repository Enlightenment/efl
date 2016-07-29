-- serialization functions for use by elua apps/modules

local M = {}

local is_array = function(t)
    local i = 0
    while t[i + 1] do i = i + 1 end
    for _ in pairs(t) do
        i = i - 1 if i < 0 then return false end
    end
    return i == 0
end

local str_escapes = setmetatable({
    ["\n"] = "\\n", ["\r"] = "\\r",
    ["\a"] = "\\a", ["\b"] = "\\b",
    ["\f"] = "\\f", ["\t"] = "\\t",
    ["\v"] = "\\v", ["\\"] = "\\\\",
    ['"' ] = '\\"', ["'" ] = "\\'"
}, {
    __index = function(self, c) return ("\\%03d"):format(c:byte()) end
})

local escape_string = function(s)
    -- a space optimization: decide which string quote to
    -- use as a delimiter (the one that needs less escaping)
    local nsq, ndq = 0, 0
    for c in s:gmatch("'") do nsq = nsq + 1 end
    for c in s:gmatch('"') do ndq = ndq + 1 end
    local sd = (ndq > nsq) and "'" or '"'
    return sd .. s:gsub("[\\"..sd.."%z\001-\031]", str_escapes) .. sd
end

local function serialize_fn(v, stream, kwargs, simp, tables, indent)
    if simp then
        v = simp(v)
    end
    local tv = type(v)
    if tv == "string" then
        stream(escape_string(v))
    elseif tv == "number" or tv == "boolean" then
        stream(tostring(v))
    elseif tv == "table" then
        local mline   = kwargs.multiline
        local indstr  = kwargs.indent
        local asstr   = kwargs.assign or "="
        local sepstr  = kwargs.table_sep or ","
        local isepstr = kwargs.item_sep
        local endsep  = kwargs.end_sep
        local optk    = kwargs.optimize_keys
        local arr = is_array(v)
        local nline   = arr and kwargs.narr_line or kwargs.nrec_line or 0
        if tables[v] then
            stream() -- let the stream know about an error
            return false,
                "circular table reference detected during serialization"
        end
        tables[v] = true
        stream("{")
        if mline then stream("\n") end
        local first = true
        local n = 0
        for k, v in (arr and ipairs or pairs)(v) do
            if first then first = false
            else
                stream(sepstr)
                if mline then
                    if n == 0 then
                        stream("\n")
                    elseif isepstr then
                        stream(isepstr)
                    end
                end
            end
            if mline and indstr and n == 0 then
                for i = 1, indent do stream(indstr) end
            end
            if arr then
                local ret, err = serialize_fn(v, stream, kwargs, simp, tables,
                    indent + 1)
                if not ret then return ret, err end
            else
                if optk and type(k) == "string"
                and k:match("^[%a_][%w_]*$") then
                    stream(k)
                else
                    stream("[")
                    local ret, err = serialize_fn(k, stream, kwargs, simp,
                        tables, indent + 1)
                    if not ret then return ret, err end
                    stream("]")
                end
                stream(asstr)
                local ret, err = serialize_fn(v, stream, kwargs, simp, tables,
                    indent + 1)
                if not ret then return ret, err end
            end
            n = (n + 1) % nline
        end
        if not first then
            if endsep then stream(sepstr) end
            if mline then stream("\n") end
        end
        if mline and indstr then
            for i = 2, indent do stream(indstr) end
        end
        stream("}")
    else
        stream()
        return false, ("invalid value type: " .. tv)
    end
    return true
end

local defkw = {
    multiline = false, indent = nil, assign = "=", table_sep = ",",
    end_sep = false, optimize_keys = true
}

local defkwp = {
    multiline = true, indent = "    ", assign = " = ", table_sep = ",",
    item_sep = " ", narr_line = 4, nrec_line = 2, end_sep = false,
    optimize_keys = true
}

--[[
    Serializes the given table, returning a string containing a literal
    representation of the table. It tries to be compact by default so it
    avoids whitespace and newlines. Arrays and associative arrays are
    serialized differently (for compact output).

    Besides tables this can also serialize other Lua values. It serializes
    them in the same way as values inside a table, returning their literal
    representation (if serializable, otherwise just their tostring). The
    serializer allows strings, numbers, booleans and tables.

    Circular tables can't be serialized. The function normally returns either
    the string output or nil + an error message (which can signalize either
    circular references or invalid types).

    The function allows you to pass in a "kwargs" table as the second argument.
    It's a table of options. Those can be multiline (boolean, false by default,
    pretty much pretty-printing), indent (string, nil by default, specifies
    how an indent level looks), assign (string, "=" by default, specifies how
    an assignment between a key and a value looks), table_sep (table separator,
    by default ",", can also be ";" for tables, separates items in all cases),
    item_sep (item separator, string, nil by default, comes after table_sep
    but only if it isn't followed by a newline), narr_line (number, 0 by
    default, how many array elements to fit on a line), nrec_line (same,
    just for key-value pairs), end_sep (boolean, false by default, makes
    the serializer put table_sep after every item including the last one),
    optimize_keys (boolean, true by default, optimizes string keys like
    that it doesn't use string literals for keys that can be expressed
    as Lua names).

    If kwargs is nil or false, the values above are used. If kwargs is a
    boolean value true, pretty-printing defaults are used (multiline is
    true, indent is 4 spaces, assign is " = ", table_sep is ",", item_sep
    is one space, narr_line is 4, nrec_line is 2, end_sep is false,
    optimize_keys is true).

    A third argument, "stream" can be passed. As a table is serialized
    by pieces, "stream" is called each time a new piece is saved. It's
    useful for example for file I/O. When a custom stream is supplied,
    the function doesn't return a string, instead it returns true
    or false depending on whether it succeeded and the error message
    if any.

    And finally there is the fourth argument, "simplifier". It's a
    function that takes a value and "simplifies" it (returns another
    value it should be replaced by). By default nothing is simplified
    of course.

    This function is externally available as "table_serialize".
]]
M.serialize = function(val, kwargs, stream, simplifier)
    if kwargs == true then
        kwargs = defkwp
    elseif not kwargs then
        kwargs = defkw
    else
        if  kwargs.optimize_keys == nil then
            kwargs.optimize_keys = true
        end
    end
    if stream then
        return serialize_fn(val, stream, kwargs, simplifier, {}, 1)
    else
        local t = {}
        local ret, err = serialize_fn(val, function(out)
            t[#t + 1] = out end, kwargs, simplifier, {}, 1)
        if not ret then
            return nil, err
        else
            return table.concat(t)
        end
    end
end

local lex_get = function(ls)
    while true do
        local c = ls.curr
        if not c then break end
        ls.tname, ls.tval = nil, nil
        if c == "\n" or c == "\r" then
            local prev = c
            c = ls.rdr()
            if (c == "\n" or c == "\r") and c ~= prev then
                c = ls.rdr()
            end
            ls.curr = c
            ls.linenum = ls.linenum + 1
        elseif c == " " or c == "\t" or c == "\f" or c == "\v" then
            ls.curr = ls.rdr()
        elseif c == "." or c:byte() >= 48 and c:byte() <= 57 then
            local buf = { ls.curr }
            ls.curr = ls.rdr()
            while ls.curr and ls.curr:match("[epxEPX0-9.+-]") do
                buf[#buf + 1] = ls.curr
                ls.curr = ls.rdr()
            end
            local str = table.concat(buf)
            local num = tonumber(str)
            if not num then error(("%d: malformed number near '%s'")
                :format(ls.linenum, str), 0) end
            ls.tname, ls.tval = "<number>", num
            return "<number>"
        elseif c == '"' or c == "'" then
            local d = ls.curr
            ls.curr = ls.rdr()
            local buf = {}
            while ls.curr ~= d do
                local c = ls.curr
                if c == nil then
                    error(("%d: unfinished string near '<eos>'")
                        :format(ls.linenum), 0)
                elseif c == "\n" or c == "\r" then
                    error(("%d: unfinished string near '<string>'")
                        :format(ls.linenum), 0)
                -- not complete escape sequence handling: handles only these
                -- that are or can be in the serialized output
                elseif c == "\\" then
                    c = ls.rdr()
                    if c == "a" then
                        buf[#buf + 1] = "\a" ls.curr = ls.rdr()
                    elseif c == "b" then
                        buf[#buf + 1] = "\b" ls.curr = ls.rdr()
                    elseif c == "f" then
                        buf[#buf + 1] = "\f" ls.curr = ls.rdr()
                    elseif c == "n" then
                        buf[#buf + 1] = "\n" ls.curr = ls.rdr()
                    elseif c == "r" then
                        buf[#buf + 1] = "\r" ls.curr = ls.rdr()
                    elseif c == "t" then
                        buf[#buf + 1] = "\t" ls.curr = ls.rdr()
                    elseif c == "v" then
                        buf[#buf + 1] = "\v" ls.curr = ls.rdr()
                    elseif c == "\\" or c == '"' or c == "'" then
                        buf[#buf + 1] = c
                        ls.curr = ls.rdr()
                    elseif not c then
                        error(("%d: unfinished string near '<eos>'")
                            :format(ls.linenum), 0)
                    else
                        if not c:match("%d") then
                            error(("%d: invalid escape sequence")
                                :format(ls.linenum), 0)
                        end
                        local dbuf = { c }
                        c = ls.rdr()
                        if c:match("%d") then
                            dbuf[2] = c
                            c = ls.rdr()
                            if c:match("%d") then
                                dbuf[3] = c
                                c = ls.rdr()
                            end
                        end
                        ls.curr = c
                        buf[#buf + 1] = table.concat(dbuf):char()
                    end
                else
                    buf[#buf + 1] = c
                    ls.curr = ls.rdr()
                end
            end
            ls.curr = ls.rdr() -- skip delim
            ls.tname, ls.tval = "<string>", table.concat(buf)
            return "<string>"
        elseif c:match("[%a_]") then
            local buf = { c }
            ls.curr = ls.rdr()
            while ls.curr and ls.curr:match("[%w_]") do
                buf[#buf + 1] = ls.curr
                ls.curr = ls.rdr()
            end
            local str = table.concat(buf)
            if str == "true" or str == "false" or str == "nil" then
                ls.tname, ls.tval = str, nil
                return str
            else
                ls.tname, ls.tval = "<name>", str
                return "<name>"
            end
        else
            ls.curr = ls.rdr()
            ls.tname, ls.tval = c, nil
            return c
        end
    end
end

local function assert_tok(ls, tok, ...)
    if not tok then return nil end
    if ls.tname ~= tok then
        error(("%d: unexpected symbol near '%s'"):format(ls.linenum,
            ls.tname), 0)
    end
    lex_get(ls)
    assert_tok(ls, ...)
end

local function parse(ls)
    local tok = ls.tname
    if tok == "<string>" or tok == "<number>" then
        local v = ls.tval
        lex_get(ls)
        return v
    elseif tok == "true"  then lex_get(ls) return true
    elseif tok == "false" then lex_get(ls) return false
    elseif tok == "nil"   then lex_get(ls) return nil
    else
        assert_tok(ls, "{")
        local tbl = {}
        if ls.tname == "}" then
            lex_get(ls)
            return tbl
        end
        repeat
            if ls.tname == "<name>" then
                local key = ls.tval
                lex_get(ls)
                assert_tok(ls, "=")
                tbl[key] = parse(ls)
            elseif ls.tname == "[" then
                lex_get(ls)
                local key = parse(ls)
                assert_tok(ls, "]", "=")
                tbl[key] = parse(ls)
            else
                tbl[#tbl + 1] = parse(ls)
            end
        until (ls.tname ~= "," and ls.tname ~= ";") or not lex_get(ls)
        assert_tok(ls, "}")
        return tbl
    end
end

--[[
    Takes a previously serialized table and converts it back to the original.
    Uses a simple tokenizer and a recursive descent parser to build the result,
    so it's safe (doesn't evaluate anything). The input can also be a callable
    value that return the next character each call.
    External as "table_deserialize". This returns the deserialized value on
    success and nil + the error message on failure.
]]
M.deserialize = function(s)
    local stream = (type(s) == "string") and s:gmatch(".") or s
    local ls = { curr = stream(), rdr = stream, linenum = 1 }
    local r, v = pcall(lex_get, ls)
    if not r then return nil, v end
    r, v = pcall(parse, ls)
    if not r then return nil, v end
    return v
end

return M
