local ffi = require("ffi")
local util = erequire("util")

local elm, evas

local M = {}

ffi.cdef [[
    typedef struct Evas_Object Evas_Object;

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
    void elm_box_pack_end(Evas_Object *obj, Evas_Object *subobj);
    void elm_object_part_text_set(Evas_Object *obj, const char *part, const char *text);

    void evas_object_show(Evas_Object *obj);
    void evas_object_size_hint_weight_set(Evas_Object *obj, double x, double y);
    void evas_object_resize(Evas_Object *obj, int w, int h);

    void evas_object_smart_callback_add(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data);
    void evas_object_smart_callback_del_full(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data);
]]

local callbacks = {}

local smart_cb_wrapper = ffi.cast("Evas_Smart_Cb",
    util.register_callbacks(callbacks))

util.init_module(function()
    elm, evas = ffi.load("elementary"), ffi.load("evas")
    elm.elm_init(0, nil)
end, function()
    elm.elm_exit()
end)

local Evas_Object = {
    resize = function(self, w, h)
        evas.evas_object_resize(self.__o, w, h)
    end,
    show = function(self) evas.evas_object_show(self.__o) end,
    size_hint_weight_set = function(self, x, y)
        evas.evas_object_size_hint_weight_set(self.__o, x, y)
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

M.Window = function(name, title)
    local o = setmetatable({
        __o = elm.elm_win_add(nil, name, 0),
        resize_object_add = function(self, o)
            elm.elm_win_resize_object_add(self.__o, o.__o)
        end
    }, { __index = Evas_Object })
    elm.elm_win_title_set(o.__o, title)
    return o
end

M.Background = function(parent)
    return setmetatable({
        __o = elm.elm_bg_add(parent.__o)
    }, { __index = Evas_Object })
end

M.Label = function(parent)
    return setmetatable({
        __o = elm.elm_label_add(parent.__o),
        text_set = function(self, label)
            elm.elm_object_part_text_set(self.__o, nil, label)
        end
    }, { __index = Evas_Object })
end

M.Button = function(parent)
    return setmetatable({
        __o = elm.elm_button_add(parent.__o),
        text_set = function(self, label)
            elm.elm_object_part_text_set(self.__o, nil, label)
        end
    }, { __index = Evas_Object })
end

M.Box = function(parent)
    return setmetatable({
        __o = elm.elm_box_add(parent.__o),
        pack_end = function(self, obj)
            elm.elm_box_pack_end(self.__o, obj.__o)
        end
    }, { __index = Evas_Object })
end

M.exit = function()
    elm.elm_exit()
end

_G["elm"] = M