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
    local writer = require("docgen.writer")
    local dutil = require("docgen.util")
    for lang, rfs in pairs(key_refs) do
        local f = writer.Writer({ "ref", lang, "keyword-list" })
        local arr = {}
        for refn, v in pairs(rfs) do
            arr[#arr + 1] = refn
        end
        table.sort(arr)
        f:write_raw(table.concat(arr, "\n"))
        f:write_nl()
        f:finish()
        local lf = writer.Writer({ "ref", lang, "keyword-link" })
        lf:write_raw("/", dutil.path_join(
            dutil.get_root(), "ref", lang, "key", "{FNAME}"
        ))
        lf:write_nl()
        lf:finish()
    end
end

return M
