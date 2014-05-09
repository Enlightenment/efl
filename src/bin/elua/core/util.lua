-- elua core utilities used in other modules

local ffi  = require("ffi")
local cast = ffi.cast
local new  = ffi.new
local copy = ffi.copy
local str  = ffi.string
local gc   = ffi.gc

local C = ffi.C

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

ffi.cdef [[
    typedef struct _Str_Buf {
        char  *buf;
        size_t len, cap;
    } Str_Buf;

    void *malloc(size_t);
    void    free(void*);
    size_t  strlen(const char *str);

    int isalnum(int c);
    int isdigit(int c);
]]

local fmt   = string.format
local pcall = pcall
local error = error
local type  = type
local tostr = tostring

local bytes = { ("cdeEfgGiopuxXsq"):byte(1, #("cdeEfgGiopuxXsq")) }
for i, v in ipairs(bytes) do bytes[v] = true end

local Str_Buf = ffi.metatype("Str_Buf", {
    __new = function(self)
        local r = new("Str_Buf")
        r.buf = C.malloc(8)
        r.len = 0
        r.cap = 8
        gc(r, self.free)
        return r
    end,
    __tostring = function(self)
        return str(self.buf, self.len)
    end,
    __index = {
        free = function(self)
            C.free(self.buf)
            self.buf = nil
        end,
        clear = function(self)
            self.len = 0
        end,
        grow = function(self, newcap)
            local oldcap = self.cap
            if oldcap >= newcap then return end
            local buf = C.malloc(newcap)
            copy(buf, self.buf, self.len)
            C.free(self.buf)
            self.buf = buf
            self.cap = newcap
        end,
        append_char = function(self, c)
            local len = self.len
            self:grow (len + 1)
            self.buf  [len] = c
            self.len = len + 1
        end,
        append_str = function(self, str, strlen)
            if type(str) == "string" then strlen = strlen or #str end
            local strp = cast("const char*", str)
            strlen = strlen or C.strlen(strp)
            local len = self.len
            self:grow(len + strlen)
            for i = 0, strlen - 1 do
                self.buf[len + i] = strp[i]
            end
            self.len = len + strlen
        end
    }
})

local fmterr = function(idx, msg, off)
    local argerr = (type(idx) == "number")
        and ("#" .. idx)
         or ("'" .. idx .. "'")
    error("bad argument " .. argerr .. " to '%' (" .. msg .. ")",
        3 + (off or 0))
end

-- simulates lua's coercion
local checktype = function(c, idx, val)
    if c == 115 or c == 112 then -- s, p
        return
    end
    local tv = type(val)
    if c == 113 then -- q
        if tv ~= "string" or tv ~= "number" then
            fmterr(idx, "string expected, got " .. tv, 1)
        end
        return
    end
    if tv == "number" then return end
    if tv == "string" and tonumber(tv) then return end
    fmterr(idx, "number expected, got " .. tv, 1)
end

getmetatable("").__mod = function(fmts, params)
    if not fmts then return nil end
    if type(params) ~= "table" then
        params = { params }
    end

    local buf, nbuf = Str_Buf(), Str_Buf()
    local s         = cast("const char*", fmts)
    local c, s      = s[0], s + 1
    local argn      = 1

    while c ~= 0 do
        if c == 37 then -- %
            c, s = s[0], s + 1
            while c ~= 0 and C.isalnum(c) ~= 0 do
                nbuf:append_char(c)
                c, s = s[0], s + 1
            end
            if c == 36 then -- $
                c, s = s[0], s + 1
                local n = tostr(nbuf)
                nbuf:clear()
                while C.isdigit(c) ~= 0 or c == 45 or c == 46 do -- -, .
                    nbuf:append_char(c)
                    c, s = s[0], s + 1
                end
                if not bytes[c] then
                    buf:append_str(n)
                    buf:append_char(36) -- $
                    buf:append_char(c)
                else
                    nbuf:append_char(c)
                    local idx = tonumber(n) or n
                    if type(idx) == "number" and idx > #params then
                        fmterr(idx, "no value")
                    end
                    local v = params[idx]
                    checktype(c, idx, v)
                    buf:append_str(("%" .. tostr(nbuf)):format(v))
                    nbuf:clear()
                end
            else
                while c ~= 0 and (bytes[c] or C.isdigit(c) ~= 0
                or c == 45 or c == 46) do
                    nbuf:append_char(c)
                    c, s = s[0], s + 1
                end
                if argn > #params then
                    fmterr(idx, "no value")
                end
                local stat, val = pcall(fmt, "%" .. tostr(nbuf),
                    params[argn])
                nbuf:clear()
                if stat then
                    buf:append_str(val)
                else
                    fmterr(argn, val:match("%((.+)%)"))
                end
                if c then buf:append_char(c) end
                argn = argn + 1
            end
        else
            buf:append_char(c)
        end
        c, s = s[0], s + 1
    end
    return tostr(buf)
end

return M