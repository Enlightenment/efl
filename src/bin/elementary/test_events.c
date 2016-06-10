#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define DEFAULT_TEXT "Click the white rectangle to get started"

typedef struct {
   int down;
   Eo *evdown, *evup, *evmove, *evkeydown, *evkeyup;
   Eo *win, *button, *text;
   int id;
   Eina_Promise *p;
   Eo *timer;
} testdata;

static Eina_Bool
_pointer_down(void *data, const Eo_Event *ev)
{
   testdata *td = data;
   td->down = 1;
   eo_del(td->evdown);
   td->evdown = efl_event_dup(ev->info);
   return 1;
}

static Eina_Bool
_pointer_move(void *data, const Eo_Event *ev)
{
   testdata *td = data;
   eo_del(td->evmove);
   td->evmove = efl_event_dup(ev->info);
   return 1;
}

static Eina_Bool
_pointer_up(void *data, const Eo_Event *ev)
{
   testdata *td = data;
   td->down = 0;
   eo_del(td->evup);
   td->evup = efl_event_dup(ev->info);
   return 1;
}

static Eina_Bool
_key_down(void *data, const Eo_Event *ev)
{
   testdata *td = data;
   char str[1024];

   // FIXME: By default the elm_win object is the focussed object
   // this means that evas callbacks will transfer the KEY_UP/KEY_DOWN events
   // to the elm_win. So, we get two key_down & two key_up events:
   // 1. ecore_evas -> evas -> elm_win forward -> here
   // 2. ecore_evas -> evas -> focussed obj (elm_win) -> here

   sprintf(str, "key=%s keyname=%s string=%s compose=%s",
           efl_event_key_get(ev->info),
           efl_event_key_name_get(ev->info),
           efl_event_key_string_get(ev->info),
           efl_event_key_compose_get(ev->info));
   elm_object_text_set(td->text, str);

   if (!efl_event_fake_get(ev->info))
     {
        eo_del(td->evkeydown);
        td->evkeydown = efl_event_dup(ev->info);
     }

   return 1;
}

#if 0
static void
_timeout_cb(void *data, void *value EINA_UNUSED, Eina_Promise *promise EINA_UNUSED)
{
   testdata *td = data;
   elm_object_text_set(td->text, NULL);
   td->p = NULL;
}
#else
static Eina_Bool
_ecore_timeout_cb(void *data)
{
   testdata *td = data;

   elm_object_text_set(td->text, DEFAULT_TEXT);
   td->timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

#endif

static Eina_Bool
_key_up(void *data, const Eo_Event *ev)
{
   testdata *td = data;

   if (!efl_event_fake_get(ev->info))
     {
        eo_del(td->evkeyup);
        td->evkeyup = efl_event_dup(ev->info);
     }

   // FIXME: how to use efl_loop_timeout?
   // 1. I can't cancel it (it crashes)
   // 2. I can't get a handle on the loop without calling ecore_main_loop_get()
   // 2bis I need to pass the loop object itself rather than a provider
   // 3. All calls to eina_promise crash if the promise is null

   if (td->timer) eo_del(td->timer);
   td->timer = ecore_timer_add(0.5, _ecore_timeout_cb, td);

   return 1;
}

static Eina_Bool
_clicked_button1(void *data, const Eo_Event *ev EINA_UNUSED)
{
   testdata *td = data;
   Eo *txt = td->text;
   char buf[256];

   // Note: can't do efl_event_fake_get(ev->info) because this is a click evt

   td->id++;
   sprintf(buf, "Button was clicked %d time%s", td->id, td->id > 1 ? "s" : "");
   elm_object_text_set(txt, buf);

   return 1;
}

static Eina_Bool
_clicked_button2(void *data, const Eo_Event *ev EINA_UNUSED)
{
   testdata *td = data;
   Eo *bt = td->button;
   int x, y, w, h;

   if (!td->evkeyup)
     {
        efl_gfx_position_get(bt, &x, &y);
        efl_gfx_size_get(bt, &w, &h);

        x = x + w / 2;
        y = y + h / 2;
        efl_event_pointer_position_set(td->evmove, x, y);
        efl_event_pointer_position_set(td->evdown, x, y);
        efl_event_pointer_position_set(td->evup, x, y);

        eo_event_callback_call(td->win, EFL_EVENT_POINTER_MOVE, td->evmove);
        eo_event_callback_call(td->win, EFL_EVENT_POINTER_DOWN, td->evdown);
        eo_event_callback_call(td->win, EFL_EVENT_POINTER_UP, td->evup);
     }
   else
     {
        eo_event_callback_call(td->win, EFL_EVENT_KEY_DOWN, td->evkeydown);
        eo_event_callback_call(td->win, EFL_EVENT_KEY_UP, td->evkeyup);
        eo_del(td->evkeydown);
        eo_del(td->evkeyup);
        td->evkeydown = NULL;
        td->evkeyup = NULL;
     }

   return 1;
}

static Eina_Bool
_win_del(void *data, const Eo_Event *ev EINA_UNUSED)
{
   testdata *td = data;
   free(td);
   return 1;
}

EO_CALLBACKS_ARRAY_DEFINE(rect_pointer_callbacks,
{ EFL_EVENT_POINTER_DOWN, _pointer_down },
{ EFL_EVENT_POINTER_MOVE, _pointer_move },
{ EFL_EVENT_POINTER_UP, _pointer_up })

EO_CALLBACKS_ARRAY_DEFINE(win_key_callbacks,
{ EFL_EVENT_KEY_DOWN, _key_down },
{ EFL_EVENT_KEY_UP, _key_up })

void
test_events(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   /* test fake POINTER and KEY events */

   Evas_Object *bx, *bt, *txt, *o, *win;
   testdata *td = calloc(1, sizeof(*td));

   win = elm_win_util_standard_add("buttons", "Buttons");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = eo_add(EFL_UI_BOX_CLASS, win, efl_orientation_set(eo_self, EFL_ORIENT_VERTICAL));
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, -1, -1);
   efl_pack(win, bx);
   evas_object_show(bx);
   td->win = win;

   txt = elm_label_add(win);
   evas_object_size_hint_weight_set(txt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(txt, -1, -1);
   efl_pack(bx, txt);
   elm_object_text_set(txt, DEFAULT_TEXT);
   evas_object_show(txt);
   td->text = txt;

   bt = elm_button_add(win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, -1, -1);
   elm_object_text_set(bt, "Click me!");
   efl_pack(bx, bt);
   evas_object_show(bt);
   td->button = bt;

   bt = elm_button_add(win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, -1, -1);
   elm_object_text_set(bt, "Send fake event");
   efl_pack(bx, bt);
   evas_object_show(bt);

   o = eo_add(EFL_CANVAS_RECTANGLE_CLASS, win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, -1, -1);
   efl_pack(bx, o);
   evas_object_show(o);

   eo_event_callback_add(td->button, EFL_UI_EVENT_CLICKED, _clicked_button1, td);
   eo_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _clicked_button2, td);
   eo_event_callback_add(win, EO_EVENT_DEL, _win_del, td);
   eo_event_callback_array_add(o, rect_pointer_callbacks(), td);
   eo_event_callback_array_add(win, win_key_callbacks(), td);

   evas_object_resize(td->win, 200, 100);
   evas_object_show(td->win);
}
