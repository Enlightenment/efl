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
      auto cb_a = std::bind([=](
                            efl::eina::strbuf_wrapper& sb,
                            efl::eina::value_view const& value) {
           try {
              sb.append_strftime("%b. %y", efl::eina::get<tm>(value));
           } catch (std::system_error const&)  {
              sb.append(value.to_string());
           }
           std::cout << "Month: " << std::string(sb) << std::endl;
        }, _1, _2);
      cal.format_cb_set(cb_a);
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
