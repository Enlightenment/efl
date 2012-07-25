#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
cb_changed(void *data __UNUSED__, Evas_Object *o __UNUSED__, void *event)
{
   Elm_Object_Item *it = event;
   printf("item %p %s\n", it, elm_object_item_text_get(it));
}

void
test_segment_control(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *ic, *ic1, *ic2, *ic3, *ic4, *ic5;
   Elm_Object_Item *seg_it;

   Evas_Object *bx;
   Evas_Object *sc;
   unsigned int i;
   struct exp_mode {
      struct {
         double w, h;
      } weight;
      struct {
         double x, y;
      } align;
   } exp_modes[2] = {
     {{EVAS_HINT_EXPAND, 0.0}, {EVAS_HINT_FILL, EVAS_HINT_FILL}},
     {{0.0, 0.0}, {0.5, 0.5}}
   };

   win = elm_win_util_standard_add("segmentcontrol", "Segment Control");
   elm_win_autodel_set(win, EINA_TRUE);


   bx = elm_box_add(win);
   elm_box_padding_set(bx, 0, 10);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(exp_modes); i++)
     {
        const struct exp_mode *em = exp_modes + i;
        ic = elm_icon_add(bx);
        elm_icon_standard_set(ic, "home");
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        ic1 = elm_icon_add(bx);
        elm_icon_standard_set(ic1, "home");
        evas_object_size_hint_aspect_set(ic1, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        sc = elm_segment_control_add(win);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "Only Text");
        seg_it = elm_segment_control_item_add(sc, ic, NULL);
        elm_segment_control_item_selected_set(seg_it, EINA_TRUE);
        elm_segment_control_item_add(sc, ic1, "Text_Icon_test");
        elm_segment_control_item_add(sc, NULL, "Seg4");
        elm_segment_control_item_add(sc, NULL, "Seg5");
        evas_object_smart_callback_add(sc, "changed", cb_changed, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        sc = elm_segment_control_add(win);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "SegmentItem");
        seg_it = elm_segment_control_item_add(sc, NULL, "SegmentItem");
        elm_segment_control_item_selected_set(seg_it, EINA_TRUE);
        elm_segment_control_item_add(sc, NULL, "SegmentControlItem");
        elm_segment_control_item_add(sc, NULL, "SegmentItem");
        evas_object_smart_callback_add(sc, "changed", cb_changed, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        ic2 = elm_icon_add(bx);
        elm_icon_standard_set(ic2, "home");
        evas_object_size_hint_aspect_set(ic2, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        ic3 = elm_icon_add(bx);
        elm_icon_standard_set(ic3, "home");
        evas_object_size_hint_aspect_set(ic3, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        ic4 = elm_icon_add(bx);
        elm_icon_standard_set(ic4, "home");
        evas_object_size_hint_aspect_set(ic4, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        sc = elm_segment_control_add(win);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, ic2, NULL);
        seg_it = elm_segment_control_item_add(sc, ic3, NULL);
        elm_segment_control_item_selected_set(seg_it, EINA_TRUE);
        elm_segment_control_item_add(sc, ic4, NULL);
        evas_object_smart_callback_add(sc, "changed", cb_changed, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        ic5 = elm_icon_add(bx);
        elm_icon_standard_set(ic5, "home");
        evas_object_size_hint_aspect_set(ic5, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        sc = elm_segment_control_add(win);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "Disabled");
        seg_it = elm_segment_control_item_add(sc, ic5, "Disabled");
        elm_segment_control_item_selected_set(seg_it, EINA_TRUE);
        elm_segment_control_item_add(sc, NULL, "Disabled");
        elm_object_disabled_set(sc, EINA_TRUE);
        evas_object_smart_callback_add(sc, "changed", cb_changed, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        sc = elm_segment_control_add(win);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "Single");
        evas_object_smart_callback_add(sc, "changed", cb_changed, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        sc = elm_segment_control_add(win);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "Single Disabled");
        elm_object_disabled_set(sc, EINA_TRUE);
        evas_object_smart_callback_add(sc, "changed", cb_changed, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);
     }

   evas_object_show(bx);

   evas_object_resize(win, 400, 300);
   evas_object_show(win);
}
#endif
