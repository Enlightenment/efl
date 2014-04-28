-- elua gettext module

local M = {}

local gettext = ...

local bind_textdomain = gettext.bind_textdomain
local dgettext        = gettext.dgettext

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

M.gettext = function(dom, msgid)
    if not domains[dom] then return msgid end
    return dgettext(dom, msgid)
end

return M