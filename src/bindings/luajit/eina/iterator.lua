-- EFL LuaJIT bindings: Eina (iterator module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

ffi.cdef [[
    typedef struct _Eina_Iterator Eina_Iterator;
    typedef unsigned char Eina_Bool;

    void      eina_iterator_free         (Eina_Iterator *iterator);
    Eina_Bool eina_iterator_next         (Eina_Iterator *iterator, void **data);
    Eina_Bool eina_iterator_lock         (Eina_Iterator *iterator);
    Eina_Bool eina_iterator_unlock       (Eina_Iterator *iterator);
    void     *eina_iterator_container_get(Eina_Iterator *iterator);
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

ffi.metatype("Eina_Iterator", {
    __index = {
        free = function(self) eina.eina_iterator_free(ffi.gc(self, nil)) end,
        next = function(self)
            local data = ffi.new("void*[1]")
            local r = eina.eina_iterator_next(self, data)
            if r == nil then return nil end
            return data[0]
        end,
        lock   = function(self) return eina.eina_iterator_lock  (self) ~= 0 end,
        unlock = function(self) return eina.eina_iterator_unlock(self) ~= 0 end,
        container_get = function(self)
            local v = eina.eina_iterator_container_get(self)
            if v == nil then return nil end
            return v
        end
    }
})

cutil.init_module(init, shutdown)

M.Iterator = util.Object:clone {
    __ctor = function(self, iter)
        -- prevent null stuff
        if iter == nil then iter = nil end
        if iter then ffi.gc(iter, iter.free) end
        self.__eq = function(self, other)
            return self.__iterator == other.__iterator
        end
        self.__call = function(self)
            return self:next()
        end
        self.__iterator = iter
    end,

    free = function(self)
        if not self.__iterator then return end
        self.__iterator:free()
        self.__iterator = nil
    end,

    disown = function(self)
        local iter = self.__iterator
        self.__iterator = nil
        return ier
    end,

    rebase = function(self, iter)
        self:free()
        self.__iterator = iter:disown()
    end,

    next = function(self)
        if not self.__iterator then return end
        return self.__iterator:next()
    end,

    lock = function(self)
        if not self.__iterator then return end
        return self.__iterator:lock()
    end,

    unlock = function(self)
        if not self.__iterator then return end
        return self.__iterator:unlock()
    end,

    container_get = function(self)
        if not self.__iterator then return end
        return self.__iterator:container_get()
    end
}

return M