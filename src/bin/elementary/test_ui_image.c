#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include <Efl_Ui.h>

static const struct {
   Efl_Gfx_Orientation orient;
   const char *name;
} images_orient[] = {
  { EFL_GFX_ORIENTATION_NONE, "None" },
  { EFL_GFX_ORIENTATION_RIGHT, "Rotate 90" },
  { EFL_GFX_ORIENTATION_DOWN, "Rotate 180" },
  { EFL_GFX_ORIENTATION_LEFT, "Rotate 270" },
  { EFL_GFX_ORIENTATION_FLIP_HORIZONTAL, "Horizontal Flip" },
  { EFL_GFX_ORIENTATION_FLIP_VERTICAL, "Vertical Flip" },
  { 0, NULL }
};

static Eo *
win_add(const char *name, const char *title)
{
   return efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
     efl_ui_win_name_set(efl_added, name),
     efl_ui_win_autodel_set(efl_added, EINA_TRUE),
     efl_text_set(efl_added, title));
}

static Eo *
img_add(Eo *win, const char *file)
{
   char buf[PATH_MAX];
   Eo *im;

   im = efl_add(EFL_UI_IMAGE_CLASS, win,
     efl_gfx_hint_weight_set(efl_added, 1.0, 1.0),
     efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE)
   );
  if (file)
    {
       snprintf(buf, sizeof(buf), "%s%s", elm_app_data_dir_get(), file);
       efl_file_simple_load(im, buf, NULL);
    }
   efl_key_data_set(win, "im", im);
   return im;
}

static void
my_im_ch(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *im = efl_key_data_get(win, "im");
   Eo *rdg = efl_key_data_get(win, "rdg");
   Efl_Gfx_Orientation v = efl_ui_radio_state_value_get(efl_ui_radio_selected_object_get(rdg));

   efl_gfx_orientation_set(im, v);
   fprintf(stderr, "Set %i and got %i\n",
           v, efl_gfx_orientation_get(im));
}

void
test_ui_image(void *data EINA_UNUSED, Eo *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *im, *rd, *rdg = NULL;
   int i;

   win = win_add("image test", "Image Test");

   box = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(box, 1.0, 1.0);
   efl_content_set(win, box);

   im = img_add(win, "/images/logo.png");
   efl_pack(box, im);

   for (i = 0; images_orient[i].name; ++i)
     {
        rd = efl_add(EFL_UI_RADIO_CLASS, win);
        efl_gfx_hint_fill_set(rd, EINA_TRUE, EINA_TRUE);
        efl_gfx_hint_weight_set(rd, 1.0, 0.0);
        efl_ui_radio_state_value_set(rd, images_orient[i].orient);
        efl_text_set(rd, images_orient[i].name);
        efl_pack(box, rd);
        efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, my_im_ch, win);
        if (!rdg)
          {
             rdg = rd;
             efl_key_data_set(win, "rdg", rdg);
          }
        else
          {
             efl_ui_radio_group_add(rd, rdg);
          }
     }

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 480));
}


