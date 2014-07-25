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
            if r == 0 then return nil end
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

local dgetmt = debug.getmetatable

M.Iterator = util.Readonly_Object:clone {
    __ctor = function(self, selfmt, iter)
        -- prevent null stuff
        if iter == nil then iter = nil end
        if iter then ffi.gc(iter, iter.free) end
        selfmt.__eq = function(self, other)
            return selfmt.__iterator == dgetmt(other).__iterator
        end
        selfmt.__call = function(self)
            return self:next()
        end
        selfmt.__iterator = iter
    end,

    free = function(self)
        self = dgetmt(self)
        if not self.__iterator then return end
        self.__iterator:free()
        self.__iterator = nil
    end,

    next = function(self)
        self = dgetmt(self)
        if not self.__iterator then return nil end
        return self.__iterator:next()
    end,

    lock = function(self)
        self = dgetmt(self)
        if not self.__iterator then return false end
        return self.__iterator:lock()
    end,

    unlock = function(self)
        self = dgetmt(self)
        if not self.__iterator then return false end
        return self.__iterator:unlock()
    end,

    container_get = function(self)
        self = dgetmt(self)
        if not self.__iterator then return nil end
        return self.__iterator:container_get()
    end,

    to_array = function(self)
        local ret = {}
        for v in self do
            ret[#ret + 1] = v
        end
        return ret
    end
}

local Iterator = M.Iterator

M.Ptr_Iterator = Iterator:clone {
    __ctor = function(self, selfmt, ptrtype, iter)
        Iterator.__ctor(self, selfmt, iter)
        selfmt.ptrtype = ptrtype
    end,

    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        return ffi.cast(dgetmt(self).ptrtype, v)
    end
}

M.String_Iterator = Iterator:clone {
    next = function(self)
        local  v = Iterator.next(self)
        if not v then return nil end
        return ffi.string(v)
    end
}

return M
