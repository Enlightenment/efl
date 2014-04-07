-- EFL LuaJIT bindings: Eina (tiler module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

local iterator = require("eina.iterator")
require("eina.rectangle")

ffi.cdef [[
    typedef struct _Eina_Tiler Eina_Tiler;

    typedef struct _Eina_Tile_Grid_Info {
        unsigned long  col, row;
        Eina_Rectangle rect;
        Eina_Bool      full;
    } Eina_Tile_Grid_Info;

    typedef sruct_Eina_Tile_Grid_Slicer {
        unsigned long       col1, col2, row1, row2;
        int                 tile_w, tile_h;
        int                 x_rel, y_rel;
        int                 w1_rel, h1_rel;
        int                 w2_rel, h2_rel;
        Eina_Tile_Grid_Info info;
        Eina_Bool           first;
    } Eina_Tile_Grid_Slicer;

    typedef unsigned char Eina_Bool;

    Eina_Tiler        *eina_tiler_new(int w, int h);
    void               eina_tiler_free(Eina_Tiler *t);
    void               eina_tiler_tile_size_set(Eina_Tiler *t, int w, int h);
    void               eina_tiler_area_size_set(Eina_Tiler *t, int w, int h);
    void               eina_tiler_area_size_get(const Eina_Tiler *t, int *w, int *h);
    void               eina_tiler_strict_set(Eina_Tiler *t, Eina_Bool strict);
    Eina_Bool          eina_tiler_empty(Eina_Tiler *t);
    Eina_Bool          eina_tiler_rect_add(Eina_Tiler *t, const Eina_Rectangle *r);
    void               eina_tiler_rect_del(Eina_Tiler *t, const Eina_Rectangle *r);
    void               eina_tiler_clear(Eina_Tiler *t);
    Eina_Iterator     *eina_tiler_iterator_new(const Eina_Tiler *t);

    Eina_Iterator     *eina_tile_grid_slicer_iterator_new(int x, int y, int w, int h, int tile_w, int tile_h);
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

M.Tiler_Iterator = Iterator:clone {
    __ctor = function(self, tiler)
        return Iterator.__ctor(self, eina.eina_tiler_iterator_new(tiler))
    end,

    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        return ffi.cast("Eina_Rectangle*", v)
    end
}

M.Tile_Grid_Slicer_Iterator = Iterator:clone {
    __ctor = function(self, x, y, w, h, tile_w, tile_h)
        return Iterator.__ctor(self, eina.eina_tile_grid_slicer_iterator_new(
            x, y, w, h, tile_w, tile_h))
    end,

    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        return ffi.cast("Eina_Tile_Grid_Info*", v)
    end
}

M.Tiler = ffi.metatype("Eina_Tiler", {
    __new = function(self, w, h)
        return ffi.gc(eina.eina_tiler_new(w, h), self.free)
    end,
    __index = {
        free = function(self) eina.eina_tiler_free(ffi.gc(self, nil)) end,

        tile_size_set = function(self, w, h)
            eina.eina_tiler_tile_size_set(self, w, h)
        end,

        area_size_set = function(self, w, h)
            eina.eina_tiler_area_sie_set(self, w, h)
        end,

        area_size_get = function(self)
            local w, h = ffi.new("int[1]"), ffi.new("int[2]")
            eina.eina_tiler_area_size_get(self, w, h)
            return w[0], h[0]
        end,

        strict_set = function(self, strict)
            eina.eina_tiler_strict_set(self, strict)
        end,

        is_empty = function(self)
            return eina.eina_tiler_empty(self) == 1
        end,

        rect_add = function(self, r)
            return eina.eina_tiler_rect_add(self, r) == 1
        end,
        rect_del = function(self, r)
            eina.eina_tiler_rect_del(self, r)
        end,

        clear = function(self) eina.eina_tiler_clear(self) end,

        each = function(self)
            return M.Tiler_Iterator(self)
        end,

        Iterator = M.Tiler_Iterator
    }
})

M.Tile_Grid_Slicer = ffi.metatype("Eina_Tile_Grid_Slicer", {
    __index = {
        setup = function(self, x, y, w, h, tile_w, tile_h)
            return eina.eina_tile_grid_slicer_setup(x, y, w, h,
                tile_w, tile_h) == 1
        end,

        next = function(self)
            local info = ffi.new("Eina_Tile_Grid_Info*[1]")
            if eina.eina_tile_grid_slicer_next(self, info) == 0 then
                return nil
            end
            return info[0]
        end,

        each = function(self)
            return function()
                return self:next()
            end
        end,

        Iterator = M.Tile_Grid_Slicer_Iterator
    }
})

return M