static void
im_align_cb(void *data, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   double h,v;
   Eo *win = data;
   Eo *im = efl_key_data_get(win, "im");
   Eo *h_sl = efl_key_data_get(win, "h_sl");
   Eo *v_sl = efl_key_data_get(win, "v_sl");

   h = elm_slider_value_get(h_sl);
   v = elm_slider_value_get(v_sl);
   efl_gfx_hint_align_set(im, h, v);
   efl_gfx_hint_align_get(im, &h, &v);
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
my_im_scale_ch(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *im = efl_key_data_get(win, "im");
   Eo *rdg = efl_key_data_get(win, "rdg");
   int v = efl_ui_radio_state_value_get(efl_ui_radio_selected_object_get(rdg));

   efl_gfx_image_scale_type_set(im, images_scale_type[v].scale_type);
   fprintf(stderr, "Set %d[%s] and got %d\n",
   images_scale_type[v].scale_type, images_scale_type[v].name, efl_gfx_image_scale_type_get(im));
}

void
test_ui_image_scale_type(void *data EINA_UNUSED, Eo *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *im, *rd, *rdg = NULL;
   int i;

   win = win_add("image test scale type", "Image Test Scale Type");

   box = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(box, 1.0, 1.0);
   efl_content_set(win, box);

   im = efl_add(EFL_UI_IMAGE_CLASS, win);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(im, buf, NULL);
   efl_gfx_hint_weight_set(im, 1.0, 1.0);
   efl_gfx_hint_fill_set(im, EINA_TRUE, EINA_TRUE);
   efl_pack(box, im);

   efl_key_data_set(win, "im", im);

   for (i = 0; images_scale_type[i].name; ++i)
     {
        rd = efl_add(EFL_UI_RADIO_CLASS, win);
        efl_gfx_hint_fill_set(rd, EINA_TRUE, EINA_TRUE);
        efl_gfx_hint_weight_set(rd, 1.0, 0.0);
        efl_ui_radio_state_value_set(rd, i);
        efl_text_set(rd, images_scale_type[i].name);
        efl_pack(box, rd);
        efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, my_im_scale_ch, win);
        if (!rdg)
          {
             rdg = rd;
             efl_key_data_set(win, "rdg", rdg);
          }
        else
          {
             efl_ui_radio_group_add(rd, rdg);
          }
     }

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 480));
}

void
test_ui_image_swallow_align(void *data EINA_UNUSED, Eo *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *im, *ly, *sl;
   char buf[PATH_MAX];

   win = win_add("image align", "Test Align Inside Layout");

   box = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(box, 1.0, 1.0);
   efl_content_set(win, box);

   ly = efl_add(EFL_UI_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   efl_file_simple_load(ly, buf, "image_align");
   efl_gfx_hint_weight_set(ly, 1.0, 1.0);
   efl_gfx_hint_fill_set(ly, EINA_TRUE, EINA_TRUE);
   efl_pack(box, ly);

   im = img_add(win, "/images/logo.png");
   efl_gfx_hint_weight_set(im, 0, 0);
   efl_gfx_hint_fill_set(im, EINA_FALSE, EINA_FALSE);
   efl_content_set(efl_part(ly, "swallow"), im);

   sl = elm_slider_add(win);
   elm_slider_value_set(sl, 0.5);
   efl_text_set(sl, "Horiz Align");
   efl_gfx_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(sl, "changed", im_align_cb, win);
   efl_pack(box, sl);
   evas_object_show(sl);
   efl_key_data_set(win, "h_sl", sl);

   sl = elm_slider_add(win);
   elm_slider_value_set(sl, 0.5);
   efl_text_set(sl, "Vert Align");
   efl_gfx_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(sl, "changed", im_align_cb, win);
   efl_pack(box, sl);
   evas_object_show(sl);
   efl_key_data_set(win, "v_sl", sl);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 600));
}

static void
_download_start_cb(void *data, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win = data, *txt, *im;
   const char *url = NULL;
   char buf[4096] = {0};

   txt = efl_key_data_get(win, "txt");
   im = efl_key_data_get(win, "im");
   url = efl_file_get(im);
   snprintf(buf, sizeof(buf) - 1, "Remote image download started:\n%s", url);
   efl_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);
}

static void
_download_progress_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, void *event_info)
{
   Elm_Image_Progress *p = event_info;
   Eo *win = data, *txt;
   char buf[4096] = {0};

   txt = efl_key_data_get(win, "txt");
   snprintf(buf, sizeof(buf) - 1, "Remote image download progress %.2f/%.2f.", p->now, p->total);
   efl_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);
}

static void
_download_done_cb(void *data, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win = data, *txt;
   char buf[4096] = {0};

   txt = efl_key_data_get(win, "txt");
   snprintf(buf, sizeof(buf) - 1, "Remote image download done.");
   efl_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);

   evas_object_hide(txt);
}

static void
_download_error_cb(void *data, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win = data, *txt;
   char buf[4096] = {0};

   txt = efl_key_data_get(win, "txt");
   snprintf(buf, sizeof(buf) - 1, "Remote image download failed.");
   efl_text_set(txt, buf);
   printf("%s\n", buf);
   fflush(stdout);

}

