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
local dgetmt = debug.getmetatable
local newproxy = newproxy

if not newproxy then
    -- tables can have __gc from 5.2
    newproxy = function(b)
        if b then
            return setmetatable({}, {})
        end
        return {}
    end
end

-- multiple inheritance index with depth-first search
local proto_lookup = function(protos, name)
    if not protos then return nil end
    for i = 1, #protos do
        local proto = protos[i]
        local v = proto[name]
        if v ~= nil then
            return v
        end
    end
end

local Object_MT = {
    __index = function(self, name)
        local v = proto_lookup(self.__mixins, name)
        if v == nil then
            return proto_lookup(self.__protos, name)
        end
    end,

    __tostring = function(self)
        local  f = self["__tostring"]
        if not f then
            return ("Object: %s"):format(self.name or "unnamed")
        end
        return f(self)
    end,

    __call = function(self, ...)
        return self["__call"](self, ...)
    end
}

local obj_gc = function(px)
    local obj = dgetmt(px).__obj
    local dtor = obj and obj.__dtor or nil
    if dtor then dtor(obj) end
end

M.Object = {
    __enable_dtor = false,

    __call = function(self, ...)
        local r = self:clone()
        if self.__enable_dtor then
            local px = newproxy(true)
            local pxmt = dgetmt(px)
            r.__gcproxy = px
            pxmt.__gc = obj_gc
            pxmt.__obj = r
        end
        if self.__ctor then return r, self.__ctor(r, ...) end
        return r
    end,

    clone = function(self, o)
        o = o or {}
        o.__protos, o.__mixins = { self }, {}
        setmetatable(o, Object_MT)
        return o
    end,

    is_a = function(self, base)
        if self == base then return true end
        local protos = self.__protos
        for i = 1, #protos do
            if protos[i]:is_a(base) then
                return true
            end
        end
        return false
    end,

    add_parent = function(self, parent)
        local protos = self.__protos
        protos[#protos + 1] = parent
    end,

    add_mixin = function(self, mixin)
        local mixins = self.__mixins
        mixins[#mixins + 1] = mixin
    end
}

local robj_gc = function(px)
    local dtor = px.__dtor
    if dtor then dtor(px) end
end

M.Readonly_Object = M.Object:clone {}
M.Readonly_Object.__call = function(self, ...)
    local r = newproxy(true)
    local rmt = dgetmt(r)
    rmt.__index = self
    rmt.__tostring = Object_MT.__tostring
    rmt.__metatable = false
    if self.__enable_dtor then
        rmt.__gc = robj_gc
    end
    if self.__ctor then return r, self.__ctor(r, rmt, ...) end
    return r
end

local loaded_libs = {}
local loaded_libc = {}

local load_lib_win = function(libname, ev)
    local succ, v
    if not ev or ev == "" then
        succ, v = pcall(ffi.load, libname)
        if not succ then
            succ, v = pcall(ffi.load, "lib" .. libname)
        end
    else
        succ, v = pcall(ffi.load, ev .. "\\" .. libname .. ".dll")
        if not succ then
            succ, v = pcall(ffi.load, ev .. "\\lib" .. libname .. ".dll")
        end
    end
    return succ, v
end

local load_lib = function(libname, ev)
    local succ, v
    if ffi.os == "Windows" then
        succ, v = load_lib_win(libname, ev)
    elseif not ev or ev == "" then
        succ, v = pcall(ffi.load, libname)
    else
        local ext = (ffi.os == "OSX") and ".dylib" or ".so"
        succ, v = pcall(ffi.load, ev .. "/lib" .. libname .. ext)
    end
    return succ, v
end

-- makes sure we only keep one handle for each lib
-- reference counted
M.lib_load = function(libname)
    local  lib = loaded_libs[libname]
    if not lib then
        local ev = os.getenv("ELUA_" .. libname:upper() .. "_LIBRARY_PATH")
        local succ, v = load_lib(libname, ev)
        if not succ then
            error(v, 2)
        end
        lib = v
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
        size_t len;
        size_t cap;
    } Str_Buf;

    void *malloc(size_t);
    void    free(void *);
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
            self.len = 0
            self.cap = 0
        end,
        clear = function(self)
            self.len = 0
        end,
        grow = function(self, newcap)
            local oldcap = self.cap
            if oldcap >= newcap then return end
            local buf = C.malloc(newcap)
            if self.len ~= 0   then copy(buf, self.buf, self.len) end
            if self.buf ~= nil then C.free(self.buf) end
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
        return val
    end
    local tv = type(val)
    if c == 113 then -- q
        if tv ~= "string" or tv ~= "number" then
            fmterr(idx, "string expected, got " .. tv, 1)
        end
        return val
    end
    if tv == "number" then return val end
    if tv == "string" then
        local v = tonumber(val)
        if v then return v end
    end
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
            if c ~= 37 then
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
                        v = checktype(c, idx, v)
                        buf:append_str(("%" .. tostr(nbuf)):format(v))
                        nbuf:clear()
                    end
                else
                    local fmtmark = (nbuf.len > 0) and nbuf.buf[0] or nil
                    if not fmtmark then
                        while c ~= 0 and (C.isdigit(c) ~= 0 or c == 45
                        or c == 46) do
                            nbuf:append_char(c)
                            c, s = s[0], s + 1
                        end
                        if bytes[c] then fmtmark = c end
                    end
                    if fmtmark then
                        if argn > #params then
                            fmterr(argn, "no value")
                        end
                        local v = params[argn]
                        v = checktype(fmtmark, argn, v)
                        buf:append_str(("%" .. tostr(nbuf)):format(v))
                        nbuf:clear()
                        argn = argn + 1
                    else
                        buf:append_str(nbuf.buf, nbuf.len)
                        nbuf:clear()
                    end
                    if c ~= 0 then buf:append_char(c) end
                end
            else
                buf:append_char(c)
            end
        else
            buf:append_char(c)
        end
        c, s = s[0], s + 1
    end
    nbuf:free()
    local ret = tostr(buf)
    buf:free()
    return ret
end

-- file utils

M.find_file = function(fname, paths)
    for i, path in ipairs(paths) do
        local actual_path
        if path:match(".*/") then
            actual_path = path .. fname
        else
            actual_path = path .. "/" .. fname
        end
        local f = io.open(actual_path)
        if f then
            f:close()
            return actual_path
        end
    end
end

-- table utils

table.uniq = function(tbl)
    local ret  = {}
    local used = {}
    for i, v in ipairs(tbl) do
        if not used[v] then
            ret[#ret + 1], used[v] = v, true
        end
    end
    return ret
end

M.get_namespace = function(M, nspaces)
    local last_m = M
    for i, v in ipairs(nspaces) do
        local nsp = M[v]
        if not nsp then
            nsp = {}
            last_m[v] = nsp
        end
        last_m = nsp
    end
    return last_m
end

return M
