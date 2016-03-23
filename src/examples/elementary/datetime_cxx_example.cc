#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Datetime");
   win.autohide_set(true);

   ::elm::bg bg(efl::eo::parent = win);
   bg.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bg);
   bg.visible_set(true);

   ::elm::box bx(efl::eo::parent = win);
   bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bx);
   bx.visible_set(true);

   ::elm::datetime datetime(efl::eo::parent = bx);
   datetime.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   datetime.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   datetime.field_visible_set(ELM_DATETIME_HOUR, false);
   datetime.field_visible_set(ELM_DATETIME_MINUTE, false);
   datetime.field_visible_set(ELM_DATETIME_AMPM, false);
   bx.pack_end(datetime);
   datetime.visible_set(true);

   ::elm::datetime datetime2(efl::eo::parent = bx);
   datetime2.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   datetime2.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   datetime2.field_visible_set(ELM_DATETIME_YEAR, false);
   datetime2.field_visible_set(ELM_DATETIME_MONTH, false);
   datetime2.field_visible_set(ELM_DATETIME_DATE, false);
   bx.pack_end(datetime2);
   datetime2.visible_set(true);

   ::elm::datetime datetime3(efl::eo::parent = bx);
   datetime3.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   datetime3.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   bx.pack_end(datetime3);
   datetime3.visible_set(true);

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