static void
_url_activate_cb(void *data, Eo *obj, void *event_info EINA_UNUSED)
{
   Eo *win = data, *txt, *im;
   const char *url;

   im = efl_key_data_get(win, "im");
   txt = efl_key_data_get(win, "txt");

   url = elm_object_text_get(obj);
   elm_image_file_set(im, url, NULL);

   evas_object_show(txt);
}

void
test_remote_ui_image(void *data EINA_UNUSED, Eo *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *im, *rd, *rdg = NULL, *box2, *o;
   int i;

   win = win_add("image test", "Image Test");

   box = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(box, 1.0, 1.0);
   efl_content_set(win, box);

   o = efl_add(EFL_UI_TEXT_CLASS, box, efl_text_interactive_editable_set(efl_added, EINA_FALSE));
   efl_text_wrap_set(o, EFL_TEXT_FORMAT_WRAP_MIXED);
   efl_gfx_hint_weight_set(o, 1.0, 1.0);
   efl_gfx_hint_fill_set(o, EINA_TRUE, EINA_TRUE);
   efl_pack(box, o);
   efl_key_data_set(win, "txt", o);
   evas_object_hide(o);

   im = o = img_add(win, NULL);
   efl_pack(box, o);

   evas_object_smart_callback_add(im, "download,start", _download_start_cb, win);
   evas_object_smart_callback_add(im, "download,progress", _download_progress_cb, win);
   evas_object_smart_callback_add(im, "download,done", _download_done_cb, win);
   evas_object_smart_callback_add(im, "download,error", _download_error_cb, win);

   for (i = 0; images_orient[i].name; ++i)
     {
        rd = efl_add(EFL_UI_RADIO_CLASS, win);
        efl_gfx_hint_fill_set(rd, EINA_TRUE, EINA_TRUE);
        efl_gfx_hint_weight_set(rd, 1.0, 0.0);
        efl_ui_radio_state_value_set(rd, images_orient[i].orient);
        efl_text_set(rd, images_orient[i].name);
        efl_pack(box, rd);
        efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, my_im_ch, win);
        if (!rdg)
          {
             rdg = rd;
             efl_key_data_set(win, "rdg", rdg);
          }
        else
          {
             efl_ui_radio_group_add(rd, rdg);
          }
     }

   box2 = o = efl_add(EFL_UI_BOX_CLASS, box);
   efl_ui_direction_set(o, EFL_UI_DIR_RIGHT);
   efl_gfx_hint_weight_set(o, 1.0, 0);
   efl_gfx_hint_fill_set(o, EINA_TRUE, EINA_TRUE);

   o = efl_add(EFL_UI_TEXT_CLASS, box2,
     efl_text_interactive_editable_set(efl_added, EINA_FALSE)
   );
   efl_text_set(o, "URL:");
   efl_pack(box2, o);

   o = elm_entry_add(box2);
   elm_entry_scrollable_set(o, 1);
   elm_entry_single_line_set(o, 1);
   efl_gfx_hint_weight_set(o, 1.0, 0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   //elm_entry_entry_set(o, "http://41.media.tumblr.com/29f1ecd4f98aaff73fb21f479b450d4c/tumblr_mqsxdciQmB1rrju89o1_1280.jpg");
   elm_entry_entry_set(o, "http://68.media.tumblr.com/d14765b2cc4ec25d1e7d640f3ec77a40/tumblr_ohtpjtRNlm1rrju89o1_500.jpg");
   evas_object_smart_callback_add(o, "activated", _url_activate_cb, win);
   evas_object_show(o);
   efl_pack(box2, o);

   efl_pack(box, box2);

   // set file now
   _url_activate_cb(win, o, NULL);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 480));
}

static void
_img_clicked_cb(void *data EINA_UNUSED, Eo *obj, void *event_info EINA_UNUSED)
{
   fprintf(stderr, "%p - clicked\n", obj);
}

