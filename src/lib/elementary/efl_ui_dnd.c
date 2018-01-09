#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED
#define ELM_INTERFACE_ATSPI_TEXT_EDITABLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#define EFL_SELECTION_MANAGER_BETA

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

extern int _wl_default_seat_id_get(Evas_Object *obj);

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

EOLIAN static void
_efl_ui_dnd_drag_start(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Format format, Eina_Slice data,
                       Efl_Selection_Action action, void *icon_func_data, Efl_Dnd_Drag_Icon_Create icon_func, Eina_Free_Cb icon_func_free_cb,
                       unsigned int seat)
{
   ERR("In");
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_drag_start(sel_man, obj, format, data, action,
                                    icon_func_data, icon_func, icon_func_free_cb,
                                    seat);
}

EOLIAN static void
_efl_ui_dnd_drag_cancel(Eo *obj, void *pd EINA_UNUSED, unsigned int seat)
{
   ERR("In");
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_drag_cancel(sel_man, obj, seat);
}

EOLIAN static void
_efl_ui_dnd_drag_action_set(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Action action, unsigned int seat)
{
   ERR("In");
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_drag_action_set(sel_man, obj, action, seat);
}


EOLIAN static void
_efl_ui_dnd_drop_target_add(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Format format, unsigned int seat)
{
   ERR("In");

   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_drop_target_add(sel_man, obj, format, seat);
}

EOLIAN static void
_efl_ui_dnd_drop_target_del(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Format format, unsigned int seat)
{
   ERR("In");
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_drop_target_del(sel_man, obj, format, seat);
}

#include "efl_ui_dnd.eo.c"
