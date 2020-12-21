/* This header is not to be installed and should only be used by EFL internals */
#ifndef _EVAS_INTERNAL_H
#define _EVAS_INTERNAL_H

#include <evas_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EFL_INTERNAL_UNSTABLE
# error This file can not be included outside EFL
#endif

#include <Efl.h>

typedef struct _Evas_Object_Pointer_Data Evas_Object_Pointer_Data;

EVAS_API EVAS_API_WEAK const Eina_List *efl_input_device_children_get(const Eo *obj);

EVAS_API EVAS_API_WEAK void efl_input_device_evas_set(Eo *obj, Evas *e);
EVAS_API EVAS_API_WEAK Evas *efl_input_device_evas_get(const Eo *obj);

EVAS_API EVAS_API_WEAK void efl_input_device_subclass_set(Eo *obj, Evas_Device_Subclass sub_clas);
EVAS_API EVAS_API_WEAK Evas_Device_Subclass efl_input_device_subclass_get(const Eo *obj);

EVAS_API EVAS_API_WEAK void efl_input_device_grab_register(Eo *obj, Efl_Canvas_Object *grab, Evas_Object_Pointer_Data *pdata);
EVAS_API EVAS_API_WEAK void efl_input_device_grab_unregister(Eo *obj, Efl_Canvas_Object *grab, Evas_Object_Pointer_Data *pdata);

typedef struct _Efl_Input_Pointer_Data  Efl_Input_Pointer_Data;
typedef struct _Efl_Input_Key_Data      Efl_Input_Key_Data;
typedef struct _Efl_Input_Hold_Data     Efl_Input_Hold_Data;
typedef struct _Efl_Input_Focus_Data    Efl_Input_Focus_Data;

#ifndef _EVAS_TYPES_EOT_H_
typedef struct _Evas_Modifier Evas_Modifier;
typedef struct _Evas_Lock Evas_Lock;
#endif

struct _Efl_Input_Pointer_Data
{
   Eo             *eo;
   unsigned int    timestamp; /* FIXME: store as double? */
   int             button;
   unsigned int    pressed_buttons;
   int             touch_id; /* finger or tool ID */
   double          radius, radius_x, radius_y;
   double          pressure, distance, azimuth, tilt, twist;
   double          angle;
   /* current, previous positions in window coordinates.
    * raw can be either un-smoothed, un-predicted x,y or a tablet's raw input.
    * norm is the normalized value in [0..1] for tablet input.
    */
   Eina_Vector2    cur, prev, raw, norm;
   struct {
      int          z;
      Eina_Bool    horizontal;
   } wheel;
   Efl_Gfx_Entity                    *source; /* could it be ecore? */
   Efl_Input_Device           *device;
   Efl_Pointer_Action          action;
   Efl_Pointer_Flags           button_flags;
   Efl_Input_Flags             event_flags;
   void                       *data; /* evas data - whatever that is */
   Eina_Bool                   window_pos; /* true if positions are window-relative
                                              (see input vs. feed: this is "input") */
   Evas_Modifier              *modifiers;
   Evas_Lock                  *locks;
   void                       *legacy; /* DO NOT TOUCH THIS */
   uint32_t                    value_flags;
   Eina_Bool                   has_norm : 1; /* not in value_flags */
   Eina_Bool                   has_raw : 1; /* not in value_flags */
   Eina_Bool                   evas_done : 1; /* set by evas */
   Eina_Bool                   fake : 1;
   Eina_Bool                   win_fed : 1;
};

struct _Efl_Input_Key_Data
{
   Eo                *eo;
   unsigned int       timestamp; /* FIXME: store as double? */

   Eina_Bool          pressed; /* 1 = pressed/down, 0 = released/up */
   Eina_Stringshare  *keyname;
   Eina_Stringshare  *key;
   Eina_Stringshare  *string;
   Eina_Stringshare  *compose;
   unsigned int       keycode;

   void              *data;
   Evas_Modifier     *modifiers;
   Evas_Lock         *locks;
   Efl_Input_Flags    event_flags;
   Efl_Input_Device  *device;
   void              *legacy; /* DO NOT TOUCH THIS */
   Eina_Bool          evas_done : 1; /* set by evas */
   Eina_Bool          fake : 1;
   Eina_Bool          win_fed : 1;
   Eina_Bool          no_stringshare : 1;
};

struct _Efl_Input_Hold_Data
{
   Eo               *eo;
   double            timestamp;
   Efl_Input_Flags   event_flags;
   Efl_Input_Device *device;
   void             *data;
   void             *legacy; /* DO NOT TOUCH THIS */
   Eina_Bool         hold : 1;
   Eina_Bool         evas_done : 1; /* set by evas */
};

struct _Efl_Input_Focus_Data
{
   Eo *eo;
   Efl_Input_Device *device; //The seat
   Eo *object_wref; // wref on the focused object - Efl.Canvas.Object or Efl.Canvas.
   double timestamp;
   Efl_Input_Flags   event_flags;
};

