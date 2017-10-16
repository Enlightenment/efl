#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

//#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
//#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED
//#define ELM_INTERFACE_ATSPI_TEXT_EDITABLE_PROTECTED
//#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_cnp_private.h"

#define MY_CLASS EFL_CNP_MIXIN
#define MY_CLASS_NAME "Efl.Cnp"

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
_efl_cnp_efl_selection_selection_get(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type, Efl_Selection_Format format,
                                     void *data_func_data, Efl_Selection_Data_Ready data_func, Eina_Free_Cb data_func_free_cb, Efl_Input_Device *seat)
{
   ERR("In");
   Eo *sel_man = _selection_manager_get(obj);
   ERR("sel_man: %p", sel_man);
   efl_selection_manager_selection_get(sel_man, obj, type, format,
                                       data_func_data, data_func,
                                       data_func_free_cb, seat);
}

EOLIAN static void
_efl_cnp_efl_selection_selection_set(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type, Efl_Selection_Format format, const void *buf, int len, Efl_Input_Device *seat)
{
   ERR("In");
   Eo *sel_man = _selection_manager_get(obj);
   ERR("sel_man: %p", sel_man);
   efl_selection_manager_selection_set(sel_man, obj, type, format, buf, len, seat);
}

EOLIAN static void
_efl_cnp_efl_selection_selection_clear(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type, Efl_Input_Device *seat)
{
   ERR("In");
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_selection_clear(sel_man, obj, type, seat);
}

//issue: selection clear only come with window-level
//if a window has two entries, selection moves from one entry to the other
//the selection clear does not come (still in that window)
//fix: make selection manager: 1 selection manager per one window
//it manages selection, calls selection loss even selection moves inside window.
//Implementation: has SelMan mixin, ui_win includes it.
//this cnp handles all converter, while SelMan manages selection requests.
/*static Eina_Bool
_x11_efl_cnp_selection_clear_cb(void *data, int type, void *event)
{
   return ECORE_CALLBACK_PASS_ON;
}*/

//Selection loss event callback:: name
//future or event???
/*EOLIAN static Efl_Future *
_efl_cnp_efl_selection_selection_loss_feedback(Eo *obj, Efl_Cnp_Data *pd, Efl_Selection_Type type)
{
   ERR("In");
   Eo *sel_man = _selection_manager_get(obj);
   return efl_selection_manager_selection_loss_feedback(sel_man, obj, type);
}*/

#include "efl_cnp.eo.c"
