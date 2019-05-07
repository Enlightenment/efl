// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` popup_cxx_example.cc -o popup_cxx_example

#define EFL_CXXPERIMENTAL

#include <Evas.hh>
#include <Efl_Ui.hh>
#include <iostream>

using efl::eo::instantiate;

static efl::ui::Popup g_popup(nullptr);
static efl::ui::Button g_repeat_events_test_btn(nullptr);

static efl::ui::Popup
create_popup(efl::ui::Win win)
{
   efl::ui::Popup popup(instantiate, win);

   efl::ui::Button btn(instantiate, popup);
   btn.text_set("Efl.Ui.Popup");
   popup.content_set(btn);

   popup.size_set({160, 160});

   auto timeout_cb(std::bind([]() {
        g_popup = nullptr;
        std::cout << "Popup times out and it is deleted" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Popup::timeout_event, popup, timeout_cb);


   return popup;
}

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);


   //Popup Window
   efl::ui::Win win(instantiate);
   win.text_set("Efl.Ui.Popup");
   win.autohide_set(true);
   win.size_set({500, 500});


   //Popup Function Window
   efl::ui::Win func_win(instantiate);
   func_win.text_set("Efl.Ui.Popup Function");
   func_win.autohide_set(true);
   func_win.size_set({300, 500});


   //Popup Window Delete callback
   auto wfunc_win(func_win._get_wref());
   auto win_del_cb(std::bind([wfunc_win]() {
        g_popup = nullptr;
        g_repeat_events_test_btn = nullptr;
        ::efl_del(wfunc_win->_eo_ptr());
        std::cout << "Window is deleted" << std::endl;
     }));
   efl::eolian::event_add(win.delete_request_event, win, win_del_cb);


   //Popup Function Window Delete callback
   auto wwin(win._get_wref());
   auto func_win_del_cb(std::bind([wwin]() {
        g_popup = nullptr;
        g_repeat_events_test_btn = nullptr;
        ::efl_del(wwin->_eo_ptr());
        std::cout << "Window is deleted" << std::endl;
     }));
   efl::eolian::event_add(func_win.delete_request_event, func_win, func_win_del_cb);


   //Dummy Button for Repeat Events Test
   efl::ui::Button repeat_events_test_btn(instantiate, win);
   repeat_events_test_btn.text_set("Repeat Event Test");
   repeat_events_test_btn.position_set({0, 0});
   repeat_events_test_btn.size_set({100, 100});
   repeat_events_test_btn.visible_set(false);

   auto repeat_events_test_cb(std::bind([]() {
        std::cout << "Repeat Test Button is clicked" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, repeat_events_test_btn, repeat_events_test_cb);
   g_repeat_events_test_btn = repeat_events_test_btn;


   //Popup
   g_popup = create_popup(win);


   //Create Button
   efl::ui::Button create_btn(instantiate, func_win);
   create_btn.text_set("Create Popup");
   create_btn.position_set({0, 0});
   create_btn.size_set({150, 50});

   auto create_cb(std::bind([wwin]() {
        if (g_popup)
          std::cout << "Efl.Ui.Popup is already created" << std::endl;
        else {
          std::cout << "Efl.Ui.Popup is created" << std::endl;
          auto lwin = wwin.lock();
          if (lwin) {
             auto win = lwin.get();
             g_popup = create_popup(win);
          }
        }
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, create_btn, create_cb);


   //Delete Button
   efl::ui::Button delete_btn(instantiate, func_win);
   delete_btn.text_set("Delete Popup");
   delete_btn.position_set({150, 0});
   delete_btn.size_set({150, 50});

   auto delete_cb(std::bind([wwin]() {
        if (g_popup) {
           auto wpopup(g_popup._get_wref());
           ::efl_del(wpopup->_eo_ptr());
           g_popup = nullptr;
        }
        else
          std::cout << "Efl.Ui.Popup does not exist" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, delete_btn, delete_cb);


   //Repeat Events Button
   efl::ui::Button repeat_events_btn(instantiate, func_win);
   repeat_events_btn.text_set("Repeat Events Set");
   repeat_events_btn.position_set({0, 50});
   repeat_events_btn.size_set({150, 50});

   auto wrepeat_events_btn(repeat_events_btn._get_wref());
   auto repeat_events_cb(std::bind([wrepeat_events_btn]() {
        if (!g_popup) return;

        auto lrepeat_events_btn = wrepeat_events_btn.lock();
        if (!lrepeat_events_btn) return;

        auto repeat_events_btn = lrepeat_events_btn.get();
        if (!repeat_events_btn) return;

        if (efl::eo::downcast<efl::canvas::Object>(g_popup.part_get("backwall")).repeat_events_get()) {
           efl::eo::downcast<efl::canvas::Object>(g_popup.part_get("backwall")).repeat_events_set(false);
           repeat_events_btn.text_set("Set Repeat Events");
        }
        else {
           efl::eo::downcast<efl::canvas::Object>(g_popup.part_get("backwall")).repeat_events_set(true);
           repeat_events_btn.text_set("Unset Repeat Events");
        }
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, repeat_events_btn, repeat_events_cb);


   //Dummy Button
   efl::ui::Button dummy_btn(instantiate, func_win);
   dummy_btn.text_set("Show Dummy Button");
   dummy_btn.position_set({150, 50});
   dummy_btn.size_set({150, 50});

   auto wdummy_btn(dummy_btn._get_wref());
   auto dummy_cb(std::bind([wdummy_btn]() {
        if (!g_popup) return;

        auto ldummy_btn = wdummy_btn.lock();
        if (!ldummy_btn) return;

        auto dummy_btn = ldummy_btn.get();
        if (!dummy_btn) return;

        if (g_repeat_events_test_btn.visible_get()) {
           g_repeat_events_test_btn.visible_set(false);
           dummy_btn.text_set("Show Dummy Button");
        }
        else {
           g_repeat_events_test_btn.visible_set(true);
           dummy_btn.text_set("Hide Dummy Button");
        }
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, dummy_btn, dummy_cb);


   //Backwall Button
   efl::ui::Button backwall_btn(instantiate, func_win);
   backwall_btn.text_set("Set Backwall");
   backwall_btn.position_set({0, 100});
   backwall_btn.size_set({150, 50});

   auto wbackwall_btn(backwall_btn._get_wref());
   auto backwall_cb(std::bind([wbackwall_btn]() {
        static bool backwall_visible = false;

        if (!g_popup) return;

        auto lbackwall_btn = wbackwall_btn.lock();
        if (!lbackwall_btn) return;

        auto backwall_btn = lbackwall_btn.get();
        if (!backwall_btn) return;

        backwall_visible = !backwall_visible;
        if (backwall_visible) {
           efl::eo::downcast<efl::File>(g_popup.part_get("backwall")).file_set("./sky_01.jpg");
           backwall_btn.text_set("Unset Backwall");
        }
        else {
           efl::eo::downcast<efl::File>(g_popup.part_get("backwall")).file_set(nullptr);
           backwall_btn.text_set("Set Backwall");
        }
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, backwall_btn, backwall_cb);


   //Timeout Button
   efl::ui::Button timeout_btn(instantiate, func_win);
   timeout_btn.text_set("Set Timeout");
   timeout_btn.position_set({150, 100});
   timeout_btn.size_set({150, 50});

   auto timeout_set_cb(std::bind([]() {
        if (!g_popup) return;

        g_popup.timeout_set(3);
        std::cout << "Timeout is set to 3 seconds" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, timeout_btn, timeout_set_cb);


   //Center Button
   efl::ui::Button center_btn(instantiate, func_win);
   center_btn.text_set("Align Center");
   center_btn.position_set({0, 150});
   center_btn.size_set({150, 50});

   auto center_align_cb(std::bind([]() {
        if (!g_popup) return;

        g_popup.align_set(EFL_UI_POPUP_ALIGN_CENTER);
        std::cout << "Align Center" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, center_btn, center_align_cb);


   //Top Button
   efl::ui::Button top_btn(instantiate, func_win);
   top_btn.text_set("Align Top");
   top_btn.position_set({150, 150});
   top_btn.size_set({150, 50});

   auto top_align_cb(std::bind([]() {
        if (!g_popup) return;

        g_popup.align_set(EFL_UI_POPUP_ALIGN_TOP);
        std::cout << "Align Top" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, top_btn, top_align_cb);


   //Left Button
   efl::ui::Button left_btn(instantiate, func_win);
   left_btn.text_set("Align Left");
   left_btn.position_set({0, 200});
   left_btn.size_set({150, 50});

   auto left_align_cb(std::bind([]() {
        if (!g_popup) return;

        g_popup.align_set(EFL_UI_POPUP_ALIGN_LEFT);
        std::cout << "Align Left" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, left_btn, left_align_cb);


   //Right Button
   efl::ui::Button right_btn(instantiate, func_win);
   right_btn.text_set("Align Right");
   right_btn.position_set({150, 200});
   right_btn.size_set({150, 50});

   auto right_align_cb(std::bind([]() {
        if (!g_popup) return;

        g_popup.align_set(EFL_UI_POPUP_ALIGN_RIGHT);
        std::cout << "Align Right" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, right_btn, right_align_cb);


   //Bottom Button
   efl::ui::Button bottom_btn(instantiate, func_win);
   bottom_btn.text_set("Align Bottom");
   bottom_btn.position_set({0, 250});
   bottom_btn.size_set({150, 50});

   auto bottom_align_cb(std::bind([]() {
        if (!g_popup) return;

        g_popup.align_set(EFL_UI_POPUP_ALIGN_BOTTOM);
        std::cout << "Align Bottom" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, bottom_btn, bottom_align_cb);


   //Position Button
   efl::ui::Button position_btn(instantiate, func_win);
   position_btn.text_set("Set Position to (0, 0)");
   position_btn.position_set({150, 250});
   position_btn.size_set({150, 50});

   auto position_set_cb(std::bind([]() {
        if (!g_popup) return;

        g_popup.position_set({0, 0});
        std::cout << "Position is set to (0, 0)" << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, position_btn, position_set_cb);
}
EFL_MAIN()
