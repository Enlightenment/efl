-- EFL LuaJIT bindings: Eina (xattr module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

local iterator = require("eina.iterator")

ffi.cdef [[
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

return M