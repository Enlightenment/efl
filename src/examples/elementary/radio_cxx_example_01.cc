#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   static int val = 1;

   ::elm::win_standard win;
   win.title_set("Radio");
   win.autohide_set(true);

   ::efl::ui::Box bx(efl::eo::parent = win);
   bx.horizontal_set(true);
   bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bx);
   bx.visible_set(true);

   ::elm::radio radio(efl::eo::parent = win);
   ::elm::radio group(efl::eo::parent = win);
   group = radio;
   radio.text_set("elm.text", "Radio 1");
   radio.state_value_set(1);
   radio.value_pointer_set(&val);
   ::elm::icon ic(efl::eo::parent = win);
   ic.standard_set("home");
   radio.content_set("icon", ic);
   bx.pack_end(radio);
   radio.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   radio.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   radio.visible_set(true);

   auto cb_val = std::bind([] () { std::cout << "val is now: " << val << std::endl; });

   radio.callback_changed_add(cb_val);

   ::elm::radio radio2(efl::eo::parent = win);
   radio2.text_set("elm.text", "Radio 2");
   radio2.state_value_set(2);
   radio2.value_pointer_set(&val);
   radio2.group_add(group);
   ::elm::icon ic2(efl::eo::parent = win);
   ic2.standard_set("file");
   radio2.content_set("icon", ic2);
   bx.pack_end(radio2);
   radio2.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   radio2.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   radio2.visible_set(true);
   radio2.callback_changed_add(cb_val);

   ::elm::radio radio3(efl::eo::parent = win);
   radio3.text_set("elm.text", "Radio 3");
   radio3.state_value_set(3);
   radio3.value_pointer_set(&val);
   radio3.group_add(group);
   bx.pack_end(radio3);
   radio3.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   radio3.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   radio3.visible_set(true);
   radio3.callback_changed_add(cb_val);

   ::elm::radio radio4(efl::eo::parent = win);
   radio4.text_set("elm.text", "Radio 4");
   radio4.state_value_set(4);
   radio4.value_pointer_set(&val);
   radio4.group_add(group);
   bx.pack_end(radio4);
   radio4.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   radio4.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   radio4.visible_set(true);
   radio4.callback_changed_add(cb_val);

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
