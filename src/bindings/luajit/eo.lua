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
        const Eo_Class *klass_id, Eo *parent, Eina_Bool ref);

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

    const Eo_Class *eo_base_class_get(void);
]]

local addr_d = ffi.typeof("union { double d; const Eo_Class *p; }")
local eo_class_addr_get = function(x)
    local v = addr_d()
    v.p = x
    return tonumber(v.d)
end

local cutil = require("cutil")
local util  = require("util")

local M = {}

local eo

local classes = {}
local eo_classes = {}

local init = function()
    eo = util.lib_load("eo")
    eo.eo_init()
    local eocl = eo.eo_base_class_get()
    local addr = eo_class_addr_get(eocl)
    classes["Eo_Base"] = util.Object:clone {
        connect = function(self, ename, func)
            local ev = self.__events[ename]
            if not ev then
                error("invalid event '" .. ename .. "'", 2)
            end
            local cl = eo_classes["Eo_Base"]
            M.__do_start(self, cl)
            eo.eo_event_callback_priority_add(ev, 0,
                function(data, obj, desc, einfo)
                    return func(obj, einfo) ~= false
                end,
            nil)
            M.__do_end()
        end,

        __events = util.Object:clone {},
        __properties = util.Object:clone {}
    }
    classes[addr] = classes["Eo_Base"]
    eo_classes["Eo_Base"] = eocl
    eo_classes[addr] = eocl
end

local shutdown = function()
    M.class_unregister("Eo_Base")
    eo.eo_shutdown()
    util.lib_unload("eo")
end

local getinfo = debug.getinfo

local getfuncname = function(info)
    return info.name or "<" .. tostring(info.func) .. ">"
end

M.class_get = function(name)
    return classes[name]
end

M.eo_class_get = function(name)
    return eo_classes[name]
end

M.class_register = function(name, parent, body, eocl)
    parent = classes[parent]
    if body.__events then
        body.__events = parent.__events:clone(body.__events)
    end
    if body.__properties then
        body.__properties = parent.__properties:clone(body.__properties)
    end
    local addr = eo_class_addr_get(eocl)
    classes[name] = parent:clone(body)
    classes[addr] = classes[name]
    eo_classes[name] = eocl
    eo_classes[addr] = eocl
end

M.class_unregister = function(name)
    local addr = eo_class_addr_get(eo_classes[name])
    classes[name] = nil
    classes[addr] = nil
    eo_classes[name] = nil
    eo_classes[addr] = nil
end

local mixin_tbl = function(cl, mixin, field)
    local mxt = mixin[field]
    if mxt then
        local clt = cl[field]
        if not clt then
            cl[field] = mxt
        else
            for k, v in pairs(mxt) do clt[k] = v end
        end
        mixin[field] = nil
    end
end

M.class_mixin = function(name, mixin)
    local cl = classes[name]
    -- mixin properties/events
    mixin_tbl(cl, mixin, "__properties")
    mixin_tbl(cl, mixin, "__events")
    -- mixin the rest
    cl:mixin(classes[mixin])
end

local obj_gccb = function(obj)
    eo.eo_unref(obj)
end

M.__ctor_common = function(klass, parent, ctor, loff, ...)
    local info   = getinfo(2 + (loff or 0), "nlSf")
    local source = info.source
    local func   = getfuncname(info)
    local line   = info.currentline
    local ret    = eo._eo_add_internal_start(source, line, klass, parent, true)
    local retval
    if eo._eo_do_start(ret, nil, false, source, func, line) ~= 0 then
        eo.eo_constructor()
        if ctor then ctor(ret, ...) end
        ret = eo.eo_finalize()
        eo._eo_do_end(nil)
    end
    ffi.gc(ret, obj_gccb)
    return ret
end

M.__do_start = function(self, klass)
    if eo.eo_isa(self, klass) == 0 then
        error("method call on an invalid object", 3)
    end
    local info = getinfo(3, "nlSf")
    return eo._eo_do_start(self, nil, false, info.source,
        getfuncname(info), info.currentline) ~= 0
end

M.__do_end = function()
    eo._eo_do_end(nil) -- the parameter is unused and originally there
                       -- only for cleanup (dtor)
end

local get_obj_mt = function(obj)
    local cl = eo.eo_class_get(obj)
    if cl == nil then return nil end
    return classes[eo_class_addr_get(cl)]
end

local prop_proxy_meta = {
    __index = function(self, key)
        if self.nkeys > 1 and type(key) == "table" then
            if self.nvals > 1 then
                return { self(unpack(key)) }
            else
                return self(unpack(key))
            end
        else
            if self.nvals > 1 then
                return { self(key) }
            else
                return self(key)
            end
        end
    end,

    __newindex = function(self, key, val)
        local nkeys = self.nkeys
        if nkeys > 1 then
            -- ultra slow path, q66 failed optimizing this
            local atbl
            if type(key) == "table" then
                atbl = { unpack(key) }
            else
                atbl = { key }
            end
            if self.nvals > 1 and type(val) == "table" then
                for i, v in ipairs(val) do atbl[nkeys + i] = v end
            else
                atbl[nkeys + 1] = val
            end
            self.mt[self.key .. "_set"](self.obj, unpack(atbl))
        else
            if self.nvals > 1 and type(val) == "table" then
                -- somewhat less slow but still slow path
                self.mt[self.key .. "_set"](self.obj, key, unpack(val))
            else
                -- least slow, no unpacks, no temporaries, just proxy
                self.mt[self.key .. "_set"](self.obj, key, val)
            end
        end
    end,

    -- provides alt syntax for getters with keys
    __call = function(self, ...)
        return self.mt[self.key .. "_get"](self.obj, ...)
    end
}

ffi.metatype("Eo", {
    __index = function(self, key)
        local mt = get_obj_mt(self)
        if mt == nil then return nil end
        local pt = mt.__properties
        local pp = pt[key]
        if not pp then
            return mt[key]
        end
        if not pp[3] then
            error("property '" .. key .. "' is not gettable", 2)
        end
        local nkeys, nvals = pp[1], pp[2]
        if nkeys ~= 0 then
            -- proxy - slow path... TODO: find a better way
            return setmetatable({ nkeys = nkeys, nvals = nvals,
                obj = self, key = key, mt = mt }, prop_proxy_meta)
        end
        if nvals > 1 then
            return { mt[key .. "_get"](self) }
        else
            return mt[key .. "_get"](self)
        end
    end,

    __newindex = function(self, key, val)
        local mt = get_obj_mt(self)
        if mt == nil then return nil end
        local pt = mt.__properties
        local pp = pt[key]
        if not pp then
            error("no such property '" .. key .. "'", 2)
        end
        if not pp[4] then
            error("property '" .. key .. "' is not settable", 2)
        end
        if pp[1] ~= 0 then
            error("property '" .. key .. "' requires " .. pp[1] .. " keys", 2)
        end
        local nvals = pp[2]
        if nvals > 1 and type(val) == "table" then
            mt[key .. "_set"](self, unpack(val))
        else
            mt[key .. "_set"](self, val)
        end
    end
})

cutil.init_module(init, shutdown)

return M
