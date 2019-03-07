#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

static const struct {
   Evas_Image_Orient orient;
   const char *name;
} images_orient[] = {
  { EVAS_IMAGE_ORIENT_NONE, "None" },
  { EVAS_IMAGE_ORIENT_90, "Rotate 90" },
  { EVAS_IMAGE_ORIENT_180, "Rotate 180" },
  { EVAS_IMAGE_ORIENT_270, "Rotate 270" },
  { EVAS_IMAGE_FLIP_HORIZONTAL, "Horizontal Flip" },
  { EVAS_IMAGE_FLIP_VERTICAL, "Vertical Flip" },
  { EVAS_IMAGE_FLIP_TRANSPOSE, "Transpose" },
  { EVAS_IMAGE_FLIP_TRANSVERSE, "Transverse" },
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


static void
im_align_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   double h,v;
   Evas_Object *win = data;
   Evas_Object *im = evas_object_data_get(win, "im");
   Evas_Object *h_sl = evas_object_data_get(win, "h_sl");
   Evas_Object *v_sl = evas_object_data_get(win, "v_sl");

   h = elm_slider_value_get(h_sl);
   v = elm_slider_value_get(v_sl);
   evas_object_size_hint_align_set(im, h, v);
   evas_object_size_hint_align_get(im, &h, &v);
   printf("align %.3f %.3f\n", h, v);
}

static const struct {
   Efl_Gfx_Image_Scale_Type scale_type;
   const char *name;
} images_scale_type[] = {
  { EFL_GFX_IMAGE_SCALE_TYPE_NONE, "None" },
  { EFL_GFX_IMAGE_SCALE_TYPE_FILL, "Fill" },
  { EFL_GFX_IMAGE_SCALE_TYPE_FIT_INSIDE, "Fit Inside" },
  { EFL_GFX_IMAGE_SCALE_TYPE_FIT_OUTSIDE, "Fit Outside" },
  { EFL_GFX_IMAGE_SCALE_TYPE_TILE, "Tile" },
  { 0, NULL }
};

static void
my_im_scale_ch(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *im = evas_object_data_get(win, "im");
   Evas_Object *rdg = evas_object_data_get(win, "rdg");
   int v = elm_radio_value_get(rdg);

   efl_gfx_image_scale_type_set(im, images_scale_type[v].scale_type);
   fprintf(stderr, "Set %d[%s] and got %d\n",
   images_scale_type[v].scale_type, images_scale_type[v].name, efl_gfx_image_scale_type_get(im));
}

void
test_image_scale_type(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *im, *rd, *rdg = NULL;
   int i;

   win = elm_win_util_standard_add("image test scale type", "Image Test Scale Type");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   im = efl_add(EFL_UI_IMAGE_CLASS, win);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(im, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, im);
   evas_object_show(im);

   evas_object_data_set(win, "im", im);

   for (i = 0; images_scale_type[i].name; ++i)
     {
        rd = elm_radio_add(win);
        evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
        elm_radio_state_value_set(rd, i);
        elm_object_text_set(rd, images_scale_type[i].name);
        elm_box_pack_end(box, rd);
        evas_object_show(rd);
        evas_object_smart_callback_add(rd, "changed", my_im_scale_ch, win);
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
test_image_swallow_align(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *im, *ly, *sl;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("image align", "Test Align Inside Layout");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "image_align");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ly);
   evas_object_show(ly);

   im = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(im, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set(ly, "swallow", im);
   evas_object_show(im);
   evas_object_data_set(win, "im", im);

   sl = elm_slider_add(win);
   elm_slider_value_set(sl, 0.5);
   elm_object_text_set(sl, "Horiz Align");
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(sl, "changed", im_align_cb, win);
   elm_box_pack_end(box, sl);
   evas_object_show(sl);
   evas_object_data_set(win, "h_sl", sl);

   sl = elm_slider_add(win);
   elm_slider_value_set(sl, 0.5);
   elm_object_text_set(sl, "Vert Align");
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(sl, "changed", im_align_cb, win);
   elm_box_pack_end(box, sl);
   evas_object_show(sl);
   evas_object_data_set(win, "v_sl", sl);

   evas_object_resize(win, 300, 600);
   evas_object_show(win);
}

static void
_download_start_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data, *txt;
   const char *url = NULL;
   char buf[4096] = {0};

   txt = evas_object_data_get(win, "txt");
   elm_image_file_get(txt, &url, NULL);
   snprintf(buf, sizeof(buf) - 1, "Remote image download started:\n%s", url);
   elm_object_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);
}

static void
_download_progress_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Image_Progress *p = event_info;
   Evas_Object *win = data, *txt;
   char buf[4096] = {0};

   txt = evas_object_data_get(win, "txt");
   snprintf(buf, sizeof(buf) - 1, "Remote image download progress %.2f/%.2f.", p->now, p->total);
   elm_object_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);
}

