/**
 * Simple Elementary's <b>panes widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g panes_example.c -o panes_example `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

static void
_press(void *data, Evas_Object *obj, void *event_info)
{
   printf("Pressed\n");
}

static void
_unpress(void *data, Evas_Object *obj, void *event_info)
{
   printf("Unpressed, size : %f\n", elm_panes_content_left_size_get(obj));
}

static void
_clicked(void *data, Evas_Object *obj, void *event_info)
{
   printf("Clicked\n");
}

static void
_clicked_double(void *data, Evas_Object *obj, void *event_info)
{
   static double size = 0.0;
   double tmp_size = 0.0;

   tmp_size = elm_panes_content_left_size_get(obj);
   if (tmp_size > 0)
     {
        elm_panes_content_left_size_set(obj, 0.0);
        printf("Double clicked, hidding.\n");
     }
   else
     {
        elm_panes_content_left_size_set(obj, size);
        printf("Double clicked, restoring size.\n");
     }
   size = tmp_size;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *panes, *panes_h, *bt;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("panes", "Panes Example");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   panes = elm_panes_add(win);
   evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, panes);
   evas_object_show(panes);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);
   elm_object_part_content_set(panes, "left", bt);

   panes_h = elm_panes_add(win);
   elm_panes_horizontal_set(panes_h, EINA_TRUE);
   evas_object_size_hint_weight_set(panes_h, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panes_h, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panes_h);
   elm_object_part_content_set(panes, "right", panes_h);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Up");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);
   elm_object_part_content_set(panes_h, "left", bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Down");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);
   elm_object_part_content_set(panes_h, "right", bt);

   evas_object_smart_callback_add(panes, "clicked", _clicked, panes);
   evas_object_smart_callback_add(panes, "clicked,double", _clicked_double,
                                  panes);
   evas_object_smart_callback_add(panes, "press", _press, panes);
   evas_object_smart_callback_add(panes, "unpress", _unpress, panes);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
