/* This header file is intended for EFL internal use, and is not part of
 * EFL stable API.
 * Place here structs and enums that need to be shared between core EFL
 * components, such as Ecore, Evas, etc...
 */

#ifndef EFL_COMMON_INTERNAL_H
#define EFL_COMMON_INTERNAL_H

#ifndef EFL_INTERNAL_UNSTABLE
# error This file can not be included outside EFL
#endif

#include <Efl.h>

typedef struct _Efl_Input_Pointer_Data  Efl_Input_Pointer_Data;
typedef struct _Efl_Input_Key_Data      Efl_Input_Key_Data;
typedef struct _Efl_Input_Device_Data   Efl_Input_Device_Data;
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
   int             tool; /* finger or tool ID */
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

struct _Efl_Input_Device_Data
{
   Eo               *eo;
   Eo               *evas; /* Evas */
   Efl_Input_Device *source;  /* ref */
   Eina_List        *children; /* ref'ed by efl_parent, not by this list */
   unsigned int      id;
   Efl_Input_Device_Type klass;
   unsigned int      subclass; // Evas_Device_Subclass (unused)
   unsigned int      pointer_count;
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

#endif
