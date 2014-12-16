#include <Elementary.hh>

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Button example");
   win.autohide_set(true);

   ::evas::object icon_still(nullptr);
   ::elm::button mid(efl::eo::parent = win);
   ::elm::button up(efl::eo::parent = win);
   ::elm::button down(efl::eo::parent = win);
   ::elm::button left(efl::eo::parent = win);
   ::elm::button right(efl::eo::parent = win);

   ::elm::box box(efl::eo::parent = win);
   box.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(box);
   box.visible_set(true);

   ::elm::box box_initial(efl::eo::parent = win);
   box_initial.horizontal_set(true);
   box_initial.size_hint_weight_set(EVAS_HINT_EXPAND, 0.0);
   box.pack_end(box_initial);
   box_initial.visible_set(true);

   ::elm::button btn(efl::eo::parent = win);
   btn.text_set("elm.text", "Initial: 0.0");
   box_initial.pack_end(btn);
   btn.visible_set(true);

   auto btn_options = std::bind([&] (evas::clickable_interface ci)
                      {
                         ::elm::button b(eo_ref(ci._eo_ptr()));
                         std::string::size_type ptr;
                         efl::eina::optional<std::string> lbl = b.text_get("elm.text");

                         ptr = lbl->find(":");
                         ptr += 2;
                         double t = std::stod(lbl->substr(ptr));

                         if (lbl->compare(0,7,"Initial") == 0)
                           {
                              up.autorepeat_initial_timeout_set(t);
                              down.autorepeat_initial_timeout_set(t);
                              left.autorepeat_initial_timeout_set(t);
                              right.autorepeat_initial_timeout_set(t);
                           }
                         else if (lbl->compare(0,3,"Gap") == 0)
                           {
                              up.autorepeat_gap_timeout_set(t);
                              down.autorepeat_gap_timeout_set(t);
                              left.autorepeat_gap_timeout_set(t);
                              right.autorepeat_gap_timeout_set(t);
                           }
                      }, std::placeholders::_1);

   btn.callback_clicked_add(btn_options);

   ::elm::button btn2(efl::eo::parent = win);
   btn2.text_set("elm.text", "Initial: 1.0");
   box_initial.pack_end(btn2);
   btn2.visible_set(true);
   btn2.callback_clicked_add(btn_options);

   ::elm::button btn3(efl::eo::parent = win);
   btn3.text_set("elm.text", "Initial: 5.0");
   box_initial.pack_end(btn3);
   btn3.visible_set(true);
   btn3.callback_clicked_add(btn_options);

   ::elm::box box_gap(efl::eo::parent = win);
   box_gap.horizontal_set(true);
   box_gap.size_hint_weight_set(EVAS_HINT_EXPAND, 0.0);
   box.pack_end(box_gap);
   box_gap.visible_set(true);

   ::elm::button btn4(efl::eo::parent = win);
   btn4.text_set("elm.text", "Gap: 0.1");
   box_gap.pack_end(btn4);
   btn4.visible_set(true);
   btn4.callback_clicked_add(btn_options);

   ::elm::button btn5(efl::eo::parent = win);
   btn5.text_set("elm.text", "Gap: 0.5");
   box_gap.pack_end(btn5);
   btn5.visible_set(true);
   btn5.callback_clicked_add(btn_options);

   ::elm::button btn6(efl::eo::parent = win);
   btn6.text_set("elm.text", "Gap: 1.0");
   box_gap.pack_end(btn6);
   btn6.visible_set(true);
   btn6.callback_clicked_add(btn_options);

   up.autorepeat_set(true);
   up.autorepeat_initial_timeout_set(1.0);
   up.autorepeat_gap_timeout_set(0.5);
   up.size_hint_weight_set(EVAS_HINT_EXPAND, 0.0);
   up.size_hint_align_set(EVAS_HINT_FILL, 0.0);
   box.pack_end(up);
   up.visible_set(true);

   auto btn_cursors_move = std::bind([&] (evas::clickable_interface ci2)
                           {
                              ::elm::button button = efl::eo::downcast<elm::button>(ci2);
                              double ax, ay;

                              if (!icon_still)
                                {
                                   ::elm::icon obj(efl::eo::parent = mid);
                                   efl::eina::optional<evas::object> icon_still_p = mid.content_unset("icon");
                                   if(icon_still_p)
                                     {
                                        icon_still = *icon_still_p;
                                        icon_still.visible_set(false);
                                     }
                                   obj.standard_set("chat");
                                   mid.content_set("icon", obj);
                                }

                              mid.size_hint_align_get(&ax, &ay);
                              if (button == up)
                                {
                                   ay -= 0.05;
                                   if (ay < 0.0)
                                     ay = 0.0;
                                }
                              else if (button == down)
                                {
                                   ay += 0.05;
                                   if (ay > 1.0)
                                     ay = 1.0;
                                }
                              else if (button == left)
                                {
                                   ax -= 0.05;
                                   if (ax < 0.0)
                                     ax = 0.0;
                                }
                              else if (button == right)
                                {
                                   ax += 0.05;
                                   if (ax > 1.0)
                                     ax = 1.0;
                                }
                              mid.size_hint_align_set(ax, ay);
                           }, std::placeholders::_1);

   up.callback_repeated_add(btn_cursors_move);

   auto btn_cursors_release = std::bind([&] ()
                              {
                                 if (icon_still)
                                   {
                                      mid.content_set("icon", icon_still);
                                      icon_still = evas::object(nullptr);
                                   }
                              });

   up.callback_unpressed_add(btn_cursors_release);

   ::elm::icon icon_up(efl::eo::parent = win);
   icon_up.standard_set("arrow_up");
   up.content_set("icon", icon_up);

   ::elm::box box_inferior(efl::eo::parent = win);
   box_inferior.horizontal_set(true);
   box_inferior.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   box_inferior.size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL);
   box.pack_end(box_inferior);
   box_inferior.visible_set(true);

   left.autorepeat_set(true);
   left.autorepeat_initial_timeout_set(1.0);
   left.autorepeat_gap_timeout_set(0.5);
   left.size_hint_weight_set(0.0, EVAS_HINT_EXPAND);
   left.size_hint_align_set(0.0, EVAS_HINT_FILL);
   box_inferior.pack_end(left);
   left.visible_set(true);
   left.callback_repeated_add(btn_cursors_move);
   left.callback_unpressed_add(btn_cursors_release);

   ::elm::icon icon_left(efl::eo::parent = win);
   icon_left.standard_set("arrow_left");
   left.content_set("icon", icon_left);

   mid.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   box_inferior.pack_end(mid);
   mid.visible_set(true);

   ::elm::icon icon_mid(efl::eo::parent = win);
   icon_mid.standard_set("close");
   mid.content_set("icon", icon_mid);

   right.autorepeat_set(true);
   right.autorepeat_initial_timeout_set(1.0);
   right.autorepeat_gap_timeout_set(0.5);
   right.size_hint_weight_set(0.0, EVAS_HINT_EXPAND);
   right.size_hint_align_set(0.0, EVAS_HINT_FILL);
   box_inferior.pack_end(right);
   right.visible_set(true);
   right.callback_repeated_add(btn_cursors_move);
   right.callback_unpressed_add(btn_cursors_release);

   ::elm::icon icon_right(efl::eo::parent = win);
   icon_right.standard_set("arrow_right");
   right.content_set("icon", icon_right);

   down.autorepeat_set(true);
   down.autorepeat_initial_timeout_set(1.0);
   down.autorepeat_gap_timeout_set(0.5);
   down.size_hint_weight_set(EVAS_HINT_EXPAND, 0.0);
   down.size_hint_align_set(EVAS_HINT_FILL, 0.0);
   box.pack_end(down);
   down.visible_set(true);
   down.callback_repeated_add(btn_cursors_move);
   down.callback_unpressed_add(btn_cursors_release);

   ::elm::icon icon_down(efl::eo::parent = win);
   icon_down.standard_set("arrow_down");
   down.content_set("icon", icon_down);

   win.size_set(300, 320);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
