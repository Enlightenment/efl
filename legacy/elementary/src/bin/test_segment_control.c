#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
changed_cb(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event)
{
   Elm_Object_Item *it = event;
   printf("Segment Item (%p) %s\n", it, elm_object_item_text_get(it));
}

void
test_segment_control(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *sc, *ic;
   Elm_Object_Item *seg_it;
   unsigned int i;
   char buf[PATH_MAX];

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

   //box
   bx = elm_box_add(win);
   elm_box_padding_set(bx, 0, 5);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());

   for (i = 0; i < EINA_C_ARRAY_LENGTH(exp_modes); i++)
     {
        const struct exp_mode *em = exp_modes + i;

        //segment control
        sc = elm_segment_control_add(bx);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "Text Only");

        //icon
        ic = elm_icon_add(sc);
        elm_image_file_set(ic, buf, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        seg_it = elm_segment_control_item_add(sc, ic, NULL);
        elm_segment_control_item_selected_set(seg_it, EINA_TRUE);

        //icon
        ic = elm_icon_add(sc);
        elm_image_file_set(ic, buf, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1,
                                         1);
        elm_segment_control_item_add(sc, ic, "Text + Icon");
        evas_object_smart_callback_add(sc, "changed", changed_cb, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        //segment control 2
        sc = elm_segment_control_add(bx);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "Segment Control Item");
        seg_it = elm_segment_control_item_add(sc, NULL, "Segment Control Item");
        elm_segment_control_item_selected_set(seg_it, EINA_TRUE);
        elm_segment_control_item_add(sc, NULL, "Segment Control Item");
        elm_segment_control_item_add(sc, NULL, "Segment Control Item");
        evas_object_smart_callback_add(sc, "changed", changed_cb, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        //segment control 3 (icon only)
        sc = elm_segment_control_add(bx);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);

        //icon
        ic = elm_icon_add(sc);
        elm_image_file_set(ic, buf, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
        elm_segment_control_item_add(sc, ic, NULL);

        //icon
        ic = elm_icon_add(sc);
        elm_image_file_set(ic, buf, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
        seg_it = elm_segment_control_item_add(sc, ic, NULL);
        elm_segment_control_item_selected_set(seg_it, EINA_TRUE);

        //icon
        ic = elm_icon_add(sc);
        elm_image_file_set(ic, buf, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
        elm_segment_control_item_add(sc, ic, NULL);

        evas_object_smart_callback_add(sc, "changed", changed_cb, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        //segment control 4 (text only disabled)
        sc = elm_segment_control_add(bx);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "Disabled");

        //icon
        ic = elm_icon_add(sc);
        elm_image_file_set(ic, buf, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

        seg_it = elm_segment_control_item_add(sc, ic, "Disabled");
        elm_segment_control_item_selected_set(seg_it, EINA_TRUE);
        elm_segment_control_item_add(sc, NULL, "Disabled");
        elm_object_disabled_set(sc, EINA_TRUE);
        evas_object_smart_callback_add(sc, "changed", changed_cb, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        //segment control 5 (single)
        sc = elm_segment_control_add(bx);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "Single");
        evas_object_smart_callback_add(sc, "changed", changed_cb, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);

        //segment control 6 (single disabled)
        sc = elm_segment_control_add(bx);
        evas_object_size_hint_weight_set(sc, em->weight.w, em->weight.h);
        evas_object_size_hint_align_set(sc, em->align.x, em->align.y);
        elm_segment_control_item_add(sc, NULL, "Single Disabled");
        elm_object_disabled_set(sc, EINA_TRUE);
        evas_object_smart_callback_add(sc, "changed", changed_cb, NULL);
        evas_object_show(sc);
        elm_box_pack_end(bx, sc);
     }

   evas_object_resize(win, 400, 300);
   evas_object_show(win);
}
