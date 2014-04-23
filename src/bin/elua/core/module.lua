-- elua module system

local M = {}

local cutil = select(2, ...)

local preload = {
    ffi   = package.preload["ffi"],
    cutil = function() return cutil end
}

local loaded = {
}

M.loaded  = loaded
M.preload = preload

for k, v in pairs(package.loaded) do loaded[k] = v end

M.path  = "./?.lua;/?/init.lua"
M.cpath = ""

local loadlib = package.loadlib

local cloader_load = function(soname, modname)
    local hyp = modname:find("%-")
    if hyp then modname = modname:sub(hyp + 1) end
    local  f, err = loadlib(soname, "luaopen_" .. (modname:gsub("%.", "_")))
    if not f then
        error("error loading module '" .. modname .. "' from file '"
            .. fname .. "':\n" .. err, 3)
    end
    return f
end

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
        local  f, err = loadfile(fname)
        if not f then
            error("error loading module '" .. modname .. "' from file '"
                .. fname .. "':\n" .. err, 2)
        end
        return f
    end,
    function(modname)
        local  fname, err = package.searchpath(modname, M.cpath)
        if not fname then return err end
        local hyp = modname:find("%-")
        if hyp then modname = modname:sub(hyp + 1) end
        return cloader_load(fname, modname)
    end,
    function(modname)
        local rootname, dot = modname, modname:find("%.")
        if dot then
            rootname = rootname:sub(dot + 1)
        end
        local  fname, err = package.searchpath(rootname, M.cpath)
        if not fname then return err end
        return cloader_load(fname, modname)
    end
}
M.loaders = loaders

local find_loader = function(modname, env)
    env = env or _G
    local err = { ("module '%s' not found\n"):format(modname) }
    for i = 1, #loaders do
        local v = loaders[i](modname)
        if type(v) == "function" then
            local status, ret = pcall(function() return setfenv(v, env) end)
            return status and ret or v
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

M.config     = package.config
M.searchpath = package.searchpath
M.loadlib    = package.loadlib

_G["require"] = M.require
_G["package"] = M

return M