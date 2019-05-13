// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` table_cxx_example_01.cc -o table_cxx_example_01

#include <Evas.hh>
#include <Efl_Ui.hh>

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Table example");
   win.autohide_set(true);

   efl::ui::Table table(instantiate, win);
   win.content_set(table);
   table.content_padding_set(5, 5, true);

   efl::canvas::Rectangle rect(instantiate, win);
   rect.color_set(255, 0, 0, 255);
   rect.hint_size_min_set({100, 50});
   table.pack_table(rect, 0, 0, 2, 1);

   efl::canvas::Rectangle rect2(instantiate, win);
   rect2.color_set(0, 255, 0, 255);
   rect2.hint_size_min_set({50, 100});
   table.pack_table(rect2, 0, 1, 1, 2);

   efl::canvas::Rectangle rect3(instantiate, win);
   rect3.color_set(0, 0, 255, 255);
   rect3.hint_size_min_set({50, 50});
   table.pack_table(rect3, 1, 1, 1, 1);

   efl::canvas::Rectangle rect4(instantiate, win);
   rect4.color_set(255, 255, 0, 255);
   rect4.hint_size_min_set({50, 50});
   table.pack_table(rect4, 1, 2, 1, 1);
}
EFL_MAIN()
