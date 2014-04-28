-- elua gettext module

local ffi = require("ffi")

local M = {}

local gettext = ...

local bind_textdomain = gettext.bind_textdomain
local dgettext        = gettext.dgettext

if  dgettext then
    dgettext = ffi.cast("char *(*)(const char*, const char*)", dgettext)
end

local domains = {}

M.register_domain = function(dom, dir)
    local d, err = bind_textdomain(dom, dir)
    if not d then
        return false, err
    end
    domains[dom] = d
    return true
end

M.get_domain = function(dom)
    return domains[dom]
end

local cast, ffistr = ffi.cast, ffi.string

if dgettext then
    M.gettext = function(dom, msgid)
        if not domains[dom] or not msgid then return msgid end
        local cmsgid = cast("const char*", msgid)
        local lmsgid = dgettext(dom, cmsgid)
        if cmsgid == lmsgid then
            return msgid
        end
        return ffistr(lmsgid)
    end
else
    M.gettext = function(dom, msgid) return msgid end
end

return M