/* Internal helpers */

static inline const char *
_efl_input_modifier_to_string(Efl_Input_Modifier mod)
{
   switch (mod)
     {
      default:
      case EFL_INPUT_MODIFIER_NONE:    return NULL;
      case EFL_INPUT_MODIFIER_ALT:     return "Alt";
      case EFL_INPUT_MODIFIER_CONTROL: return "Control";
      case EFL_INPUT_MODIFIER_SHIFT:   return "Shift";
      case EFL_INPUT_MODIFIER_META:    return "Meta";
      case EFL_INPUT_MODIFIER_ALTGR:   return "AltGr";
      case EFL_INPUT_MODIFIER_HYPER:   return "Hyper";
      case EFL_INPUT_MODIFIER_SUPER:   return "Super";
     }
}

static inline const char *
_efl_input_lock_to_string(Efl_Input_Lock lock)
{
   switch (lock)
     {
      default:
      case EFL_INPUT_LOCK_NONE:    return NULL;
      case EFL_INPUT_LOCK_NUM:     return "Num";
      case EFL_INPUT_LOCK_CAPS:    return "Caps";
      case EFL_INPUT_LOCK_SCROLL:  return "Scroll";
      case EFL_INPUT_LOCK_SHIFT:   return "Shift";
     }
}

static inline Eina_Bool
_efl_input_value_has(const Efl_Input_Pointer_Data *pd, Efl_Input_Value key)
{
   return (pd->value_flags & (1u << (int) key)) != 0;
}

static inline void
_efl_input_value_mark(Efl_Input_Pointer_Data *pd, Efl_Input_Value key)
{
   pd->value_flags |= (1u << (int) key);
}

#define _efl_input_value_mask(key) (1u << (int) key)

typedef struct _Efl_Canvas_Output Efl_Canvas_Output;

