#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

typedef struct _pbdata
{
   Eo *win;
   Eo *pb1;
   Eo *pb2;
   Eo *pb3;
   Eo *pb4;
   Eo *pb5;
   Eo *check;
   Eo *btn_start;
   Eo *btn_stop;
   Eo *btn_reset;
   Ecore_Timer *timer;
} pbdata;

static Eina_Bool
_set_progress_val(Eo *pb, double inc_value)
{
   double progress;

   progress = efl_ui_range_value_get(pb);

   if (progress < 100.0)
     progress += inc_value;
   else
     return EINA_TRUE;

   efl_ui_range_value_set(pb, progress);

   return EINA_FALSE;
}

static Eina_Bool
_pb_timer_cb(void *d)
{
   pbdata *pd = d;
   double progress_val;

   if (!pd) return ECORE_CALLBACK_CANCEL;

   progress_val = efl_ui_range_value_get(pd->pb1);
   if (progress_val < 100.0)
     {
        progress_val += 1;
        efl_ui_range_value_set(pd->pb2, progress_val);
        efl_ui_range_value_set(pd->pb3, progress_val);
        efl_ui_range_value_set(pd->pb4, progress_val);
        efl_ui_range_value_set(pd->pb5, progress_val);
     }

   if (!_set_progress_val(pd->pb1, 0.5))
     return ECORE_CALLBACK_RENEW;

   pd->timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_start_btn_clicked_cb(void *d, const Efl_Event *ev EINA_UNUSED)
{
   pbdata *pd = d;

   if (!pd) return;

   printf("start button is clicked\n");

   efl_ui_widget_disabled_set(pd->btn_start, EINA_TRUE);
   efl_ui_widget_disabled_set(pd->btn_stop, EINA_FALSE);

   if (!pd->timer) pd->timer = ecore_timer_add(0.1, _pb_timer_cb, pd);
}

static void
_stop_btn_clicked_cb(void *d, const Efl_Event *ev EINA_UNUSED)
{
   pbdata *pd = d;

   if (!pd) return;
   printf("stop button is clicked\n");

   efl_ui_widget_disabled_set(pd->btn_start, EINA_FALSE);
   efl_ui_widget_disabled_set(pd->btn_stop, EINA_TRUE);

   if (pd->timer)
     {
        ecore_timer_del(pd->timer);
        pd->timer = NULL;
     }
}

static void
_reset_btn_clicked_cb(void *d, const Efl_Event *ev EINA_UNUSED)
{
   pbdata *pd = d;

   if (!pd) return;
   printf("reset button is clicked\n");

   efl_ui_range_value_set(pd->pb1, 0.0);
   efl_ui_range_value_set(pd->pb2, 0.0);
   efl_ui_range_value_set(pd->pb3, 0.0);
   efl_ui_range_value_set(pd->pb4, 0.0);
   efl_ui_range_value_set(pd->pb5, 0.0);
}

static void
_win_delete_req_cb(void *d, const Efl_Event *ev EINA_UNUSED)
{
   pbdata *pd = d;

   if (pd->timer) ecore_timer_del(pd->timer);
   efl_unref(pd->win);
   free(pd);
}

static Eina_Bool
_custom_format_cb(void *data EINA_UNUSED, Eina_Strbuf *str, const Eina_Value value)
{
   double v;
   eina_value_get(&value, &v);
   if (v < 25.f) eina_strbuf_append_printf(str, "Starting up...");
   else if (v < 50.f) eina_strbuf_append_printf(str, "Working...");
   else if (v < 75.f) eina_strbuf_append_printf(str, "Getting there...");
   else if (v < 100.f) eina_strbuf_append_printf(str, "Almost done...");
   else eina_strbuf_append_printf(str, "Done!");
   return EINA_TRUE;
}

static void
_toggle_progress_label(void *data, const Efl_Event *ev)
{
   Efl_Ui_Check *check = ev->object;
   Efl_Ui_Progressbar *pb3 = data;
   Eina_Bool state = efl_ui_selectable_selected_get(check);

   efl_ui_progressbar_show_progress_label_set(pb3, state);
}

void
test_ui_progressbar(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx, *btbx;
   pbdata *pd;
   char buf[PATH_MAX];

   pd = (pbdata *)calloc(1, sizeof(pbdata));

   pd->win = win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                                      efl_text_set(efl_added, "Efl.Ui.Progressbar"),
                           efl_ui_win_autodel_set(efl_added, EINA_TRUE),
                           efl_event_callback_add(efl_added, EFL_UI_WIN_EVENT_DELETE_REQUEST,
                                                  _win_delete_req_cb, pd)
                          );

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added)
               );

   pd->pb1 = efl_add(EFL_UI_PROGRESSBAR_CLASS, bx,
                     efl_pack(bx, efl_added),
                     efl_text_set(efl_added, "percent"),
                     efl_ui_range_limits_set(efl_added, 0, 100),
                     efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(250, 20))
                    );

   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   efl_add(EFL_UI_IMAGE_CLASS, pd->pb1,
           efl_file_set(efl_added, buf),
           efl_file_load(efl_added),
           efl_content_set(pd->pb1, efl_added)
          );

   pd->pb2 = efl_add(EFL_UI_PROGRESSBAR_CLASS, bx,
                     efl_pack(bx, efl_added),
                     efl_text_set(efl_added, "10-100"),
                     efl_ui_progressbar_infinite_mode_set(efl_added, EINA_TRUE),
                     efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(250, 20)),
                     efl_ui_range_limits_set(efl_added, 10, 100),
                     efl_ui_range_value_set(efl_added, 10)
                    );

   pd->pb3 = efl_add(EFL_UI_PROGRESSBAR_CLASS, bx,
                     efl_pack(bx, efl_added),
                     efl_text_set(efl_added, "Toggle progress label"),
                     efl_ui_range_limits_set(efl_added, 0, 100),
                     efl_ui_progressbar_show_progress_label_set(efl_added, EINA_FALSE),
                     efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(250, 20))
                    );
   pd->check = efl_add(EFL_UI_CHECK_CLASS, bx,
                       efl_pack(bx, efl_added),
                       efl_event_callback_add(efl_added, EFL_UI_EVENT_SELECTED_CHANGED,
                                              _toggle_progress_label, pd->pb3),
                       efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(250, 20))
                      );
   efl_text_set(pd->check, "Show progress label of above progressbar"),

   pd->pb4 = efl_add(EFL_UI_PROGRESSBAR_CLASS, bx,
                     efl_pack(bx, efl_added),
                     efl_text_set(efl_added, "Custom string"),
                     efl_ui_range_limits_set(efl_added, 0, 100),
                     efl_ui_format_string_set(efl_added, "%d rabbits", EFL_UI_FORMAT_STRING_TYPE_SIMPLE),
                     efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(250, 20))
                    );

   pd->pb5 = efl_add(EFL_UI_PROGRESSBAR_CLASS, bx,
                     efl_pack(bx, efl_added),
                     efl_text_set(efl_added, "Custom func"),
                     efl_ui_range_limits_set(efl_added, 0, 100),
                     efl_ui_format_func_set(efl_added, NULL, _custom_format_cb, NULL),
                     efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(250, 20))
                    );

   btbx = efl_add(EFL_UI_BOX_CLASS, bx,
                  efl_pack(bx, efl_added),
                  efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL)
                 );

   pd->btn_start = efl_add(EFL_UI_BUTTON_CLASS, btbx,
                           efl_text_set(efl_added, "start"),
                           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                                  _start_btn_clicked_cb, pd),
                           efl_pack(btbx, efl_added)
                          );

   pd->btn_stop = efl_add(EFL_UI_BUTTON_CLASS, btbx,
                          efl_text_set(efl_added, "stop"),
                          efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                                 _stop_btn_clicked_cb, pd),
                          efl_pack(btbx, efl_added)
                         );
   pd->btn_reset = efl_add(EFL_UI_BUTTON_CLASS, btbx,
                           efl_text_set(efl_added, "reset"),
                           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                                  _reset_btn_clicked_cb, pd),
                           efl_pack(btbx, efl_added)
                          );
}
