#include "main.h"
#include "docs.h"

void
eo_gen_source_gen(const Eolian_Class *cl, Eina_Strbuf *buf)
{
   if (!cl)
     return;

   /* event section, they come first */
   {
      Eina_Iterator *itr = eolian_class_events_get(cl);
      Eolian_Event *ev;
      EINA_ITERATOR_FOREACH(itr, ev)
        {
           Eina_Stringshare *evn = eolian_event_c_name_get(ev);
           eina_strbuf_append(buf, "EOAPI const Efl_Event_Description _");
           eina_strbuf_append(buf, evn);
           eina_strbuf_append(buf, " =\n   EFL_EVENT_DESCRIPTION");
           if (eolian_event_is_hot(ev))
             eina_strbuf_append(buf, "_HOT");
           if (eolian_event_is_restart(ev))
             eina_strbuf_append(buf, "_RESTART");
           eina_strbuf_append_printf(buf, "(\"%s\");\n", eolian_event_name_get(ev));
           eina_stringshare_del(evn);
        }
      eina_iterator_free(itr);
   }
}
