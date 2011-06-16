//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` animator_example_01.c -o animator_example_01

#include <Elementary.h>
#include <limits.h>

static void
on_done(void *data, Evas_Object *obj, void *event_info)
{
   elm_exit();
}

static void
_op_cb(void *data, Elm_Animator *animator, double frame)
{
   evas_object_resize(data, 100 * frame, 100 * frame);
   evas_object_move(data, 50 * frame, 50 * frame);
   evas_object_color_set(data, 255 * frame, 0, 255 * (1 - frame), 255);
}

static void
_end_cb(void *data)
{
   printf("Wow, you're very patient!\n");
}

static void
_pause_resume(void *data, Evas_Object *obj, void *event_info)
{
   static Eina_Bool paused = EINA_FALSE;
   if (!paused)
     {
        elm_animator_pause(data);
        elm_button_label_set(obj, "Resume");
        paused = EINA_TRUE;
     }
   else
     {
        elm_animator_resume(data);
        elm_button_label_set(obj, "Pause");
        paused = EINA_FALSE;
     }
}

static void
_stop(void *data, Evas_Object *obj, void *event_info)
{
   elm_animator_stop(data);
}

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *button, *rectangle;
   Elm_Animator *animator;

   win = elm_win_add(NULL, "animator", ELM_WIN_BASIC);
   elm_win_title_set(win, "Animator");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_resize(bg, 200, 200);
   evas_object_show(bg);

   rectangle = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rectangle, 0, 0, 255, 255);
   evas_object_show(rectangle);

   animator = elm_animator_add(NULL);
   elm_animator_duration_set(animator, 1);
   elm_animator_auto_reverse_set(animator, EINA_TRUE);
   elm_animator_repeat_set(animator, UINT_MAX);
   elm_animator_curve_style_set(animator, ELM_ANIMATOR_CURVE_IN_OUT);
   elm_animator_operation_callback_set(animator, _op_cb, rectangle);
   elm_animator_completion_callback_set(animator, _end_cb, NULL);
   elm_animator_animate(animator);

   button = elm_button_add(win);
   elm_button_label_set(button, "Pause");
   evas_object_resize(button, 50, 30);
   evas_object_move(button, 10, 210);
   evas_object_smart_callback_add(button, "clicked", _pause_resume, animator);
   evas_object_show(button);

   button = elm_button_add(win);
   elm_button_label_set(button, "Stop");
   evas_object_resize(button, 50, 30);
   evas_object_move(button, 70, 210);
   evas_object_smart_callback_add(button, "clicked", _stop, animator);
   evas_object_show(button);

   evas_object_resize(win, 200, 250);
   evas_object_show(win);

   elm_run();

   evas_object_del(rectangle);
   elm_animator_del(animator);

   return 0;
}
ELM_MAIN()
