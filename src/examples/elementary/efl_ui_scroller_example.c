//Compile with:
//gcc -g efl_ui_scroller_example.c -o efl_ui_scroller_example `pkg-config --cflags --libs elementary`

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eo *win, *scroller, *content;
   char buf[64];

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_app_info_set(efl_main, "elementary", "images/plant_01.jpg");

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL, "TEST", ELM_WIN_BASIC,
		         efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 400));

   scroller = efl_add(EFL_UI_SCROLLER_CLASS, win);
   efl_content_set(win, scroller);

   content = efl_add(EFL_UI_IMAGE_CLASS, scroller);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   efl_file_set(content, buf, NULL);
   efl_gfx_entity_size_set(content, EINA_SIZE2D(5000, 5000));
   efl_content_set(scroller, content);
}
EFL_MAIN()
