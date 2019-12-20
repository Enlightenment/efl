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
#define EFL_CXXPERIMENTAL

#include <Efl_Ui.hh>

using namespace std::placeholders;
using efl::eo::add;

struct appData
{
   appData() : m_win(nullptr) {}

   ~appData() {
      std::cout << "Good bye!" << std::endl;
   }

   void create() {
      std::cout << "Hello!" << std::endl;

      add(m_win);
      m_win.text_set("Calendar Layout Formatting Example");
      m_win.delete_request_event_cb_add([&](){ destroy(); });

      efl::ui::Calendar cal(add, m_win);
      m_win.content_set(cal);

      auto wcal(cal._get_wref());

      // FIXME: How does one figure out the argument types for the function?
      auto cb_a = std::bind([](
                            efl::eina::strbuf_wrapper& sb,
                            efl::eina::value_view const& value) -> bool {
           try {
              sb.append_strftime("%b. %y", efl::eina::get<tm>(value));
           } catch (std::system_error const&)  {
              sb.append(value.to_string());
           }
           std::cout << "Month: " << std::string(sb) << std::endl;
           return true;
        }, _1, _2);
      // FIXME XAR: I broke this and I do not know how to fix it
      // cal.format_func_set(cb_a);
      (void)cb_a;
   }

   void destroy() {
      // FIXME: need del() function and no error on unref().
      ::efl_allow_parent_unref_set(m_win, true);
      m_win = nullptr;
   }

private:
   efl::ui::Win m_win;
};

static appData app;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   app.create();
}
EFL_MAIN()
