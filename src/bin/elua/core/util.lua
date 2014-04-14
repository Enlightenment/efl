-- elua core utilities used in other modules

local ffi = require("ffi")

local M = {}

local getmetatable, setmetatable = getmetatable, setmetatable

M.Object = {
    __call = function(self, ...)
        local r = {
            __index = self, __proto = self, __call = self.__call,
            __tostring = self.__tostring
        }
        setmetatable(r, r)
        if self.__ctor then self.__ctor(r, ...) end
        return r
    end,

    clone = function(self, o)
        o = o or {}
        o.__index, o.__proto, o.__call = self, self, self.__call
        if not o.__tostring then
            o.__tostring = self.__tostring
        end
        setmetatable(o, o)
        return o
    end,

    is_a = function(self, base)
        if self == base then return true end
        local pt = self.__proto
        local is = (pt == base)
        while not is and pt do
            pt = pt.__proto
            is = (pt == base)
        end
        return is
    end,

    __tostring = function(self)
        return ("Object: %s"):format(self.name or "unnamed")
    end
}

local newproxy = newproxy

M.Readonly_Object = M.Object:clone {
    __call = function(self, ...)
        local r = newproxy(true)
        local rmt = getmetatable(r)
        rmt.__index = self
        rmt.__tostring = self.__tostring
        rmt.__metatable = false
        if self.__ctor then self.__ctor(r, rmt, ...) end
        return r
    end
}

local loaded_libs = {}
local loaded_libc = {}

-- makes sure we only keep one handle for each lib
-- reference counted
M.lib_load = function(libname)
    local  lib = loaded_libs[libname]
    if not lib then
        lib = ffi.load(libname)
        loaded_libs[libname] = lib
        loaded_libc[libname] = 0
    end
    loaded_libc[libname] = loaded_libc[libname] + 1
    return lib
end

M.lib_unload = function(libname)
    local  cnt = loaded_libc[libname]
    if not cnt then return end
    if cnt == 1 then
        loaded_libs[libname], loaded_libc[libname] = nil, nil
    else
        loaded_libc[libname] = cnt - 1
    end
end

return M