EVAS_API Efl_Canvas_Output *efl_canvas_output_add(Evas *canvas);
EVAS_API void efl_canvas_output_del(Efl_Canvas_Output *output);
EVAS_API void efl_canvas_output_view_set(Efl_Canvas_Output *output,
                                     Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
EVAS_API void efl_canvas_output_view_get(Efl_Canvas_Output *output,
                                     Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
EVAS_API Eina_Bool efl_canvas_output_engine_info_set(Efl_Canvas_Output *output,
                                                 Evas_Engine_Info *info);
EVAS_API Evas_Engine_Info *efl_canvas_output_engine_info_get(Efl_Canvas_Output *output);
EVAS_API Eina_Bool efl_canvas_output_lock(Efl_Canvas_Output *output);
EVAS_API Eina_Bool efl_canvas_output_unlock(Efl_Canvas_Output *output);

EVAS_API void evas_render_pending_objects_flush(Evas *eo_evas);

EVAS_API void efl_input_pointer_finalize(Efl_Input_Pointer *obj);

EVAS_API Eina_Iterator *efl_canvas_iterator_create(Eo *obj, Eina_Iterator *real_iterator, Eina_List *list);

static inline void
evas_object_size_hint_combined_min_get(const Eo *obj, int *w, int *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_hint_size_combined_min_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

static inline void
evas_object_size_hint_combined_max_get(const Eo *obj, int *w, int *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_hint_size_combined_max_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

/* Internal EO APIs */
EVAS_API Eo *evas_find(const Eo *obj);
EVAS_API EVAS_API_WEAK void efl_canvas_object_legacy_ctor(Eo *obj);
EVAS_API EVAS_API_WEAK void efl_canvas_object_type_set(Eo *obj, const char *type);
EVAS_API EVAS_API_WEAK void efl_canvas_group_add(Eo *obj);
EVAS_API EVAS_API_WEAK void efl_canvas_group_del(Eo *obj);
EVAS_API EVAS_API_WEAK void efl_canvas_group_clipped_set(Eo *obj, Eina_Bool unclipped);
EVAS_API EVAS_API_WEAK void evas_canvas_touch_point_list_nth_xy_get(Eo *obj, unsigned int n, double *x, double *y);
EVAS_API EVAS_API_WEAK void evas_canvas_seat_focus_in(Eo *obj, Efl_Input_Device *seat);
EVAS_API EVAS_API_WEAK void evas_canvas_seat_focus_out(Eo *obj, Efl_Input_Device *seat);
EVAS_API EVAS_API_WEAK Eo* evas_canvas_seat_focus_get(const Eo *obj, Efl_Input_Device *seat);

EVAS_API EVAS_API_WEAK void *efl_input_legacy_info_get(const Eo *obj);

EVAS_API EVAS_API_WEAK Eo *efl_input_focus_instance_get(Efl_Object *owner, void **priv);
EVAS_API EVAS_API_WEAK Eo *efl_input_hold_instance_get(Efl_Object *owner, void **priv);
EVAS_API EVAS_API_WEAK Eo *efl_input_key_instance_get(Efl_Object *owner, void **priv);
EVAS_API EVAS_API_WEAK Eo *efl_input_pointer_instance_get(Efl_Object *owner, void **priv);
/**
 * @brief If @c true the object belongs to the window border decorations.
 *
 * This will be @c false by default, and should be @c false for all objects
 * created by the application, unless swallowed in some very specific parts of
 * the window.
 *
 * It is very unlikely that an application needs to call this manually, as the
 * window will handle this feature automatically.
 *
 * @param[in] obj The object.
 * @param[in] is_frame @c true if the object is a frame, @c false otherwise
 *
 * @since 1.2
 *
 * @ingroup Efl_Canvas_Object
 */
EVAS_API EVAS_API_WEAK void efl_canvas_object_is_frame_object_set(Eo *obj, Eina_Bool is_frame);

/**
 * @brief If @c true the object belongs to the window border decorations.
 *
 * This will be @c false by default, and should be @c false for all objects
 * created by the application, unless swallowed in some very specific parts of
 * the window.
 *
 * It is very unlikely that an application needs to call this manually, as the
 * window will handle this feature automatically.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the object is a frame, @c false otherwise
 *
 * @since 1.2
 *
 * @ingroup Efl_Canvas_Object
 */
EVAS_API EVAS_API_WEAK Eina_Bool efl_canvas_object_is_frame_object_get(const Eo *obj);

EVAS_API EVAS_API_WEAK extern const Efl_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE;
#define EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE (&(_EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE))

EVAS_API EVAS_API_WEAK extern const Efl_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_POST;
#define EVAS_CANVAS_EVENT_RENDER_FLUSH_POST (&(_EVAS_CANVAS_EVENT_RENDER_FLUSH_POST))

EVAS_API EVAS_API_WEAK extern const Efl_Event_Description _EVAS_CANVAS_EVENT_AXIS_UPDATE;
#define EVAS_CANVAS_EVENT_AXIS_UPDATE (&(_EVAS_CANVAS_EVENT_AXIS_UPDATE))

EVAS_API EVAS_API_WEAK extern const Efl_Event_Description _EVAS_CANVAS_EVENT_VIEWPORT_RESIZE;
#define EVAS_CANVAS_EVENT_VIEWPORT_RESIZE (&(_EVAS_CANVAS_EVENT_VIEWPORT_RESIZE))

#define EFL_CANVAS_GROUP_DEL_OPS(kls) EFL_OBJECT_OP_FUNC(efl_canvas_group_del, _##kls##_efl_canvas_group_group_del)
#define EFL_CANVAS_GROUP_ADD_OPS(kls) EFL_OBJECT_OP_FUNC(efl_canvas_group_add, _##kls##_efl_canvas_group_group_add)
#define EFL_CANVAS_GROUP_ADD_DEL_OPS(kls) EFL_CANVAS_GROUP_ADD_OPS(kls), EFL_CANVAS_GROUP_DEL_OPS(kls)

/* Efl.Animation.Player */
EVAS_API EVAS_API_WEAK extern const Efl_Event_Description _EFL_ANIMATION_PLAYER_EVENT_PRE_STARTED;
#define EFL_ANIMATION_PLAYER_EVENT_PRE_STARTED (&(_EFL_ANIMATION_PLAYER_EVENT_PRE_STARTED))
/* Efl.Animation.Player END */



/*TextBlock Internal function*/
/**
 * Check if cursor is at cluster with one glyph (replace codepoints with new codepoint).
 *
 * @param cur the cursor.
 * @param forward if Eina_True check cluster after cusror position, else before cursor position.
 */
EVAS_API Eina_Bool evas_textblock_cursor_at_cluster_as_single_glyph(Evas_Textblock_Cursor *cur,Eina_Bool forward);




/*Attribute Factory Internal function*/
EVAS_API const char * efl_text_formatter_attribute_get(Efl_Text_Attribute_Handle *annotation);
EVAS_API Eina_Iterator * efl_text_formatter_range_attributes_get(const Efl_Text_Cursor_Object *start, const Efl_Text_Cursor_Object *end);
EVAS_API void efl_text_formatter_attribute_cursors_get(const Efl_Text_Attribute_Handle *handle, Efl_Text_Cursor_Object *start, Efl_Text_Cursor_Object *end);
EVAS_API void efl_text_formatter_remove(Efl_Text_Attribute_Handle *annotation);
EVAS_API Eina_Bool efl_text_formatter_attribute_is_item(Efl_Text_Attribute_Handle *annotation);
EVAS_API Eina_Bool efl_text_formatter_item_geometry_get(const Efl_Text_Attribute_Handle *annotation, int *x, int *y, int *w, int *h);

#ifdef __cplusplus
}
#endif

#endif
