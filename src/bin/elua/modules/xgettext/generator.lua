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
        if msg[2] then
            ret[#ret + 1] = "msgid_plural " .. gen_message(msg[2])
            ret[#ret + 1] = "msgstr[0] \"\"\nmsgstr[1] \"\""
        else
            ret[#ret + 1] = "msgstr \"\""
        end
        rets[#rets + 1] = table.concat(ret, "\n")
        msg             = ps()
    end
    return table.concat(rets, "\n\n")
end }