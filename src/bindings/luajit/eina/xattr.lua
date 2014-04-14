-- EFL LuaJIT bindings: Eina (xattr module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

local iterator = require("eina.iterator")

ffi.cdef [[
    typedef unsigned char Eina_Bool;

    typedef enum {
        EINA_XATTR_INSERT,
        EINA_XATTR_REPLACE,
        EINA_XATTR_CREATED
    } Eina_Xattr_Flags;

    typedef struct _Eina_Xattr Eina_Xattr;
    struct _Eina_Xattr {
        const char *name;
        const char *value;
        size_t length;
    };

    EAPI Eina_Iterator *eina_xattr_ls(const char *file);
    EAPI Eina_Iterator *eina_xattr_value_ls(const char *file);
    EAPI Eina_Iterator *eina_xattr_fd_ls(int fd);
    EAPI Eina_Iterator *eina_xattr_value_fd_ls(int fd);
    EAPI Eina_Bool eina_xattr_copy(const char *src, const char *dst);
    EAPI Eina_Bool eina_xattr_fd_copy(int src, int dst);
    EAPI void *eina_xattr_get(const char *file, const char *attribute, ssize_t *size);
    EAPI void *eina_xattr_fd_get(int fd, const char *attribute, ssize_t *size);
    EAPI Eina_Bool eina_xattr_set(const char *file, const char *attribute, const void *data, ssize_t length, Eina_Xattr_Flags flags);
    EAPI Eina_Bool eina_xattr_fd_set(int fd, const char *attribute, const void *data, ssize_t length, Eina_Xattr_Flags flags);
    EAPI Eina_Bool eina_xattr_del(const char *file, const char *attribute);
    EAPI Eina_Bool eina_xattr_fd_del(int fd, const char *attribute);
    EAPI Eina_Bool eina_xattr_string_set(const char *file, const char *attribute, const char *data, Eina_Xattr_Flags flags);
    EAPI char *eina_xattr_string_get(const char *file, const char *attribute);
    EAPI Eina_Bool eina_xattr_double_set(const char *file, const char *attribute, double value, Eina_Xattr_Flags flags);
    EAPI Eina_Bool eina_xattr_double_get(const char *file, const char *attribute, double *value);
    EAPI Eina_Bool eina_xattr_int_set(const char *file, const char *attribute, int value, Eina_Xattr_Flags flags);
    EAPI Eina_Bool eina_xattr_int_get(const char *file, const char *attribute, int *value);

    void free(void*);
]]

local cutil = require("cutil")
local util  = require("util")

local M = {}

local eina

local init = function()
    eina = util.lib_load("eina")
end

local shutdown = function()
    util.lib_unload("eina")
end

cutil.init_module(init, shutdown)

local Iterator = iterator.Iterator

local Name_Iterator = Iterator:clone {
    __ctor = function(self, selfmt, file)
        return Iterator.__ctor(self, selfmt, eina.eina_xattr_ls(file))
    end,
    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        return ffi.string(v)
    end
}

M.ls = function(file) return Name_Iterator(file) end

local Value_Iterator = Iterator:clone {
    __ctor = function(self, selfmt, file)
        return Iterator.__ctor(self, selfmt, eina.eina_xattr_value_ls(file))
    end,
    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        v = ffi.cast(v, "Eina_Xattr*")
        return ffi.string(v.name), ffi.string(v.value, v.length)
    end
}

M.value_ls = function(file) return Value_Iterator(file) end

M.copy = function(src, dst)
    return eina.eina_xatr_copy(src, dst) ~= 0
end

M.get = function(file, attribute)
    local size = ffi.new("size_t[1]")
    local v = eina.eina_xattr_get(file, attribute, size)
    if v == nil then return nil end
    local r = ffi.string(v, size[0])
    C.free(v)
    return r
end

M.flags = {
    INSERT  = 0,
    REPLACE = 1,
    CREATED = 2
}

M.set = function(file, attribute, data, flags)
    if not data then return false end
    return eina.eina_xattr_set(file, attribute, data, #data, flags or 0) ~= 0
end

M.del = function(file, attribute)
    return eina.eina_xattr_del(file, attribute) ~= 0
end

M.string_set = function(file, attribute, data, flags)
    return eina.eina_xattr_set(file, attribute, data, #data + 1,
        flags or 0) ~= 0
end

M.string_get = function(file, attribute)
    local v = eina.eina_xattr_string_get(file, attribute)
    if v == nil then return nil end
    local r = ffi.string(v)
    C.free(v)
    return r
end

M.double_set = function(file, attribute, value, flags)
    return eina.eina_xattr_double_set(file, attribute, value, flags) ~= 0
end

M.double_get = function(file, attribute)
    local v = ffi.new("double[1]")
    local r = eina.eina_xattr_double_get(file, attribute, v)
    if r == 0 then return false end
    return true, tonumber(v[0])
end

M.int_set = function(file, attribute, value, flags)
    return eina.eina_xattr_int_set(file, attribute, value, flags) ~= 0
end

M.int_get = function(file, attribute)
    local v = ffi.new("int[1]")
    local r = eina.eina_xattr_int_get(file, attribute, v)
    if r == 0 then return false end
    return true, tonumber(v[0])
end

return M