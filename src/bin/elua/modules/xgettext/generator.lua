-- Elua xgettext: generator

local parser = require("xgettext.parser")

local gen_comment = function(cmt)
    local cmtret = {}
    for line in cmt:gmatch("([^\n]+)") do
        cmtret[#cmtret + 1] = "#. " .. line:match("^%s*(.+)$")
    end
    return table.concat(cmtret, "\n")
end

local gen_message = function(str)
    local mline = not not str:find("\n")
    if not mline then
        return '"' .. str .. '"'
    end
    local ret = { '""' }
    for line in cmt:gmatch("([^\n]+)") do
        ret[#ret + 1] = '"' .. line .. '\\n"'
    end
    return table.concat(ret, "\n")
end

local gen_msgstr = function(str, prefix, suffix)
    if not prefix and not suffix then
        return '""'
    end
    return gen_message(table.concat(prefix
        and { prefix, str, suffix } or { str, suffix }))
end

return { init  = function(chunkname, input, keywords, flags, opts)
    local ps   = parser.init(chunkname, input, keywords, flags, opts)
    local msg  = ps()
    local rets = {}
    while msg do
        local ret = {}
        if msg.comment then
            ret[#ret + 1] = gen_comment(msg.comment)
        end
        if msg.xcomment then
            ret[#ret + 1] = gen_comment(msg.xcomment)
        end
        if msg.context then
            ret[#ret + 1] = "msgctxt " .. gen_message(msg.context)
        end
        ret[#ret + 1] = "msgid " .. gen_message(msg[1])
        local spf, ssf = opts["m"], opts["M"]
        if msg[2] then
            ret[#ret + 1] = "msgid_plural " .. gen_message(msg[2])
            ret[#ret + 1] = "msgstr[0] " .. gen_msgstr(msg[1], spf, ssf)
            ret[#ret + 1] = "msgstr[1] " .. gen_msgstr(msg[2], spf, ssf)
        else
            ret[#ret + 1] = "msgstr " .. gen_msgstr(msg[1], spf, ssf)
        end
        rets[#rets + 1] = table.concat(ret, "\n")
        msg             = ps()
    end
    return table.concat(rets, "\n\n")
end }