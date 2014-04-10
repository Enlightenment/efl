-- EFL LuaJIT bindings: Eina (counter module)
-- For use with Elua

local ffi = require("ffi")
local C = ffi.C

ffi.cdef [[
    int eina_hamster_count(void);

    typedef struct _Eina_Counter Eina_Counter;
    Eina_Counter *eina_counter_new  (const char   *name);
    void          eina_counter_free (Eina_Counter *counter);
    void          eina_counter_start(Eina_Counter *counter);
    void          eina_counter_stop (Eina_Counter *counter, int specimen);
    char         *eina_counter_dump (Eina_Counter *counter);

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

M.Counter = ffi.metatype("Eina_Counter", {
    __new = function(self, name)
        return ffi.gc(eina.eina_counter_new(name), self.free)
    end,
    __index = {
        free = function(self)
            return eina.eina_counter_free(ffi.gc(self, nil))
        end,

        start = function(self)
            return eina.eina_counter_start(self)
        end,

        stop = function(self, specimen)
            return eina.eina_counter_stop(self, specimen)
        end,

        dump = function(self)
            local v = eina.eina_counter_dump(self)
            if v == nil then return nil end
            local r = ffi.string(v)
            C.free(v)
            return r
        end
    }
})

return M