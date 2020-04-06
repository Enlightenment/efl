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

typedef struct {
   Ecore_Evas *ee;
   Eina_Bool registered;
} Efl_Ui_Dnd_Data;

typedef struct {
   Eo *win;
   Efl_Ui_Dnd *obj;
} Efl_Ui_Drag_Start;

static void
_ecore_evas_drag_terminated(Ecore_Evas *ee EINA_UNUSED, unsigned int seat, void *data, Eina_Bool accepted)
{
   Efl_Ui_Drag_Start *start = data;
   Efl_Ui_Drag_Finished_Event ev = {seat, accepted};
   efl_event_callback_call(start->obj, EFL_UI_DND_EVENT_DRAG_FINISHED, &ev);
   efl_del(start->win);
   free(start);
}

EOLIAN static Efl_Content*
_efl_ui_dnd_drag_start(Eo *obj, Efl_Ui_Dnd_Data *pd, Eina_Content *content, const char* action, unsigned int seat)
{
   Eo *drag_win;
   Efl_Ui_Drag_Start *start;
   Efl_Ui_Drag_Started_Event ev = {seat};
   Ecore_Evas *drag_ee;
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->ee, NULL);

   start = calloc(1, sizeof(Efl_Ui_Drag_Start));
   start->obj = obj;
   start->win = drag_win = elm_win_add(NULL, "Elm-Drag", ELM_WIN_DND);
   elm_win_alpha_set(drag_win, EINA_TRUE);
   elm_win_override_set(drag_win, EINA_TRUE);
   elm_win_borderless_set(drag_win, EINA_TRUE);
   drag_ee = ecore_evas_ecore_evas_get(evas_object_evas_get(drag_win));

   if (!ecore_evas_drag_start(pd->ee, seat, content, drag_ee, action, _ecore_evas_drag_terminated, start))
     {
        efl_del(drag_win);
        free(start);
        drag_win = NULL;
     }
   else
     {
        evas_object_show(drag_win);
        efl_event_callback_call(obj, EFL_UI_DND_EVENT_DRAG_STARTED, &ev);
     }

   return drag_win;
}

EOLIAN static void
_efl_ui_dnd_drag_cancel(Eo *obj EINA_UNUSED, Efl_Ui_Dnd_Data *pd, unsigned int seat)
{
   ecore_evas_drag_cancel(pd->ee, seat);
}

EOLIAN static Eina_Future*
_efl_ui_dnd_drop_data_get(Eo *obj EINA_UNUSED, Efl_Ui_Dnd_Data *pd, unsigned int seat, Eina_Iterator *acceptable_types)
{
   return ecore_evas_selection_get(pd->ee, seat, ECORE_EVAS_SELECTION_BUFFER_DRAG_AND_DROP_BUFFER, acceptable_types);
}

EOLIAN static Efl_Object *
_efl_ui_dnd_efl_object_constructor(Eo *obj, Efl_Ui_Dnd_Data *pd)
{
   if (!efl_constructor(efl_super(obj, EFL_UI_DND_MIXIN)))
     return NULL;

   pd->ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));

   return obj;
}

EOLIAN static void
_efl_ui_dnd_efl_object_invalidate(Eo *obj, Efl_Ui_Dnd_Data *pd)
{
   if (pd->registered)
     {
        _drop_event_unregister(obj);
     }
   efl_invalidate(efl_super(obj, EFL_UI_DND_MIXIN));

}

#define IS_DROP_EVENT(D) ( \
(D == EFL_UI_DND_EVENT_DROP_POSITION_CHANGED) || \
(D == EFL_UI_DND_EVENT_DROP_DROPPED) || \
(D == EFL_UI_DND_EVENT_DROP_LEFT) || \
(D == EFL_UI_DND_EVENT_DROP_ENTERED) \
)

EOLIAN static Efl_Object*
_efl_ui_dnd_efl_object_finalize(Eo *obj, Efl_Ui_Dnd_Data *pd)
{
   if (pd->registered)
     _drop_event_register(obj);

   return efl_finalize(efl_super(obj, EFL_UI_DND_MIXIN));
}


EOLIAN static Eina_Bool
_efl_ui_dnd_efl_object_event_callback_priority_add(Eo *obj, Efl_Ui_Dnd_Data *pd,
                                        const Efl_Event_Description *desc,
                                        Efl_Callback_Priority priority,
                                        Efl_Event_Cb func,
                                        const void *user_data)
{
  if (IS_DROP_EVENT(desc) && !pd->registered)
    {
       pd->registered = EINA_TRUE;
       if (efl_finalized_get(obj))
         _drop_event_register(obj);
    }

  return efl_event_callback_priority_add(efl_super(obj, EFL_UI_DND_MIXIN), desc, priority, func, user_data);
}

EOLIAN static Eina_Bool
_efl_ui_dnd_efl_object_event_callback_array_priority_add(Eo *obj, Efl_Ui_Dnd_Data *pd,
                                              const Efl_Callback_Array_Item *array,
                                              Efl_Callback_Priority priority,
                                              const void *user_data)
{
   for (int i = 0; array[i].desc; ++i)
     {
        if (IS_DROP_EVENT(array[i].desc) && !pd->registered)
          {
             pd->registered = EINA_TRUE;
             if (efl_finalized_get(obj))
               _drop_event_register(obj);
          }
     }
   return efl_event_callback_array_priority_add(efl_super(obj, EFL_UI_DND_MIXIN), array, priority, user_data);
}

#define EFL_UI_DND_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_event_callback_priority_add, _efl_ui_dnd_efl_object_event_callback_priority_add), \
   EFL_OBJECT_OP_FUNC(efl_event_callback_array_priority_add, _efl_ui_dnd_efl_object_event_callback_array_priority_add), \

#include "efl_ui_dnd.eo.c"
