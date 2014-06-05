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

M.class_register = function(name, val)
    classes[name] = val
end

M.Eo_Base = util.Object:clone {
    __ctor_common = function(self, klass, parent, ctor, loff, ...)
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
    end,

    __do_start = function(self, klass)
        if eo.eo_isa(self.__obj, klass) == 0 then
            error("method call on an invalid object", 3)
        end
        local info = getinfo(3, "nlSf")
        return eo._eo_do_start(self.__obj, nil, false, info.source,
            getfuncname(info), info.currentline) ~= 0
    end,

    __do_end = function(self)
        eo._eo_do_end(nil) -- the parameter is unused and originally there
                           -- only for cleanup (dtor)
    end,

    parent_get = function(self)
        return self.__parent
    end,

    parent_set = function(self, parent)
        self:__do_start()
        self.__parent = parent
        eo.eo_parent_set(parent.__obj)
        self:__do_end()
    end,

    event_global_freeze_count_get = function(self)
        self:__do_start()
        local v = eo.eo_event_global_freeze_count_get()
        self:__do_end()
        return v
    end,

    event_freeze_count_get = function(self)
        self:__do_start()
        local v = eo.eo_event_freeze_count_get()
        self:__do_end()
        return v
    end
}

--[[
   methods {
      event_callback_forwarder_del {
         /*@ Remove an event callback forwarder for an event and an object. */
         params {
            @in const Eo_Event_Description* desc; /*@ The description of the event to listen to */
            @in Eo* new_obj; /*@ The object to emit events from */
         }
      }
      event_thaw {
         /*@ thaw events of object.
Lets event callbacks be called for the object. */
      }
      event_freeze {
         /*@ freeze events of object.
Prevents event callbacks from being called for the object. */
      }
      event_global_freeze {
         /*@ freeze events of object.
Prevents event callbacks from being called for the object. */
      }
      event_callback_array_del {
         /*@ Del a callback array with a specific data associated to it for an event. */
         params {
            @in const Eo_Callback_Array_Item* array; /*@ an #Eo_Callback_Array_Item of events to listen to */
            @in const void* user_data; /*@ The data to compare */
         }
      }
      wref_del {
         /*@ Delete the weak reference passed. */
         params {
            @inout Eo** wref;
         }
      }
      destructor {
         /*@ Call the object's destructor.
Should not be used with #eo_do. Only use it with #eo_do_super. */
      }
      key_data_set {
         /*@ Set generic data to object. */
         params {
            @in const char* key; /*@ the key associated with the data */
            @in const void* data; /*@ the data to set */
            @in eo_key_data_free_func free_func; /*@ the func to free data with (NULL means */
         }
      }
      key_data_get {
         /*@ Get generic data from object. */
         params {
            @in const char* key; /*@ the key associated with the data */
         }
         return void *; /* the data for the key */
      }
      event_callback_del {
         /*@ Del a callback with a specific data associated to it for an event. */
         params {
            @in const Eo_Event_Description* desc; /*@ The description of the event to listen to */
            @in Eo_Event_Cb func; /*@ the callback to delete */
            @in const void* user_data; /*@ The data to compare */
         }
      }
      event_global_thaw {
         /*@ thaw events of object.
Lets event callbacks be called for the object. */
      }
      key_data_del {
         /*@ Del generic data from object. */
         params {
            @in const char* key; /*@ the key associated with the data */
         }
      }
      event_callback_array_priority_add {
         /*@ Add a callback array for an event with a specific priority.
callbacks of the same priority are called in reverse order of creation. */
         params {
            @in const Eo_Callback_Array_Item* array; /*@ an #Eo_Callback_Array_Item of events to listen to */
            @in Eo_Callback_Priority priority; /*@ The priority of the callback */
            @in const void* data; /*@ additional data to pass to the callback */
         }
      }
      wref_add {
         /*@ Add a new weak reference to obj.
This function registers the object handle pointed by wref to obj so when obj is deleted it'll be updated to NULL. This functions should be used when you want to keep track of an object in a safe way, but you don't want to prevent it from being freed. */
         params {
            @inout Eo** wref;
         }
      }
      dbg_info_get {
         /*@ Get dbg information from the object. */
         params {
            @in Eo_Dbg_Info* root_node; /*@ node of the tree */
         }
      }
      event_callback_forwarder_add {
         /*@ Add an event callback forwarder for an event and an object. */
         params {
            @in const Eo_Event_Description* desc; /*@ The description of the event to listen to */
            @in Eo* new_obj; /*@ The object to emit events from */
         }
      }
      event_callback_call {
         /*@ Call the callbacks for an event of an object. */
         params {
            @in const Eo_Event_Description* desc; /*@ The description of the event to call */
            @in const void* event_info; /*@ Extra event info to pass to the callbacks */
         }
         return Eina_Bool; /* @c EINA_TRUE if one of the callbacks aborted the call, @c EINA_FALSE otherwise */
      }
      event_callback_priority_add {
         /*@ Add a callback for an event with a specific priority.
callbacks of the same priority are called in reverse order of creation. */
         params {
            @in const Eo_Event_Description* desc; /*@ The description of the event to listen to */
            @in Eo_Callback_Priority priority; /*@ The priority of the callback */
            @in Eo_Event_Cb cb; /*@ the callback to call */
            @in const void* data; /*@ additional data to pass to the callback */
         }
      }
      children_iterator_new {
         /*@ Get an iterator on all childrens */
         params {
            @inout Eina_Iterator** it;
         }
      }
   }
   events {
      callback,add; /*@ A callback was added. */
      callback,del; /*@ A callback was deleted. */
      del; /*@ Obj is being deleted. */
   }
}
]]

return M