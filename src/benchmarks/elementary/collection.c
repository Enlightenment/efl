#include <Elementary.h>
#include <Efl_Ui.h>

static Efl_Ui_Win *win;
static Efl_Ui_Collection *collection;
static Eo *first, *last, *middle;
static int timer = 15;
static int frames = 0;
static double start_time;

static void
_timer_tick(void *data, const Efl_Event *ev)
{
   if (timer % 2 == 0)
     {
         efl_ui_collection_item_scroll(data, last, EINA_TRUE);
     }
   else
     {
         efl_ui_collection_item_scroll(data, first, EINA_TRUE);
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
   efl_ui_collection_item_scroll(data, middle, EINA_FALSE);
   //give time to stabelize
   efl_add(EFL_LOOP_TIMER_CLASS, efl_main_loop_get(),
      efl_loop_timer_interval_set(efl_added, 15.0),
      efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _started_cb, data)
   );
   efl_event_callback_del(ev->object, EFL_CANVAS_SCENE_EVENT_RENDER_POST, _first_frame_cb, data);
}

static void
_build_list(int items)
{

   collection = efl_add(EFL_UI_LIST_CLASS, win);
   efl_content_set(win, collection);

   for (int i = 0; i < items; ++i)
     {
        Eo *il = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, collection);
        double r = 10+((double)190/(double)10)*(i%10);

        if (i == 0)
          first = il;
        else if (i == items/2)
          middle = il;
        else if (i == (items - 1))
          last = il;
        efl_gfx_color_set(il, r, 10, 10, 255);
        efl_gfx_hint_size_min_set(il, EINA_SIZE2D(40, 40+(i%2)*40));
        efl_pack_end(collection, il);
     }
}

static void
_build_grid(int items)
{
   collection = efl_add(EFL_UI_GRID_CLASS, win);
   efl_content_set(win, collection);

   for (int i = 0; i < items; ++i)
     {
        Eo *il = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, collection);
        double r = 10+((double)190/(double)10)*(i%10);

        if (i == 0)
          first = il;
        else if (i == items/2)
          middle = il;
        else if (i == (items - 1))
          last = il;
        efl_gfx_color_set(il, r, 10, 10, 255);
        efl_gfx_hint_size_min_set(il, EINA_SIZE2D(80, 150+(i%2)*40));
        efl_pack_end(collection, il);
     }
}

static void
print_help(void)
{
   printf("The following options are optional:\n");
   printf(" --list Run the benchmark with the list position manager.\n");
   printf(" --grid Run the benchmark with the list position manager.\n");
   printf(" --items X Run the benchmark with X items.\n");
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Accessor *cml;
   const char *part;
   int c;
   int items = 5000;
   Eina_Bool grid = EINA_FALSE;

   printf("Started on pid: %d\n", getpid());

   cml = efl_core_command_line_command_access(efl_main_loop_get());

   EINA_ACCESSOR_FOREACH(cml, c, part)
     {
        if (c == 0) continue;
        if (eina_streq(part, "--items"))
          {
             c++;
             EINA_SAFETY_ON_FALSE_GOTO(eina_accessor_data_get(cml, c, (void**)&part), err);
             items = atoi(part);
             if (!items)
               goto err;
          }
        else if (eina_streq(part, "--list"))
          {
             grid = EINA_FALSE;
          }
        else if (eina_streq(part, "--grid"))
          {
             grid = EINA_TRUE;
          }
        else
          goto err;
     }

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Item_Container benchmark"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE)
                );
   printf("Building %d objects\n", items);
   if (grid)
     _build_grid(items);
   else
     _build_list(items);
   printf("Done!\n");
   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 500));

   efl_event_callback_add(evas_object_evas_get(win), EFL_CANVAS_SCENE_EVENT_RENDER_POST, _first_frame_cb, collection);
   return;
err:
   print_help();
   efl_exit(-1);
}
EFL_MAIN()
