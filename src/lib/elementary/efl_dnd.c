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

#define MY_CLASS EFL_DND_MIXIN
#define MY_CLASS_NAME "Efl.Dnd"

//FIXME: Efl_Cnp_Format should be changed to Efl_Data_Format


EOLIAN static void
_efl_ui_dnd_drag_efl_dnd_drag_start(Eo *obj, Efl_Ui_Dnd_Drag_Data *pd, Efl_Cnp_Format format, const void *buf, int len, Efl_Dnd_Drag_Action action, void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb icon_func_free_cb)
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
_efl_ui_dnd_drop_efl_dnd_drop_target_add(Eo *obj, Efl_Ui_Dnd_Drop_Data *pd, Efl_Cnp_Format format)
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
_efl_ui_dnd_drop_efl_dnd_drop_target_del(Eo *obj, Efl_Ui_Dnd_Drop_Data *pd, Efl_Cnp_Format format)
{
   ERR("In");
   pd->format = format;
}

EOLIAN static void
_efl_ui_dnd_drop_efl_dnd_drop_item_container_add(Eo *obj, Efl_Ui_Dnd_Drop_Data *pd, Efl_Cnp_Format format, void *item_func_data, Efl_Dnd_Drop_Item_Get item_func, Eina_Free_Cb item_func_free_cb)
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
