// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` button_cxx_example_00.cc -o button_cxx_example_00

#define EFL_CXXPERIMENTAL
#define EFL_BETA_API_SUPPORT
#include <Efl_Ui.hh>
#include <iostream>

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Button Example");
   win.autohide_set(true);

   efl::ui::Box box(instantiate, win);
   win.content_set(box);

   efl::ui::Button bt(instantiate, win);
   bt.text_set("Hello world!");
   box.pack(bt);

   auto wbt(bt._get_wref());
   auto cb(std::bind([wbt]() {
        std::cout << wbt->text_get() << std::endl;
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, bt, cb);

   efl::ui::Button bt2(instantiate, win);
   bt2.text_set("Click to quit");
   box.pack(bt2);

   auto wwin(win._get_wref());
   auto cb2(std::bind([wwin]() {
        ::efl_del(wwin->_eo_ptr()); // FIXME: No proper C++ API to delete win
     }));
   efl::eolian::event_add(efl::ui::Clickable::clicked_event, bt2, cb2);

   win.size_set({320,160});
}
EFL_MAIN()
