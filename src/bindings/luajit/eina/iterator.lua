-- EFL LuaJIT bindings: Eina (iterator module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

ffi.cdef [[
    typedef struct _Eina_Iterator Eina_Iterator;
    typedef unsigned char Eina_Bool;

    void      eina_iterator_free  (Eina_Iterator *iterator);
    Eina_Bool eina_iterator_next  (Eina_Iterator *iterator, void **data);
    Eina_Bool eina_iterator_lock  (Eina_Iterator *iterator);
    Eina_Bool eina_iterator_unlock(Eina_Iterator *iterator);
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
        free = function(self) C.eina_iterator_free(self) end,
        next = function(self)
            local data = ffi.new("void*[1]")
            local r = C.eina_iterator_next(self, data)
            if r == 0 then return nil end
            return data[0]
        end,
        lock   = function(self) return C.eina_iterator_lock  (self) == 1 end,
        unlock = function(self) return C.eina_iterator_unlock(self) == 1 end
    }
})

cutil.init_module(init, shutdown)

M.Iterator = util.Object:clone {
    __ctor = function(self, iter)
        self.__iterator = iter
        if self.__iterator then ffi.gc(self.__iterator, function(iter)
            iter:free()
        end) end
        self.__eq = function(self, other)
            return self.__iterator == other.__iterator
        end
        self.__call = function(self)
            return self:next()
        end
    end,

    free = function(self)
        if not self.__iterator then return end
        self.__iterator.free(ffi.gc(self.__iterator, nil))
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
    end
}

return M