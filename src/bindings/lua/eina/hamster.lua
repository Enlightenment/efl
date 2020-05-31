-- EFL LuaJIT bindings: Eina (hamster module)
-- For use with Elua

local ffi = require("ffi")

local tonum = ffi.tonumber or tonumber

ffi.cdef [[
    int eina_hamster_count(void);
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

M.count = function()
    return tonum(eina.eina_hamster_count())
end

return M
