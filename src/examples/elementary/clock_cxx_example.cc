#ifdef HAVE_CONFIG_H
#include "config.h"
#include "elementary_config.h"
#endif

#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::win::Standard win;
   //win.title_set("Clock Example");
   win.autohide_set(true);

   efl::ui::Box bx(win);
   //bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //win.resize_object_add(bx);
   bx.eo_cxx::efl::Gfx::size_set(300,300);
   bx.visible_set(true);

   ::elm::Clock ck(win);
   bx.pack_end(ck);
   ck.visible_set(true);

   ::elm::Clock ck2(win);
   ck2.show_am_pm_set(true);
   bx.pack_end(ck2);
   ck2.visible_set(true);

   ::elm::Clock ck3(win);
   ck3.show_seconds_set(true);
   ck3.time_set(10, 11, 12);
   bx.pack_end(ck3);
   ck3.visible_set(true);

   ::elm::Clock ck4(win);
   ck4.edit_set(true);
   ck4.show_seconds_set(true);
   ck4.show_am_pm_set(true);
   ck4.time_set(10, 11, 12);
   bx.pack_end(ck4);
   ck4.visible_set(true);

   ::elm::Clock ck5(win);
   ck5.show_seconds_set(true);
   ck5.edit_set(true);
   int digedit = ELM_CLOCK_EDIT_HOUR_UNIT | ELM_CLOCK_EDIT_MIN_UNIT | ELM_CLOCK_EDIT_SEC_UNIT;
   ck5.edit_mode_set(static_cast<Elm_Clock_Edit_Mode>(digedit));
   bx.pack_end(ck5);
   ck5.visible_set(true);

   win.eo_cxx::efl::Gfx::size_set(500,500);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
