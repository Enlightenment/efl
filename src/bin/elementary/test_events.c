#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define DEFAULT_TEXT "Click the white rectangle to get started"

typedef struct {
   int down;
   Eo *evdown, *evup, *evmove, *evkeydown, *evkeyup;
   Eo *win, *button, *text;
   int id;
   Eina_Future *f;
} testdata;

static void
_pointer_down(void *data, const Efl_Event *ev)
{
   testdata *td = data;
   td->down = 1;
   efl_unref(td->evdown);
   td->evdown = efl_duplicate(ev->info);
}

static void
_pointer_move(void *data, const Efl_Event *ev)
{
   testdata *td = data;
   efl_unref(td->evmove);
   td->evmove = efl_duplicate(ev->info);
}

static void
_pointer_up(void *data, const Efl_Event *ev)
{
   testdata *td = data;
   td->down = 0;
   efl_unref(td->evup);
   td->evup = efl_duplicate(ev->info);
}

static void
_key_down(void *data, const Efl_Event *ev)
{
   testdata *td = data;
   char str[1024];

   // FIXME: By default the elm_win object is the focused object
   // this means that evas callbacks will transfer the KEY_UP/KEY_DOWN events
   // to the elm_win. So, we get two key_down & two key_up events:
   // 1. ecore_evas -> evas -> elm_win forward -> here
   // 2. ecore_evas -> evas -> focused obj (elm_win) -> here

   sprintf(str, "key=%s keyname=%s string=%s compose=%s",
           efl_input_key_get(ev->info),
           efl_input_key_name_get(ev->info),
           efl_input_key_string_get(ev->info),
           efl_input_key_compose_get(ev->info));
   elm_object_text_set(td->text, str);

   if (!efl_input_fake_get(ev->info))
     {
        efl_unref(td->evkeydown);
        td->evkeydown = efl_duplicate(ev->info);
     }
}

static Eina_Value
_ecore_timeout_cb(void *data,
                  const Eina_Value t,
                  const Eina_Future  *dead EINA_UNUSED)
{
   testdata *td = data;

   elm_object_text_set(td->text, DEFAULT_TEXT);
   td->f = NULL;

   return t;
}

static void
_key_up(void *data, const Efl_Event *ev)
{
   testdata *td = data;

   if (!efl_input_fake_get(ev->info))
     {
        efl_unref(td->evkeyup);
        td->evkeyup = efl_duplicate(ev->info);
     }

   if (td->f) eina_future_cancel(td->f);
   td->f = efl_loop_timeout(efl_provider_find(ev->object, EFL_LOOP_CLASS), 0.5);
   eina_future_then(td->f, _ecore_timeout_cb, td, NULL);
}

static void
_clicked_button1(void *data, const Efl_Event *ev EINA_UNUSED)
{
   testdata *td = data;
   Eo *txt = td->text;
   char buf[256];

   // Note: can't do efl_input_fake_get(ev->info) because this is a click evt

   td->id++;
   sprintf(buf, "Button was clicked %d time%s", td->id, td->id > 1 ? "s" : "");
   elm_object_text_set(txt, buf);
}

static void
_clicked_button2(void *data, const Efl_Event *ev EINA_UNUSED)
{
   testdata *td = data;
   Eo *bt = td->button;
   Eina_Rect r;

   if (!td->evkeyup)
     {
        r = efl_gfx_entity_geometry_get(bt);

        r.x = r.x + r.w / 2;
        r.y = r.y + r.h / 2;
        efl_input_pointer_position_set(td->evmove, r.pos);
        efl_input_pointer_position_set(td->evdown, r.pos);
        efl_input_pointer_position_set(td->evup, r.pos);

        efl_event_callback_call(td->win, EFL_EVENT_POINTER_MOVE, td->evmove);
        efl_event_callback_call(td->win, EFL_EVENT_POINTER_DOWN, td->evdown);
        efl_event_callback_call(td->win, EFL_EVENT_POINTER_UP, td->evup);
     }
   else
     {
        efl_event_callback_call(td->win, EFL_EVENT_KEY_DOWN, td->evkeydown);
        efl_event_callback_call(td->win, EFL_EVENT_KEY_UP, td->evkeyup);
        efl_unref(td->evkeydown);
        efl_unref(td->evkeyup);
        td->evkeydown = NULL;
        td->evkeyup = NULL;
     }
}

static void
_win_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   testdata *td = data;
   efl_unref(td->evdown);
   efl_unref(td->evup);
   efl_unref(td->evmove);
   efl_unref(td->evkeydown);
   efl_unref(td->evkeyup);
   free(td);
}

static void
_button_pointer_down(void *data, const Efl_Event *ev)
{
   if (((intptr_t) data) != 0x1) abort();
   printf("Button raw event: DOWN. Fake = %d\n", efl_input_fake_get(ev->info));
   fflush(stdout);
}

static void
_button_pointer_up(void *data, const Efl_Event *ev)
{
   if (((intptr_t) data) != 0x1) abort();
   printf("Button raw event: UP.   Fake = %d\n", efl_input_fake_get(ev->info));
   fflush(stdout);
}

EFL_CALLBACKS_ARRAY_DEFINE(rect_pointer_callbacks,
{ EFL_EVENT_POINTER_DOWN, _pointer_down },
{ EFL_EVENT_POINTER_MOVE, _pointer_move },
{ EFL_EVENT_POINTER_UP, _pointer_up })

EFL_CALLBACKS_ARRAY_DEFINE(win_key_callbacks,
{ EFL_EVENT_KEY_DOWN, _key_down },
{ EFL_EVENT_KEY_UP, _key_up })

EFL_CALLBACKS_ARRAY_DEFINE(button_pointer_callbacks,
{ EFL_EVENT_POINTER_DOWN, _button_pointer_down },
{ EFL_EVENT_POINTER_UP, _button_pointer_up })

void
test_events(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   /* test fake POINTER and KEY events */

   Evas_Object *bx, *bt, *txt, *o, *win;
   testdata *td = calloc(1, sizeof(*td));

   win = elm_win_util_standard_add("buttons", "Buttons");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   efl_content_set(win, bx);
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

   o = efl_add(EFL_CANVAS_RECTANGLE_CLASS, win);
   efl_pack(bx, o);

   efl_event_callback_add(td->button, EFL_UI_EVENT_CLICKED, _clicked_button1, td);
   efl_event_callback_array_add(td->button, button_pointer_callbacks(), (void*)(intptr_t)0x1);
   efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _clicked_button2, td);
   efl_event_callback_add(win, EFL_EVENT_DEL, _win_del, td);
   efl_event_callback_array_add(o, rect_pointer_callbacks(), td);
   efl_event_callback_array_add(win, win_key_callbacks(), td);

   evas_object_resize(td->win, 200, 100);
   evas_object_show(td->win);
}
