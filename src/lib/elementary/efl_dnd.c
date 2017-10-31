#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED
#define ELM_INTERFACE_ATSPI_TEXT_EDITABLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#include "efl_dnd_private.h"

#define MY_CLASS_DRAG EFL_UI_DND_DRAG_MIXIN
#define MY_CLASS_DROP EFL_UI_DND_DROP_MIXIN

//FIXME: Efl_Selection_Format should be changed to Efl_Data_Format

static inline Eo*
_selection_manager_get(Eo *obj)
{
   Eo *top = elm_widget_top_get(obj);
   if (!top)
     {
        top = obj;
     }
   Eo *sel_man = efl_key_data_get(top, "__selection_manager");
   if (!sel_man)
     {
        sel_man = efl_add(EFL_SELECTION_MANAGER_CLASS, top);
        efl_key_data_set(top, "__selection_manager", sel_man);
     }
   return sel_man;
}



static Ecore_X_Window
_x11_xwin_get(Evas_Object *obj)
{
   if (!obj) return 0;

   Ecore_X_Window xwin = 0;
   //get top
   Evas_Object *top = obj;
   Evas_Object *parent = obj;
   while(parent)
     {
        top = parent;
        parent = efl_parent_get(parent);
     }
   if (efl_isa(top, EFL_UI_WIN_CLASS))
     {
        xwin = elm_win_xwindow_get(top);
     }
   if (!xwin)
     {
        Ecore_Evas *ee;
        Evas *evas = evas_object_evas_get(obj);
        if (!evas) return 0;
        ee = ecore_evas_ecore_evas_get(evas);
        if (!ee) return 0;

        while(!xwin)
          {
             const char *engine_name = ecore_evas_engine_name_get(ee);
             if (!strcmp(engine_name, ELM_BUFFER))
               {
                  ee = ecore_evas_buffer_ecore_evas_parent_get(ee);
                  if (!ee) return 0;
                  xwin = _elm_ee_xwin_get(ee);
               }
             else
               {
                  xwin = _elm_ee_xwin_get(ee);
                  if (!xwin) return 0;
               }
          }
     }

   return xwin;
}


EOLIAN static void
_efl_ui_dnd_drag_efl_dnd_drag_start(Eo *obj, Efl_Ui_Dnd_Drag_Data *pd, Efl_Selection_Format format, const void *buf, int len, Efl_Dnd_Drag_Action action, void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb icon_func_free_cb)
{
   ERR("In");
   Efl_Dnd_Drag_Accept da;
   da.accepted = EINA_FALSE;

   pd->icon_create = icon_func;
   pd->icon_create_data = icon_func_data;
   pd->icon_create_free_cb = icon_func_free_cb;

   efl_event_callback_call(obj, EFL_DND_DRAG_EVENT_DRAG_ACCEPT, &da);

   Efl_Dnd_Drag_Pos dp;
   dp.x = 0;
   dp.y = 0;
   dp.action = EFL_DND_DRAG_ACTION_COPY;
   efl_event_callback_call(obj, EFL_DND_DRAG_EVENT_DRAG_POS, &dp); //event_info??

   efl_event_callback_call(obj, EFL_DND_DRAG_EVENT_DRAG_DONE, &da);

   //TODO: check seat
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_drag_start(sel_man, obj, format, buf, len, action, icon_func_data, icon_func, icon_func_free_cb, NULL);
}

EOLIAN static void
_efl_ui_dnd_drag_efl_dnd_drag_cancel(Eo *obj, Efl_Ui_Dnd_Drag_Data *pd)
{
   ERR("In");
}

EOLIAN static void
_efl_ui_dnd_drag_efl_dnd_drag_action_set(Eo *obj, Efl_Ui_Dnd_Drag_Data *pd , Efl_Dnd_Drag_Action action)
{
   ERR("In");
   pd->action = action;
}

EOLIAN static void
_efl_ui_dnd_drag_efl_dnd_drag_item_container_add(Eo *obj, Efl_Ui_Dnd_Drag_Data *pd, double time_to_drag, double anim_time,
                                 void *data_func_data, Efl_Dnd_Drag_Data_Get data_func, Eina_Free_Cb data_func_free_cb,
                                 void *item_func_data, Efl_Dnd_Drag_Item_Get item_func, Eina_Free_Cb item_func_free_cb,
                                 void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb icon_func_free_cb,
                                 void *icon_list_func_data, Efl_Dnd_Drag_Icon_List_Create icon_list_func, Eina_Free_Cb icon_list_func_free_cb)
{
   ERR("In");
}

static void
_efl_ui_dnd_drag_efl_dnd_drag_item_container_del(Eo *obj, Efl_Ui_Dnd_Drag_Data *pd)
{
   ERR("In");
}

EOLIAN static void
_efl_ui_dnd_drop_efl_dnd_drop_target_add(Eo *obj, Efl_Ui_Dnd_Drop_Data *pd, Efl_Selection_Format format)
{
   ERR("In");
   pd->format = format;

   //fixme: event_info for event callbacks
   efl_event_callback_call(obj, EFL_DND_DROP_EVENT_DRAG_ENTER, NULL);
   efl_event_callback_call(obj, EFL_DND_DROP_EVENT_DRAG_LEAVE, NULL);
   efl_event_callback_call(obj, EFL_DND_DROP_EVENT_DRAG_POS, NULL);
   efl_event_callback_call(obj, EFL_DND_DROP_EVENT_DRAG_DROP, NULL);
}

EOLIAN static void
_efl_ui_dnd_drop_efl_dnd_drop_target_del(Eo *obj, Efl_Ui_Dnd_Drop_Data *pd, Efl_Selection_Format format)
{
   ERR("In");
   pd->format = format;
}

EOLIAN static void
_efl_ui_dnd_drop_efl_dnd_drop_item_container_add(Eo *obj, Efl_Ui_Dnd_Drop_Data *pd, Efl_Selection_Format format, void *item_func_data, Efl_Dnd_Drop_Item_Get item_func, Eina_Free_Cb item_func_free_cb)
{
   ERR("In");
}

EOLIAN static void
_efl_ui_dnd_drop_efl_dnd_drop_item_container_del(Eo *obj, Efl_Ui_Dnd_Drop_Data *pd)
{
   ERR("In");
}


#include "efl_ui_dnd_drag.eo.c"
#include "efl_ui_dnd_drop.eo.c"
