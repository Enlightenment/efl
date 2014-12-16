#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Clock Example");
   win.autohide_set(true);

   ::elm::box bx(efl::eo::parent = win);
   bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bx);
   bx.visible_set(true);

   ::elm::clock ck(efl::eo::parent = win);
   bx.pack_end(ck);
   ck.visible_set(true);

   ::elm::clock ck2(efl::eo::parent = win);
   ck2.show_am_pm_set(true);
   bx.pack_end(ck2);
   ck2.visible_set(true);

   ::elm::clock ck3(efl::eo::parent = win);
   ck3.show_seconds_set(true);
   ck3.time_set(10, 11, 12);
   bx.pack_end(ck3);
   ck3.visible_set(true);

   ::elm::clock ck4(efl::eo::parent = win);
   ck4.edit_set(true);
   ck4.show_seconds_set(true);
   ck4.show_am_pm_set(true);
   ck4.time_set(10, 11, 12);
   bx.pack_end(ck4);
   ck4.visible_set(true);

   ::elm::clock ck5(efl::eo::parent = win);
   ck5.show_seconds_set(true);
   ck5.edit_set(true);
   int digedit = ELM_CLOCK_EDIT_HOUR_UNIT | ELM_CLOCK_EDIT_MIN_UNIT | ELM_CLOCK_EDIT_SEC_UNIT;
   ck5.edit_mode_set(static_cast<Elm_Clock_Edit_Mode>(digedit));
   bx.pack_end(ck5);
   ck5.visible_set(true);

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
