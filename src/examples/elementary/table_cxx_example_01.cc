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
// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` table_cxx_example_01.cc -o table_cxx_example_01

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
   table.content_padding_set(5, 5);
//   FIXME
//   table.homogeneous_set(true);

   efl::ui::Textbox label(instantiate, win);
   label.text_set("label 0");
   table.pack_table(label, 0, 0, 1, 1);

   efl::ui::Textbox label1(instantiate, win);
   label1.text_set("label 1");
   table.pack_table(label1, 1, 0, 1, 1);

   efl::ui::Textbox label2(instantiate, win);
   label2.text_set("label 2");
   table.pack_table(label2, 0, 1, 1, 1);

   efl::ui::Textbox label3(instantiate, win);
   label3.text_set("label 3");
   table.pack_table(label3, 1, 1, 1, 1);
}
EFL_MAIN()
