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
/* To compile:
 * g++ `pkg-config --cflags --libs elementary-cxx eo-cxx efl-cxx eina-cxx ecore-cxx evas-cxx edje-cxx` ./bg_cxx_example_02.cc -o bg_cxx_example_02
 *
 * To run:
 * ./bg_cxx_example_02
 */

#define EFL_CXXPERIMENTAL
#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#include <Efl_Ui.hh>

using efl::eo::instantiate;

static efl::ui::Win win;

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Loop_Arguments *args = static_cast<Efl_Loop_Arguments *>(ev->info);

   win = efl::ui::Win(instantiate);
   win.text_set("Bg Image");
   win.autohide_set(true);
   win.delete_request_event_cb_add([](){ win = nullptr; efl_exit(0); });

   std::string path;
   /* argv[0] is the program itself; the image, if any, is the next one. */
   if (eina_array_count(args->argv) > 1)
     path = static_cast<const char *>(eina_array_data_get(args->argv, 1));
   else
     {
        /* Where the C example finds it: the installed (or in-tree) data of
         * elementary. */
        elm_app_info_set(reinterpret_cast<void *>(efl_main), "elementary",
                         "images/plant_01.jpg");
        path = std::string(elm_app_data_dir_get()) + "/images/plant_01.jpg";
     }

   efl::ui::Bg bg(instantiate, win);
   bg.scale_method_set(EFL_GFX_IMAGE_SCALE_METHOD_FILL);
   bg.file_set(path);
   bg.load();
   win.content_set(bg);

   win.size_set({640, 400});
}
EFL_MAIN()
