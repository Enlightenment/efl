//Compile with:
//gcc -g efl_ui_scroller_example.c -o efl_ui_scroller_example `pkg-config --cflags --libs elementary`

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Eo *win, *scroller, *content;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = efl_add(EFL_UI_WIN_CLASS, NULL, "TEST", ELM_WIN_BASIC);
   efl_gfx_size_set(win, 300, 400);
   efl_gfx_visible_set(win, EINA_TRUE);

   scroller = efl_add(EFL_UI_SCROLLER_CLASS, win);
   efl_gfx_visible_set(scroller, EINA_TRUE);
   elm_win_resize_object_add(win, scroller);
   evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   elm_run();

   return 0;
}
ELM_MAIN()
