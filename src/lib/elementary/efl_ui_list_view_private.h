#ifndef EFL_UI_LIST_VIEW_PRIVATE_H
#define EFL_UI_LIST_VIEW_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "efl_ui_list_view_relayout.eo.h"
#include "elm_priv.h"

typedef struct _Efl_Ui_List_View_Data Efl_Ui_List_View_Data;
int efl_ui_list_view_item_index_get(Efl_Ui_List_View_Layout_Item *item);

#include "efl_ui_list_view_seg_array.h"

struct _Efl_Ui_List_View_Data
{
   Eo                           *obj;
   Eo                           *scrl_mgr;
   Efl_Ui_List_View_Pan         *pan_obj;
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
   Efl_Ui_List_View_Relayout    *relayout;
   Efl_Ui_List_View_Seg_Array    *seg_array;
   int                          seg_array_first;

   Elm_Object_Select_Mode       select_mode;
   Eina_Size2D                  min;

   Eina_Bool                    homogeneous : 1;
   Eina_Bool                    scrl_freeze : 1;
};

typedef struct _Efl_Ui_List_View_Pan_Data Efl_Ui_List_View_Pan_Data;

struct _Efl_Ui_List_View_Pan_Data
{
   Eo                     *wobj;
   Eina_Rect              gmt;
};

#define EFL_UI_LIST_VIEW_DATA_GET(o, ptr) \
  Efl_Ui_List_View_Data * ptr = efl_data_scope_get(o, EFL_UI_LIST_VIEW_CLASS)

#define EFL_UI_LIST_VIEW_DATA_GET_OR_RETURN(o, ptr)       \
  EFL_UI_LIST_VIEW_DATA_GET(o, ptr);                      \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_LIST_VIEW_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_LIST_VIEW_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return val;                                      \
    }

#endif
