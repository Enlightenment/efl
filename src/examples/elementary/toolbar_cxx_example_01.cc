#define ELM_WIDGET_PROTECTED
#define ELM_WIDGET_CLASS_PROTECTED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#warning This example requires yet unfinished EO APIs

#define EFL_BETA_API_SUPPORT
#include <Elementary.hh>

EAPI int
elm_main(int argc, char* argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   using efl::eo::instantiate;

   efl::ui::Win win_1
     (instantiate,
      [&]
      {
        win_1.text_set("Toolbar");
        win_1.win_name_set("toolbar");
        win_1.win_type_set(EFL_UI_WIN_TYPE_BASIC);
      });

#if 0
   win_1.eo_cxx::efl::Gfx::size_set({320, 300});

   efl::ui::Box box_1(instantiate, win_1);
   box_1.hint_weight_set(1.0, 1.0);
   box_1.visible_set(true);

   elm::Toolbar toolbar_1(instantiate, win_1);
   toolbar_1.shrink_mode_set(ELM_TOOLBAR_SHRINK_MENU);
   toolbar_1.hint_weight_set(0, 0);
   toolbar_1.hint_align_set(0.5, 0);
   toolbar_1.hint_fill_set(ture, false);
   toolbar_1.menu_parent_set(win_1);
   elm::toolbar::Item item_1 = efl::eo::downcast<elm::toolbar::Item>
     (toolbar_1.item_append("document-print", "Hello", nullptr, nullptr));
   item_1.disabled_set(true);
   item_1.priority_set(100);
   elm::toolbar::Item item_2 = efl::eo::downcast<elm::toolbar::Item>
     (toolbar_1.item_append("folder-new", "World", nullptr, nullptr));
   item_2.priority_set(100);
   elm::toolbar::Item item_3 = efl::eo::downcast<elm::toolbar::Item>
     (toolbar_1.item_append("object-rotate-right", "H", nullptr, nullptr));
   item_3.priority_set(150);
   elm::toolbar::Item item_4 = efl::eo::downcast<elm::toolbar::Item>
     (toolbar_1.item_append("mail-send", "Comes", nullptr, nullptr));
   item_4.priority_set(0);
   elm::toolbar::Item item_5 = efl::eo::downcast<elm::toolbar::Item>
     (toolbar_1.item_append("clock", "Elementary", nullptr, nullptr));
   item_5.priority_set(200);
   elm::toolbar::Item item_6 = efl::eo::downcast<elm::toolbar::Item>
     (toolbar_1.item_append("refresh", "Menu", nullptr, nullptr));
   item_6.menu_set(true);
   item_6.priority_set(9999);
   elm::Menu menu_1 = efl::eo::downcast<elm::Menu>
     (item_6.menu_get());
   elm::menu::Item item_7 = efl::eo::downcast<elm::menu::Item>
     (menu_1.item_add(nullptr, "edit-cut", "Shrink", nullptr, nullptr));
   elm::menu::Item item_8 = efl::eo::downcast<elm::menu::Item>
     (menu_1.item_add(nullptr, "edit-copy", "Mode", nullptr, nullptr));
   elm::menu::Item item_9 = efl::eo::downcast<elm::menu::Item>
     (menu_1.item_add(item_8, "edit-paste", "is set to", nullptr, nullptr));
   elm::menu::Item item_10 = efl::eo::downcast<elm::menu::Item>
     (menu_1.item_add(nullptr, "edit-delete", "Menu", nullptr, nullptr));

   box_1.pack_end(toolbar_1);
   elm::Widget table_1(elm_table_add(win_1._eo_ptr()));
   table_1.hint_weight_set(0.0, 1.0);
   table_1.hint_fill_set(true, true);
   table_1.visible_set(true);

   elm::Widget photo_1(elm_photo_add(win_1._eo_ptr()));
   elm_photo_size_set(photo_1._eo_ptr(), 40);
   efl_file_simple_load(photo_1._eo_ptr(), "/opt/e/share/elementary/images/plant_01.jpg", nullptr);
   photo_1.hint_weight_set(1, 1);
   photo_1.hint_align_set(0.5, 0.5);
   photo_1.visible_set(true);

   elm_table_pack(table_1._eo_ptr(), photo_1._eo_ptr(), 0, 0, 1, 1);
   elm::Widget photo_2(elm_photo_add(win_1._eo_ptr()));
   elm_photo_size_set(photo_2._eo_ptr(), 80);
   photo_2.hint_weight_set(1, 1);
   photo_2.hint_align_set(0.5, 0.5);
   photo_2.visible_set(true);

   elm_table_pack(table_1._eo_ptr(), photo_2._eo_ptr(), 1, 0, 1, 1);
   elm::Widget photo_3(elm_photo_add(win_1._eo_ptr()));
   elm_photo_size_set(photo_3._eo_ptr(), 20);
   efl_file_simple_load(photo_3._eo_ptr(), "/opt/e/share/elementary/images/sky_01.jpg", nullptr);
   photo_3.hint_weight_set(1, 1);
   photo_3.hint_align_set(0.5, 0.5);
   photo_3.visible_set(true);

   elm_table_pack(table_1._eo_ptr(), photo_3._eo_ptr(), 0, 1, 1, 1);
   elm::Widget photo_4(elm_photo_add(win_1._eo_ptr()));
   elm_photo_size_set(photo_4._eo_ptr(), 60);
   efl_file_simple_load(photo_4._eo_ptr(), "/opt/e/share/elementary/images/sky_02.jpg", nullptr);
   photo_4.hint_weight_set(1, 1);
   photo_4.hint_align_set(0.5, 0.5);
   photo_4.visible_set(true);

   elm_table_pack(table_1._eo_ptr(), photo_4._eo_ptr(), 1, 1, 1, 1);
   box_1.pack_end(table_1);

   win_1.content_set(box_1);

   auto _item_2_selected_cb = std::bind([&] () {
       efl_file_simple_load(photo_1._eo_ptr(), "/opt/e/share/elementary/images/rock_01.jpg", nullptr);
     });
   efl::eolian::event_add(efl::ui::Selectable::selected_event, item_2, _item_2_selected_cb);

   auto _item_3_selected_cb = std::bind([&] () {
       efl_file_simple_load(photo_4._eo_ptr(), "/opt/e/share/elementary/images/wood_01.jpg", nullptr);
     });

   efl::eolian::event_add(efl::ui::Selectable::selected_event, item_3, _item_3_selected_cb);
   auto _item_4_selected_cb = std::bind([&] () {
       efl_file_simple_load(photo_4._eo_ptr(), "/opt/e/share/elementary/images/sky_03.jpg", nullptr);
     });

   efl::eolian::event_add(efl::ui::Selectable::selected_event, item_4, _item_4_selected_cb);
   auto _item_5_selected_cb = std::bind([&] () {
       efl_file_simple_load(photo_4._eo_ptr(), nullptr, nullptr);
     });

   efl::eolian::event_add(efl::ui::Selectable::selected_event, item_5, _item_5_selected_cb);

#endif
   
   elm_run();
   return 0;
}
ELM_MAIN()
