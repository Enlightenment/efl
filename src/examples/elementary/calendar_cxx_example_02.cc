#ifdef HAVE_CONFIG_H
#include "config.h"
#include "elementary_config.h"
#endif

#include <Efl.hh>
#include <Elementary.hh>

using efl::eo::instantiate;

// FIXME: Function callbacks need a lot of love in C++
static void
_format_cb(void *data EINA_UNUSED, Eina_Strbuf *str, const Eina_Value value)
{
   if (::eina_value_type_get(&value) != ::EINA_VALUE_TYPE_TM)
     {
        // FIXME: val.to_string()
        char *convert = ::eina_value_to_string(&value);
        eina_strbuf_append(str, convert);
        free(convert);
     }
   else
     {
        struct tm time;
        eina_value_get(&value, &time);
        eina_strbuf_append_strftime(str, "%b. %y", &time);
     }
}

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Calendar Layout Formatting Example");
   win.autohide_set(true);

   auto cal = efl::ui::Calendar(instantiate, win);
   win.content_set(cal);

   // FIXME: Function cb doesn't work (C++ variant)
   cal.format_cb_set(_format_cb);
   ::efl_ui_format_cb_set(cal._eo_ptr(), NULL, _format_cb, NULL);
}
EFL_MAIN()
