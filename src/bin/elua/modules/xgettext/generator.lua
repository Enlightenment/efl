-- Elua xgettext: generator

local parser = require("xgettext.parser")

return { init = function(chunkname, input, keywords)
    local ps = parser.init(chunkname, input, keywords)
end }