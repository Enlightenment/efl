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

local spans_intersect = function(c1, l1, c2, l2)
    return not (((c2 + l2) <= c1) or (c2 >= (c1 + l1)))
end

M.spans_intersect = spans_intersect

M.Rectangle = ffi.metatype("Eina_Rectangle", {
    __new = function(self, x, y, w, h)
        return ffi.gc(eina.eina_rectangle_new(x, y, w, h), self.free)
    end,
    __index = {
        free = function(self) eina.eina_rectangle_free(ffi.gc(self, nil)) end,

        get_pool = function(self)
            local  v = eina.eina_rectangle_pool_get(self)
            return v ~= nil and v or nil
        end,

        is_empty = function(self)
            return (self.w < 1) or (self.h < 1)
        end,

        coords_from = function(self, x, y, w, h)
            self.x, self.y, self.w, self.h = x, y, w, h
        end,

        intersects = function(self, other)
            return spans_intersect(self.x, self.w, other.x, other.w)
               and spans_interscet(self.y, self.h, other.y, other.h)
        end,

        xcoord_inside = function(self, x)
            return ((x >= self.x) and (x < (self.x + self.w)))
        end,

        ycoord_inside = function(self, y)
            return ((y >= self.y) and (y < (self.y + self.h)))
        end,

        coords_inside = function(self, x, y)
            return self:xcoord_inside(x) and self:ycoord_inside(y)
        end,

        union = function(dst, src)
            if dst.x > src.x then
                dst.w = dst.w + dst.x - src.x
                dst.x = src.x
            end
            if (dst.x + dst.w) < (src.x + src.w) then
                dst.w = src.x + src.w - dst.x
            end
            if dst.y > src.y then
                dst.h = dst.h + dst.y - src.y
                dst.y = src.y
            end
            if (dst.y + dst.h) < (src.y + src.h) then
                dst.h = src.y + src.h - dst.y
            end
        end,

        intersection = function(dst, src)
            if not dst:intersects(src) then
                return false
            end

            if dst.x < src.x then
                dst.w = dst.w + dst.x - src.x
                dst.x = src.x
                if dst.w < 0 then dst.w = 0 end
            end
            if (dst.x + dst.w) > (src.x + src.w) then
                dst.w = src.x + src.w - dst.x
            end
            if dst.y < src.y then
                dst.h = dst.h + dst.y - src.y
                dst.y = src.y
                if dst.h < 0 then dst.h = 0 end
            end
            if (dst.y + dst.h) > (src.y + src.h) then
                dst.h = src.y + src.h - dst.y
            end

            return true
        end,

        rescale_in = function(self, inr, outr)
            self.x = inr.x - outr.x
            self.y = inr.y - outr.y
            self.w = inr.w
            self.h = inr.h
        end,

        rescale_out = function(self, inr, outr)
            self.x = outr.x + inr.x
            self.y = outr.y + inr.y
            self.w = outr.w
            self.h = outr.h
        end,

        is_valid = function(self)
            return self.w > 0 and self.h > 0
        end,

        max_x = function(self) return self.x + self.w end,
        max_y = function(self) return self.y + self.h end,

        x_cut = function(self, slice, leftover, amount)
            if amount > self.w then return false end
            if  slice then
                slice:coords_from(self.x, self.y, amount, self.h)
            end
            if  leftover then
                leftover:coords_from(self.x + amount, self.y,
                                     self.w - amount, self.h)
            end
            return true
        end,

        y_cut = function(self, slice, leftover, amount)
            if amount > self.h then return false end
            if  slice then
                slice:coords_from(self.x, self.y, self.w, amount)
            end
            if  leftover then
                leftover:coords_from(self.x, self.y + amount,
                                     self.w, self.h - amount)
            end
            return true
        end,

        width_cut = function(self, slice, leftover, amount)
            if (self.w - amount) < 0 then return false end
            if  slice then
                slice:coords_from(self.x + (self.w - amount), self.y,
                                  amount, self.h)
            end
            if  leftover then
                leftover:coords_from(self.x, self.y, self.w - amount, self.h)
            end
            return true
        end,

        height_cut = function(self, slice, leftover, amount)
            if (self.h - amount) < 0 then return false end
            if  slice then
                slice:coords_from(self.x, self.y + (self.h - amount),
                                  self.w, amount)
            end
            if  leftover then
                leftover:coords_from(self.x, self.y, self.w, self.h - amount)
            end
            return true
        end,

        subtract = function(self, other)
            if not self:is_valid() then return false end

            local o1, o2, o3, o4 = ffi.new("Eina_Rectangle"),
                                   ffi.new("Eina_Rectangle"),
                                   ffi.new("Eina_Rectangle"),
                                   ffi.new("Eina_Rectangle")

            local intersection = ffi.new("Eina_Rectangle", self)
            if not intersection:intersection(other) then
                o1:coords_from(self)
                return true, o1, o2, o3, o4
            end

            local leftover = ffi.new("Eina_Rectangle")

            local cut = self.h - (intersection.y - self.y)
            if cut > self.h then cut = self.h end
            self:height_cut(self, leftover, o1, cut)

            local tmp = ffi.new("Eina_Rectangle", leftover)
            if tmp:intersection(intersection) then
                cut = leftover.h - (leftover:max_y() - tmp:max_y())
                if cut > leftover.h then cut = leftover.h end
                leftover:y_cut(leftover, o2, cut)
            end

            local tmp = ffi.new("Eina_Rectangle", leftover)
            if tmp:intersection(intersection) then
                cut = leftover.w - (tmp.x - leftover.x)
                if cut > leftover.w then cut = leftover.w end
                leftover:width_cut(leftover, o3, cut)
            end

            local tmp = ffi.new("Eina_Rectangle", leftover)
            if tmp:intersection(intersection) then
                cut = leftover.w - (leftover:max_x() - tmp:max_x())
                if cut > leftover.w then cut = leftover.w end
                leftover:x_cut(leftover, o4, cut)
            end

            return true, o1, o2, o3, o4
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