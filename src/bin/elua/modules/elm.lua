local ffi = require("ffi")
local cutil = require("cutil")
local util = require("util")

local elm, evas

local M = {}

ffi.cdef [[
    typedef struct Evas_Object Evas_Object;
    typedef unsigned char Eina_Bool;

    typedef void(*Evas_Smart_Cb)(void *data, Evas_Object *obj, void *event_info);

    int elm_init(int argc, char **argv);
    int elm_exit(void);

    Evas_Object *elm_win_add(Evas_Object *parent, const char *name, int type);
    void elm_win_title_set(Evas_Object *obj, const char *title);
    void elm_win_resize_object_add(Evas_Object *obj, Evas_Object *subobj);

    Evas_Object *elm_bg_add(Evas_Object *parent);
    Evas_Object *elm_label_add(Evas_Object *parent);
    Evas_Object *elm_button_add(Evas_Object *parent);
    Evas_Object *elm_box_add(Evas_Object *parent);
    Evas_Object *elm_entry_add(Evas_Object *parent);
    void elm_box_pack_end(Evas_Object *obj, Evas_Object *subobj);
    void elm_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);
    void elm_entry_scrollable_set(Evas_Object *obj, Eina_Bool scroll);
    void elm_entry_entry_set(Evas_Object *obj, const char *entry);
    const char *elm_entry_entry_get(const Evas_Object *obj);
    void elm_object_part_text_set(Evas_Object *obj, const char *part, const char *text);
    void elm_object_focus_set(Evas_Object *obj, Eina_Bool focus);

    void evas_object_show(Evas_Object *obj);
    void evas_object_size_hint_weight_set(Evas_Object *obj, double x, double y);
    void evas_object_size_hint_align_set(Evas_Object *obj, double x, double y);
    void evas_object_resize(Evas_Object *obj, int w, int h);

    void evas_object_smart_callback_add(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data);
    void evas_object_smart_callback_del_full(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data);
]]

local callbacks = {}

local smart_cb_wrapper = ffi.cast("Evas_Smart_Cb",
    cutil.register_callbacks(callbacks))

cutil.init_module(function()
    elm, evas = ffi.load("elementary"), ffi.load("evas")
    elm.elm_init(0, nil)
end, function()
    elm.elm_exit()
end)

local Evas_Object = util.Object:clone {
    resize = function(self, w, h)
        evas.evas_object_resize(self.__o, w, h)
    end,
    show = function(self) evas.evas_object_show(self.__o) end,
    size_hint_weight_set = function(self, x, y)
        evas.evas_object_size_hint_weight_set(self.__o, x, y)
    end,
    size_hint_align_set = function(self, x, y)
        evas.evas_object_size_hint_align_set(self.__o, x, y)
    end,
    smart_callback_add = function(self, ev, cb)
        local  cbt = callbacks[cb]
        if not cbt then
            callbacks[#callbacks + 1] = cb
            cbt = { #callbacks, 0 }
            callbacks[cb] = cbt
        end
        cbt[2] = cbt[2] + 1
        evas.evas_object_smart_callback_add(self.__o, ev, smart_cb_wrapper,
            ffi.cast("void*", cbt[1]))
    end,
    smart_callback_del = function(self, ev, cb)
        local cbt = callbacks[cb]
        if not cbt then return end
        local cbi = cbt[1]
        evas.evas_object_smart_callback_del_full(self.__o, ev, smart_cb_wrapper,
            ffi.cast("void*", cbi))
        local cnt = cbt[2] - 1
        if cnt == 0 then
            callbacks[cbi], callbacks[cb] = nil, nil
        else
            cbt[2] = cnt
        end
    end
}

M.Window = Evas_Object:clone {
    __ctor = function(self, name, title)
        self.__o = elm.elm_win_add(nil, name, 0)
        elm.elm_win_title_set(self.__o, title)
    end,

    resize_object_add = function(self, o)
        elm.elm_win_resize_object_add(self.__o, o.__o)
    end
}

M.Background = Evas_Object:clone {
    __ctor = function(self, parent)
        self.__o = elm.elm_bg_add(parent.__o)
    end
}

M.Label = Evas_Object:clone {
    __ctor = function(self, parent)
        self.__o = elm.elm_label_add(parent.__o)
    end,

    text_set = function(self, label, part)
        elm.elm_object_part_text_set(self.__o, part, label)
    end
}

M.Button = Evas_Object:clone {
    __ctor = function(self, parent)
        self.__o = elm.elm_button_add(parent.__o)
    end,

    text_set = function(self, label, part)
        elm.elm_object_part_text_set(self.__o, part, label)
    end
}

M.Box = Evas_Object:clone {
    __ctor = function(self, parent)
        self.__o = elm.elm_box_add(parent.__o)
    end,

    pack_end = function(self, obj)
        elm.elm_box_pack_end(self.__o, obj.__o)
    end
}

M.Entry = Evas_Object:clone {
    __ctor = function(self, parent)
        self.__o = elm.elm_entry_add(parent.__o)
    end,

    single_line_set = function(self, v)
        elm.elm_entry_single_line_set(self.__o, v)
    end,

    scrollable_set = function(self, v)
        elm.elm_entry_scrollable_set(self.__o, v)
    end,

    focus_set = function(self, v)
        elm.elm_object_focus_set(self.__o, v)
    end,

    text_set = function(self, label, part)
        elm.elm_object_part_text_set(self.__o, part, label)
    end,

    entry_set = function(self, v)
        elm.elm_entry_entry_set(self.__o, v)
    end,

    entry_get = function(self, v)
        return elm.elm_entry_entry_get(self.__o)
    end
}

M.exit = function()
    elm.elm_exit()
end

_G["elm"] = M