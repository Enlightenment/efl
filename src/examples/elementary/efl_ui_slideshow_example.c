#include <Efl_Ui.h>

#include <Elementary.h>

static Efl_Ui_Active_View_Container *container;
static Eina_Bool play_state = EINA_FALSE;
static Efl_Loop_Timer *show_timer = NULL;

char *img_files[] = {
  "logo.png",
  "plant_01.jpg",
  "rock_01.jpg",
  "rock_02.jpg",
  "sky_01.jpg",
  "wood_01.jpg",
  "mystrale.jpg",
  "mystrale_2.jpg"
};
#define IMG_NUM 8

static void
_show_next(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   if (!show_timer) return;
   efl_del(show_timer);
   show_timer = NULL;

   int current_index = efl_ui_active_view_active_index_get(container);
   int new_index = (current_index + 1) % efl_content_count(container);
   efl_ui_active_view_active_index_set(container, new_index);
}

static void
put_play(void)
{
   play_state = EINA_TRUE;
   show_timer = efl_add(EFL_LOOP_TIMER_CLASS, container,
                        efl_loop_timer_interval_set(efl_added, 2.0),
                        efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _show_next, NULL));
}

static void
put_pause(void)
{
   play_state = EINA_FALSE;
   if (show_timer)
     efl_del(show_timer);
   show_timer = NULL;

}

static void
_container_end(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   if (play_state)
     {
        show_timer = efl_add(EFL_LOOP_TIMER_CLASS, efl_app_main_get(),
                             efl_loop_timer_interval_set(efl_added, 2.0),
                             efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _show_next, NULL));
     }
}

static void
_play_toggle(void *data EINA_UNUSED, const Efl_Event *ev)
{
   if (play_state)
     {
        put_pause();
        efl_text_set(ev->object, "Play");
     }
   else
     {
        put_play();
        efl_text_set(ev->object, "Pause");
     }
}

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Win *win;
   Efl_Ui_Table *table;

   elm_app_info_set(efl_main, "elementary", "images");

   win = efl_new(EFL_UI_WIN_CLASS,
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE),
                 efl_text_set(efl_added, "Slideshow Example")
   );
   table = efl_add(EFL_UI_TABLE_CLASS, win);
   efl_content_set(win, table);

   container = efl_add(EFL_UI_ACTIVE_VIEW_CONTAINER_CLASS, win,
                       efl_event_callback_add(efl_added, EFL_UI_ACTIVE_VIEW_EVENT_TRANSITION_END, _container_end, NULL),
                       efl_pack_table(table, efl_added, 0, 0, 1, 1));
   efl_ui_active_view_indicator_set(container, efl_new(EFL_UI_ACTIVE_VIEW_INDICATOR_ICON_CLASS));
   efl_ui_active_view_manager_set(container, efl_new(EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_STACK_CLASS));

   for (int i = 0; i < IMG_NUM; i++)
     {
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/images/%s", elm_app_data_dir_get(), img_files[i]);
        efl_add(EFL_UI_IMAGE_CLASS, container,
                efl_file_set(efl_added, path),
                efl_file_load(efl_added),
                efl_pack_end(container, efl_added));
     }

   efl_add(EFL_UI_BUTTON_CLASS, table,
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack_table(table, efl_added, 0, 1, 1, 1),
           efl_text_set(efl_added, "Play"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _play_toggle, NULL));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 200));
}

EFL_MAIN();
