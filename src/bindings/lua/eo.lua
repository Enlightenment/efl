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

    Eina_Iterator *eo_children_iterator_new(void);

    const Eo_Class *eo_base_class_get(void);

    extern const Eo_Event_Description _EO_BASE_EVENT_DEL;
]]

local tonum = ffi.tonumber or tonumber

local addr_d = ffi.typeof("union { double d; const Eo *p; }")
local eo_obj_addr_get = function(x)
    local v = addr_d()
    v.p = x
    return tonum(v.d)
end

local cutil = require("cutil")
local util  = require("util")

local M = {}

local eo

local classes = {}
local eo_classes = {}

-- event system

local eo_callbacks, eo_cbidcache = {}, {}

local eo_event_del, eo_event_cb

local eo_event_del_fun = function(data, obj, desc, einfo)
    local addr = eo_obj_addr_get(obj)
    if  eo_callbacks[addr] then
        eo_callbacks[addr], eo_cbidcache[addr] = nil, nil
    end
end

local eo_event_cb_fun = function(data, obj, desc, einfo)
    local  addr = eo_obj_addr_get(obj)
    local  cbs  = eo_callbacks[addr]
    assert(cbs)
    local cidx = tonum(ffi.cast("intptr_t", data))
    local fun  = cbs[cidx]
    assert(fun)
    return fun() ~= false
end

local connect = function(self, ename, func, priority)
    local ev = self.__events[ename]
    if not ev then
        return false, "attempt to connect an invalid event '" .. ename .. "'"
    end
    local cl = eo_classes["Eo_Base"]
    -- add the callback to the respective array
    local cdel = false
    local addr = eo_obj_addr_get(self)
    local  cbs = eo_callbacks[addr]
    local  cbi = eo_cbidcache[addr]
    if not cbs then
        cbs, cbi = {}, {}
        eo_callbacks[addr], eo_cbidcache[addr] = cbs, cbi
        cdel = true
    end
    local cidx = #cbs + 1
    cbs[cidx], cbi[func] = func, cidx
    M.__do_start(self, cl)
    eo.eo_event_callback_priority_add(ev, priority or 0,
        eo_event_cb, ffi.cast("void *", cidx))
    eo.eo_event_callback_priority_add(eo._EO_BASE_EVENT_DEL, 0, eo_event_del,
        nil)
    M.__do_end()
    return true
end

local disconnect = function(self, ename, func)
    local ev = self.__events[ename]
    if not ev then
        return false, "attempt to disconnect an invalid event '" .. ename .. "'"
    end
    local cl = eo_classes["Eo_Base"]
    -- like connect, but the other way around
    local addr = eo_obj_addr_get(self)
    local  cbs = eo_callbacks[addr]
    if not cbs then
        return false
    end
    local  cbi = eo_cbidcache[addr]
    assert(cbi)
    local cidx = cbi[func]
    if not cidx then
        return false
    end
    cbs[cidx] = nil
    cbi[func] = nil
    M.__do_start(self, cl)
    eo.eo_event_callback_del(ev, eo_event_cb, ffi.cast("void *", cidx))
    M.__do_end()
    return true
end

local init = function()
    eo = util.lib_load("eo")
    eo.eo_init()
    local eocl = eo.eo_base_class_get()
    local addr = eo_obj_addr_get(eocl)
    classes["Eo_Base"] = util.Object:clone {
        connect = connect,
        disconnect = disconnect,
        __events = util.Object:clone {},
        __properties = util.Object:clone {}
    }
    classes[addr] = classes["Eo_Base"]
    eo_classes["Eo_Base"] = eocl
    eo_classes[addr] = eocl
    eo_event_del = ffi.cast("Eo_Event_Cb", eo_event_del_fun)
    eo_event_cb  = ffi.cast("Eo_Event_Cb", eo_event_cb_fun)
end

local shutdown = function()
    classes, eo_classes = {}, {}
    eo.eo_shutdown()
    eo_event_del:free()
    eo_event_cb:free()
    eo_event_del = nil
    eo_event_cb  = nil
    eo_callbacks = {}
    eo_cbidcache = {}
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

local inherit_meta = function(body, field, parents, mixins)
    local o = body[field]
    if o then
        o = parents[1][field]:clone(o)
        for i = 2, #parents do o:add_parent(parents[i][field]) end
        for i = 1, #mixins  do o:add_mixin (mixins [i][field]) end
        body[field] = o
    end
end

