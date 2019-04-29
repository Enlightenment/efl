// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` table_cxx_example_01.cc -o table_cxx_example_01

#include <Elementary.hh>

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
//   FIXME
//   table.homogeneous_set(true);

   efl::ui::Text label(instantiate, win);
   label.text_set("label 0");
   table.pack_table(label, 0, 0, 1, 1);

   efl::ui::Text label1(instantiate, win);
   label1.text_set("label 1");
   table.pack_table(label1, 1, 0, 1, 1);

   efl::ui::Text label2(instantiate, win);
   label2.text_set("label 2");
   table.pack_table(label2, 0, 1, 1, 1);

   efl::ui::Text label3(instantiate, win);
   label3.text_set("label 3");
   table.pack_table(label3, 1, 1, 1, 1);
}
EFL_MAIN()
