-- EFL LuaJIT bindings: Eina (rectangle module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

ffi.cdef [[
    typedef struct _Eina_Rectangle {
        int x, y, w, h;
    } Eina_Rectangle;

    typedef struct _Eina_Rectangle_Pool Eina_Rectangle_Pool;

    typedef unsigned char Eina_Bool;

    Eina_Rectangle_Pool *eina_rectangle_pool_new(int w, int h);
    Eina_Rectangle_Pool *eina_rectangle_pool_get(Eina_Rectangle *rect);
    Eina_Bool            eina_rectangle_pool_geometry_get(Eina_Rectangle_Pool *pool, int *w, int *h);
    void                *eina_rectangle_pool_data_get(Eina_Rectangle_Pool *pool);
    void                 eina_rectangle_pool_data_set(Eina_Rectangle_Pool *pool, const void *data);
    void                 eina_rectangle_pool_free(Eina_Rectangle_Pool *pool);
    int                  eina_rectangle_pool_count(Eina_Rectangle_Pool *pool);
    Eina_Rectangle      *eina_rectangle_pool_request(Eina_Rectangle_Pool *pool, int w, int h);
    void                 eina_rectangle_pool_release(Eina_Rectangle *rect);

    Eina_Rectangle *eina_rectangle_new (int x, int y, int w, int h);
    void            eina_rectangle_free(Eina_Rectangle *rect);
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

M.Rectangle = ffi.metatype("Eina_Rectangle", {
    __new = function(self, x, y, w, h)
        return ffi.gc(eina.eina_rectangle_new(x, y, w, h), self.free)
    end,
    __index = {
        free = function(self) eina.eina_rectangle_free(ffi.gc(self, nil)) end,
        set  = function(self, x, y, w, h)
            self.x, self.y, self.w, self.h = x, y, w, h
        end,
        get_pool = function(self)
            local  v = eina.eina_rectangle_pool_get(self)
            return v ~= nil and v or nil
        end
    }
})

M.Rectangle_Pool = ffi.metatype("Eina_Rectangle_Pool", {
    __new = function(self, w, h)
        return ffi.gc(eina.eina_rectangle_pool_new(w, h), self.free)
    end,

    __len = function(self) return eina.eina_rectangle_pool_count(self) end,

    __index = {
        free = function(self)
            eina.eina_rectangle_pool_free(ffi.gc(self, nil))
        end,
        get = function(rect) return rect:get_pool() end,

        geometry_get = function(self)
            local w, h = ffi.new("int[1]"), ffi.new("int[1]")
            eina.eina_rectangle_pool_geometry_get(self, w, h)
            return w[0], h[0]
        end,

        --[[
        data_get = function(self)
            return eina.eina_rectangle_pool_data_get(self)
        end,
        data_set = function(self, v)
            eina.eina_rectangle_pool_data_set(self, v)
        end,
        ]]

        request = function(self, w, h)
            local v = eina.eina_rectangle_pool_request(self, w, h)
            return v ~= nil and v or nil
        end,

        release = function(self, rect)
            eina.eina_rectangle_pool_release(rect)
        end
    }
})

return M