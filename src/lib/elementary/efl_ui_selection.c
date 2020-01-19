#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif


#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_SELECTION_MIXIN
#define MY_CLASS_NAME "Efl.Ui.Selection"

typedef struct {
   Ecore_Evas *ee;
   Eina_Bool registered : 1;
} Efl_Ui_Selection_Data;

static inline Ecore_Evas_Selection_Buffer
_ee_buffer_get(Efl_Ui_Cnp_Buffer buffer)
{
   if (buffer == EFL_UI_CNP_BUFFER_SELECTION)
     return ECORE_EVAS_SELECTION_BUFFER_SELECTION_BUFFER;
   else
     return ECORE_EVAS_SELECTION_BUFFER_COPY_AND_PASTE_BUFFER;
}

EOLIAN static Eina_Future*
_efl_ui_selection_selection_get(Eo *obj EINA_UNUSED, Efl_Ui_Selection_Data *pd, Efl_Ui_Cnp_Buffer buffer, unsigned int seat, Eina_Iterator *acceptable_types)
{
   return ecore_evas_selection_get(pd->ee, seat, _ee_buffer_get(buffer), acceptable_types);
}

EOLIAN static void
_efl_ui_selection_selection_set(Eo *obj, Efl_Ui_Selection_Data *pd, Efl_Ui_Cnp_Buffer buffer, Eina_Content *content, unsigned int seat)
{
   _register_selection_changed(obj);
   ecore_evas_selection_set(pd->ee, seat, _ee_buffer_get(buffer), content);
}

EOLIAN static void
_efl_ui_selection_selection_clear(Eo *obj EINA_UNUSED, Efl_Ui_Selection_Data *pd, Efl_Ui_Cnp_Buffer buffer, unsigned int seat)
{
   ecore_evas_selection_set(pd->ee, seat, _ee_buffer_get(buffer), NULL);
}

EOLIAN static Eina_Bool
_efl_ui_selection_has_selection(Eo *obj EINA_UNUSED, Efl_Ui_Selection_Data *pd, Efl_Ui_Cnp_Buffer buffer, unsigned int seat)
{
   return ecore_evas_selection_exists(pd->ee, seat, _ee_buffer_get(buffer));
}

EOLIAN static Efl_Object*
_efl_ui_selection_efl_object_constructor(Eo *obj, Efl_Ui_Selection_Data *pd)
{
  if (!efl_constructor(efl_super(obj, EFL_UI_SELECTION_MIXIN)))
    return NULL;

  pd->ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));

  return obj;
}

EOLIAN static void
_efl_ui_selection_efl_object_invalidate(Eo *obj, Efl_Ui_Selection_Data *pd)
{
   if (pd->registered)
     {
        _selection_changed_event_unregister(obj);
     }
   efl_invalidate(efl_super(obj, EFL_UI_SELECTION_MIXIN));
}

EOLIAN static Eina_Bool
_efl_ui_selection_efl_object_event_callback_priority_add(Eo *obj, Efl_Ui_Selection_Data *pd,
                                        const Efl_Event_Description *desc,
                                        Efl_Callback_Priority priority,
                                        Efl_Event_Cb func,
                                        const void *user_data)
{
  if (desc == EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED && !pd->registered)
    {

       pd->registered = EINA_TRUE;
       if (efl_finalized_get(obj))
         _selection_changed_event_register(obj);
    }

  return efl_event_callback_priority_add(efl_super(obj, EFL_UI_SELECTION_MIXIN), desc, priority, func, user_data);
}

EOLIAN static Eina_Bool
_efl_ui_selection_efl_object_event_callback_array_priority_add(Eo *obj, Efl_Ui_Selection_Data *pd,
                                              const Efl_Callback_Array_Item *array,
                                              Efl_Callback_Priority priority,
                                              const void *user_data)
{
   for (int i = 0; array[i].desc; ++i)
     {
        if (array[i].desc == EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED && !pd->registered)
          {
             pd->registered = EINA_TRUE;
             if (efl_finalized_get(obj))
               _selection_changed_event_register(obj);
          }
     }
   return efl_event_callback_array_priority_add(efl_super(obj, EFL_UI_SELECTION_MIXIN), array, priority, user_data);
}


EOLIAN static Efl_Object*
_efl_ui_selection_efl_object_finalize(Eo *obj, Efl_Ui_Selection_Data *pd)
{
   if (pd->registered)
     _selection_changed_event_register(obj);

   return efl_finalize(efl_super(obj, MY_CLASS));
}


#define EFL_UI_SELECTION_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_event_callback_priority_add, _efl_ui_selection_efl_object_event_callback_priority_add), \
   EFL_OBJECT_OP_FUNC(efl_event_callback_array_priority_add, _efl_ui_selection_efl_object_event_callback_array_priority_add), \

#include "efl_ui_selection.eo.c"
