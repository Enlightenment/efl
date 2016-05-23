
#ifdef HAVE_CONFIG_H
# include "config.h"
# include "elementary_config.h"
#endif

#include <Elementary.hh>
#include <Evas.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Separator");
   win.autohide_set(true);

   ::elm::bg bg(efl::eo::parent = win);
   bg.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bg);
   bg.visible_set(true);

   ::elm::box bx(efl::eo::parent = win);
   bx.horizontal_set(true);
   bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bx);
   bx.visible_set(true);

   evas::rectangle rect(efl::eo::parent = win);
   rect.color_set( 0, 255, 0, 255);
   rect.size_hint_min_set( 90, 200);
   rect.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   rect.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   rect.visible_set(true);
   bx.pack_end(rect);

   ::elm::separator separator(efl::eo::parent = win);
   separator.horizontal_set(true);
   separator.visible_set(true);
   bx.pack_end(separator);

   evas::rectangle rect2(efl::eo::parent = win);
   rect2.color_set( 0, 0, 255, 255);
   rect2.size_hint_min_set( 90, 200);
   rect2.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   rect2.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   rect2.visible_set(true);
   bx.pack_end(rect2);

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

