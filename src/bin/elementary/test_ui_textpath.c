#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>

#define CX 180
#define CY 150
#define CR 100

#define TEST_UI_TEXTPATH_LONG_TEXT "This text follows the path which you defined. This is a &lt;long&gt; text designed to make it ellipsis."
#define TEST_UI_TEXTPATH_SHORT_TEXT "This text is short."

static Evas_Object *angle_sld, *slice_sld, *dir_chk;
static int path_type;

static void
_ellipsis_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *txtpath = data;
   efl_ui_textpath_ellipsis_set(txtpath, elm_check_state_get(event->object));
}

static void
_direction_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *txtpath = data;
   int angle = elm_slider_value_get(angle_sld);
   Eina_Bool val = elm_check_selected_get(event->object);
   Efl_Ui_Textpath_Direction dir = val ? EFL_UI_TEXTPATH_DIRECTION_CW :
                                         EFL_UI_TEXTPATH_DIRECTION_CCW;
   efl_ui_textpath_circle_set(txtpath, CX, CY, CR, angle, dir);
}

static void
_angle_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *txtpath = data;
   int angle = elm_slider_value_get(event->object);
   printf("angle: %d\n", angle);
   Eina_Bool val = elm_check_selected_get(dir_chk);
   Efl_Ui_Textpath_Direction dir = val ? EFL_UI_TEXTPATH_DIRECTION_CW :
                                         EFL_UI_TEXTPATH_DIRECTION_CCW;
   efl_ui_textpath_circle_set(txtpath, CX, CY, CR, angle, dir);
}

static void
_short_text_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *txtpath = data;
   Eina_Bool val = elm_check_selected_get(event->object);

   if (val)
     efl_text_set(txtpath, TEST_UI_TEXTPATH_SHORT_TEXT);
   else
     efl_text_set(txtpath, TEST_UI_TEXTPATH_LONG_TEXT);
}

static void
_change_shape_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *txtpath = data;

   efl_gfx_path_reset(txtpath);
   path_type = (path_type + 1) % 2;
   if (path_type == 1)
     {
        efl_gfx_path_append_move_to(txtpath, 20, 300);
        efl_gfx_path_append_cubic_to(txtpath, 80, 80, 240, 270, 300, 20);
     }
   else
     {
        int angle = elm_slider_value_get(angle_sld);
        Eina_Bool val = elm_check_selected_get(dir_chk);
        Efl_Ui_Textpath_Direction dir = val ? EFL_UI_TEXTPATH_DIRECTION_CW :
                                              EFL_UI_TEXTPATH_DIRECTION_CCW;
        efl_ui_textpath_circle_set(txtpath, CX, CY, CR, angle, dir);
     }
}

static void
_slice_no_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *txtpath = data;
   int slice_no = elm_slider_value_get(event->object);
   printf("Slice no: %d\n", slice_no);
   efl_ui_textpath_slice_number_set(txtpath, slice_no);
}

void
test_ui_textpath(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *txtpath, *box, *hbox, *chk, *sld, *btn;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   win = elm_win_util_standard_add("efl.ui.textpath", "TextPath test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_FALSE);
   efl_gfx_hint_weight_set(box, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_gfx_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_gfx_entity_visible_set(box, EINA_TRUE);

   txtpath = efl_add(EFL_UI_TEXTPATH_CLASS, win);
   elm_box_pack_end(box, txtpath);

   efl_text_set(txtpath, TEST_UI_TEXTPATH_LONG_TEXT);

   efl_ui_textpath_circle_set(txtpath, CX, CY, CR, 0, EFL_UI_TEXTPATH_DIRECTION_CCW);
   efl_gfx_entity_visible_set(txtpath, EINA_TRUE);
   path_type = 0;

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   efl_gfx_hint_weight_set(hbox, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_gfx_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_gfx_entity_visible_set(hbox, EINA_TRUE);
   elm_box_pack_end(box, hbox);

   chk = elm_check_add(win);
   elm_object_text_set(chk, "Ellipsis");
   elm_check_state_set(chk, efl_ui_textpath_ellipsis_get(txtpath));
   efl_event_callback_add(chk, EFL_UI_CHECK_EVENT_CHANGED, _ellipsis_changed_cb, txtpath);
   elm_box_pack_end(hbox, chk);
   efl_gfx_entity_visible_set(chk, EINA_TRUE);

   chk = elm_check_add(win);
   elm_object_text_set(chk, "Clockwise");
   efl_event_callback_add(chk, EFL_UI_CHECK_EVENT_CHANGED, _direction_changed_cb, txtpath);
   elm_box_pack_end(hbox, chk);
   efl_gfx_entity_visible_set(chk, EINA_TRUE);
   dir_chk = chk;

   chk = elm_check_add(win);
   elm_object_text_set(chk, "Short text");
   efl_event_callback_add(chk, EFL_UI_CHECK_EVENT_CHANGED, _short_text_changed_cb, txtpath);
   elm_box_pack_end(hbox, chk);
   efl_gfx_entity_visible_set(chk, EINA_TRUE);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   efl_gfx_hint_weight_set(hbox, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_gfx_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_gfx_entity_visible_set(hbox, EINA_TRUE);
   elm_box_pack_end(box, hbox);

   sld = elm_slider_add(win);
   elm_object_text_set(sld, "Angle");
   elm_slider_min_max_set(sld, 0, 360);
   elm_slider_value_set(sld, 0);
   efl_gfx_hint_align_set(sld, 0.5, EVAS_HINT_FILL);
   efl_gfx_hint_weight_set(sld, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_gfx_hint_size_min_set(sld, EINA_SIZE2D(150, 0));
   efl_event_callback_add(sld, EFL_UI_SLIDER_EVENT_CHANGED, _angle_changed_cb, txtpath);
   elm_box_pack_end(hbox, sld);
   efl_gfx_entity_visible_set(sld, EINA_TRUE);
   angle_sld = sld;

   sld = elm_slider_add(win);
   elm_object_text_set(sld, "Slice No");
   elm_slider_min_max_set(sld, 20, 300);
   elm_slider_value_set(sld, 99);
   efl_gfx_hint_align_set(sld, 0.5, EVAS_HINT_FILL);
   efl_gfx_hint_weight_set(sld, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_gfx_hint_size_min_set(sld, EINA_SIZE2D(150, 0));
   efl_event_callback_add(sld, EFL_UI_SLIDER_EVENT_CHANGED, _slice_no_changed_cb, txtpath);
   elm_box_pack_end(hbox, sld);
   efl_gfx_entity_visible_set(sld, EINA_TRUE);
   slice_sld = sld;

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   efl_gfx_hint_weight_set(hbox, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_gfx_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_gfx_entity_visible_set(hbox, EINA_TRUE);
   elm_box_pack_end(box, hbox);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Change Path");
   evas_object_smart_callback_add(btn, "clicked", _change_shape_cb, txtpath);
   elm_box_pack_end(hbox, btn);
   efl_gfx_entity_visible_set(btn, EINA_TRUE);

   elm_win_resize_object_add(win, box);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(400,  400));
   efl_gfx_entity_visible_set(win, 1);
}
