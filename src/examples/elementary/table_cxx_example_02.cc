
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

   ::elm::win win(elm_win_util_standard_add("table", "Table"));
   win.autohide_set(true);

   ::elm::table table(efl::eo::parent = win);
   win.resize_object_add(table);
   table.visible_set(true);
   table.homogeneous_set(false);

   evas::rectangle rect(efl::eo::parent = win);
   rect.color_set( 255, 0, 0, 255);
   rect.size_hint_min_set( 100, 50);
   rect.visible_set(true);
   table.pack(rect, 0, 0, 2, 1);

   evas::rectangle rect2(efl::eo::parent = win);
   rect2.color_set( 0, 255, 0, 255);
   rect2.size_hint_min_set(50, 100);
   rect2.visible_set(true);
   table.pack(rect2, 0, 1, 1, 2);

   evas::rectangle rect3(efl::eo::parent = win);
   rect3.color_set( 0, 0, 255, 255);
   rect3.size_hint_min_set(50, 50);
   rect3.visible_set(true);
   table.pack(rect3, 1, 1, 1, 1);

   evas::rectangle rect4(efl::eo::parent = win);
   rect4.color_set( 255, 255, 0, 255);
   rect4.size_hint_min_set(50, 50);
   rect4.visible_set(true);
   table.pack(rect4, 1, 2, 1, 1);

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
