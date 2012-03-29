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
_on_done(void *data,
        Evas_Object *obj,
        void *event_info)
{
   elm_exit();
}

static double size = 0.0;

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
   if (elm_panes_content_left_size_get(obj) > 0)
     {
        size = elm_panes_content_left_size_get(obj);
        elm_panes_content_left_size_set(obj, 0.0);
        printf("Double clicked, hidding.\n");
     }
   else
     {
        elm_panes_content_left_size_set(obj, size);
        printf("Double clicked, restoring size.\n");
     }
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *panes, *panes_h, *bt;

   win = elm_win_add(NULL, "panes", ELM_WIN_BASIC);
   elm_win_title_set(win, "Panes Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   evas_object_show(bg);

   panes = elm_panes_add(win);
   elm_win_resize_object_add(win, panes);
   evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);
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
   elm_shutdown();

   return 0;
}
ELM_MAIN()
