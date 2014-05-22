-- Elua xgettext: parser

local lexer = require("xgettext.lexer")

local yield = coroutine.yield

local saved_comments = {}

local parse = function(ls, keywords)
    yield()
    local tok = ls.token
    while tok.name do
        if tok.name == "<comment>" then
            saved_comments[#saved_comments + 1] = tok.value
            ls:get()
        elseif tok.name == "<name>" and keywords[tok.value] then
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