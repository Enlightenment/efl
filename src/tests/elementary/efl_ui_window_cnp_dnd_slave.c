#define EFL_BETA_API_SUPPORT 1

#include <Efl.h>
#include <Efl_Ui.h>
#include <Elementary.h>
#include "efl_ui_grid_view.eo.h"

static Ecore_Evas *ee;

static Eina_Value
_deliverty_cb(void *data, const Eina_Value value, const Eina_Future *dead_future EINA_UNUSED)
{
   Ecore_Evas_Selection_Buffer buffer = (intptr_t)data;
   Eina_Content *content;

   if (eina_value_type_get(&value) != EINA_VALUE_TYPE_CONTENT)
     {
        char *error = eina_value_to_string(&value);
        printf("Value not a content, message: \"%s\"\n", error);
        return EINA_VALUE_EMPTY;
     }

   content = eina_value_to_content(&value);
   printf("Got Content of selection %d with type %s\n", buffer, eina_content_type_get(content));
   if (eina_str_has_prefix(eina_content_type_get(content),"text"))
     {
        printf("Content: %s\n", (char*)eina_content_data_get(content).mem);
     }

   return EINA_VALUE_EMPTY;
}

static void
_selection_changed(Ecore_Evas *ee, unsigned int seat EINA_UNUSED, Ecore_Evas_Selection_Buffer selection)
{
   printf("Selection %d of %p has changed\n", selection, ee);
}

static void
_request_selection(Ecore_Evas *ee, unsigned int seat EINA_UNUSED, Ecore_Evas_Selection_Buffer selection)
{
   const char *types[] = {eina_stringshare_add("text/plain"), eina_stringshare_add("text/plain;charset=utf-8")};
   printf("Selection %d of %p has changed\n", selection, ee);
   Eina_Future *future = ecore_evas_selection_get(ee, 0, selection, EINA_C_ARRAY_ITERATOR_NEW(types));
   eina_future_then(future, _deliverty_cb, .data = ((void*)(intptr_t)selection));
}

static void
_motion_cb(Ecore_Evas *ee, unsigned int seat EINA_UNUSED, Eina_Position2D p)
{
   printf("Drag and Drop has moved on the window %p (%d, %d)\n", ee, p.x, p.y);
}

static void
_enter_state_change_cb(Ecore_Evas *ee, unsigned int seat EINA_UNUSED, Eina_Position2D p, Eina_Bool inside)
{
   if (inside)
     printf("Drag and Drop has entered the window %p (%d, %d)\n", ee, p.x, p.y);
   else
     printf("Drag and Drop has left the window %p (%d, %d)\n", ee, p.x, p.y);
}

static void
_drop_cb(Ecore_Evas *ee, unsigned int seat EINA_UNUSED, Eina_Position2D p, const char *action EINA_UNUSED)
{
   const char *types[] = {eina_stringshare_add("text/plain")};
   printf("Drag and Drop has droped on the window %p (%d, %d)\n", ee, p.x, p.y);
   Eina_Future *f = ecore_evas_selection_get(ee, 0, ECORE_EVAS_SELECTION_BUFFER_DRAG_AND_DROP_BUFFER, EINA_C_ARRAY_ITERATOR_NEW(types));
   eina_future_then(f, _deliverty_cb, .data = ((void*)(intptr_t)ECORE_EVAS_SELECTION_BUFFER_DRAG_AND_DROP_BUFFER));
}

static void
_efl_ui_terminated(Ecore_Evas *ee EINA_UNUSED, unsigned int seat EINA_UNUSED, void *data, Eina_Bool accepted EINA_UNUSED)
{
   efl_del(data);
}

static Eo*
_start_dnd(Ecore_Evas *ee)
{
   Ecore_Evas *ee2;
   Eina_Content *content = eina_content_new((Eina_Slice)EINA_SLICE_STR_FULL("This is sample content"), "text/plain");
   Efl_Ui_Win *win;
   Efl_Ui_Button *btn;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get());
   ee2 = ecore_evas_ecore_evas_get(evas_object_evas_get(win));

   btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(btn, "Test");
   efl_content_set(win, btn);

   evas_object_geometry_set(win, 0, 0, 100, 100);

   ecore_evas_drag_start(ee, 0, content, ee2, "copy", _efl_ui_terminated, win);

   return win;
}

static void
_start_op(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _start_dnd(data);
}

static Eina_Value
_delete_cb(Eo *obj, void *data EINA_UNUSED, const Eina_Value value EINA_UNUSED)
{
   Ecore_Evas *ee ;
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));

   ecore_evas_drag_cancel(ee, 0);

   return EINA_VALUE_EMPTY;
}

static void
_start_delayed_del_op(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _start_dnd(data);
   efl_future_then(ev->object, efl_loop_timeout(efl_main_loop_get(), 2.0), _delete_cb);
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Textbox *txt, *win, *bx, *btn;
   Efl_Loop_Arguments *args = ev->info;
   char *goal;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get());
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win));

   bx = efl_add(EFL_UI_BOX_CLASS, win);

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_text_set(txt, "Sample for CNP and DND interaction");
   efl_pack_end(bx, txt);

   btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_gfx_hint_weight_set(btn, 1.0, 0.0);
   efl_event_callback_add(btn, EFL_INPUT_EVENT_PRESSED, _start_op, ee);
   efl_text_set(btn, "Start DND op");
   efl_pack_end(bx, btn);

   btn = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_gfx_hint_weight_set(btn, 1.0, 0.0);
   efl_event_callback_add(btn, EFL_INPUT_EVENT_PRESSED, _start_delayed_del_op, ee);
   efl_text_set(btn, "Start DND op self destroy after 2 sec");
   efl_pack_end(bx, btn);

   efl_content_set(win, bx);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 320));

   goal = eina_array_data_get(args->argv, 1);

   if (eina_streq(goal, "--monitor"))
     {
        ecore_evas_callback_selection_changed_set(ee, _selection_changed);
        ecore_evas_callback_drop_drop_set(ee, _drop_cb);
        ecore_evas_callback_drop_motion_set(ee, _motion_cb);
        ecore_evas_callback_drop_state_changed_set(ee, _enter_state_change_cb);
     }
   else if (eina_streq(goal, "--show-selections"))
     {
        ecore_evas_callback_selection_changed_set(ee, _request_selection);
     }
   else if (eina_streq(goal, "--set-selection"))
     {
        if (eina_array_count(args->argv) < 3)
          {
             printf("Error, --set-selection only requires exactly 1 keyword (The selection to set).\n");
             return;
          }
        char *selection = eina_array_data_get(args->argv, 2);
        Eina_Content *content = eina_content_new((Eina_Slice)EINA_SLICE_STR_FULL(selection), "text/plain");
        ecore_evas_selection_set(ee, 0, ECORE_EVAS_SELECTION_BUFFER_COPY_AND_PASTE_BUFFER, content);
     }
   else if (eina_streq(goal, "--show-owner"))
     {
        for (int i = 0; i < ECORE_EVAS_SELECTION_BUFFER_LAST; ++i)
          {
             printf("Selection buffer %d : %d\n", i, ecore_evas_selection_exists(ee, 0, i));
          }
     }
   else
     {
        printf("Error, goal %s not found\n", goal);
     }
}
EFL_MAIN()
