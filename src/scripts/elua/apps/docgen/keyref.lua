local M = {}

local key_refs = {}

M.add = function(key, lang)
    local rfs = key_refs[lang]
    if not rfs then
        key_refs[lang] = {}
        rfs = key_refs[lang]
    end
    rfs[key] = true
end

M.build = function()
    for lang, rfs in pairs(key_refs) do
        local f = require("docgen.writer").Writer({ "ref", lang, "keyword-list" })
        local arr = {}
        for refn, v in pairs(rfs) do
            arr[#arr + 1] = refn
        end
        table.sort(arr)
        f:write_raw(table.concat(arr, "\n"))
        f:write_nl()
        f:finish()
    end
end

return M
