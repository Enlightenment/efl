/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <Efl_Ui.hh>

EAPI_MAIN int
elm_main (int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   efl::eina::eina_init eina_init;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(efl::eo::instantiate);
   win.text_set("Menu Example");
   win.autohide_set(true);

#if 0
   evas::rectangle rect(efl::eo::parent = win);
   win.resize_object_add(rect);
   rect.size_hint_min_set(0, 0);
   rect.color_set( 0, 0, 0, 0);
   rect.visible_set(true);

   ::elm::widget_item no_parent(nullptr);
   ::elm::menu menu(efl::eo::parent = win);
   menu.item_add(no_parent, nullptr, "first item", nullptr, nullptr);
   elm::widget_item 
     menu_it = menu.item_add(no_parent, "mail-reply-all", "second item", nullptr, nullptr);

   menu.item_add(menu_it, "object-rotate-left", "menu 1", NULL, NULL);
   ::elm::button button(efl::eo::parent = win);
   button.text_set("elm.text", "button - delete items");
   elm::widget_item menu_it1
     = menu.item_add(menu_it, nullptr, nullptr, NULL, NULL);
   menu_it1.part_content_set(nullptr, button);

   auto del_it = std::bind([&] ()
                 {
                   auto list = efl::eo::downcast<elm::menu_item>(menu_it).subitems_get();
                    for(auto& item : list)
                      {
                         elm_object_item_del(item._eo_ptr());
                      }
                 });

   button.callback_clicked_add(del_it);
   menu.item_separator_add(menu_it);
   menu.item_add(menu_it, nullptr, "third item", NULL, NULL);
   menu.item_add(menu_it, nullptr, "fourth item", NULL, NULL);
   menu.item_add(menu_it, "window-new", "sub menu", NULL, NULL);

   elm::widget_item menu_it2 = menu.item_add(no_parent, nullptr, "third item", nullptr, nullptr);
   menu_it2.disabled_set(EINA_TRUE);

   auto show = std::bind([&] (void *event_info)
               {
                  Evas_Event_Mouse_Down *ev = static_cast<Evas_Event_Mouse_Down*>(event_info);
                  menu.move(ev->canvas.x, ev->canvas.y);
                  menu.visible_set(true);
               }, std::placeholders::_3);

   rect.callback_mouse_down_add( show );
   menu.visible_set(true);

#endif
   win.size_set({250, 350});

   elm_run();
   return 0;
}
ELM_MAIN()
