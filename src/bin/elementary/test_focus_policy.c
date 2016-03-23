#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_rdg_changed_cb(void *data, Evas_Object *obj,
                void *event_info EINA_UNUSED)
{
   Evas_Object *bt = data;
   int value = elm_radio_value_get(obj);

   // set focus move policy to the test button
   switch (value)
     {
      case 0:
        elm_object_text_set(bt, "Test Button (MOUSE CLICK or KEY)");
        elm_object_focus_move_policy_set(bt, ELM_FOCUS_MOVE_POLICY_CLICK);
        break;
      case 1:
        elm_object_text_set(bt, "Test Button (MOUSE IN or KEY)");
        elm_object_focus_move_policy_set(bt, ELM_FOCUS_MOVE_POLICY_IN);
        break;
      case 2:
        elm_object_text_set(bt, "Test Button (KEY ONLY)");
        elm_object_focus_move_policy_set(bt, ELM_FOCUS_MOVE_POLICY_KEY_ONLY);
        break;
      default:
        break;
     }
}

void
test_focus_object_policy(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fr, *fr2, *bx, *fr_bx, *bt, *test_bt, *lbl, *rdg, *rd;

   win = elm_win_util_standard_add("focus-object-policy", "Focus Object Policy");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);
   elm_win_focus_highlight_style_set(win, "glow");

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, fr);
   elm_object_style_set(fr, "pad_large");
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   /* frame 1 for normal buttons */
     {
        fr2 = elm_frame_add(bx);
        evas_object_size_hint_weight_set(fr2, EVAS_HINT_EXPAND, 0);
        evas_object_size_hint_align_set(fr2, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_object_text_set(fr2, "Focusable Buttons");
        elm_box_pack_end(bx, fr2);
        evas_object_show(fr2);

        fr_bx = elm_box_add(fr2);
        elm_object_content_set(fr2, fr_bx);
        evas_object_show(fr_bx);

        bt = elm_button_add(fr_bx);
        elm_object_text_set(bt, "Button 1");
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(fr_bx, bt);
        evas_object_show(bt);
        elm_object_focus_set(bt, EINA_TRUE);

        bt = elm_button_add(fr_bx);
        elm_object_text_set(bt, "Button 2");
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(fr_bx, bt);
        evas_object_show(bt);

        // a button to test focus object policy
        test_bt = elm_button_add(fr_bx);
        elm_object_text_set(test_bt, "Test Button (MOUSE CLICK or KEY)");
        evas_object_size_hint_weight_set(test_bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(test_bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(fr_bx, test_bt);
        evas_object_show(test_bt);

        elm_object_focus_move_policy_set(bt, ELM_FOCUS_MOVE_POLICY_CLICK);
        bt = elm_button_add(fr_bx);
        elm_object_text_set(bt, "Button 4");
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(fr_bx, bt);
        evas_object_show(bt);
     }

   /* a frame 2 for the test button */
     {
        fr2 = elm_frame_add(bx);
        evas_object_size_hint_weight_set(fr2, EVAS_HINT_EXPAND, 0);
        evas_object_size_hint_align_set(fr2, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_object_text_set(fr2, "Focus Options for a TEST button");
        elm_box_pack_end(bx, fr2);
        evas_object_show(fr2);

        fr_bx = elm_box_add(fr2);
        elm_object_content_set(fr2, fr_bx);
        evas_object_show(fr_bx);

        lbl = elm_label_add(fr_bx);
        evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, 0);
        elm_object_text_set(lbl, "This focus option will be applied only for a TEST button. <br/>The focus policies of other buttons will be remain in MOUSE CLICK status.");
        elm_box_pack_end(fr_bx, lbl);
        evas_object_show(lbl);

        // radios to select focus object policy
        rd = elm_radio_add(fr_bx);
        elm_radio_state_value_set(rd, 0);
        evas_object_size_hint_align_set(rd, 0.0, 0.5);
        elm_object_text_set(rd, "Focus Move Pollicy Mouse Click");
        elm_box_pack_end(fr_bx, rd);
        evas_object_show(rd);
        evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, test_bt);

        rdg = rd;

        rd = elm_radio_add(fr_bx);
        elm_radio_state_value_set(rd, 1);
        elm_radio_group_add(rd, rdg);
        evas_object_size_hint_align_set(rd, 0.0, 0.5);
        elm_object_text_set(rd, "Focus Move Policy Mouse In");
        elm_box_pack_end(fr_bx, rd);
        evas_object_show(rd);
        evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, test_bt);

        rd = elm_radio_add(fr_bx);
        elm_radio_state_value_set(rd, 2);
        elm_radio_group_add(rd, rdg);
        evas_object_size_hint_align_set(rd, 0.0, 0.5);
        elm_object_text_set(rd, "Focus Move Pollicy Key Only");
        elm_box_pack_end(fr_bx, rd);
        evas_object_show(rd);
        evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, test_bt);
     }

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}
