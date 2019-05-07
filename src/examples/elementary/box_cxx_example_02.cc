// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` box_cxx_example_02.cc -o box_cxx_example_02

#include <Efl_Ui.hh>

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Simple Box Example");
   win.autohide_set(true);

   efl::ui::Box box(instantiate, win);
   win.content_set(box);

   for (int i = 0; i < 5; i++)
     {
        efl::ui::Button bt(instantiate, win);
        bt.text_set("Button " + std::to_string(i));
        box.pack_end(bt);
     }

   win.size_set({320,320});
}
EFL_MAIN()
