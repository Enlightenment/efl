#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_SELECTION_MANAGER_BETA

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_SELECTION_MIXIN
#define MY_CLASS_NAME "Efl.Selection"

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
_efl_selection_selection_get(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Type type, Efl_Selection_Format format,
                                     void *data_func_data, Efl_Selection_Data_Ready data_func, Eina_Free_Cb data_func_free_cb, unsigned int seat)
{
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_selection_get(sel_man, obj, type, format,
                                       data_func_data, data_func,
                                       data_func_free_cb, seat);
}

EOLIAN static Eina_Future *
_efl_selection_selection_set(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Type type, Efl_Selection_Format format, Eina_Slice data, unsigned int seat)
{
   Eo *sel_man = _selection_manager_get(obj);
   return efl_selection_manager_selection_set(sel_man, obj, type, format, data, seat);
}

EOLIAN static void
_efl_selection_selection_clear(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Type type, unsigned int seat)
{
   Eo *sel_man = _selection_manager_get(obj);
   efl_selection_manager_selection_clear(sel_man, obj, type, seat);
}

EOLIAN static Eina_Bool
_efl_selection_has_owner(Eo *obj, void *pd EINA_UNUSED, Efl_Selection_Type type, unsigned int seat)
{
    Eo *sel_man = _selection_manager_get(obj);
    return efl_selection_manager_selection_has_owner(sel_man, obj, type, seat);
}


#include "efl_selection.eo.c"
