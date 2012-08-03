#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

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
my_im_ch(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
test_image(void *data __UNUSED__, Evas_Object *obj  __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *box, *im, *rd, *rdg = NULL;
   int i;

   win = elm_win_util_standard_add("image test", "Image Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   elm_win_resize_object_add(win, box);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(box);

   im = elm_image_add(win);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
   elm_image_resizable_set(im, EINA_TRUE, EINA_TRUE);
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
#endif
