#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_WIDGET_PROTECTED
#include <Elementary.h>

typedef struct Test_Data
{
   Evas_Object *pb1;
   Evas_Object *pb2;
   Evas_Object *pb3;
   Evas_Object *pb4;
   Evas_Object *pb5;
   Evas_Object *ico;
   Evas_Object *btn_start;
   Evas_Object *btn_stop;
   int loop;
} Test_Data;

static void
animate(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Test_Data *td = data;
   int loop = td->loop++;
   double radius;

   loop = (loop % 120) < 60 ? (loop % 60) : (60 - (loop % 60));
   radius = 3.0 + (loop * 32.0 / 60.0);
   radius *= elm_config_scale_get() / elm_app_base_scale_get();
   efl_gfx_blur_radius_set(efl_part(td->ico, "shadow"), radius, radius);
}

static void
pulse_start(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Test_Data *td = data;

   efl_ui_progressbar_pulse_set(td->pb1, EINA_TRUE);
   efl_ui_progressbar_pulse_set(td->pb2, EINA_TRUE);
   efl_ui_progressbar_pulse_set(td->pb3, EINA_TRUE);
   efl_ui_progressbar_pulse_set(td->pb4, EINA_TRUE);
   efl_ui_progressbar_pulse_set(td->pb5, EINA_TRUE);

   efl_ui_widget_disabled_set(td->btn_start, EINA_TRUE);
   efl_ui_widget_disabled_set(td->btn_stop, EINA_FALSE);

   efl_event_callback_add(td->ico, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, animate, td);
}

static void
pulse_stop(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Test_Data *td = data;

   efl_ui_progressbar_pulse_set(td->pb1, EINA_FALSE);
   efl_ui_progressbar_pulse_set(td->pb2, EINA_FALSE);
   efl_ui_progressbar_pulse_set(td->pb3, EINA_FALSE);
   efl_ui_progressbar_pulse_set(td->pb4, EINA_FALSE);
   efl_ui_progressbar_pulse_set(td->pb5, EINA_FALSE);

   efl_ui_widget_disabled_set(td->btn_start, EINA_FALSE);
   efl_ui_widget_disabled_set(td->btn_stop, EINA_TRUE);

   efl_event_callback_del(td->ico, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, animate, td);
}

static void
_delete_cb(void *data, const Efl_Event *ev)
{
   Test_Data *td = data;

   pulse_stop(td, NULL);
   efl_unref(ev->object);
   free(td);
}

