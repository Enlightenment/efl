#ifndef ELM_WIDGET_CONFORMANT_H
#define ELM_WIDGET_CONFORMANT_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-conformant-class The Elementary Conformant Class
 *
 * Elementary, besides having the @ref Conformant widget, exposes its
 * foundation -- the Elementary Conformant Class -- in order to create other
 * widgets which are a conformant with some more logic on top.
 */

/**
 * Base layout smart data extended with conformant instance data.
 */
typedef struct _Elm_Conformant_Smart_Data Elm_Conformant_Smart_Data;
struct _Elm_Conformant_Smart_Data
{
   Evas_Object                   *portrait_indicator;
   Evas_Object                   *landscape_indicator;
   Evas_Object                   *softkey;
   Evas_Object                   *virtualkeypad;
   Evas_Object                   *clipboard;
   Evas_Object                   *scroller;
#ifdef HAVE_ELEMENTARY_X
   Ecore_Event_Handler           *prop_hdl;
   Ecore_X_Virtual_Keyboard_State vkb_state;
   Ecore_X_Illume_Clipboard_State clipboard_state;
#endif
   struct
   {
      Ecore_Animator *animator; // animaton timer
      double          start; // time started
      Evas_Coord      auto_x, auto_y; // desired delta
      Evas_Coord      x, y; // current delta
   } delta;
   Ecore_Job                     *show_region_job;
   Elm_Win_Indicator_Mode         indmode;
   Elm_Win_Indicator_Opacity_Mode ind_o_mode;
   Ecore_Timer                   *land_indi_timer;
   Ecore_Timer                   *port_indi_timer;

   int                            rot;
};

/* Enum to identify conformant swallow parts */
typedef enum _Conformant_Part_Type Conformant_Part_Type;
enum _Conformant_Part_Type
{
   ELM_CONFORMANT_INDICATOR_PART      = 1,
   ELM_CONFORMANT_SOFTKEY_PART        = 2,
   ELM_CONFORMANT_VIRTUAL_KEYPAD_PART = 4,
   ELM_CONFORMANT_CLIPBOARD_PART    = 8
};

/**
 * @}
 */

#define ELM_CONFORMANT_DATA_GET(o, sd) \
  Elm_Conformant_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_CONFORMANT_CLASS)

#define ELM_CONFORMANT_DATA_GET_OR_RETURN(o, ptr)    \
  ELM_CONFORMANT_DATA_GET(o, ptr);                   \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_CONFORMANT_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_CONFORMANT_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                                 \
    {                                                      \
       CRI("No widget data for object %p (%s)",            \
           o, evas_object_type_get(o));                    \
       return val;                                         \
    }

#define ELM_CONFORMANT_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_CONFORMANT_CLASS))) \
    return

#endif
