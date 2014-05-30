-- Elua xgettext: generator

local lexer = require("xgettext.lexer")
local parser = require("xgettext.parser")

local tconc = table.concat

local gen_comment = function(cmt)
    local cmtret = {}
    for line in cmt:gmatch("([^\n]+)") do
        cmtret[#cmtret + 1] = "#. " .. line:match("^%s*(.+)$")
    end
    return tconc(cmtret, "\n")
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
    return tconc(ret, "\n")
end

local gen_msgstr = function(str, prefix, suffix)
    if not prefix and not suffix then
        return '""'
    end
    return gen_message(tconc(prefix
        and { prefix, str, suffix } or { str, suffix }))
end

local cmp_msgs = function(msg1, msg2)
    return  msg1[1]       == msg2[1]      and msg1[2]      == msg2[2]
        and msg1.context  == msg2.context and msg1.comment == msg2.comment
        and msg1.xcomment == msg2.xcomment
end

local new_msg = function(msg)
    return {
        msg[1], msg[2], msg.context, msg.comment, msg.xcomment, lines = {
            msg.line
        }
    }
end

local gen_grouped_messages = function(ps)
    local msg = ps()
    local ret = { new_msg(msg) }
    msg = ps()
    while msg do
        local found = false
        for i, amsg in ipairs(ret) do
            if cmp_msgs(msg, amsg) then
                amsg.lines[#amsg.lines + 1] = msg.line
                found = true
                break
            end
        end
        if not found then
            ret[#ret + 1] = new_msg(msg)
        end
        msg = ps()
    end
end

local gen_line_info = function(chunkname, lines)
    local cname = lexer.source_to_msg(chunkname)
    local linestrs = {}
    local linestr  = "#:"
    local i = 1
    while i <= #lines do
        local tmps = linestr .. tconc { " ", cname, ":", lines[i] }
        if #tmps > 80 then
            linestrs[#linestrs + 1] = linestr
            linestr = "#:"
        else
            linestr = tmps
            i = i + 1
        end
    end
    linestrs[#linestrs + 1] = linestr
    return tconc(linestrs, "\n")
end

return { init  = function(chunkname, input, keywords, flags, opts)
    local rets = {}
    for i, msg in ipairs(gen_grouped_messages(parser.init(chunkname,
    input, keywords, flags, opts))) do
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
        ret[#ret + 1] = gen_line_info(chunkname, msg.lines)
        ret[#ret + 1] = "msgid " .. gen_message(msg[1])
        local spf, ssf = opts["m"], opts["M"]
        if msg[2] then
            ret[#ret + 1] = "msgid_plural " .. gen_message(msg[2])
            ret[#ret + 1] = "msgstr[0] " .. gen_msgstr(msg[1], spf, ssf)
            ret[#ret + 1] = "msgstr[1] " .. gen_msgstr(msg[2], spf, ssf)
        else
            ret[#ret + 1] = "msgstr " .. gen_msgstr(msg[1], spf, ssf)
        end
        rets[#rets + 1] = tconc(ret, "\n")
        msg             = ps()
    end
    return tconc(rets, "\n\n")
end }