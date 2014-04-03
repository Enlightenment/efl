-- elua module system

local M = {}

local util = select(2, ...)

local preload = {
    ffi  = package.preload["ffi"],
    util = function() return util end
}

local loaded = {
}

for k, v in pairs(package.loaded) do loaded[k] = v end

M.path = "./?.lua;/?/init.lua"

local loaders = {
    function(modname)
        local  v = preload[modname]
        if not v then
            return ("\no field preload['%s']"):format(modname)
        end
        return v
    end,
    function(modname)
        local  fname, err = package.searchpath(modname, M.path)
        if not fname then return err end
        local f, err = loadfile(fname)
        if not f then
            error("error loading module '" .. modname .. "' from file '"
                .. fname .. "':\n" .. err, 2)
        end
        return f
    end
}
M.loaders = loaders

local find_loader = function(modname, env)
    env = env or _G
    local err = { ("module '%s' not found\n"):format(modname) }
    for i = 1, #loaders do
        local v = loaders[i](modname)
        if type(v) == "function" then
            return setfenv(v, env)
        elseif type(v) == "string" then
            err[#err + 1] = v
        end
    end
    return nil, table.concat(err)
end

M.require = function(modname)
    local v = loaded[modname]
    if v ~= nil then return v end
    local  loader, err = find_loader(modname)
    if not loader then error(err, 2) end
    local ret = loader(modname)
    if ret ~= nil then
        loaded[modname] = ret
        return ret
    elseif loaded[modname] == nil then
        loaded[modname] = true
        return true
    end
    return loaded[modname]
end

M.preload = preload
M.loaded  = loaded

-- register require
M.path = (...)(M.require, M.path)

_G["require"] = M.require
_G["package"] = M

return M