static void
_download_done_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data, *txt;
   char buf[4096] = {0};

   txt = evas_object_data_get(win, "txt");
   snprintf(buf, sizeof(buf) - 1, "Remote image download done.");
   elm_object_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);

   evas_object_hide(txt);
}

static void
_download_error_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data, *txt;
   char buf[4096] = {0};

   txt = evas_object_data_get(win, "txt");
   snprintf(buf, sizeof(buf) - 1, "Remote image download failed.");
   elm_object_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);

   evas_object_show(txt);
}

static void
_url_activate_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data, *txt, *im;
   const char *url;

   im = evas_object_data_get(win, "im");
   txt = evas_object_data_get(win, "txt");

   url = elm_object_text_get(obj);
   elm_image_file_set(im, url, NULL);

   evas_object_show(txt);
}

void
test_remote_image(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *im, *rd, *rdg = NULL, *box2, *o, *tbl;
   int i;

   win = elm_win_util_standard_add("image test", "Image Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   tbl = o = elm_table_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   o = elm_label_add(box);
   elm_label_line_wrap_set(o, 1);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tbl, o, 0, 0, 1, 1);
   evas_object_data_set(win, "txt", o);
   evas_object_hide(o);

   im = o = elm_image_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_data_set(win, "im", o);
   elm_table_pack(tbl, o, 0, 0, 1, 1);
   evas_object_show(o);

   elm_box_pack_end(box, tbl);
   evas_object_show(tbl);

   evas_object_smart_callback_add(im, "download,start", _download_start_cb, win);
   evas_object_smart_callback_add(im, "download,progress", _download_progress_cb, win);
   evas_object_smart_callback_add(im, "download,done", _download_done_cb, win);
   evas_object_smart_callback_add(im, "download,error", _download_error_cb, win);

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

   box2 = o = elm_box_add(box);
   elm_box_horizontal_set(o, 1);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   o = elm_label_add(box2);
   elm_object_text_set(o, "URL:");
   elm_box_pack_end(box2, o);
   evas_object_show(o);

   o = elm_entry_add(box2);
   elm_entry_scrollable_set(o, 1);
   elm_entry_single_line_set(o, 1);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   //elm_object_text_set(o, "http://41.media.tumblr.com/29f1ecd4f98aaff73fb21f479b450d4c/tumblr_mqsxdciQmB1rrju89o1_1280.jpg");
   elm_object_text_set(o, "http://68.media.tumblr.com/d14765b2cc4ec25d1e7d640f3ec77a40/tumblr_ohtpjtRNlm1rrju89o1_500.jpg");
   evas_object_smart_callback_add(o, "activated", _url_activate_cb, win);
   elm_box_pack_end(box2, o);
   evas_object_show(o);

   elm_box_pack_end(box, box2);
   evas_object_show(box2);

   // set file now
   _url_activate_cb(win, o, NULL);

   evas_object_show(win);
   evas_object_resize(win, 320, 480);
}

static void
_img_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   fprintf(stderr, "%p - clicked\n", obj);
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

#define STATUS_SET(obj, fmt) do { \
   elm_object_text_set(obj, fmt); \
   fprintf(stderr, "%s\n", fmt); fflush(stderr); \
   } while (0)

static void
_img_load_open_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *status_text = data;

   STATUS_SET(status_text, "Async file open done.");
}

static void
_img_load_ready_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *status_text = data;

   STATUS_SET(status_text, "Image is ready to show.");
}

static void
_img_load_error_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *status_text = data;

   STATUS_SET(status_text, "Async file load failed.");
}

static void
_img_load_cancel_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *status_text = data;

   STATUS_SET(status_text, "Async file open has been cancelled.");
}