void
test_click_ui_image(void *data EINA_UNUSED, Eo *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *im, *label;

   win = win_add("image test", "Image Test");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   box = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(box, 1.0, 1.0);
   efl_content_set(win, box);

   im = img_add(win, "/images/logo.png");
   elm_object_focus_allow_set(im, EINA_TRUE);
   evas_object_smart_callback_add(im, "clicked", _img_clicked_cb, im);
   efl_pack(box, im);
   elm_object_focus_set(im, EINA_TRUE);

   label = efl_add(EFL_UI_TEXT_CLASS, win, efl_text_interactive_editable_set(efl_added, EINA_FALSE));
   efl_text_set(label, "<b>Press Return/Space/KP_Return key on image to transit.</b>");
   efl_gfx_hint_weight_set(label, 0.0, 0.0);
   efl_gfx_hint_fill_set(label, EINA_TRUE, EINA_TRUE);
   efl_pack(box, label);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 480));
}

#define STATUS_SET(obj, fmt) do { \
   efl_text_set(obj, fmt); \
   fprintf(stderr, "%s\n", fmt); fflush(stderr); \
   } while (0)

static void
_img_load_open_cb(void *data, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *status_text = data;

   STATUS_SET(status_text, "Async file open done.");
}

static void
_img_load_ready_cb(void *data, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *status_text = data;

   STATUS_SET(status_text, "Image is ready to show.");
}

static void
_img_load_error_cb(void *data, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *status_text = data;

   STATUS_SET(status_text, "Async file load failed.");
}

static void
_img_load_cancel_cb(void *data, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *status_text = data;

   STATUS_SET(status_text, "Async file open has been cancelled.");
}

static void
_create_image(Eo *data, Eina_Bool async, Eina_Bool preload, Eina_Bool logo)
{
   Eo *win = data;
   Eo *im, *status_text;
   Eo *box = efl_key_data_get(win, "box");
   char buf[PATH_MAX] = {0};

   im = img_add(win, NULL);
   elm_image_async_open_set(im, async);
   elm_image_preload_disabled_set(im, preload);

   efl_pack_begin(box, im);

   status_text = efl_key_data_get(win, "phld");
   if (!status_text)
     {
        status_text = efl_add(EFL_UI_TEXT_CLASS, win, efl_text_interactive_editable_set(efl_added, EINA_FALSE));
        efl_gfx_hint_weight_set(status_text, 1.0, 0);
        efl_gfx_hint_fill_set(status_text, EINA_TRUE, EINA_TRUE);
        efl_key_data_set(win, "phld", status_text);
        efl_pack_after(box, status_text, im);
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
   efl_file_simple_load(im, buf, NULL);
}

static void
_reload_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *im = efl_key_data_get(win, "im");
   Eo *chk1 = efl_key_data_get(win, "chk1");
   Eo *chk2 = efl_key_data_get(win, "chk2");
   Eina_Bool async = efl_ui_check_selected_get(chk1);
   Eina_Bool preload = efl_ui_check_selected_get(chk2);
   Eina_Bool logo = EINA_FALSE;
   const char *file = NULL;

   file = efl_file_get(im);
   logo = (file && strstr(file, "logo"));

   efl_del(im);
   _create_image(win, async, preload, logo);
}

static void
_switch_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *im = efl_key_data_get(win, "im");
   Eo *chk1 = efl_key_data_get(win, "chk1");
   Eo *chk2 = efl_key_data_get(win, "chk2");
   Eina_Bool async = efl_ui_check_selected_get(chk1);
   Eina_Bool preload = efl_ui_check_selected_get(chk2);
   char buf[PATH_MAX] = {0};
   Eina_Bool logo = EINA_FALSE;
   const char *file = NULL;

   file = efl_file_get(im);
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
   efl_file_simple_load(im, buf, NULL);
}

