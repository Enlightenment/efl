
#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win win(elm_win_util_standard_add("table", "Table"));
   win.autohide_set(true);

   ::elm::table table(efl::eo::parent = win);
   win.resize_object_add(table);
   table.visible_set(true);
   table.padding_set(5, 5);
   table.homogeneous_set(true);

   ::elm::label label(efl::eo::parent = win);
   label.text_set(nullptr, "label 0");
   label.visible_set(true);
   table.pack(label, 0, 0, 1, 1);

   ::elm::label label1(efl::eo::parent = win);
   label1.text_set(nullptr, "label 1");
   label1.visible_set(true);
   table.pack(label1, 1, 0, 1, 1);

   ::elm::label label2(efl::eo::parent = win);
   label2.text_set(nullptr, "label 2");
   label2.visible_set(true);
   table.pack(label2, 0, 1, 1, 1);

   ::elm::label label3(efl::eo::parent = win);
   label3.text_set(nullptr, "label 3");
   label3.visible_set(true);
   table.pack(label3, 1, 1, 1, 1);

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

