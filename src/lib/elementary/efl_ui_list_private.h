#ifndef EFL_UI_LIST_PRIVATE_H
#define EFL_UI_LIST_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_SELECTION_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_List_Item Efl_Ui_List_Item;

struct _Efl_Ui_List_Item
{
   Eo                   *obj;
   Efl_Model            *model;
   Elm_Layout           *layout;
   Efl_Future           *future;
   unsigned int         index;
   Evas_Coord           x, y, w, h;
   Eina_Bool            down: 1;
   Eina_Bool            selected: 1;
   Eina_Bool            longpressed : 1;
   Ecore_Timer         *long_timer;
};


typedef struct _Efl_Ui_List_Data Efl_Ui_List_Data;

struct _Efl_Ui_List_Data
{
   Eo                           *obj;
   Evas_Object                  *hit_rect;
   Efl_Model                    *model;

   Efl_Orient orient;
   Eina_Bool                    homogeneous : 1;
   Eina_Bool                    delete_me : 1;
   Eina_Bool                    recalc : 1;
   Eina_Bool                    on_hold : 1;
   Eina_Bool                    was_selected : 1;

   struct {
      double h, v;
      Eina_Bool scalable: 1;
   } pad;

   struct {
      double h, v;
   } align;

   Eina_List                    *items, *selected;
   Eina_List                    *realizes;
   Eina_Stringshare             *style;
   Elm_Object_Select_Mode       select_mode;
   Elm_List_Mode                mode;
   unsigned int                 re_idx;

   Evas_Object                  *pan_obj;
   Eina_Bool                    pan_changed : 1;
   Evas_Coord                   pan_x, pan_y, minw, minh, dx, dy, rlzw, rlzh;
   Efl_Ui_List_Item             *focused;
};

typedef struct _Efl_Ui_List_Pan_Data Efl_Ui_List_Pan_Data;

struct _Efl_Ui_List_Pan_Data
{
   Eo                     *wobj;
   Efl_Ui_List_Data       *wpd;
   Ecore_Job              *resize_job;
};


#define EFL_UI_LIST_DATA_GET(o, ptr) \
  Efl_Ui_List_Data * ptr = efl_data_scope_get(o, EFL_UI_LIST_CLASS)

#define EFL_UI_LIST_DATA_GET_OR_RETURN(o, ptr)       \
  EFL_UI_LIST_DATA_GET(o, ptr);                      \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_LIST_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_LIST_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       CRI("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return val;                                      \
    }

#endif
