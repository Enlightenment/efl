-- EFL LuaJIT bindings: Eina (accessor module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

ffi.cdef [[
    typedef struct _Eina_Accessor Eina_Accessor;
    typedef unsigned char Eina_Bool;

    void           eina_accessor_free         (Eina_Accessor *accessor);
    Eina_Bool      eina_accessor_data_get     (Eina_Accessor *accessor,
                                               unsigned int   position,
                                               void         **data);
    Eina_Bool      eina_accessor_lock         (Eina_Accessor *accessor);
    Eina_Bool      eina_accessor_unlock       (Eina_Accessor *accessor);
    Eina_Accessor *eina_accessor_clone        (Eina_Accessor *accessor);
    void          *eina_accessor_container_get(Eina_Accessor *accessor);
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

ffi.metatype("Eina_Accessor", {
    __index = {
        free = function(self) eina.eina_accessor_free(ffi.gc(self, nil)) end,
        data_get = function(self, position)
            local data = ffi.new("void*[1]")
            local r = eina.eina_accessor_data_get(self, position - 1, data)
            if r == 0 then return nil end
            return data[0]
        end,
        lock   = function(self) return eina.eina_accessor_lock  (self) ~= 0 end,
        unlock = function(self) return eina.eina_accessor_unlock(self) ~= 0 end,
        clone  = function(self)
            local v = eina.eina_accessor_clone(self)
            if v == nil then return nil end
            return v
        end,
        container_get = function(self)
            local v = eina.eina_accessor_container_get(self)
            if v == nil then return nil end
            return v
        end
    }
})

cutil.init_module(init, shutdown)

local dgetmt = debug.getmetatable

M.Accessor = util.Readonly_Object:clone {
    __ctor = function(self, selfmt, acc)
        -- prevent null stuff
        if acc == nil then acc = nil end
        if acc then ffi.gc(acc, acc.free) end
        selfmt.__eq = function(self, other)
            return selfmt.__accessor == dgetmt(other).__accessor
        end
        selfmt.__call = function(self)
            return self:next()
        end
        local oi = selfmt.__index
        selfmt.__index = function(self, name)
            if type(name) == "number" then return self:data_get(name) end
            return oi[name]
        end
        selfmt.__accessor = acc
    end,

    free = function(self)
        self = dgetmt(self)
        if not self.__accessor then return end
        self.__accessor:free()
        self.__accessor = nil
    end,

    lock = function(self)
        self = dgetmt(self)
        if not self.__accessor then return false end
        return self.__accessor:lock()
    end,

    unlock = function(self)
        self = dgetmt(self)
        if not self.__accessor then return false end
        return self.__accessor:unlock()
    end,

    clone = function(self)
        self = dgetmt(self)
        if not self.__accessor then return nil end
        return self.__proto(self.__accessor:clone())
    end,

    data_get = function(self, pos)
        self = dgetmt(self)
        if not self.__accessor then return nil end
        return self.__accessor:data_get(pos)
    end,

    container_get = function(self)
        self = dgetmt(self)
        if not self.__accessor then return end
        return self.__accessor:container_get()
    end,

    each = function(self)
        local  acc = dgetmt(self).__accessor
        if not acc then return nil end
        local cnt = 0
        return function()
            cnt = cnt + 1
            local v = self:data_get(cnt)
            if v then return cnt, v end
        end
    end
}

return M
