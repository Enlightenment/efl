#ifndef EFL_UI_TREE_VIEW_PRIVATE_H
#define EFL_UI_TREE_VIEW_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_ui_tree_view.eo.h"
#include "efl_ui_tree_view_layouter.eo.h"

#include <assert.h>

typedef struct _Efl_Ui_Tree_View_Data Efl_Ui_Tree_View_Data;
#include "efl_ui_tree_view_seg_array.h"

struct _Efl_Ui_Tree_View_Data
{
   Eo                           *obj;
   Eo                           *scrl_mgr;
   Efl_Ui_Tree_View_Pan         *pan_obj;
   Efl_Model                    *model;

   struct {
      Eina_Future *future;

      int start;
      int count;
   } slice;

   Efl_Ui_Layout_Factory        *factory;
   Eina_List                    *selected_items;
   Efl_Ui_Focus_Manager         *manager;
   Efl_Ui_Tree_View_Relayout    *layouter;
   Efl_Ui_Tree_View_Seg_Array   *segarray;
   int                          segarray_first;

   Eina_Size2D                  min;
   Eina_Bool                    scrl_freeze : 1;
   Eina_Bool                    show_root: 1;
};

typedef struct _Efl_Ui_Tree_View_Pan_Data Efl_Ui_Tree_View_Pan_Data;
struct _Efl_Ui_Tree_View_Pan_Data
{
   Eo                     *wobj;
   Eina_Rect              gmt;
};

#define EFL_UI_TREE_VIEW_DATA_GET(o, ptr) \
  Efl_Ui_Tree_View_Data * ptr = efl_data_scope_get(o, EFL_UI_TREE_VIEW_CLASS)

#define EFL_UI_TREE_VIEW_DATA_GET_OR_RETURN(o, ptr)       \
  EFL_UI_TREE_VIEW_DATA_GET(o, ptr);                      \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_TREE_VIEW_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_TREE_VIEW_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                              \
    {                                                   \
       ERR("No widget data for object %p (%s)",         \
           o, evas_object_type_get(o));                 \
       return val;                                      \
    }

#endif