static void
_create_image(Evas_Object *data, Eina_Bool async, Eina_Bool preload, Eina_Bool logo)
{
   Evas_Object *win = data;
   Evas_Object *im, *status_text;
   Evas_Object *box = evas_object_data_get(win, "box");
   char buf[PATH_MAX] = {0};

   im = elm_image_add(win);
   elm_image_async_open_set(im, async);
   elm_image_preload_disabled_set(im, preload);

   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(im, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_data_set(win, "im", im);
   elm_box_pack_start(box, im);
   evas_object_show(im);

   status_text = evas_object_data_get(win, "phld");
   if (!status_text)
     {
        status_text = elm_label_add(win);
        evas_object_size_hint_weight_set(status_text, EVAS_HINT_EXPAND, 0);
        evas_object_size_hint_align_set(status_text, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_data_set(win, "phld", status_text);
        elm_box_pack_after(box, status_text, im);
        evas_object_show(status_text);
     }

   evas_object_smart_callback_add(im, "load,open", _img_load_open_cb, status_text);
   evas_object_smart_callback_add(im, "load,ready", _img_load_ready_cb, status_text);
   evas_object_smart_callback_add(im, "load,error", _img_load_error_cb, status_text);
   evas_object_smart_callback_add(im, "load,cancel", _img_load_cancel_cb, status_text);

   STATUS_SET(status_text, "Loading image...");
   if (!logo)
     snprintf(buf, sizeof(buf) - 1, "%s/images/insanely_huge_test_image.jpg", elm_app_data_dir_get());
   else
     snprintf(buf, sizeof(buf) - 1, "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
}

static void
_reload_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *im = evas_object_data_get(win, "im");
   Evas_Object *chk1 = evas_object_data_get(win, "chk1");
   Evas_Object *chk2 = evas_object_data_get(win, "chk2");
   Eina_Bool async = elm_check_state_get(chk1);
   Eina_Bool preload = elm_check_state_get(chk2);
   Eina_Bool logo = EINA_FALSE;
   const char *file = NULL;

   elm_image_file_get(im, &file, NULL);
   logo = (file && strstr(file, "logo"));

   evas_object_del(im);
   _create_image(win, async, preload, logo);
}

static void
_switch_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *im = evas_object_data_get(win, "im");
   Evas_Object *chk1 = evas_object_data_get(win, "chk1");
   Evas_Object *chk2 = evas_object_data_get(win, "chk2");
   Eina_Bool async = elm_check_state_get(chk1);
   Eina_Bool preload = elm_check_state_get(chk2);
   char buf[PATH_MAX] = {0};
   Eina_Bool logo = EINA_FALSE;
   const char *file = NULL;

   elm_image_file_get(im, &file, NULL);
   logo = (file && strstr(file, "logo"));

   if (logo)
     {
        snprintf(buf, sizeof(buf) - 1, "%s/images/insanely_huge_test_image.jpg", elm_app_data_dir_get());
     }
   else
     {
        snprintf(buf, sizeof(buf) - 1, "%s/images/logo.png", elm_app_data_dir_get());
     }

   elm_image_async_open_set(im, async);
   elm_image_preload_disabled_set(im, preload);
   elm_image_file_set(im, buf, NULL);
}

void
test_load_image(void *data EINA_UNUSED, Evas_Object *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *hbox, *label, *chk1, *chk2, *bt;

   win = elm_win_util_standard_add("image test", "Image Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   elm_box_align_set(box, 0.5, 1.0);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);
   evas_object_data_set(win, "box", box);

   _create_image(win, EINA_FALSE, EINA_FALSE, EINA_FALSE);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_align_set(hbox, 0, 0.5);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, 0.0);
   {
      label = elm_label_add(win);
      elm_object_text_set(label, "Async load options:");
      evas_object_size_hint_weight_set(label, 0.0, 0.0);
      evas_object_size_hint_align_set(label, EVAS_HINT_FILL, 0.5);
      elm_box_pack_end(hbox, label);
      evas_object_show(label);

      chk1 = elm_check_add(hbox);
      elm_object_text_set(chk1, "Async file open");
      evas_object_size_hint_weight_set(chk1, 0.0, 0.0);
      evas_object_size_hint_align_set(chk1, EVAS_HINT_FILL, 0.5);
      elm_box_pack_end(hbox, chk1);
      evas_object_data_set(win, "chk1", chk1);
      evas_object_show(chk1);

      chk2 = elm_check_add(hbox);
      elm_object_text_set(chk2, "Disable preload");
      evas_object_size_hint_weight_set(chk2, 0.0, 0.0);
      evas_object_size_hint_align_set(chk2, EVAS_HINT_FILL, 0.5);
      elm_box_pack_end(hbox, chk2);
      evas_object_data_set(win, "chk2", chk2);
      evas_object_show(chk2);
   }
   evas_object_show(hbox);
   elm_box_pack_end(box, hbox);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_box_align_set(hbox, 0.5, 0.5);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, 0.0);
   {
      bt = elm_button_add(win);
      elm_object_text_set(bt, "Image Reload");
      evas_object_smart_callback_add(bt, "clicked", _reload_clicked, win);
      elm_box_pack_end(hbox, bt);
      evas_object_show(bt);

      bt = elm_button_add(win);
      elm_object_text_set(bt, "Image Switch");
      evas_object_smart_callback_add(bt, "clicked", _switch_clicked, win);
      elm_box_pack_end(hbox, bt);
      evas_object_show(bt);
   }
   evas_object_show(hbox);
   elm_box_pack_end(box, hbox);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

static void
_cb_prescale_radio_changed(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *o_bg = data;
   int size;
   size = elm_radio_value_get((Evas_Object *)obj);
   elm_image_prescale_set(o_bg, size);
}

void
test_image_prescale(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *im;
   Evas_Object *box, *hbox;
   Evas_Object *rd, *rdg;
   char buf[PATH_MAX];


   win = elm_win_util_standard_add("image-prescale", "Image Prescale Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   im = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(im, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, im);
   evas_object_show(im);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 50);
   elm_object_text_set(rd, "50");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_prescale_radio_changed, im);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   rdg = rd;

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 100);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "100");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_prescale_radio_changed, im);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 200);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "200");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_prescale_radio_changed, im);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);

   elm_radio_value_set(rdg, 200);

   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}
