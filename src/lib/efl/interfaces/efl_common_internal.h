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

typedef struct _Efl_Pointer_Event_Data  Efl_Pointer_Event_Data;
typedef struct _Efl_Input_Device_Data   Efl_Input_Device_Data;
typedef struct _Efl_Input_State_Data    Efl_Input_State_Data;

#ifndef _EVAS_TYPES_EOT_H_
/* FIXME */
typedef struct _Evas_Modifier Evas_Modifier;
typedef struct _Evas_Lock Evas_Lock;
#endif

#if 0
struct _Efl_Pointer_Event_Data
{
   Eo             *eo;
   const Eo_Event_Description *event;
   unsigned int    timestamp;
   int             button;
   unsigned int    pressed_buttons;
   struct {
      int          x, y;
   } output;
   struct {
      int          x, y;
   } canvas;
   Eo             *source; /* may be ecore or evas or evas object (?) */
   Efl_Pointer_Action       action;
   Efl_Pointer_Button_Flags button_flags;
   Efl_Pointer_Event_Flags  event_flags;
   void           *data;   /* evas data - whatever that is */
};

struct _Efl_Input_State_Data
{
   Eo             *eo;
   Evas_Modifier  *modifiers;
   Evas_Lock      *locks;
};
#endif

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

#endif
