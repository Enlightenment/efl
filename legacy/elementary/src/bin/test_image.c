#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static const struct {
   Elm_Image_Orient orient;
   const char *name;
} images_orient[] = {
  { ELM_IMAGE_ORIENT_NONE, "None" },
  { ELM_IMAGE_ROTATE_90, "Rotate 90" },
  { ELM_IMAGE_ROTATE_180, "Rotate 180" },
  { ELM_IMAGE_ROTATE_270, "Rotate 270" },
  { ELM_IMAGE_FLIP_HORIZONTAL, "Horizontal Flip" },
  { ELM_IMAGE_FLIP_VERTICAL, "Vertical Flip" },
  { ELM_IMAGE_FLIP_TRANSPOSE, "Transpose" },
  { ELM_IMAGE_FLIP_TRANSVERSE, "Transverse" },
  { 0, NULL }
};

static void
my_im_ch(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *im = evas_object_data_get(win, "im");
   Evas_Object *rdg = evas_object_data_get(win, "rdg");
   Elm_Image_Orient v = elm_radio_value_get(rdg);

   elm_image_orient_set(im, v);
   fprintf(stderr, "Set %i and got %i\n",
           v, elm_image_orient_get(im));
}

void
test_image(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *im, *rd, *rdg = NULL;
   int i;

   win = elm_win_util_standard_add("image test", "Image Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   im = elm_image_add(win);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(im, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, im);
   evas_object_show(im);

   evas_object_data_set(win, "im", im);

   for (i = 0; images_orient[i].name; ++i)
     {
        rd = elm_radio_add(win);
        evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
        elm_radio_state_value_set(rd, images_orient[i].orient);
        elm_object_text_set(rd, images_orient[i].name);
        elm_box_pack_end(box, rd);
        evas_object_show(rd);
        evas_object_smart_callback_add(rd, "changed", my_im_ch, win);
        if (!rdg)
          {
             rdg = rd;
             evas_object_data_set(win, "rdg", rdg);
          }
        else
          {
             elm_radio_group_add(rd, rdg);
          }
     }

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

void
test_remote_image(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *im, *rd, *rdg = NULL;
   int i;

   win = elm_win_util_standard_add("image test", "Image Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   im = elm_image_add(win);
   elm_image_file_set(im, "http://31.media.tumblr.com/29f1ecd4f98aaff73fb21f479b450d4c/tumblr_mqsxdciQmB1rrju89o1_1280.jpg", NULL);
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(im, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_box_pack_end(box, im);
   evas_object_show(im);

   evas_object_data_set(win, "im", im);

   for (i = 0; images_orient[i].name; ++i)
     {
        rd = elm_radio_add(win);
        evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
        elm_radio_state_value_set(rd, images_orient[i].orient);
        elm_object_text_set(rd, images_orient[i].name);
        elm_box_pack_end(box, rd);
        evas_object_show(rd);
        evas_object_smart_callback_add(rd, "changed", my_im_ch, win);
        if (!rdg)
          {
             rdg = rd;
             evas_object_data_set(win, "rdg", rdg);
          }
        else
          {
             elm_radio_group_add(rd, rdg);
          }
     }

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

static void
_img_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;
   static int degree = 0;

   fprintf(stderr, "%p - clicked\n", obj);

   trans = elm_transit_add();
   elm_transit_object_add(trans, data);
   if (degree == 0)
     {
        elm_transit_effect_rotation_add(trans, 0, 180);
        degree = 180;
     }
   else
     {
        elm_transit_effect_rotation_add(trans, 180, 360);
        degree = 0;
     }
   elm_transit_duration_set(trans, 3.0);
   elm_transit_objects_final_state_keep_set(trans, EINA_TRUE);
   elm_transit_go(trans);
}

void
test_click_image(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *im, *label;

   win = elm_win_util_standard_add("image test", "Image Test");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   im = elm_image_add(win);
   elm_object_focus_allow_set(im, EINA_TRUE);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(im, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(im, "clicked", _img_clicked_cb, im);
   elm_box_pack_end(box, im);
   evas_object_show(im);
   elm_object_focus_set(im, EINA_TRUE);

   label = elm_label_add(win);
   elm_object_text_set(label, "<b>Press Return/Space/KP_Return key on image to transit.</b>");
   evas_object_size_hint_weight_set(label, 0.0, 0.0);
   evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, label);
   evas_object_show(label);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