void
test_load_ui_image(void *data EINA_UNUSED, Eo *obj  EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *hbox, *label, *chk1, *chk2, *bt;

   win = win_add("image test", "Image Test");

   box = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_arrangement_content_align_set(box, 0.5, 1.0);
   efl_gfx_hint_weight_set(box, 1.0, 1.0);
   efl_content_set(win, box);
   efl_key_data_set(win, "box", box);

   _create_image(win, EINA_FALSE, EINA_FALSE, EINA_FALSE);

   hbox = efl_add(EFL_UI_BOX_CLASS, win);
   efl_ui_direction_set(hbox, EFL_UI_DIR_RIGHT);
   efl_gfx_arrangement_content_align_set(hbox, 0, 0.5);
   efl_gfx_hint_weight_set(hbox, 1.0, 0.0);
   efl_gfx_hint_fill_set(hbox, EINA_TRUE, EINA_FALSE);
   {
      label = efl_add(EFL_UI_TEXT_CLASS, win,
        efl_text_set(efl_added, "Async load options:"),
        efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
        efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
        efl_text_interactive_editable_set(efl_added, EINA_FALSE)
      );
      efl_pack(hbox, label);

      chk1 = efl_add(EFL_UI_CHECK_CLASS, hbox,
        efl_text_set(efl_added, "Async file open"),
        efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
        efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE)
      );
      efl_pack(hbox, chk1);
      efl_key_data_set(win, "chk1", chk1);

      chk2 = efl_add(EFL_UI_CHECK_CLASS, hbox,
        efl_text_set(efl_added, "Disable preload"),
        efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
        efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE)
      );
      efl_pack(hbox, chk2);
      efl_key_data_set(win, "chk2", chk2);
   }
   efl_pack(box, hbox);

   hbox = efl_add(EFL_UI_BOX_CLASS, win);
   efl_ui_direction_set(hbox, EFL_UI_DIR_RIGHT);
   efl_gfx_arrangement_content_align_set(hbox, 0.5, 0.5);
   efl_gfx_hint_weight_set(hbox, 1.0, 0.0);
   efl_gfx_hint_fill_set(hbox, EINA_TRUE, EINA_FALSE),
   efl_gfx_hint_align_set(hbox, 0.5, 0.0);
   {
      bt = efl_add(EFL_UI_BUTTON_CLASS, win,
        efl_text_set(efl_added, "Image Reload"),
        efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _reload_clicked, win)
      );
      efl_pack(hbox, bt);

      bt = efl_add(EFL_UI_BUTTON_CLASS, win,
        efl_text_set(efl_added, "Image Switch"),
        efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _switch_clicked, win)
      );
      efl_pack(hbox, bt);
   }
   efl_pack(box, hbox);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 480));
}

static void
_cb_prescale_radio_changed(void *data, const Efl_Event *ev)
{
   Eo *o_bg = data;
   int size;
   size = efl_ui_radio_state_value_get(efl_ui_radio_selected_object_get(ev->object));
   //FIXME
   elm_image_prescale_set(o_bg, size);
}

void
test_ui_image_prescale(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *im;
   Eo *box, *hbox;
   Eo *rd, *rdg;

   win = win_add("image-prescale", "Image Prescale Test");

   box = efl_add(EFL_UI_BOX_CLASS, win);
   efl_content_set(win, box);

   im = img_add(win, "/images/plant_01.jpg");
   efl_pack(box, im);

   hbox = efl_add(EFL_UI_BOX_CLASS, win);
   efl_ui_direction_set(hbox, EFL_UI_DIR_RIGHT);
   efl_gfx_hint_weight_set(hbox, 1.0, 1.0);
   efl_gfx_hint_fill_set(hbox, EINA_TRUE, EINA_TRUE);

   rd = efl_add(EFL_UI_RADIO_CLASS, win);
   efl_ui_radio_state_value_set(rd, 50);
   efl_text_set(rd, "50");
   efl_gfx_hint_weight_set(rd, 1.0, 1.0);
   efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, _cb_prescale_radio_changed, im);
   efl_pack(hbox, rd);
   rdg = rd;

   rd = efl_add(EFL_UI_RADIO_CLASS, win);
   efl_ui_radio_state_value_set(rd, 100);
   efl_ui_radio_group_add(rd, rdg);
   efl_text_set(rd, "100");
   efl_gfx_hint_weight_set(rd, 1.0, 1.0);
   efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, _cb_prescale_radio_changed, im);
   efl_pack(hbox, rd);

   rd = efl_add(EFL_UI_RADIO_CLASS, win);
   efl_ui_radio_state_value_set(rd, 200);
   efl_ui_radio_group_add(rd, rdg);
   efl_text_set(rd, "200");
   efl_gfx_hint_weight_set(rd, 1.0, 1.0);
   efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, _cb_prescale_radio_changed, im);
   efl_pack(hbox, rd);

   elm_radio_value_set(rdg, 200);

   efl_pack(box, hbox);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 320));
}
