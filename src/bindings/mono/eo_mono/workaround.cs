
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct ClassDescription
{
    public uint version;
    [MarshalAs(UnmanagedType.LPStr)] public String name;
    public int class_type;
    public UIntPtr data_size;
    public IntPtr class_initializer;
    public IntPtr class_constructor;
    public IntPtr class_destructor;
}

[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct Efl_Op_Description
{
    public IntPtr api_func;
    public IntPtr func;
}

[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct Efl_Object_Ops
{
    public IntPtr descs; /**< The op descriptions array of size count. */
    public UIntPtr count; /**< Number of op descriptions. */
};

[StructLayout(LayoutKind.Sequential)]
public struct EolianPD
{
    public IntPtr pointer;
}

#pragma warning disable 0169
public struct Evas_Object_Box_Layout
{
    IntPtr o;
    IntPtr priv;
    IntPtr user_data;
};
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct Evas_Object_Box_Data
{
}
public delegate void Eina_Free_Cb(IntPtr data);
public struct Evas_Object_Box_Option {
    IntPtr obj;
    [MarshalAsAttribute(UnmanagedType.U1)] bool max_reached;
    [MarshalAsAttribute(UnmanagedType.U1)] bool min_reached;
    evas.Coord alloc_size;
};
#pragma warning restore 0169

namespace efl { namespace kw_event {

[StructLayout(LayoutKind.Sequential)]
public struct Description {
    IntPtr pointer; // Opaque type, just pass the pointer. What about hot/freeze/etc?

    private static Dictionary<string, IntPtr> descriptions = new Dictionary<string, IntPtr>();

    public Description(string name)
    {
        if (!descriptions.ContainsKey(name))
        {
            IntPtr data = efl.eo.Globals.dlsym(efl.eo.Globals.RTLD_DEFAULT, name);

            if (data == IntPtr.Zero) {
                string error = eina.StringConversion.NativeUtf8ToManagedString(efl.eo.Globals.dlerror());
                throw new Exception(error);
            }
            descriptions.Add(name, data);
        }

        this.pointer = descriptions[name];
    }
};

} // namespace kw_event


public delegate void Event_Cb(System.IntPtr data, ref Event evt);
#pragma warning disable 0169
public struct Callback_Array_Item {
    IntPtr desc;
    IntPtr func;
};
public struct Dbg_Info {
    IntPtr name;
    IntPtr value;
};
#pragma warning restore 0169

namespace text {

namespace cursor {

[StructLayout(LayoutKind.Sequential)]
public struct Cursor {
    IntPtr obj;
    UIntPtr pos; // UIntPtr to automatically change size_t between 32/64
    IntPtr node;
    [MarshalAsAttribute(UnmanagedType.U1)]bool changed;
}

} // namespace cursor

namespace annotate {

[StructLayout(LayoutKind.Sequential)]
public struct Annotation {
    IntPtr list;
    IntPtr obj;
    IntPtr start_node;
    IntPtr end_node;
    [MarshalAsAttribute(UnmanagedType.U1)]bool is_item;
}

} // namespace annotate

} // namespace text

public delegate void Signal_Cb(IntPtr data, IntPtr obj, IntPtr emission, IntPtr source);


namespace gfx {

public interface Buffer {}

namespace buffer {
public struct Access_Mode {}
}
        
} // namespace gfx

namespace access {

public struct State_Set {
    private ulong mask;

    public static implicit operator State_Set(ulong x)
    {
        return new State_Set{mask=x};
    }
    public static implicit operator ulong(State_Set x)
    {
        return x.mask;
    }
}

public struct Relation_Set {
    private IntPtr mask;

    public static implicit operator Relation_Set(IntPtr x)
    {
        return new Relation_Set{mask=x};
    }
    public static implicit operator IntPtr(Relation_Set x)
    {
        return x.mask;
    }
}

public struct Action_Data {
    public IntPtr name;
    public IntPtr action;
    public IntPtr param;
    public IntPtr func;
}

} // namespace access

namespace font {

public struct Size {
    private int mask;

    public static implicit operator Size(int x)
    {
        return new Size{mask=x};
    }
    public static implicit operator int(Size x)
    {
        return x.mask;
    }
}

}

} // namespace efl

namespace evas { namespace font {

}


// C# does not allow typedefs, so we use these implicit conversions.
public struct Modifier_Mask {
    private ulong mask;

    public static implicit operator Modifier_Mask(ulong x)
    {
        return new Modifier_Mask{mask=x};
    }
    public static implicit operator ulong(Modifier_Mask x)
    {
        return x.mask;
    }
}

public struct Coord {
    int val;

    public Coord(int value) { val = value; }
    static public implicit operator Coord(int val) {
        return new Coord(val);
    }
    static public implicit operator int(Coord coord) {
        return coord.val;
    }
}


}

public struct Efl_Font_Size {
    int val;

    public Efl_Font_Size(int value) { val = value; }
    static public implicit operator Efl_Font_Size(int val) {
        return new Efl_Font_Size(val);
    }
    static public implicit operator int(Efl_Font_Size coord) {
        return coord.val;
    }
}

namespace eina {

public struct Rectangle {
    public int x;
    public int y;
    public int w;
    public int h;
}

}

namespace eina {

public interface File {}

}

namespace evas {

/* Copied from Evas_Legacy.h */
public enum Text_Style_Type
{
   Plain = 0, /**< plain, standard text */
   Shadow, /**< text with shadow underneath */
   Outline, /**< text with an outline */
   SoftOutline, /**< text with a soft outline */
   Glow, /**< text with a glow effect */
   OutlineShadow, /**< text with both outline and shadow effects */
   FarShadow, /**< text with (far) shadow underneath */
   OutlineSoftShadow, /**< text with outline and soft shadow effects combined */
   SoftShadow, /**< text with (soft) shadow underneath */
   FarSoftShadow, /**< text with (far soft) shadow underneath */

   // Shadow direction modifiers
   ShadowDirectionBottomRight = 0 /* 0 >> 4 */, /**< shadow growing to bottom right */
   ShadowDirectionBottom= 16 /* 1 >> 4 */, /**< shadow growing to the bottom */
   ShadowDirectionBottomLeft = 32 /* 2 >> 4 */, /**< shadow growing to bottom left */
   ShadowDirectionLeft = 48 /* 3 >> 4 */, /**< shadow growing to the left */
   ShadowDirectionTopLeft = 64 /* 4 >> 4 */, /**< shadow growing to top left */
   ShadowDirectionTop = 80 /* 5 >> 4 */, /**< shadow growing to the top */
   ShadowDirectionTopRight = 96 /* 6 >> 4 */, /**< shadow growing to top right */
   ShadowDirectionRight = 112 /* 7 >> 4 */ /**< shadow growing to the right */
};
    

// Copied from Evas_Common.h
//
//
//

public enum Callback_Type
{
  EVAS_CALLBACK_MOUSE_IN = 0, /**< Mouse In Event */
  EVAS_CALLBACK_MOUSE_OUT, /**< Mouse Out Event */
  EVAS_CALLBACK_MOUSE_DOWN, /**< Mouse Button Down Event */
  EVAS_CALLBACK_MOUSE_UP, /**< Mouse Button Up Event */
  EVAS_CALLBACK_MOUSE_MOVE, /**< Mouse Move Event */
  EVAS_CALLBACK_MOUSE_WHEEL, /**< Mouse Wheel Event */
  EVAS_CALLBACK_MULTI_DOWN, /**< Multi-touch Down Event */
  EVAS_CALLBACK_MULTI_UP, /**< Multi-touch Up Event */
  EVAS_CALLBACK_MULTI_MOVE, /**< Multi-touch Move Event */
  EVAS_CALLBACK_FREE, /**< Object Being Freed (Called after Del) */
  EVAS_CALLBACK_KEY_DOWN, /**< Key Press Event */
  EVAS_CALLBACK_KEY_UP, /**< Key Release Event */
  EVAS_CALLBACK_FOCUS_IN, /**< Focus In Event */
  EVAS_CALLBACK_FOCUS_OUT, /**< Focus Out Event */
  EVAS_CALLBACK_SHOW, /**< Show Event */
  EVAS_CALLBACK_HIDE, /**< Hide Event */
  EVAS_CALLBACK_MOVE, /**< Move Event */
  EVAS_CALLBACK_RESIZE, /**< Resize Event */
  EVAS_CALLBACK_RESTACK, /**< Restack Event */
  EVAS_CALLBACK_DEL, /**< Object Being Deleted (called before Free) */
  EVAS_CALLBACK_HOLD, /**< Events go on/off hold */
  EVAS_CALLBACK_CHANGED_SIZE_HINTS, /**< Size hints changed event */
  EVAS_CALLBACK_IMAGE_PRELOADED, /**< Image has been preloaded */
  EVAS_CALLBACK_CANVAS_FOCUS_IN, /**< Canvas got focus as a whole */
  EVAS_CALLBACK_CANVAS_FOCUS_OUT, /**< Canvas lost focus as a whole */
  EVAS_CALLBACK_RENDER_FLUSH_PRE, /**< Called after render update regions have
                                   * been calculated, but only if update regions exist */
  EVAS_CALLBACK_RENDER_FLUSH_POST, /**< Called after render update regions have
                                    * been sent to the display server, but only
                                    * if update regions existed for the most recent frame */
  EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, /**< Canvas object got focus */
  EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT, /**< Canvas object lost focus */
  EVAS_CALLBACK_IMAGE_UNLOADED, /**< Image data has been unloaded (by some mechanism in Evas that throw out original image data) */
  EVAS_CALLBACK_RENDER_PRE, /**< Called just before rendering starts on the canvas target. @since 1.2 */
  EVAS_CALLBACK_RENDER_POST, /**< Called just after rendering stops on the canvas target. @since 1.2 */
  EVAS_CALLBACK_IMAGE_RESIZE, /**< Image size is changed. @since 1.8 */
  EVAS_CALLBACK_DEVICE_CHANGED, /**< Devices added, removed or changed on canvas. @since 1.8 */
  EVAS_CALLBACK_AXIS_UPDATE, /**< Input device changed value on some axis. @since 1.13 */
  EVAS_CALLBACK_CANVAS_VIEWPORT_RESIZE, /**< Canvas viewport resized. @since 1.15 */
  EVAS_CALLBACK_LAST /**< Sentinel value to indicate last enum field during
                      * iteration */
};

}

namespace elm {

namespace atspi {
public struct State_Set {
    private ulong val;

    public static implicit operator State_Set(ulong x)
    {
        return new State_Set{val=x};
    }
    public static implicit operator ulong(State_Set x)
    {
        return x.val;
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct Relation_Set
{
    public IntPtr pointer; // list<ptr(elm.atspi.Relation)>
}
} // namespace atspi

namespace gengrid { namespace item {
[StructLayout(LayoutKind.Sequential)]
public struct Class
{
    int version;
    uint refcount;
    [MarshalAsAttribute(UnmanagedType.U1)]bool delete_me;
    IntPtr item_style;
    IntPtr decorate_item_style;
    IntPtr decorate_all_item_style;

    // Delegates inside Elm_Gen_Item_Class_Functions
    IntPtr text_get;
    IntPtr content_get;
    IntPtr state_get;
    IntPtr del;
    IntPtr filter_get;
    IntPtr reusable_content_get;
}
} // namespace item
} // namespace gengrid

namespace genlist { namespace item {
[StructLayout(LayoutKind.Sequential)]
public struct Class
{
    int version;
    uint refcount;
    [MarshalAsAttribute(UnmanagedType.U1)]bool delete_me;
    IntPtr item_style;
    IntPtr decorate_item_style;
    IntPtr decorate_all_item_style;

    // Delegates inside Elm_Gen_Item_Class_Functions
    IntPtr text_get;
    IntPtr content_get;
    IntPtr state_get;
    IntPtr del;
    IntPtr filter_get;
    IntPtr reusable_content_get;
}
} // namespace item
} // namespace genlist

} // namespace elm

// Global delegates
public delegate IntPtr list_data_get_func_type(IntPtr l);
public delegate void region_hook_func_type(IntPtr data, IntPtr obj);
public delegate void slider_freefunc_type(IntPtr data);
public delegate void slider_func_type(double val);

public delegate int Eina_Compare_Cb(IntPtr a, IntPtr b);
public delegate void Elm_Interface_Scrollable_Cb(IntPtr obj, IntPtr data);
public delegate void Elm_Interface_Scrollable_Min_Limit_Cb(IntPtr obj,
                                                     [MarshalAsAttribute(UnmanagedType.U1)]bool w,
                                                     [MarshalAsAttribute(UnmanagedType.U1)]bool h);
public delegate void Elm_Interface_Scrollable_Resize_Cb(IntPtr obj, evas.Coord w, evas.Coord h);
public delegate void Elm_Entry_Item_Provider_Cb(IntPtr data, IntPtr obj, IntPtr item);
public delegate void Elm_Entry_Filter_Cb(IntPtr data, IntPtr obj, IntPtr text);
[return: MarshalAsAttribute(UnmanagedType.U1)]
public delegate bool Elm_Multibuttonentry_Item_Filter_Cb(IntPtr obj, IntPtr item_label, IntPtr item_data, IntPtr data);
public delegate IntPtr Elm_Multibuttonentry_Format_Cb(int count, IntPtr data);
[return: MarshalAsAttribute(UnmanagedType.U1)]
public delegate bool Elm_Fileselector_Filter_Func(IntPtr path, [MarshalAsAttribute(UnmanagedType.U1)]bool dir, IntPtr data);
public delegate void Evas_Smart_Cb(IntPtr data, IntPtr obj, IntPtr event_info);
public delegate void Elm_Gesture_Event_Cb(IntPtr data, IntPtr event_info);
public delegate void Elm_Object_Item_Signal_Cb(IntPtr data, IntPtr item, IntPtr emission, IntPtr source);
public delegate void Elm_Tooltip_Item_Content_Cb(IntPtr data, IntPtr obj, IntPtr tooltip, IntPtr item);
public delegate void Elm_Sys_Notify_Send_Cb(IntPtr data, uint id);
public delegate IntPtr Elm_Calendar_Format_Cb(IntPtr format);

namespace edje {

public delegate void Signal_Cb(IntPtr data, IntPtr obj, IntPtr emission, IntPtr source);
public delegate void Markup_Filter_Cb(IntPtr data, IntPtr obj, IntPtr part, IntPtr text );
public delegate void Item_Provider_Cb(IntPtr data, IntPtr obj, IntPtr part, IntPtr item);
// Message_Handler_Cb is now legacy

namespace text {
public delegate void Filter_Cb(IntPtr data, IntPtr obj, IntPtr part, int _type, IntPtr text);
public delegate void Change_Cb(IntPtr data, IntPtr obj, IntPtr part);
}


} // namespace edje

[StructLayout(LayoutKind.Sequential)]
public struct Elm_Code
{
    IntPtr file;
    IntPtr widgets;
    IntPtr parsers;

    // Below is inside _Elm_Code_Config
    [MarshalAsAttribute(UnmanagedType.U1)]bool config;
}

[StructLayout(LayoutKind.Sequential)]
public struct Elm_Code_Line
{
    IntPtr file;
    IntPtr content;
    uint length;
    uint number;
    IntPtr modified;

    Elm_Code_Status_Type status;
    IntPtr tokens;

    IntPtr data;
    IntPtr status_text;
}

public enum Elm_Code_Status_Type {
   ELM_CODE_STATUS_TYPE_DEFAULT = 0,
   ELM_CODE_STATUS_TYPE_CURRENT,
   ELM_CODE_STATUS_TYPE_IGNORED,
   ELM_CODE_STATUS_TYPE_NOTE,
   ELM_CODE_STATUS_TYPE_WARNING,
   ELM_CODE_STATUS_TYPE_ERROR,
   ELM_CODE_STATUS_TYPE_FATAL,

   ELM_CODE_STATUS_TYPE_ADDED,
   ELM_CODE_STATUS_TYPE_REMOVED,
   ELM_CODE_STATUS_TYPE_CHANGED,

   ELM_CODE_STATUS_TYPE_PASSED,
   ELM_CODE_STATUS_TYPE_FAILED,

   ELM_CODE_STATUS_TYPE_TODO,

   ELM_CODE_STATUS_TYPE_COUNT
};


[StructLayout(LayoutKind.Sequential)]
public struct Elm_Atspi_Action
{
    IntPtr name;
    IntPtr action;
    IntPtr param;
    IntPtr func;
}

// Some places stil use the non-namespaced Efl_Event_Cb
public delegate void Efl_Event_Cb(System.IntPtr data, ref efl.Event evt);

[StructLayout(LayoutKind.Sequential)]
public struct Tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
}

public delegate IntPtr Efl_Ui_Calendar_Format_Cb(Tm stime); // struct tm as argument
