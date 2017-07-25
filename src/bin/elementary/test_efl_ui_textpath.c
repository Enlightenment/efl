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
    efl_ui_textpath_autofit_set(txtpath, EINA_TRUE);

    //efl_text_set(txtpath, "abcdef and more you can see from this test");
    //efl_text_set(txtpath, "abcd&lt;efghi&gt;jklm");
    efl_text_set(txtpath, "abcd&lt;eghi&gt;j");
    //elm_object_text_set(txtpath, "abcd&lt;eghi&gt;j");
    //efl_text_set(txtpath, "abcd&lt;eghi&gt;j more and more, so long to make it ellipsis? No, it is not enough, want to see more?xy");

    //efl_ui_textpath_circle_set(txtpath, 0, 0, 100, 0);
    //efl_gfx_path_append_circle(txtpath, 200, 200, 100);

    efl_gfx_path_append_arc(txtpath, 0, 0, 100, 100, 0, -90);
    efl_gfx_path_append_line_to(txtpath, 100, 150);

    //efl_gfx_path_append_move_to(txtpath, 200, 300);
    //efl_gfx_path_append_line_to(txtpath, 300, 350);

    //efl_gfx_path_append_move_to(txtpath, 100, 100);
    //efl_gfx_path_append_line_to(txtpath, 200, 300);

    efl_ui_textpath_ellipsis_set(txtpath, EINA_TRUE);
    efl_ui_textpath_autofit_set(txtpath, EINA_FALSE);
    efl_gfx_geometry_set(txtpath, 120, 120, 200, 200);
    efl_gfx_visible_set(txtpath, EINA_TRUE);

    //Geometry set/get: does not work!!!
    Evas_Coord x, y, w, h;
    efl_gfx_geometry_get(txtpath, &x, &y, &w, &h);
    printf("geo: %d %d %d %d\n", x, y, w, h);

    printf("text by efl: %s\n", efl_text_get(txtpath));
    printf("text by elm: %s\n", elm_object_text_get(txtpath));

    //elm_win_resize_object_add(win, txtpath);
    evas_object_resize(win, WIDTH, HEIGHT);
    evas_object_show(win);
    elm_run();
}
ELM_MAIN();
