#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "Efl_Wl.h"
#include "Elementary.h"

static Evas_Object *win;
static Eina_Strbuf *buf;
static Ecore_Exe *exe;

static Eina_Bool
del_handler(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Exe_Event_Del *ev)
{
   if (ev->exe == exe) ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

static void
focus_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_focus_set(data, 1);
}

static Eina_Bool
dostuff(void *data)
{
   exe = efl_wl_run(data, eina_strbuf_string_get(buf));
   ecore_event_handler_add(ECORE_EXE_EVENT_DEL, (Ecore_Event_Handler_Cb)del_handler, NULL);
   evas_object_focus_set(data, 1);
   return EINA_FALSE;
}

int
main(int argc, char *argv[])
{
   Evas_Object *o;
   int i;

   if (argc < 2) return 0;
   elm_init(argc, (char**)argv);
   buf = eina_strbuf_new();
   for (i = 1; i < argc; i++)
     {
        eina_strbuf_append(buf, argv[i]);
        if (i + 1 < argc) eina_strbuf_append_char(buf, ' ');
     }

   win = elm_win_util_standard_add("comp", "comp");
   elm_win_autodel_set(win, 1);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   o = efl_wl_add(evas_object_evas_get(win));
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);
   evas_object_show(win);
   evas_object_resize(win, 640, 480);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FOCUS_IN, focus_in, o);
   ecore_timer_add(1, dostuff, o);
   
   elm_run();
   elm_shutdown();
   return 0;
}