M.class_register = function(name, parents, mixins, body, eocl)
    -- map given names to objects
    local pars = {}
    for i = 1, #parents do pars[i] = classes[parents[i]] end
    -- for mixins, we need to check if it hasn't already been mixed in
    -- in some parent (doesn't matter how deep down the tree), because
    -- if it has, we need to skip it (for proper inheritance lookup order)
    local mins = {}
    local midx = 1
    for i = 1, #mixins do
        local mixin = mixins[i]
        local ck, hasmi = "__mixin_" .. mixin, false
        if mixin[ck] then
            for i = 1, #pars do
                if pars[i][ck] then
                    hasmi = true
                    break
                end
            end
        end
        if not hasmi then
            mins[midx] = mixin
            midx       = midx + 1
        end
    end

    inherit_meta(body, "__events"    , pars, mins)
    inherit_meta(body, "__properties", pars, mins)

    local lcl = pars[1]:clone(body)
    for i = 2, #pars do lcl:add_parent(pars[i]) end
    for i = 1, #mins do lcl:add_mixin (mins[i]) end

    local addr = eo_obj_addr_get(eocl)
    classes   [name], classes   [addr] = lcl , lcl
    eo_classes[name], eo_classes[addr] = eocl, eocl
end

M.class_unregister = function(name)
    local addr = eo_obj_addr_get(eo_classes[name])
    classes   [name], classes   [addr] = nil
    eo_classes[name], eo_classes[addr] = nil
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
    if cl == ffi.nullptr then return nil end
    return classes[eo_obj_addr_get(cl)]
end

local prop_proxy_meta = {
    __index = function(self, key)
        local ngkeys = self.ngkeys
        if ngkeys <= 0 then
            error("property '" .. self.key .. "' has no getter keys", 2)
        end
        if ngkeys > 1 and type(key) == "table" then
            if self.ngvals > 1 then
                return { self(unpack(key)) }
            else
                return self(unpack(key))
            end
        else
            if self.ngvals > 1 then
                return { self(key) }
            else
                return self(key)
            end
        end
    end,

    __newindex = function(self, key, val)
        local nskeys = self.nskeys
        if nskeys <= 0 then
            error("property '" .. self.key .. "' has no setter keys", 2)
        end
        if nskeys > 1 then
            -- ultra slow path, q66 failed optimizing this
            -- if you ever get to touch this, increment this
            -- counter to let others know you failed too.
            --
            -- failures: 1
            --
            -- fortunately this one is not very commonly used.
            local atbl
            if type(key) == "table" then
                atbl = { unpack(key) }
            else
                atbl = { key }
            end
            if self.nsvals > 1 and type(val) == "table" then
                for i, v in ipairs(val) do atbl[nskeys + i] = v end
            else
                atbl[nskeys + 1] = val
            end
            self.mt[self.key .. "_set"](self.obj, unpack(atbl))
        else
            if self.nsvals > 1 and type(val) == "table" then
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
    end,

    -- locks out the proxy
    __metatable = false
}

-- each __properties field looks like this:
--
-- { NUM_GET_KEYS, NUM_SET_KEYS, NUM_GET_VALS, NUM_SET_VALS, GETTABLE, SETTABLE }
--
-- the last two are booleans (determining if the property can be get and set).
ffi.metatype("Eo", {
    -- handles property getting with no keys and also properties with keys
    __index = function(self, key)
        local mt = get_obj_mt(self)
        if not mt then return nil end
        local pt = mt.__properties
        local pp = pt[key]
        if not pp then
            return mt[key]
        end
        local ngkeys, nskeys = pp[1], pp[2]
        if ngkeys ~= 0 or nskeys ~= 0 then
            -- proxy - slow path, but no way around it
            -- basically the proxy is needed because we want nice syntax and
            -- lua can't do it by default. so we help ourselves a bit with this
            return setmetatable({ ngkeys = ngkeys, nskeys = nskeys,
                ngvals = pp[3], nsvals = pp[4], obj = self, key = key,
                mt = mt }, prop_proxy_meta)
        end
        if not pp[5] then
            error("property '" .. key .. "' is not gettable", 2)
        end
        if pp[3] > 1 then
            return { mt[key .. "_get"](self) }
        else
            return mt[key .. "_get"](self)
        end
    end,

    -- handles property setting with no keys
    __newindex = function(self, key, val)
        local mt = get_obj_mt(self)
        if not mt then return nil end
        local pt = mt.__properties
        local pp = pt[key]
        if not pp then
            error("no such property '" .. key .. "'", 2)
        end
        if not pp[6] then
            error("property '" .. key .. "' is not settable", 2)
        end
        local nkeys = pp[2]
        if nkeys ~= 0 then
            error("property '" .. key .. "' requires " .. nkeys .. " keys", 2)
        end
        local nvals = pp[4]
        if nvals > 1 and type(val) == "table" then
            mt[key .. "_set"](self, unpack(val))
        else
            mt[key .. "_set"](self, val)
        end
    end
})

cutil.init_module(init, shutdown)

return M
