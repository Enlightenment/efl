//FIXME: change to test style, not example style

#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif

#include <Elementary.h>

#define WIDTH 800
#define HEIGHT 400

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
    Evas_Object *win, *txtpath;

    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    win = elm_win_util_standard_add("efl.ui.textpath", "TextPath test");
    elm_win_autodel_set(win, EINA_TRUE);

    txtpath = efl_add(EFL_UI_TEXTPATH_CLASS, win);
    efl_ui_textpath_circle_set(txtpath, 200, 200, 100, 45);
    //efl_ui_textpath_autofit_set(txtpath, EINA_TRUE);
    //efl_text_set(txtpath, "abcdef and more you can see from this test");
    efl_text_set(txtpath, "abcdefghijklm");
    efl_gfx_geometry_set(txtpath, 0, 0, 200, 200);
    efl_gfx_visible_set(txtpath, EINA_TRUE);

    elm_win_resize_object_add(win, txtpath);
    evas_object_resize(win, WIDTH, HEIGHT);
    evas_object_show(win);
    elm_run();
}
ELM_MAIN();
