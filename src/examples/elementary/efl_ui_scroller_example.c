//Compile with:
//gcc -g efl_ui_scroller_example.c -o efl_ui_scroller_example `pkg-config --cflags --libs elementary`

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Eo *win, *scroller, *content;
   char buf[64];

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_app_info_set(elm_main, "elementary", "images/plant_01.jpg");

   win = efl_add(EFL_UI_WIN_CLASS, NULL, "TEST", ELM_WIN_BASIC);
   efl_gfx_size_set(win, 300, 400);
   efl_gfx_visible_set(win, EINA_TRUE);

   scroller = efl_add(EFL_UI_SCROLLER_CLASS, win);
   efl_gfx_visible_set(scroller, EINA_TRUE);
   efl_gfx_size_set(scroller, 200, 300);
   efl_gfx_position_set(scroller, 50, 50);

   content = elm_image_add(scroller);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_image_file_set(content, buf, NULL);
   efl_gfx_visible_set(content, EINA_TRUE);
   evas_object_size_hint_min_set(content, 500, 500);

   efl_content_set(scroller, content);

   elm_run();

   return 0;
}
ELM_MAIN()
