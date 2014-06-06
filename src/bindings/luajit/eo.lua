-- EFL LuaJIT bindings: Eo
-- For use with Elua

local ffi = require("ffi")

ffi.cdef [[
    typedef unsigned char Eina_Bool;

    typedef struct _Eo_Opaque Eo;
    typedef Eo Eo_Class;

    Eina_Bool eo_init(void);
    Eina_Bool eo_shutdown(void);

    Eina_Bool eo_isa(const Eo *obj, const Eo_Class *klass);

    void eo_constructor(void);
    void eo_destructor(void);

    Eo *_eo_add_internal_start(const char *file, int line,
        const Eo_Class *klass_id, Eo *parent);

    Eina_Bool _eo_do_start(const Eo *obj, const Eo_Class *cur_klass,
        Eina_Bool is_super, const char *file, const char *func, int line);
    void      _eo_do_end  (const Eo **ojb);

    Eo *eo_finalize(void);

    void eo_parent_set(Eo *parent);
    Eo  *eo_parent_get(void);

    void eo_event_freeze(void);
    void eo_event_thaw(void);
    int eo_event_freeze_count_get(void);
    void eo_event_global_freeze(void);
    void eo_event_global_thaw(void);
    int eo_event_global_freeze_count_get(void);
]]

local cutil = require("cutil")
local util  = require("util")

local M = {}

local eo

local init = function()
    eo = util.lib_load("eo")
    eo.eo_init()
end

local shutdown = function()
    eo.eo_shutdown()
    util.lib_unload("eo")
end

cutil.init_module(init, shutdown)

local getinfo = debug.getinfo

local getfuncname = function(info)
    return info.name or "<" .. tostring(info.func) .. ">"
end

local classes = {}

M.class_get = function(name)
    return classes[name]
end

M.class_register = function(name, val, addr)
    classes[name] = val
    if addr then
        classes[tonumber(ffi.cast("intptr_t", addr))] = val
    end
end

M.__ctor_common = function(self, klass, parent, ctor, loff, ...)
    local info    = getinfo(2 + (loff or 0), "nlSf")
    local source  = info.source
    local func    = getfuncname(info)
    local line    = info.currentline
    local tmp_obj = eo._eo_add_internal_start(source, line, klass,
        parent.__obj)
    local retval
    if eo._eo_do_start(tmp_obj, nil, false, source, func, line) ~= 0 then
        if ctor then
            retval = ctor(...)
        else
            eo.eo_constructor()
        end
        tmp_obj = eo.eo_finalize()
        eo._eo_do_end(nil)
    end
    self.__obj    = tmp_obj
    self.__parent = parent
    return retval
end

M.__do_start = function(self, klass)
    if eo.eo_isa(self.__obj, klass) == 0 then
        error("method call on an invalid object", 3)
    end
    local info = getinfo(3, "nlSf")
    return eo._eo_do_start(self.__obj, nil, false, info.source,
        getfuncname(info), info.currentline) ~= 0
end

M.__do_end = function()
    eo._eo_do_end(nil) -- the parameter is unused and originally there
                       -- only for cleanup (dtor)
end

M.Eo_Base = util.Object:clone {
}

return M