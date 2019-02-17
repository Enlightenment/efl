/*
 * edje_cc efl_ui_theme_example.edc efl_ui_theme_example.edj
 * gcc -o efl_ui_theme_example_02 efl_ui_theme_example_02.c `pkg-config --cflags --libs elementary`
 */
#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1

#define EFL_UI_WIDGET_PROTECTED
#include <Elementary.h>

#define EXAMPLE_EDJ_FILE_PATH "./efl_ui_theme_example.edj"

static void
_btn_overlay_clicked_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   static Eina_Bool loaded = EINA_TRUE;
   Efl_Ui_Theme *default_theme;

   default_theme = efl_ui_theme_default_get();

   if (loaded)
     {
        efl_ui_theme_overlay_del(default_theme, EXAMPLE_EDJ_FILE_PATH);
        efl_text_set(event->object, "Load overlay");
     }
   else
     {
        efl_ui_theme_overlay_add(default_theme, EXAMPLE_EDJ_FILE_PATH);
        efl_text_set(event->object, "Unload overlay");
     }

   loaded = !loaded;
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win, *box;
   Efl_Ui_Theme *default_theme;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   if (!ecore_file_exists(EXAMPLE_EDJ_FILE_PATH))
     {
        printf("efl_ui_theme_example.edj does not exist!\n"
               "Please execute the following command first!\n"
               "edje_cc efl_ui_theme_example.edc efl_ui_theme_example.edj\n");
        efl_exit(0);
     }

   default_theme = efl_ui_theme_default_get();
   efl_ui_theme_overlay_add(default_theme, EXAMPLE_EDJ_FILE_PATH);

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Theme example"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE)
                );

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_content_set(win, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Unload overlay"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _btn_overlay_clicked_cb, NULL),
           efl_pack_end(box, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_ui_widget_style_set(efl_added, "custom"),
           efl_text_set(efl_added, "custom style"),
           efl_pack_end(box, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "default style"),
           efl_pack_end(box, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_ui_widget_style_set(efl_added, "anchor"),
           efl_text_set(efl_added, "anchor style"),
           efl_pack_end(box, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 320));
}
EFL_MAIN()
