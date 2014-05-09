-- elua core utilities used in other modules

local ffi = require("ffi")

local M = {}

local getmetatable, setmetatable = getmetatable, setmetatable

local gen_field = function(self, fname)
    local  t = rawget(self, fname)
    if not t then
        t = {}
        rawset(self, fname, t)
        return false, t
    end
    return true, t
end

local init_getters = function(self)
    local has_gtrs, gtrs = gen_field(self, "__getters")
    if not has_gtrs then
        local oidx = self.__index
        if type(oidx) == "table" then
            local ooidx = oidx
            oidx = function(self, n) return ooidx[n] end
        end
        self.__index = function(self, n)
            local f = gtrs[n]
            if f then return f(self) end
            local v = rawget(self, n)
            if v == nil then return oidx(self, n) end
            return v
        end
    end
    return gtrs
end

local init_setters = function(self)
    local has_strs, strs = gen_field(self, "__setters")
    if not has_strs then
        local onindex = self.__newindex or rawset
        self.__newindex = function(self, n, v)
            local f = strs[n]
            if f then return f(self, v) end
            onindex(self, n, v)
        end
    end
    return strs
end

local wrap_retvals = function(f)
    return function()
        local vals = { f() }
        if #vals == 0 then return end
        if #vals  > 1 then return vals end
        return vals[1]
    end
end

M.Object = {
    __call = function(self, ...)
        local r = self:clone()
        if self.__ctor then return r, self.__ctor(r, ...) end
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

    define_property = function(self, propname, get, set)
        if get then
            init_getters(self)[propname] = wrap_retvals(get)
        end
        if set then
            init_setters(self)[propname] = wrap_retvals(set)
        end
    end,

    define_property_key = function(self, propname, get, set)
        if get then get = wrap_retvals(get) end
        if set then set = wrap_retvals(set) end
        local proxy = setmetatable({}, {
            __index = function(proxy, key)
                if get then return get(self, key) end
            end,
            __newindex = function(proxy, key, val)
                if set then return set(self, key, val) end
            end
        })
        init_getters(self)[propname] = function(self, n) return proxy end
    end,

    mixin = function(self, obj)
        for k, v in pairs(obj) do self[k] = v end
    end,

    __tostring = function(self)
        return ("Object: %s"):format(self.name or "unnamed")
    end
}

local newproxy = newproxy

M.Readonly_Object = M.Object:clone {}
M.Readonly_Object.__call = function(self, ...)
    local r = newproxy(true)
    local rmt = getmetatable(r)
    rmt.__index = self
    rmt.__tostring = self.__tostring
    rmt.__metatable = false
    if self.__ctor then return r, self.__ctor(r, rmt, ...) end
    return r
end

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

-- string fmt

local tconc = table.concat
local fmt = string.format
local pcall = pcall
local error = error
local type  = type

getmetatable("").__mod = function(s, params)
    if type(params) ~= "table" then params = { params } end
    local iter = s:gmatch(".")
    local buf = {}
    local c = iter()
    local argn = 1
    while c do
        if c == "%" then
            c = iter()
            local nbuf = {}
            while c and c:match("%w") do
                nbuf[#nbuf + 1] = c
                c = iter()
            end
            if c == "$" then
                c = iter()
                local n = tconc(nbuf)
                nbuf = {}
                while c:match("[-0-9%.]") do
                    nbuf[#nbuf + 1] = c
                    c = iter()
                end
                if not c:match("[cdeEfgGiopuxXsq]") then
                    buf[#buf + 1] = n
                    buf[#buf + 1] = "$"
                    buf[#buf + 1] = c
                else
                    nbuf[#nbuf + 1] = c
                    local idx = tonumber(n) or n
                    local stat, val = pcall(fmt, "%" .. tconc(nbuf),
                        params[idx])
                    if stat then
                        buf[#buf + 1] = val
                    else
                        local argerr = (type(idx) == "number")
                            and ("#" .. idx)
                             or ("'" .. idx .. "'")
                        error("bad argument " .. argerr .. " to '%' "
                            .. val:match("%(.+%)"), 2)
                    end
                end
            else
                while c and c:match("[-0-9%.cdeEfgGiopuxXsq]") do
                    nbuf[#nbuf + 1] = c
                    c = iter()
                end
                local stat, val = pcall(fmt, "%" .. tconc(nbuf), params[argn])
                if stat then
                    buf[#buf + 1] = val
                else
                    error("bad argument #" .. argn .. " to '%' "
                        .. val:match("%(.+%)"), 2)
                end
                if c then buf[#buf + 1] = c end
                argn = argn + 1
            end
        else
            buf[#buf + 1] = c
        end
        c = iter()
    end
    return tconc(buf)
end

return M