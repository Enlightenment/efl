-- EFL LuaJIT bindings: Eo
-- For use with Elua

local ffi = require("ffi")

ffi.cdef [[
    typedef unsigned char Eina_Bool;
    typedef struct _Eina_Iterator Eina_Iterator;

    typedef struct _Eo_Opaque Eo;
    typedef Eo Eo_Class;

    typedef short Eo_Callback_Priority;

    typedef void (*eo_key_data_free_func)(void *);

    struct _Eo_Event_Description {
        const char *name;
        const char *doc;
        Eina_Bool   unfreezable;
    };
    typedef struct _Eo_Event_Description Eo_Event_Description;

    typedef Eina_Bool (*Eo_Event_Cb)(void *data, Eo *obj,
        const Eo_Event_Description *desc, void *event_info);

    struct _Eo_Callback_Array_Item {
        const Eo_Event_Description *desc;
        Eo_Event_Cb func;
    };
    typedef struct _Eo_Callback_Array_Item Eo_Callback_Array_Item;

    Eina_Bool eo_init(void);
    Eina_Bool eo_shutdown(void);

    Eina_Bool eo_isa(const Eo *obj, const Eo_Class *klass);

    const char *eo_class_name_get(const Eo_Class *klass);

    void eo_constructor(void);
    void eo_destructor(void);

    Eo *_eo_add_internal_start(const char *file, int line,
        const Eo_Class *klass_id, Eo *parent);

    Eina_Bool _eo_do_start(const Eo *obj, const Eo_Class *cur_klass,
        Eina_Bool is_super, const char *file, const char *func, int line);
    void      _eo_do_end  (const Eo **ojb);

    const Eo_Class *eo_class_get(const Eo *obj);

    void *eo_data_xref_internal(const char *file, int line, const Eo *obj,
        const Eo_Class *klass, const Eo *ref_obj);
    void eo_data_xunref_internal(const Eo *obj, void *data, const Eo *ref_obj);
    Eo *eo_xref_internal(const char *file, int line, Eo *obj, const Eo *ref_obj);
    void eo_xunref(Eo *obj, const Eo *ref_obj);
    Eo *eo_ref(const Eo *obj);
    void eo_unref(const Eo *obj);
    int eo_ref_get(const Eo *obj);
    void eo_wref_add(Eo **wref);
    void eo_wref_del(Eo **wref);

    void eo_del(const Eo *obj);

    void eo_manual_free_set(Eo *obj, Eina_Bool manual_free);
    Eina_Bool eo_manual_free(Eo *obj);
    Eina_Bool eo_destructed_is(const Eo *obj);

    Eina_Bool eo_composite_attach(Eo *comp_obj, Eo *parent);
    Eina_Bool eo_composite_detach(Eo *comp_obj, Eo *parent);
    Eina_Bool eo_composite_is(const Eo *comp_obj);

    void eo_parent_set(Eo *parent);
    Eo  *eo_parent_get(void);

    void eo_event_freeze(void);
    void eo_event_thaw(void);
    int eo_event_freeze_count_get(void);
    void eo_event_global_freeze(void);
    void eo_event_global_thaw(void);
    int eo_event_global_freeze_count_get(void);

    Eina_Bool eo_finalized_get(void);
    Eo *eo_finalize(void);

    void eo_event_callback_forwarder_add(const Eo_Event_Description *desc,
        Eo *new_obj);
    void eo_event_callback_forwarder_del(const Eo_Event_Description *desc,
        Eo *new_obj);
    void  eo_event_callback_priority_add(const Eo_Event_Description *desc,
        Eo_Callback_Priority priority, Eo_Event_Cb cb, const void *data);
    void eo_event_callback_del(const Eo_Event_Description *desc,
        Eo_Event_Cb func, const void *user_data);
    void eo_event_callback_array_priority_add(const Eo_Callback_Array_Item *array,
        Eo_Callback_Priority priority, const void *data);
    void eo_event_callback_array_del(const Eo_Callback_Array_Item *array,
        const void *user_data);
    Eina_Bool eo_event_callback_call(const Eo_Event_Description *desc,
        void *event_info);

    void eo_key_data_set(const char *key, const void *data,
        eo_key_data_free_func free_func);
    void *eo_key_data_get(const char *key);
    void  eo_key_data_del(const char *key);

    Eina_Iterator *eo_children_iterator_new(void);
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

M.class_register = function(name, val)
    classes[name] = val
    return val
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

ffi.metatype("Eo", {
    __index = function(self, key)
        local cl = eo.eo_class_get(self)
        if cl == nil then return nil end
        local nm = eo.eo_class_name_get(cl)
        if nm == nil then return nil end
        local mt == classes[ffi.string(nm)]
        if mt == nil then return nil end
        return mt[key]
    end
})

M.Base = util.Object:clone {
}
M.class_register("Eo.Base", M.Base)

return M
