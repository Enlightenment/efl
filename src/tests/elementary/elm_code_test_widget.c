#define ELM_INTERNAL_API_ARGESFSDFEFC

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_suite.h"
#include "Elementary.h"

#include "elm_priv.h"

#undef CRI
#define CRI(...) do { } while (0);

#undef ERR
#define ERR(...) do { } while (0);

#undef WRN
#define WRN(...) do { } while (0);

#undef INF
#define INF(...) do { } while (0);

#undef DBG
#define DBG(...) do { } while (0);

EFL_START_TEST (elm_code_widget_construct)
{
   Elm_Code *code;
   Elm_Code_Widget *widget;
   Evas_Object *win;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);

   ck_assert(!!widget);
   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_widget_construct_nocode)
{
   Elm_Code_Widget *widget;
   Evas_Object *win;

   char *args[] = { "exe" };
   elm_init(1, args);

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, NULL);
   ck_assert(!widget);

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_code_widget_position)
{
   Elm_Code *code;
   Elm_Code_Widget *widget;
   Evas_Object *win;
   Evas_Coord x, y, w, h, x2, y2, w2, h2;

   char *args[] = { "exe" };
   elm_init(1, args);
   code = elm_code_create();

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   widget = elm_code_widget_add(win, code);
   evas_object_show(widget);
   evas_object_resize(widget, 100, 100);

   elm_code_file_line_append(code->file, "some content", 12, NULL);
   elm_code_file_line_append(code->file, "more", 4, NULL);

   elm_code_widget_geometry_for_position_get(widget, 1, 1, &x, &y, &w, &h);
   elm_code_widget_geometry_for_position_get(widget, 1, 2, &x2, &y2, &w2, &h2);
   ck_assert(x2 > x);
   ck_assert(y2 == y);
   ck_assert(w2 == w);
   ck_assert(h2 == h);

   elm_code_widget_geometry_for_position_get(widget, 2, 1, &x2, &y2, &w2, &h2);
   ck_assert(x2 == x);
   ck_assert(w2 == w);
   ck_assert(h2 == h);

   elm_code_free(code);
   elm_shutdown();
}
EFL_END_TEST

void elm_code_test_widget(TCase *tc)
{
   tcase_add_test(tc, elm_code_widget_construct);
   tcase_add_test(tc, elm_code_widget_construct_nocode);
   tcase_add_test(tc, elm_code_widget_position);
}
