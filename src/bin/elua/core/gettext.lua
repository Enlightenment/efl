-- elua gettext module

local ffi = require("ffi")

local M = {}

local gettext = ...

local bind_textdomain = gettext.bind_textdomain
local dgettext        = gettext.dgettext
local dngettext       = gettext.dngettext

if  dgettext then
    dgettext  = ffi.cast("char *(*)(const char*, const char*)", dgettext)
    dngettext = ffi.cast("char *(*)(const char*, const char*, const char*, "
        .. "unsigned long)", dngettext)
end

local domains = {}

local default_domain

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

M.set_default_domain = function(dom)
    if not domains[dom] then return false end
    default_domain = dom
    return true
end

local cast, ffistr = ffi.cast, ffi.string
local floor = math.floor
local type = type

if dgettext then
    M.gettext = function(dom, msgid)
        if not msgid then
            msgid = dom
            dom   = default_domain
        end
        if not domains[dom] or not msgid then return msgid end
        local cmsgid = cast("const char*", msgid)
        local lmsgid = dgettext(dom, cmsgid)
        if cmsgid == lmsgid then
            return msgid
        end
        return ffistr(lmsgid)
    end
    M.ngettext = function(dom, msgid, plmsgid, n)
        if not n then
            plmsgid = msgid
            msgid   = dom
            dom     = default_domain
        end
        n = (type(n) == "number") and floor(n) or 0
        if not domains[dom] then
            if not msgid or n == 1 then return msgid end
            return plmsgid
        end
        local cmsgid   = cast("const char*", msgid)
        local cplmsgid = cast("const char*", plmsgid)
        local lmsgid   = dngettext(dom, cmsgid, cplmsgid, n)
        if n == 1 then
            if cmsgid == lmsgid then return msgid end
        else
            if cplmsgid == lmsgid then return plmsgid end
        end
        return ffistr(lmsgid)
    end
else
    M.gettext  = function(dom, msgid) return msgid end
    M.ngettext = function(dom, msgid, plmsgid, n)
        if n == 1 then return msgid end
        return plmsgid
    end
end

return M