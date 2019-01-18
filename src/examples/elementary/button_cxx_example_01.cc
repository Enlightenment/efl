// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` button_cxx_example_01.cc -o button_cxx_example_01

#define EFL_CXXPERIMENTAL
#include <Elementary.hh>

using efl::eo::instantiate;

efl::gfx::Entity icon_still(nullptr);

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Button example");
   win.autohide_set(true);

   efl::ui::Button mid(instantiate, win);
   efl::ui::Button up(instantiate, win);
   efl::ui::Button down(instantiate, win);
   efl::ui::Button left(instantiate, win);
   efl::ui::Button right(instantiate, win);

   efl::ui::Box box(instantiate, win);
   win.content_set(box);

   efl::ui::Box box_initial(instantiate, win);
   box_initial.direction_set(EFL_UI_DIR_HORIZONTAL);
   box_initial.hint_weight_set(EFL_GFX_SIZE_HINT_EXPAND, 0.0);
   box.pack_end(box_initial);

   efl::ui::Button btn(instantiate, win);
   btn.text_set("Initial: 0.0");
   box_initial.pack_end(btn);

   auto wup = up._get_wref();
   auto wdown = up._get_wref();
   auto wleft = up._get_wref();
   auto wright = up._get_wref();
   auto btn_options = std::bind(
            [wup, wdown, wleft, wright] (efl::ui::Button &button)
   {
        efl::ui::Button b(efl::eo::downcast<efl::ui::Button>(button));
        efl::eina::optional<std::string> lbl = b.text_get();
        std::string::size_type ptr;

        ptr = lbl->find(":");
        ptr += 2;
        double t = std::stod(lbl->substr(ptr));

        if (lbl->compare(0,7,"Initial") == 0)
          {
             wup->autorepeat_initial_timeout_set(t);
             wdown->autorepeat_initial_timeout_set(t);
             wleft->autorepeat_initial_timeout_set(t);
             wright->autorepeat_initial_timeout_set(t);
          }
        else if (lbl->compare(0,3,"Gap") == 0)
          {
             wup->autorepeat_gap_timeout_set(t);
             wdown->autorepeat_gap_timeout_set(t);
             wleft->autorepeat_gap_timeout_set(t);
             wright->autorepeat_gap_timeout_set(t);
          }
     }, std::placeholders::_1);
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, btn, btn_options);

   efl::ui::Button btn2(instantiate, win);
   btn2.text_set("Initial: 1.0");
   box_initial.pack_end(btn2);
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, btn2, btn_options);

   efl::ui::Button btn3(instantiate, win);
   btn3.text_set("Initial: 5.0");
   box_initial.pack_end(btn3);
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, btn3, btn_options);

   efl::ui::Box box_gap(instantiate, win);
   box_gap.direction_set(EFL_UI_DIR_HORIZONTAL);
   box_gap.hint_weight_set(EFL_GFX_SIZE_HINT_EXPAND, 0.0);
   box.pack_end(box_gap);

   efl::ui::Button btn4(instantiate, win);
   btn4.text_set("Gap: 0.1");
   box_gap.pack_end(btn4);
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, btn4, btn_options);

   efl::ui::Button btn5(instantiate, win);
   btn5.text_set("Gap: 0.5");
   box_gap.pack_end(btn5);
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, btn5, btn_options);

   efl::ui::Button btn6(instantiate, win);
   btn6.text_set("Gap: 1.0");
   box_gap.pack_end(btn6);
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, btn6, btn_options);

   up.autorepeat_enabled_set(true);
   up.autorepeat_initial_timeout_set(1.0);
   up.autorepeat_gap_timeout_set(0.5);
   up.hint_weight_set(EFL_GFX_SIZE_HINT_EXPAND, 0.0);
   up.hint_fill_set(true, false);
   up.hint_align_set(0.5, 0.0);
   box.pack_end(up);

   auto wmid = mid._get_wref();
   auto btn_cursors_move = std::bind(
            [wmid, wup, wdown, wleft, wright] (efl::ui::Button &button)
   {
        efl::ui::Button b(efl::eo::downcast<efl::ui::Button>(button));
        auto pmid = wmid.lock();
        double ax, ay;

        if (!pmid) return;

        auto cmid = pmid.get();
        if (!icon_still)
          {
             efl::ui::Image obj(instantiate, cmid);
             efl::eina::optional<efl::gfx::Entity> icon_still_p = cmid.content_unset();
             if (icon_still_p)
               {
                  icon_still = *icon_still_p;
                  icon_still.visible_set(false);
               }
             obj.icon_set("chat");
             cmid.content_set(obj);
          }

        cmid.hint_align_get(ax, ay);
        if (b == wup)
          {
             ay -= 0.05;
             if (ay < 0.0)
               ay = 0.0;
          }
        else if (b == wdown)
          {
             ay += 0.05;
             if (ay > 1.0)
               ay = 1.0;
          }
        else if (b == wleft)
          {
             ax -= 0.05;
             if (ax < 0.0)
               ax = 0.0;
          }
        else if (b == wright)
          {
             ax += 0.05;
             if (ax > 1.0)
               ax = 1.0;
          }
        cmid.hint_align_set(ax, ay);
     }, std::placeholders::_1);
   efl::eolian::event_add(efl::ui::Clickable::repeated_event, up, btn_cursors_move);

   auto btn_cursors_release = std::bind(
            [wmid] ()
   {
        if (icon_still)
          {
             wmid->content_set(icon_still);
             icon_still = nullptr;
          }
     });
   efl::eolian::event_add(efl::ui::Clickable::unpressed_event, up, btn_cursors_release);

   efl::ui::Image icon_up(instantiate, win);
   icon_up.icon_set("arrow_up");
   up.content_set(icon_up);

   efl::ui::Box box_inferior(instantiate, win);
   box_inferior.direction_set(EFL_UI_DIR_HORIZONTAL);
   box.pack_end(box_inferior);
   box_inferior.visible_set(true);

   left.autorepeat_enabled_set(true);
   left.autorepeat_initial_timeout_set(1.0);
   left.autorepeat_gap_timeout_set(0.5);
   left.hint_weight_set(0.0, EFL_GFX_SIZE_HINT_EXPAND);
   left.hint_fill_set(false, true);
   left.hint_align_set(0.0, 0.5);
   box_inferior.pack_end(left);
   efl::eolian::event_add(efl::ui::Clickable::repeated_event, left, btn_cursors_move);
   efl::eolian::event_add(efl::ui::Clickable::unpressed_event, left, btn_cursors_release);

   efl::ui::Image icon_left(instantiate, win);
   icon_left.icon_set("arrow_left");
   left.content_set(icon_left);

   box_inferior.pack_end(mid);

   efl::ui::Image icon_mid(instantiate, win);
   icon_mid.icon_set("close");
   mid.content_set(icon_mid);

   right.autorepeat_enabled_set(true);
   right.autorepeat_initial_timeout_set(1.0);
   right.autorepeat_gap_timeout_set(0.5);
   right.hint_weight_set(0.0, EFL_GFX_SIZE_HINT_EXPAND);
   right.hint_fill_set(false, true);
   right.hint_align_set(0.0, 0.5);
   box_inferior.pack_end(right);
   efl::eolian::event_add(efl::ui::Clickable::repeated_event, right, btn_cursors_move);
   efl::eolian::event_add(efl::ui::Clickable::unpressed_event, right, btn_cursors_release);

   efl::ui::Image icon_right(instantiate, win);
   icon_right.icon_set("arrow_right");
   right.content_set(icon_right);

   down.autorepeat_enabled_set(true);
   down.autorepeat_initial_timeout_set(1.0);
   down.autorepeat_gap_timeout_set(0.5);
   down.hint_weight_set(EFL_GFX_SIZE_HINT_EXPAND, 0.0);
   down.hint_fill_set(true, false);
   down.hint_align_set(0.5, 0.0);
   box.pack_end(down);
   efl::eolian::event_add(efl::ui::Clickable::repeated_event, down, btn_cursors_move);
   efl::eolian::event_add(efl::ui::Clickable::unpressed_event, down, btn_cursors_release);

   efl::ui::Image icon_down(instantiate, win);
   icon_down.icon_set("arrow_down");
   down.content_set(icon_down);

   win.size_set({300, 320});
}
EFL_MAIN()
