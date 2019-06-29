#include <Efl_Ui.h>

static Eo *first, *last, *middle;
static int timer = 15;
static int frames = 0;
static double start_time;

static void
_timer_tick(void *data, const Efl_Event *ev)
{
   if (timer % 2 == 0)
     {
         efl_ui_item_container_item_scroll(data, last, EINA_TRUE);
     }
   else
     {
         efl_ui_item_container_item_scroll(data, first, EINA_TRUE);
     }

   timer--;

   if (timer == 0)
     {
        double runtime = ecore_time_get() - start_time;
        efl_loop_quit(efl_app_main_get(), EINA_VALUE_EMPTY);
        efl_del(ev->object);
        printf("We did %d frames in %f s seconds\n", frames, runtime);
        printf("FPS: %f\n", ((double)frames / runtime));

     }
}

static void
_caputure(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   frames ++;
}

static void
_started_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_add(EFL_LOOP_TIMER_CLASS, efl_main_loop_get(),
      efl_loop_timer_interval_set(efl_added, 1.0),
      efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _timer_tick, data)
   );
   start_time = ecore_time_get();
   efl_event_callback_add(evas_object_evas_get(data), EFL_CANVAS_SCENE_EVENT_RENDER_POST, _caputure, data);
   efl_del(ev->object);
}

static void
_first_frame_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_item_container_item_scroll(data, middle, EINA_FALSE);
   //give time to stabelize
   efl_add(EFL_LOOP_TIMER_CLASS, efl_main_loop_get(),
      efl_loop_timer_interval_set(efl_added, 15.0),
      efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _started_cb, data)
   );
   efl_event_callback_del(ev->object, EFL_CANVAS_SCENE_EVENT_RENDER_POST, _first_frame_cb, data);
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win, *item_container, *list;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Item_Container benchmark"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE)
                );

   list = efl_new(EFL_UI_LIST_POSITION_MANAGER_CLASS);
   item_container = efl_add(EFL_UI_ITEM_CONTAINER_CLASS, win,
                      efl_ui_item_container_position_manager_set(efl_added, list));
   efl_content_set(win, item_container);

   printf("Building 5000 objects\n");
   for (int i = 0; i < 5000; ++i)
     {
        Eo *il = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
        double r = 10+((double)190/(double)10)*(i%10);

        if (i == 0)
          first = il;
        else if (i == 2500)
          middle = il;
        else if (i == 4999)
          last = il;
        efl_gfx_color_set(il, r, 10, 10, 255);
        efl_gfx_hint_size_min_set(il, EINA_SIZE2D(40, 40+(i%2)*40));
        efl_pack_end(item_container, il);
     }
   printf("Done!\n");
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 200));

   efl_event_callback_add(evas_object_evas_get(win), EFL_CANVAS_SCENE_EVENT_RENDER_POST, _first_frame_cb, item_container);
}
EFL_MAIN()
