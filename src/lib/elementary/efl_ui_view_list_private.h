#ifndef EFL_UI_VIEW_LIST_PRIVATE_H
#define EFL_UI_VIEW_LIST_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_SELECTION_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_View_List_Data Efl_Ui_View_List_Data;

int efl_ui_view_list_item_index_get(Efl_Ui_View_List_LayoutItem *item);

typedef struct _Efl_Ui_View_List_Data Efl_Ui_View_List_Data;

#include "efl_ui_view_list_segarray.h"

struct _Efl_Ui_View_List_Data
{
   Eo                           *obj;
   Eo                           *scrl_mgr;
   Efl_Ui_View_List_Pan              *pan_obj;
   Efl_Model                    *model;

   Eina_Stringshare             *style;

   struct {
      Eina_Future *future;

      int start;
      int count;
   } slice;

   Efl_Ui_Layout_Factory        *factory;
   Eina_List                    *selected_items;

   Efl_Ui_Focus_Manager         *manager;
   Efl_Ui_View_List_Relayout    *relayout;

   Efl_Orient                   orient;

   int segarray_first;
   Efl_Ui_View_List_SegArray         *segarray;

   Elm_Object_Select_Mode       select_mode;
   Elm_List_Mode                mode;

   Eina_Rect                    gmt;
   Eina_Size2D                  min;

   Eina_Bool                    homogeneous : 1;
   Eina_Bool                    recalc : 1;
   Eina_Bool                    on_hold : 1;
   Eina_Bool                    scrl_freeze : 1;
};

typedef struct _Efl_Ui_View_List_Pan_Data Efl_Ui_View_List_Pan_Data;

struct _Efl_Ui_View_List_Pan_Data
{
   Eo                     *wobj;
   Eina_Rect              gmt;
   Evas_Coord             move_diff;

   Ecore_Job              *resize_job;
};

typedef struct _Efl_Ui_View_List_Slice Efl_Ui_View_List_Slice;

struct _Efl_Ui_View_List_Slice
{
   Efl_Ui_View_List_Data       *pd;
   int                    newstart, slicestart, newslice;
};

#define EFL_UI_VIEW_LIST_DATA_GET(o, ptr) \
  Efl_Ui_View_List_Data * ptr = efl_data_scope_get(o, EFL_UI_VIEW_LIST_CLASS)

#define EFL_UI_VIEW_LIST_DATA_GET_OR_RETURN(o, ptr)       \
  EFL_UI_VIEW_LIST_DATA_GET(o, ptr);                      \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_VIEW_LIST_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_VIEW_LIST_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return val;                                      \
    }

#endif
