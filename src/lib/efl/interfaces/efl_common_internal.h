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
   double          pressure;
   double          angle;
   Eina_Vector2    cur, prev, raw;
   struct {
      Efl_Orient   dir;
      int          z;
   } wheel;
   Efl_Gfx                    *source; /* could it be ecore? */
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
   Efl_Input_Device *parent;  /* no ref */
   Efl_Input_Device *source;  /* ref */
   Eina_List        *children; /* ref */
   Eina_Stringshare *name;
   Eina_Stringshare *desc;
   Efl_Input_Device_Class klass;
   Efl_Input_Device_Sub_Class subclass;
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