void
test_part_shadow(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *pb, *bx, *hbx, *bt, *bt_bx, *ic, *shadow;
   char buf[PATH_MAX];
   Test_Data *td;
   char loops[32];

   td = calloc(1, sizeof(*td));

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_name_set(efl_added, "shadow"),
                 efl_text_set(efl_added, "Shadow effects"));
   efl_event_callback_add(win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _delete_cb, td);

   bx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_content_set(win, bx);

   pb = efl_add(EFL_UI_PROGRESSBAR_CLASS, win);
   efl_gfx_hint_weight_set(pb, EFL_GFX_HINT_EXPAND, 0);
   efl_ui_progressbar_pulse_mode_set(pb, EINA_TRUE);
   efl_pack(bx, pb);
   td->pb1 = pb;

   // A black drop-down shadow
   shadow = efl_ref(efl_part(pb, "shadow"));
   efl_gfx_color_set(shadow, 0, 0, 0, 255);
   efl_gfx_blur_grow_set(shadow, ELM_SCALE_SIZE(1));
   efl_gfx_blur_radius_set(shadow, ELM_SCALE_SIZE(8), ELM_SCALE_SIZE(8));
   efl_gfx_blur_offset_set(shadow, ELM_SCALE_SIZE(5), ELM_SCALE_SIZE(5));
   efl_unref(shadow);

   ic = efl_add(EFL_UI_IMAGE_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   efl_file_set(ic, buf, NULL);
   efl_pack(bx, ic);
   td->ico = ic;

   // A red glow
   shadow = efl_ref(efl_part(ic, "shadow"));
   efl_gfx_color_set(shadow, 255, 0, 0, 255);
   efl_gfx_blur_grow_set(shadow, ELM_SCALE_SIZE(3));
   efl_gfx_blur_radius_set(shadow, ELM_SCALE_SIZE(5), ELM_SCALE_SIZE(5));
   efl_unref(shadow);

   pb = efl_add(EFL_UI_PROGRESSBAR_CLASS, win);
   efl_ui_progressbar_pulse_mode_set(pb, EINA_TRUE);
   efl_gfx_hint_weight_set(pb, EFL_GFX_HINT_EXPAND, 0);
   efl_pack(bx, pb);
   td->pb4 = pb;

   // A custom 3D shadow effect
   static const char *filter_code =
         "for i = 1,loops,1 do\n"
         "  local offset = i - loops - 1\n"
         "  local alpha = i * 128/loops\n"
         "  blend { ox = offset, oy = offset, color = color(255, 255, 255, alpha) }\n"
         "end"
         ;

   sprintf(loops, "%f", (double) ELM_SCALE_SIZE(12));
   shadow = efl_ref(efl_part(pb, "shadow"));
   efl_gfx_filter_data_set(shadow, "loops", loops, EINA_TRUE);
   efl_gfx_filter_program_set(shadow, filter_code, "custom");
   efl_unref(shadow);

   hbx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_ui_direction_set(hbx, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_hint_weight_set(hbx, EFL_GFX_HINT_EXPAND, 0);
   efl_pack(bx, hbx);

   // FIXME: Needs wheel progressbar class
   pb = efl_add(EFL_UI_PROGRESSBAR_CLASS, win,
                efl_ui_widget_style_set(efl_added, "wheel"));
   efl_ui_progressbar_pulse_mode_set(pb, EINA_TRUE);
   efl_pack(hbx, pb);
   td->pb2 = pb;

   // A cyan glow
   shadow = efl_ref(efl_part(pb, "shadow"));
   efl_gfx_color_set(shadow, 0, 255, 255, 255);
   efl_gfx_blur_grow_set(shadow, ELM_SCALE_SIZE(5));
   efl_gfx_blur_radius_set(shadow, ELM_SCALE_SIZE(8), ELM_SCALE_SIZE(8));
   efl_unref(shadow);

   pb = efl_add(EFL_UI_PROGRESSBAR_CLASS, win,
                efl_ui_widget_style_set(efl_added, "wheel"));
   efl_ui_progressbar_pulse_mode_set(pb, EINA_TRUE);
   efl_pack(hbx, pb);
   td->pb3 = pb;

   // A black drop-down shadow
   shadow = efl_ref(efl_part(pb, "shadow"));
   efl_gfx_color_set(shadow, 0, 0, 0, 255);
   efl_gfx_blur_grow_set(shadow, ELM_SCALE_SIZE(1));
   efl_gfx_blur_radius_set(shadow, ELM_SCALE_SIZE(8), ELM_SCALE_SIZE(8));
   efl_gfx_blur_offset_set(shadow, ELM_SCALE_SIZE(6), ELM_SCALE_SIZE(6));
   efl_unref(shadow);

   bt_bx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_ui_direction_set(bt_bx, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_hint_weight_set(bt_bx, EFL_GFX_HINT_EXPAND, 0);
   efl_pack(bx, bt_bx);

   bt = efl_add(EFL_UI_BUTTON_CLASS, win, efl_text_set(efl_added, "Start"));
   efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, pulse_start, td);
   efl_pack(bt_bx, bt);
   td->btn_start = bt;

   bt = efl_add(EFL_UI_BUTTON_CLASS, win, efl_text_set(efl_added, "Stop"));
   efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, pulse_stop, td);
   efl_ui_widget_disabled_set(bt, EINA_TRUE);
   efl_pack(bt_bx, bt);
   td->btn_stop = bt;

   efl_gfx_entity_size_set(win, EINA_SIZE2D(240, 320));
}
