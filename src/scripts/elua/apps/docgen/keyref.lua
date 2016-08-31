local M = {}

local key_refs = {}

M.add = function(key, link, lang)
    local rfs = key_refs[lang]
    if not rfs then
        key_refs[lang] = {}
        rfs = key_refs[lang]
    end
    rfs[key] = link
end

M.build = function()
    local writer = require("docgen.writer")
    local dutil = require("docgen.util")
    for lang, rfs in pairs(key_refs) do
        local f = writer.Writer({ "ref", lang, "keyword-list" })
        local arr = {}
        for refn, v in pairs(rfs) do
            arr[#arr + 1] = refn
            local rf = writer.Writer({ "ref", lang, "key", refn })
            v[#v + 1] = true
            rf:write_include(rf.INCLUDE_PAGE, v)
        end
        table.sort(arr)
        f:write_raw(table.concat(arr, "\n"))
        f:write_nl()
        f:finish()
        local lf = writer.Writer({ "ref", lang, "keyword-link" })
        lf:write_raw("/", dutil.path_join(
            dutil.nspace_to_path(dutil.get_root_ns()), "auto",
            "ref", lang, "key", "{FNAME}"
        ))
        lf:write_nl()
        lf:finish()
    end
end

return M
