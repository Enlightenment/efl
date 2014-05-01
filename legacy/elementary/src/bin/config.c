#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _Theme               Theme;
typedef struct _Fonts_Data          Fonts_Data;
typedef struct _Elm_Text_Class_Data Elm_Text_Class_Data;
typedef struct _Elm_Font_Size_Data  Elm_Font_Size_Data;

struct _Theme
{
   const char *label;
   const char *name;
   const char *path;
   Eina_Bool   in_search_path;
};

static const Eina_List *tsel = NULL;
static Eina_List *themes = NULL;

struct _Elm_Text_Class_Data
{
   const char    *name;
   const char    *desc;
   const char    *font;
   const char    *style;
   Evas_Font_Size size;
};

struct _Elm_Font_Size_Data
{
   const char    *size_str;
   Evas_Font_Size size;
};

struct _Fonts_Data
{
   Eina_List  *text_classes;

   Eina_Hash  *font_hash;
   Eina_List  *font_px_list;
   Eina_List  *font_scale_list;

   const char *cur_font;
   const char *cur_style;
   double      cur_size;
};

#define ELM_LIST_DISABLE(list)                         \
  do                                                   \
    {                                                  \
       const Eina_List *_l = elm_list_items_get(list); \
       if (_l)                                         \
         {                                             \
            elm_list_item_show(eina_list_data_get(_l));\
            elm_object_disabled_set(list, EINA_TRUE);  \
         }                                             \
    }                                                  \
  while (0)

#define ELM_LIST_ENABLE(list)                     \
  do                                              \
    {                                             \
       elm_object_disabled_set(list, EINA_FALSE); \
    }                                             \
  while (0)

#define LABEL_FRAME_ADD(label)                                      \
  do                                                                \
    {                                                               \
       pd = elm_frame_add(win);                                     \
       evas_object_size_hint_weight_set(pd, EVAS_HINT_EXPAND, 0.0); \
       evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);    \
       elm_object_style_set(pd, "pad_medium");                      \
       elm_box_pack_end(bx, pd);                                    \
       evas_object_show(pd);                                        \
                                                                    \
       lb = elm_label_add(win);                                     \
       evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0); \
       evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);    \
       elm_object_text_set(lb, label);                              \
       elm_object_content_set(pd, lb);                              \
       evas_object_show(lb);                                        \
    }                                                               \
  while (0)

#define CHECK_ADD(_label, _desc, _cb, _cb_param)  \
   ck = elm_check_add(win); \
   elm_object_text_set(ck, _label); \
   elm_object_tooltip_text_set(ck, _desc); \
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0); \
   evas_object_size_hint_align_set(ck, EVAS_HINT_FILL, 0.5); \
   elm_box_pack_end(bx, ck); \
   evas_object_show(ck); \
   evas_object_smart_callback_add(ck, "changed", _cb, _cb_param);

static int quiet = 0;
static int interactive = 1;

static const char *theme_set = NULL;
static const char *finger_size_set = NULL;
static const char *scale_set = NULL;
static Fonts_Data fndata = {NULL, NULL, NULL, NULL, NULL, NULL, 0.0};

static void
_font_styles_list_sel(void *data   EINA_UNUSED,
                      Evas_Object *obj,
                      void        *event_info);

static void
config_exit(void *data       EINA_UNUSED,
            Evas_Object *obj EINA_UNUSED,
            void *event_info EINA_UNUSED)
{
   Elm_Text_Class_Data *tc_data;
   Elm_Font_Size_Data *sd;

   EINA_LIST_FREE(fndata.text_classes, tc_data)
     {
        eina_stringshare_del(tc_data->name);
        eina_stringshare_del(tc_data->desc);
        eina_stringshare_del(tc_data->font);
        eina_stringshare_del(tc_data->style);
        free(tc_data);
     }

   elm_font_available_hash_del(fndata.font_hash);
   fndata.font_hash = NULL;

   EINA_LIST_FREE(fndata.font_px_list, sd)
     {
        eina_stringshare_del(sd->size_str);
        free(sd);
     }

   EINA_LIST_FREE(fndata.font_scale_list, sd)
     {
        eina_stringshare_del(sd->size_str);
        free(sd);
     }

   if (fndata.cur_font) eina_stringshare_del(fndata.cur_font);
   fndata.cur_font = NULL;

   if (fndata.cur_style) eina_stringshare_del(fndata.cur_style);
   fndata.cur_style = NULL;

   elm_config_save();
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

static void
sb_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   Eina_Bool val = elm_check_state_get(obj);
   Eina_Bool sb = elm_config_scroll_bounce_enabled_get();

   if (val == sb) return;
   elm_config_scroll_bounce_enabled_set(val);
   elm_config_all_flush();

   /*TODO: enable/disable subordinate sliders (make 'em support it 1st)*/
}

static void
bf_round(void *data       EINA_UNUSED,
         Evas_Object     *obj,
         void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
bf_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double bf = elm_config_scroll_bounce_friction_get();
   double val = elm_slider_value_get(obj);

   if (bf == val) return;
   elm_config_scroll_bounce_friction_set(val);
   elm_config_all_flush();
}

static void
ps_round(void *data       EINA_UNUSED,
         Evas_Object     *obj,
         void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
ps_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double ps = elm_config_scroll_page_scroll_friction_get();
   double val = elm_slider_value_get(obj);

   if (ps == val) return;
   elm_config_scroll_page_scroll_friction_set(val);
   elm_config_all_flush();
}

static void
bis_round(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
bis_change(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double bis = elm_config_scroll_bring_in_scroll_friction_get();
   double val = elm_slider_value_get(obj);

   if (bis == val) return;
   elm_config_scroll_bring_in_scroll_friction_set(val);
   elm_config_all_flush();
}

static void
zf_round(void *data       EINA_UNUSED,
         Evas_Object     *obj,
         void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
zf_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double zf = elm_config_scroll_zoom_friction_get();
   double val = elm_slider_value_get(obj);

   if (zf == val) return;
   elm_config_scroll_zoom_friction_set(val);
   elm_config_all_flush();
}

static void
ts_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   Eina_Bool val = elm_check_state_get(obj);
   Eina_Bool sb = elm_config_scroll_thumbscroll_enabled_get();

   if (val == sb) return;
   elm_config_scroll_thumbscroll_enabled_set(val);
   elm_config_all_flush();

   /*TODO: enable/disable subordinate sliders (make 'em support it 1st)*/
}

static void
tst_round(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tst_change(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double tst = elm_config_scroll_thumbscroll_threshold_get();
   double val = elm_slider_value_get(obj);

   if (tst == val) return;
   elm_config_scroll_thumbscroll_threshold_set(val);
   elm_config_all_flush();
}

static void
tsht_round(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsht_change(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double tst = elm_config_scroll_thumbscroll_hold_threshold_get();
   double val = elm_slider_value_get(obj);

   if (tst == val) return;
   elm_config_scroll_thumbscroll_hold_threshold_set(val);
   elm_config_all_flush();
}

static void
tsmt_round(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsmt_change(void *data       EINA_UNUSED,
            Evas_Object     *obj,
            void *event_info EINA_UNUSED)
{
   double tsmt = elm_config_scroll_thumbscroll_momentum_threshold_get();
   double val = elm_slider_value_get(obj);

   if (tsmt == val) return;
   elm_config_scroll_thumbscroll_momentum_threshold_set(val);
   elm_config_all_flush();
}

static void
tsfdt_round(void *data       EINA_UNUSED,
            Evas_Object     *obj,
            void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsfdt_change(void *data       EINA_UNUSED,
             Evas_Object     *obj,
             void *event_info EINA_UNUSED)
{
   double tsfdt = elm_config_scroll_thumbscroll_flick_distance_tolerance_get();
   double val = elm_slider_value_get(obj);

   if (tsfdt == val) return;
   elm_config_scroll_thumbscroll_flick_distance_tolerance_set(val);
   elm_config_all_flush();
}

static void
tsf_round(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsf_change(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double tsf = elm_config_scroll_thumbscroll_friction_get();
   double val = elm_slider_value_get(obj);

   if (tsf == val) return;
   elm_config_scroll_thumbscroll_friction_set(val);
   elm_config_all_flush();
}

static void
tsmf_round(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsmf_change(void *data       EINA_UNUSED,
            Evas_Object     *obj,
            void *event_info EINA_UNUSED)
{
   double tsmf = elm_config_scroll_thumbscroll_min_friction_get();
   double val = elm_slider_value_get(obj);

   if (tsmf == val) return;
   elm_config_scroll_thumbscroll_min_friction_set(val);
   elm_config_all_flush();
}

static void
tsfs_round(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsfs_change(void *data       EINA_UNUSED,
            Evas_Object     *obj,
            void *event_info EINA_UNUSED)
{
   double tsfs = elm_config_scroll_thumbscroll_friction_standard_get();
   double val = elm_slider_value_get(obj);

   if (tsfs == val) return;
   elm_config_scroll_thumbscroll_friction_standard_set(val);
   elm_config_all_flush();
}

static void
tsbf_round(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsbf_change(void *data       EINA_UNUSED,
            Evas_Object     *obj,
            void *event_info EINA_UNUSED)
{
   double tsbf = elm_config_scroll_thumbscroll_border_friction_get();
   double val = elm_slider_value_get(obj);

   if (tsbf == val) return;
   elm_config_scroll_thumbscroll_border_friction_set(val);
   elm_config_all_flush();
}

static void
tssf_round(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 20.0))) / 20.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tssf_change(void *data       EINA_UNUSED,
            Evas_Object     *obj,
            void *event_info EINA_UNUSED)
{
   double tssf = elm_config_scroll_thumbscroll_sensitivity_friction_get();
   double val = elm_slider_value_get(obj);

   if (tssf == val) return;
   elm_config_scroll_thumbscroll_sensitivity_friction_set(val);
   elm_config_all_flush();
}

static void
tsat_round(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsat_change(void *data       EINA_UNUSED,
            Evas_Object     *obj,
            void *event_info EINA_UNUSED)
{
   double tsat = elm_config_scroll_thumbscroll_acceleration_threshold_get();
   double val = elm_slider_value_get(obj);

   if (tsat == val) return;
   elm_config_scroll_thumbscroll_acceleration_threshold_set(val);
   elm_config_all_flush();
}

static void
tsatl_round(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsatl_change(void *data       EINA_UNUSED,
            Evas_Object     *obj,
            void *event_info EINA_UNUSED)
{
   double tsatl = elm_config_scroll_thumbscroll_acceleration_time_limit_get();
   double val = elm_slider_value_get(obj);

   if (tsatl == val) return;
   elm_config_scroll_thumbscroll_acceleration_time_limit_set(val);
   elm_config_all_flush();
}

static void
tsaw_round(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
tsaw_change(void *data       EINA_UNUSED,
            Evas_Object     *obj,
            void *event_info EINA_UNUSED)
{
   double tsaw = elm_config_scroll_thumbscroll_acceleration_weight_get();
   double val = elm_slider_value_get(obj);

   if (tsaw == val) return;
   elm_config_scroll_thumbscroll_acceleration_weight_set(val);
   elm_config_all_flush();
}

static void
cf_enable(void *data,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   Eina_Bool cf = elm_config_cache_flush_enabled_get();
   Eina_Bool val = elm_check_state_get(obj);

   if (cf == val) return;
   elm_object_disabled_set((Evas_Object *)data, !val);
   elm_config_cache_flush_enabled_set(val);
   elm_config_all_flush();
}

static void
cf_round(void *data       EINA_UNUSED,
         Evas_Object     *obj,
         void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 5.0))) / 5.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
cf_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double cf = elm_config_cache_flush_interval_get();
   double val = elm_slider_value_get(obj);

   if (cf == val) return;
   elm_config_cache_flush_interval_set(val);
   elm_config_all_flush();
}

static void
fc_round(void *data       EINA_UNUSED,
         Evas_Object     *obj,
         void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
fc_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double font_cache = elm_config_cache_font_cache_size_get();
   double val = elm_slider_value_get(obj);

   if (font_cache == val) return;
   elm_config_cache_font_cache_size_set(val * 1024);
   elm_config_all_flush();
}

static void
ic_round(void *data       EINA_UNUSED,
         Evas_Object     *obj,
         void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
ic_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double image_cache = elm_config_cache_image_cache_size_get();
   double val = elm_slider_value_get(obj);

   if (image_cache == val) return;
   elm_config_cache_image_cache_size_set(val * 1024);
   elm_config_all_flush();
}

static void
sc_round(void *data       EINA_UNUSED,
         Evas_Object     *obj,
         void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
sc_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double scale = elm_config_scale_get();
   double val = elm_slider_value_get(obj);

   if (scale == val) return;
   elm_config_scale_set(val);
   elm_config_all_flush();
}

static void
fs_round(void *data       EINA_UNUSED,
         Evas_Object     *obj,
         void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 5.0))) / 5.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
fs_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double fs = elm_config_finger_size_get();
   double val = elm_slider_value_get(obj);

   if (fs == val) return;
   elm_config_finger_size_set(val);
   elm_config_all_flush();
}

static void
efc_round(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 5.0))) / 5.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
efc_change(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double efc = elm_config_cache_edje_file_cache_size_get();
   double val = elm_slider_value_get(obj);

   if (efc == val) return;
   elm_config_cache_edje_file_cache_size_set(val);
   elm_config_all_flush();
}

static void
ecc_round(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 5.0))) / 5.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
ecc_change(void *data       EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   double ecc = elm_config_cache_edje_collection_cache_size_get();
   double val = elm_slider_value_get(obj);

   if (ecc == val) return;
   elm_config_cache_edje_collection_cache_size_set(val);
   elm_config_all_flush();
}

static void
ac_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   Eina_Bool val = elm_check_state_get(obj);
   Eina_Bool ac = elm_config_access_get();

   if (val == ac) return;
   elm_config_access_set(val);
   elm_config_all_flush();
}

static void
sel_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   Eina_Bool val = elm_check_state_get(obj);
   Eina_Bool sel = elm_config_selection_unfocused_clear_get();

   if (val == sel) return;
   elm_config_selection_unfocused_clear_set(val);
   elm_config_all_flush();
   elm_config_save();
}

static void
dbg_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   Eina_Bool val = elm_check_state_get(obj);
   Eina_Bool sel = elm_config_clouseau_enabled_get();

   if (val == sel) return;
   elm_config_clouseau_enabled_set(val);
   elm_config_all_flush();
   elm_config_save();
}

static void
atspi_change(void *data       EINA_UNUSED,
          Evas_Object     *obj,
          void *event_info EINA_UNUSED)
{
   Eina_Bool val = elm_check_state_get(obj);
   Eina_Bool sel = elm_config_atspi_mode_get();

   if (val == sel) return;
   elm_config_atspi_mode_set(val);
   elm_config_all_flush();
   elm_config_save();
}

static void
_status_basic(Evas_Object *win,
              Evas_Object *bx0)
{
   Evas_Object *lb, *fr;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(fr, "Information");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "Applying configuration change"
                       );
   elm_object_content_set(fr, lb);
   evas_object_show(lb);
}

static void
_status_config(Evas_Object *win,
               Evas_Object *bx0)
{
   Evas_Object *lb, *pd, *bx, *fr, *sl, *sp;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(fr, "Sizing");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);

   LABEL_FRAME_ADD("<hilight>Scale</>");

   sl = elm_slider_add(win);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_indicator_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.25, 5.0);
   elm_slider_value_set(sl, elm_config_scale_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", sc_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", sc_change, NULL);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   LABEL_FRAME_ADD("<hilight>Finger Size</><br/>");

   sl = elm_slider_add(win);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 5, 200);
   elm_slider_value_set(sl, elm_config_finger_size_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", fs_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", fs_change, NULL);

   // FIXME: add theme selector (basic mode and advanced for fallbacks)
   // FIXME: save config
   // FIXME: profile selector / creator etc.
   elm_object_content_set(fr, bx);
   evas_object_show(bx);
}

static void
_flip_to(Evas_Object *win,
         const char  *name)
{
   Evas_Object *wid, *naviframe;
   wid = evas_object_data_get(win, name);
   naviframe = evas_object_data_get(win, "naviframe");
   if (!naviframe) return;
   elm_naviframe_item_simple_promote(naviframe, wid);
}

static void
_cf_sizing(void            *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   _flip_to(data, "sizing");
}

static void
_cf_themes(void            *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   _flip_to(data, "themes");
}

static void
_cf_fonts(void            *data,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   _flip_to(data, "fonts");
}

static void
_cf_profiles(void            *data,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   _flip_to(data, "profiles");
}

static void
_cf_scrolling(void            *data,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   _flip_to(data, "scrolling");
}

static void
_cf_rendering(void            *data,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   _flip_to(data, "rendering");
}

static void
_cf_caches(void            *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   _flip_to(data, "caches");
}

static void
_cf_audio(void *data,
        Evas_Object *obj EINA_UNUSED,
        void *event_info EINA_UNUSED)
{
   _flip_to(data,"audio");
}

static void
_cf_focus(void *data,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   _flip_to(data, "focus");
}

static void
_cf_etc(void *data,
        Evas_Object *obj EINA_UNUSED,
        void *event_info EINA_UNUSED)
{
   _flip_to(data,"etc");
}

const char *
_elm_theme_current_get(const char *theme_search_order)
{
   const char *ret;
   const char *p;

   if (!theme_search_order)
     return NULL;

   for (p = theme_search_order;; p++)
     {
        if ((*p == ':') || (!*p))
          {
             if (p > theme_search_order)
               {
                  char *n = malloc(p - theme_search_order + 1);
                  if (!n)
                    return NULL;

                  strncpy(n, theme_search_order, p - theme_search_order);
                  n[p - theme_search_order] = 0;
                  ret = eina_stringshare_add(n);
                  free(n);
                  break;
               }
          }
     }

   return ret;
}

static void
_font_overlay_set_all(void            *data,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fclasses;
   Elm_Text_Class_Data *tc_data, *tc;
   Elm_Object_Item *list_it;
   Eina_List *l;

   win = data;

   fclasses = evas_object_data_get(win, "font_classes_list");
   list_it = elm_list_selected_item_get(fclasses);
   if (!list_it) return;
   tc_data = elm_object_item_data_get(list_it);

   EINA_LIST_FOREACH(fndata.text_classes, l, tc)
     {
        eina_stringshare_replace(&tc->font, tc_data->font);
        eina_stringshare_replace(&tc->style, tc_data->style);
        tc->size = tc_data->size;
     }

   elm_config_all_flush();
   elm_config_save();
}

static void
_font_overlay_reset(void            *data,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fclasses, *fnames, *fstyles, *fsizes;
   Elm_Text_Class_Data *tc_data;
   Elm_Object_Item *list_it;

   win = data;

   fclasses = evas_object_data_get(win, "font_classes_list");
   list_it = elm_list_selected_item_get(fclasses);
   if (!list_it) return;
   tc_data = elm_object_item_data_get(list_it);

   fnames = evas_object_data_get(win, "font_names_list");
   fstyles = evas_object_data_get(win, "font_styles_list");
   fsizes = evas_object_data_get(win, "font_sizes_list");

   elm_config_font_overlay_unset(tc_data->name);

   eina_stringshare_replace(&fndata.cur_font, NULL);
   eina_stringshare_replace(&tc_data->font, NULL);
   eina_stringshare_replace(&fndata.cur_style, NULL);
   eina_stringshare_replace(&tc_data->style, NULL);
   fndata.cur_size = 0.0;
   tc_data->size = 0.0;

   ELM_LIST_DISABLE(fnames);
   ELM_LIST_DISABLE(fstyles);
   ELM_LIST_DISABLE(fsizes);

   elm_config_all_flush();
   elm_config_save();
}

static void
_font_overlay_reset_all(void            *data,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fclasses, *fnames, *fstyles, *fsizes;
   Elm_Text_Class_Data *tc_data;
   Elm_Object_Item *list_it;
   Eina_List *l;

   win = data;
   eina_stringshare_replace(&fndata.cur_font, NULL);
   eina_stringshare_replace(&fndata.cur_style, NULL);
   fndata.cur_size = 0.0;

   EINA_LIST_FOREACH(fndata.text_classes, l, tc_data)
     {
        elm_config_font_overlay_unset(tc_data->name);

        eina_stringshare_replace(&tc_data->font, NULL);
        eina_stringshare_replace(&tc_data->style, NULL);
        tc_data->size = 0.0;
     }

   fclasses = evas_object_data_get(win, "font_classes_list");
   fnames = evas_object_data_get(win, "font_names_list");
   fstyles = evas_object_data_get(win, "font_styles_list");
   fsizes = evas_object_data_get(win, "font_sizes_list");

   list_it = elm_list_selected_item_get(fclasses);
   if (list_it) elm_list_item_selected_set(list_it, EINA_FALSE);

   ELM_LIST_DISABLE(fnames);
   ELM_LIST_DISABLE(fstyles);
   ELM_LIST_DISABLE(fsizes);

   elm_config_all_flush();
   elm_config_save();
}

static void
_font_overlay_change(void *data       EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   Elm_Text_Class_Data *tc_data;
   Eina_List *l;

   EINA_LIST_FOREACH(fndata.text_classes, l, tc_data)
     {
        if (tc_data->font)
          {
             char *name;

             name = elm_font_fontconfig_name_get(tc_data->font,
                                                 tc_data->style);
             elm_config_font_overlay_set(tc_data->name, name,
                                         tc_data->size ? tc_data->size : -100);
             elm_font_fontconfig_name_free(name);
          }
        else
          elm_config_font_overlay_unset(tc_data->name);
     }

   elm_config_font_overlay_apply();
   elm_config_all_flush();
   elm_config_save();

   /* TODO: apply hinting */
}

static void
_config_display_update(Evas_Object *win)
{
   int flush_interval, font_c, image_c, edje_file_c, edje_col_c, ts_threshould,
       ts_hold_threshold;
   double scale, s_bounce_friction, ts_momentum_threshold,
          ts_flick_distance_tolerance, ts_friction,
          ts_min_friction, ts_friction_standard, ts_border_friction,
          ts_sensitivity_friction, ts_acceleration_threshold,
          ts_acceleration_time_limit, ts_acceleration_weight, page_friction,
          bring_in_friction, zoom_friction;
   const char *curr_theme, *curr_engine;
   const Eina_List *l_items, *l;
   Eina_Bool s_bounce, ts;
   Elm_Object_Item *list_it;
   Elm_Theme *th;
   int fs;

   scale = elm_config_scale_get();
   fs = elm_config_finger_size_get();
   flush_interval = elm_config_cache_flush_interval_get();
   font_c = elm_config_cache_font_cache_size_get();
   image_c = elm_config_cache_image_cache_size_get();
   edje_file_c = elm_config_cache_edje_file_cache_size_get();
   edje_col_c = elm_config_cache_edje_collection_cache_size_get();

   s_bounce = elm_config_scroll_bounce_enabled_get();
   s_bounce_friction = elm_config_scroll_bounce_friction_get();
   ts = elm_config_scroll_thumbscroll_enabled_get();
   ts_threshould = elm_config_scroll_thumbscroll_threshold_get();
   ts_hold_threshold = elm_config_scroll_thumbscroll_hold_threshold_get();
   ts_momentum_threshold = elm_config_scroll_thumbscroll_momentum_threshold_get();
   ts_flick_distance_tolerance = elm_config_scroll_thumbscroll_flick_distance_tolerance_get();
   ts_friction = elm_config_scroll_thumbscroll_friction_get();
   ts_min_friction = elm_config_scroll_thumbscroll_min_friction_get();
   ts_friction_standard = elm_config_scroll_thumbscroll_friction_standard_get();
   ts_border_friction = elm_config_scroll_thumbscroll_border_friction_get();
   ts_sensitivity_friction = elm_config_scroll_thumbscroll_sensitivity_friction_get();
   ts_acceleration_threshold = elm_config_scroll_thumbscroll_acceleration_threshold_get();
   ts_acceleration_time_limit = elm_config_scroll_thumbscroll_acceleration_time_limit_get();
   ts_acceleration_weight = elm_config_scroll_thumbscroll_acceleration_weight_get();
   page_friction = elm_config_scroll_page_scroll_friction_get();
   bring_in_friction = elm_config_scroll_bring_in_scroll_friction_get();
   zoom_friction = elm_config_scroll_zoom_friction_get();

   /* gotta update root windows' atoms */
   elm_slider_value_set(evas_object_data_get(win, "scale_slider"), scale);
   elm_slider_value_set(evas_object_data_get(win, "fs_slider"), fs);

   elm_slider_value_set(evas_object_data_get(win,
                                             "cache_flush_interval_slider"),
                        flush_interval);
   elm_slider_value_set(evas_object_data_get(win, "font_cache_slider"),
                        font_c / 1024.0);
   elm_slider_value_set(evas_object_data_get(win, "image_cache_slider"),
                        image_c / 1024.0);
   elm_slider_value_set(evas_object_data_get(win, "edje_file_cache_slider"),
                        edje_file_c);
   elm_slider_value_set(evas_object_data_get(win,
                                             "edje_collection_cache_slider"),
                        edje_col_c);

   elm_check_state_set(evas_object_data_get(win, "scroll_bounce_check"),
                       s_bounce);
   elm_slider_value_set(evas_object_data_get(win, "bounce_friction_slider"),
                        s_bounce_friction);
   elm_check_state_set(evas_object_data_get(win, "thumbscroll_check"), ts);
   elm_slider_value_set(evas_object_data_get(win,
                                             "ts_threshold_slider"),
                        ts_threshould);
   elm_slider_value_set(evas_object_data_get(win,
                                             "ts_hold_threshold_slider"),
                        ts_hold_threshold);
   elm_slider_value_set(evas_object_data_get(win,
                                             "ts_momentum_threshold_slider"),
                        ts_momentum_threshold);
   elm_slider_value_set(evas_object_data_get(win,
                                             "ts_flick_distance_tolerance_slider"),
                        ts_flick_distance_tolerance);
   elm_slider_value_set(evas_object_data_get(win,
                                             "ts_friction_slider"),
                        ts_friction);
   elm_slider_value_set(evas_object_data_get(win,
                                             "ts_min_friction_slider"),
                        ts_min_friction);
   elm_slider_value_set(evas_object_data_get(win,
                                             "ts_friction_standard_slider"),
                        ts_friction_standard);
   elm_slider_value_set(evas_object_data_get(win, "ts_border_friction_slider"),
                        ts_border_friction);
   elm_slider_value_set(evas_object_data_get(win, "ts_sensitivity_friction_slider"),
                        ts_sensitivity_friction);
   elm_slider_value_set(evas_object_data_get(win, "ts_acceleration_threshold_slider"),
                        ts_acceleration_threshold);
   elm_slider_value_set(evas_object_data_get(win, "ts_acceleration_time_limit_slider"),
                        ts_acceleration_time_limit);
   elm_slider_value_set(evas_object_data_get(win, "ts_acceleration_weight_slider"),
                        ts_acceleration_weight);
   elm_slider_value_set(evas_object_data_get(win,
                                             "page_scroll_friction_slider"),
                        page_friction);
   elm_slider_value_set(evas_object_data_get(win,
                                             "bring_in_scroll_friction_slider"),
                        bring_in_friction);
   elm_slider_value_set(evas_object_data_get(win,
                                             "zoom_scroll_friction_slider"),
                        zoom_friction);

   curr_theme = _elm_theme_current_get(elm_theme_get(NULL));

   th = elm_theme_new();
   elm_theme_set(th, curr_theme);
   elm_object_theme_set(evas_object_data_get(win, "theme_preview"), th);
   elm_theme_free(th);
   eina_stringshare_del(curr_theme);

   curr_engine = elm_config_engine_get();
   l_items = elm_list_items_get(evas_object_data_get(win, "engines_list"));
   EINA_LIST_FOREACH(l_items, l, list_it)
     {
        if (!strcmp(elm_object_item_data_get(list_it), curr_engine))
          {
             elm_list_item_selected_set(list_it, EINA_TRUE);
             break;
          }
     }
}

static Eina_Bool
_config_all_changed(void *data,
                    int ev_type EINA_UNUSED,
                    void *ev EINA_UNUSED)
{
   Evas_Object *win = data;
   _config_display_update(win);
   return ECORE_CALLBACK_PASS_ON;
}

static void
_engine_use(void            *data,
            Evas_Object *obj EINA_UNUSED,
            void *event_info EINA_UNUSED)
{
   Evas_Object *li;
   const char *selection;

   li = data;
   selection = elm_object_item_data_get(elm_list_selected_item_get(li));

   if (!strcmp(elm_config_engine_get(), selection))
     return;

   elm_config_engine_set(selection);
   elm_config_all_flush();
   elm_config_save(); /* make sure new engine has its data dir */
}

static void
_profile_use(void            *data,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   Evas_Object *li;
   const char *selection;
   const char *profile;

   li = data;
   selection = elm_object_item_data_get(elm_list_selected_item_get(li));
   profile = elm_config_profile_get();

   if (!profile)
     {
        fprintf(stderr, "No profile currently set!\n"); /* FIXME: log domain */
        return;
     }

   if (!strcmp(profile, selection))
     return;

   elm_config_profile_set(selection); /* just here to update info for getters below */

   _config_display_update(elm_object_top_widget_get(li));
   elm_config_all_flush();
   elm_config_save(); /* make sure new profile has its data dir */
}

static void
_btn_todo(void *data       EINA_UNUSED,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   printf("To be done!\n");
}

static void
_profile_reset(void            *data,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   const char *selection;
   const char *pdir;
   Evas_Object *li;

   li = data;
   selection = elm_object_item_data_get(elm_list_selected_item_get(li));

   elm_config_all_flush();
   elm_config_save(); /* dump config into old profile's data dir */

   pdir = elm_config_profile_dir_get(selection, EINA_TRUE);
   if (!pdir)
     return;

   ecore_file_recursive_rm(pdir);
   elm_config_profile_dir_free(pdir);

   elm_config_reload();

   elm_config_all_flush();
   _config_display_update(elm_object_top_widget_get(li));
   elm_config_save(); /* make sure new profile has its data dir */
}

static void
_theme_use(void *data       EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   const char *defth;
   Elm_Theme *th;
   Evas_Object *win = elm_object_top_widget_get(obj);
   Evas_Object *sample = evas_object_data_get(win, "theme_preview");

   th = elm_object_theme_get(sample);
   defth = elm_theme_get(th);
   elm_theme_set(NULL, defth);
   elm_config_all_flush();
   elm_config_save();
}

static void
_theme_sel(void            *data EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   Evas_Object *win = elm_object_top_widget_get(obj);
   Evas_Object *sample = evas_object_data_get(win, "theme_preview");
   Elm_Theme *th;
   Elm_Object_Item *list_it;
   Eina_Strbuf *newth;
   Eina_List *l;

   if (!sample) return;
   tsel = elm_list_selected_items_get(obj);
   newth = eina_strbuf_new();
   EINA_SAFETY_ON_NULL_RETURN(newth);
   th = elm_theme_new();
   if (!th)
     {
        eina_strbuf_free(newth);
        return;
     }
   EINA_LIST_REVERSE_FOREACH((Eina_List*)tsel, l, list_it)
     {
        Theme *t = elm_object_item_data_get(list_it);
        eina_strbuf_append_printf(newth, "%s:", t->name);
        if ((!l->prev) && strcmp(t->name, "default"))
          /* ensure default theme is always there for fallback */
          eina_strbuf_append(newth, "default");
     }
   elm_theme_set(th, eina_strbuf_string_get(newth));
   eina_strbuf_free(newth);
   elm_object_theme_set(sample, th);
   elm_theme_free(th);
}

/*static void
   _theme_browse(void *data, Evas_Object *obj, void *event_info)
   {
   printf("not implemented\n");
   }*/

static void
_status_config_sizing(Evas_Object *win,
                      Evas_Object *naviframe)
{
   Evas_Object *lb, *pd, *bx, *sl, *sp;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);

   LABEL_FRAME_ADD("<hilight>Scale</>");

   sl = elm_slider_add(win);
   evas_object_data_set(win, "scale_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_indicator_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.25, 5.0);
   elm_slider_value_set(sl, elm_config_scale_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", sc_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", sc_change, NULL);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   LABEL_FRAME_ADD("<hilight>Finger Size</><br/>");

   sl = elm_slider_add(win);
   evas_object_data_set(win, "fs_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 5, 200);
   elm_slider_value_set(sl, elm_config_finger_size_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", fs_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", fs_change, NULL);

   evas_object_data_set(win, "sizing", bx);

   elm_naviframe_item_simple_push(naviframe, bx);
}

#define MUTE_CB(_cb, _chan) \
static void \
_cb(void *data       EINA_UNUSED, \
    Evas_Object     *obj, \
    void *event_info EINA_UNUSED) \
{ \
   Eina_Bool val = elm_check_state_get(obj); \
   Eina_Bool v = elm_config_audio_mute_get(_chan); \
   if (val == v) return; \
   elm_config_audio_mute_set(_chan, val); \
   elm_config_all_flush(); \
}

MUTE_CB(mute_effect_change, EDJE_CHANNEL_EFFECT)
MUTE_CB(mute_background_change, EDJE_CHANNEL_BACKGROUND)
MUTE_CB(mute_music_change, EDJE_CHANNEL_MUSIC)
MUTE_CB(mute_foreground_change, EDJE_CHANNEL_FOREGROUND)
MUTE_CB(mute_interface_change, EDJE_CHANNEL_INTERFACE)
MUTE_CB(mute_input_change, EDJE_CHANNEL_INPUT)
MUTE_CB(mute_alert_change, EDJE_CHANNEL_ALERT)
MUTE_CB(mute_all_change, EDJE_CHANNEL_ALL)

static void
_status_config_audio(Evas_Object *win,
                     Evas_Object *naviframe)
{
   Evas_Object *bx, *ck;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);

#define MUTE_CHECK(_label, _chan, _cb)  \
   CHECK_ADD(_label, _label, _cb, NULL) \
   elm_check_state_set(ck, elm_config_audio_mute_get(_chan));

   MUTE_CHECK("Mute Effects", EDJE_CHANNEL_EFFECT, mute_effect_change);
   MUTE_CHECK("Mute Background", EDJE_CHANNEL_BACKGROUND, mute_background_change);
   MUTE_CHECK("Mute Music", EDJE_CHANNEL_MUSIC, mute_music_change);
   MUTE_CHECK("Mute Foreground", EDJE_CHANNEL_FOREGROUND, mute_foreground_change);
   MUTE_CHECK("Mute Interface", EDJE_CHANNEL_INTERFACE, mute_interface_change);
   MUTE_CHECK("Mute Input", EDJE_CHANNEL_INPUT, mute_input_change);
   MUTE_CHECK("Mute Alert", EDJE_CHANNEL_ALERT, mute_alert_change);
   MUTE_CHECK("Mute Everything", EDJE_CHANNEL_ALL, mute_all_change);

   evas_object_data_set(win, "audio", bx);

   elm_naviframe_item_simple_push(naviframe, bx);
}

static void
_config_focus_highlight_enabled_cb(void *data EINA_UNUSED, Evas_Object *obj,
                                   void *event_info EINA_UNUSED)
{
   Eina_Bool cf = elm_config_focus_highlight_enabled_get();
   Eina_Bool val = elm_check_state_get(obj);

   printf("%d %d\n", cf, val);
   if (cf == val) return;
   elm_config_focus_highlight_enabled_set(val);
   elm_config_all_flush();
}

static void
_config_focus_highlight_anim_cb(void *data EINA_UNUSED, Evas_Object *obj,
                                void *event_info EINA_UNUSED)
{
   Eina_Bool cf = elm_config_focus_highlight_animate_get();
   Eina_Bool val = elm_check_state_get(obj);

   if (cf == val) return;
   elm_config_focus_highlight_animate_set(val);
   elm_config_all_flush();
}

static void
_config_focus_highlight_clip_cb(void *data EINA_UNUSED, Evas_Object *obj,
                                void *event_info EINA_UNUSED)
{
   Eina_Bool cf = elm_config_focus_highlight_clip_disabled_get();
   Eina_Bool val = elm_check_state_get(obj);

   if (cf == val) return;
   elm_config_focus_highlight_clip_disabled_set(val);
   elm_config_all_flush();
}

static void
_config_focus_auto_scroll_bring_in_cb(void *data EINA_UNUSED, Evas_Object *obj,
                                      void *event_info EINA_UNUSED)
{
   Eina_Bool cf = elm_config_focus_auto_scroll_bring_in_enabled_get();
   Eina_Bool val = elm_check_state_get(obj);

   if (cf == val) return;
   elm_config_focus_auto_scroll_bring_in_enabled_set(val);
   elm_config_all_flush();
}

static void
_config_focus_item_select_on_focus_cb(void *data EINA_UNUSED, Evas_Object *obj,
                                      void *event_info EINA_UNUSED)
{
   Eina_Bool cf = elm_config_item_select_on_focus_disabled_get();
   Eina_Bool val = elm_check_state_get(obj);

   if (cf == val) return;
   elm_config_item_select_on_focus_disabled_set(val);
   elm_config_all_flush();
}

static void
_status_config_focus(Evas_Object *win,
                     Evas_Object *naviframe)
{
   Evas_Object *bx, *ck;

   bx = elm_box_add(win);

   CHECK_ADD("Enable Focus Highlight (only new window)",
             "Set whether enable/disable focus highlight.<br/>"
             "This feature is disabled by default.",
             _config_focus_highlight_enabled_cb, NULL);
   elm_check_state_set(ck, elm_config_focus_highlight_enabled_get());

   CHECK_ADD("Enable Focus Highlight Animation (only new window)",
             "Set whether enable/disable focus highlight animation.<br/>"
             "This feature is disabled by default",
             _config_focus_highlight_anim_cb, NULL);
   elm_check_state_set(ck, elm_config_focus_highlight_animate_get());

   CHECK_ADD("Disable Focus Highlight clip",
             "Set whether enable/disable focus highlight clip feature.<br/>"
             "If the focus highlight clip is disabled,<br/>"
             "focus highlight object would not be clipped"
             "by the target object's parent",
             _config_focus_highlight_clip_cb, NULL);
   elm_check_state_set(ck, elm_config_focus_highlight_clip_disabled_get());

   CHECK_ADD("Enable Auto Scroll Bring-in",
             "Set whether enable/disable auto scroll bring-in feature<br/>"
             "This is disabled by default so auto scrolling works by show not"
             "by bring-in.",
             _config_focus_auto_scroll_bring_in_cb, NULL);
   elm_check_state_set(ck, elm_config_focus_auto_scroll_bring_in_enabled_get());

   CHECK_ADD("Disable Item Select on Focus",
             "Set whether item would be selected on item focus.<br/>"
             "This is enabled by default.",
             _config_focus_item_select_on_focus_cb, NULL);
   elm_check_state_set(ck, elm_config_item_select_on_focus_disabled_get());

   evas_object_data_set(win, "focus", bx);

   elm_naviframe_item_simple_push(naviframe, bx);
}

static void
_status_config_etc(Evas_Object *win,
                   Evas_Object *naviframe)
{
   Evas_Object *bx, *ck;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);

   // access
   CHECK_ADD("Enable Access Mode", "Set access mode", ac_change, NULL);
   elm_check_state_set(ck, elm_config_access_get());

   // selection
   CHECK_ADD("Enable clear selection when unfocus", "Set selection mode",
             sel_change, NULL);
   elm_check_state_set(ck, elm_config_selection_unfocused_clear_get());

   // clouseau
   CHECK_ADD("Enable clouseau", "Set clouseau mode", dbg_change, NULL);
   elm_check_state_set(ck, elm_config_clouseau_enabled_get());

   // atspi
   CHECK_ADD("Enable ATSPI support", "Set atspi mode", atspi_change, NULL);
   elm_check_state_set(ck, elm_config_atspi_mode_get());

   evas_object_data_set(win, "etc", bx);

   elm_naviframe_item_simple_push(naviframe, bx);
}

static Evas_Object *
_sample_theme_new(Evas_Object *win)
{
   Evas_Object *base, *bg, *bt, *ck, *rd, *rdg, *sl, *fr, *li, *rc, *sp;

   base = elm_table_add(win);
   evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(base, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(base, bg, 0, 0, 2, 5);
   evas_object_show(bg);

   bt = elm_button_add(win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_object_text_set(bt, "Button");
   elm_table_pack(base, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   ck = elm_check_add(win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, 0.0);
   elm_object_text_set(ck, "Check");
   elm_table_pack(base, ck, 0, 1, 1, 1);
   evas_object_show(ck);

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   elm_radio_state_value_set(rd, 0);
   elm_object_text_set(rd, "Radio 1");
   elm_table_pack(base, rd, 1, 0, 1, 1);
   evas_object_show(rd);
   rdg = rd;

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   elm_radio_state_value_set(rd, 1);
   elm_object_text_set(rd, "Radio 2");
   elm_radio_group_add(rd, rdg);
   elm_table_pack(base, rd, 1, 1, 1, 1);
   evas_object_show(rd);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_table_pack(base, sp, 0, 2, 2, 1);
   evas_object_show(sp);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Slider");
   elm_slider_span_size_set(sl, 120);
   elm_slider_min_max_set(sl, 1, 10);
   elm_slider_value_set(sl, 4);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(base, sl, 0, 3, 2, 1);
   evas_object_show(sl);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Frame");
   elm_table_pack(base, fr, 0, 4, 2, 1);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, li);
   evas_object_show(li);

   elm_list_item_append(li, "List Item 1", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Second Item", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Third Item", NULL, NULL, NULL, NULL);
   elm_list_go(li);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(rc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(rc, 160, 120);
   elm_table_pack(base, rc, 0, 4, 2, 1);

   return base;
}

static void
_status_config_themes(Evas_Object *win,
                      Evas_Object *naviframe)
{
   Evas_Object *tb, *rc, *sc, *sp, *li, *pd, *fr, *bt, *sample;
   Eina_List *list, *l;
   char *th, *s, *ext;
   Elm_Theme *d;
   Elm_Object_Item *list_it, *def_it = NULL;
   const char *theme_name, *sep[20];
   unsigned int x;

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(rc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(rc, 0, 130);
   elm_table_pack(tb, rc, 0, 0, 1, 1);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(rc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(rc, 0, 200);
   elm_table_pack(tb, rc, 0, 1, 1, 1);

   /////////////////////////////////////////////

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, pd, 0, 0, 1, 1);
   evas_object_show(pd);

   li = elm_list_add(win);
   elm_list_multi_select_set(li, EINA_TRUE);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(pd, li);
   evas_object_show(li);

   list = elm_theme_name_available_list_new();
   d = elm_theme_default_get();
   theme_name = elm_theme_get(d);
   for (x = 1, sep[0] = theme_name; x < sizeof(sep) / sizeof(sep[0]); x++)
     {
        sep[x] = strchr(sep[x - 1] + 1, ':');
        if (!sep[x]) break;
     }
   EINA_LIST_FOREACH(list, l, th)
     {
        Theme *t;
        int y;

        t = calloc(1, sizeof(Theme));
        t->name = eina_stringshare_add(th);
        s = elm_theme_list_item_path_get(th, &(t->in_search_path));
        if (s)
          {
             t->path = eina_stringshare_add(s);
             free(s);
          }
        if (t->in_search_path)
          {
             s = strdup(th);
             if (s)
               {
                  s[0] = toupper(s[0]);
                  t->label = eina_stringshare_add(s);
                  free(s);
               }
             else
               t->label = eina_stringshare_add(s);
          }
        else
          {
             s = strdup(ecore_file_file_get(th));
             if (s)
               {
                  s[0] = toupper(s[0]);
                  ext = strrchr(s, '.');
                  if (ext) *ext = 0;
                  t->label = eina_stringshare_add(s);
                  free(s);
               }
             else
               t->label = eina_stringshare_add(s);
          }
        themes = eina_list_append(themes, t);
        list_it = elm_list_item_append(li, t->label, NULL, NULL, NULL, t);
        if (!strcmp(t->name, "default")) def_it = list_it;
        for (y = x - 1 /* ignore default e theme */; y > 0; y--)
          {
             const char *start = (sep[y - 1][0] == ':') ? sep[y - 1] + 1 : sep[y - 1];
             unsigned int len = (unsigned int)(sep[y] - start);
             if (strncmp(start , t->name, len) || (strlen(t->name) != len)) continue;

             if (!elm_list_item_selected_get(list_it))
               elm_list_item_selected_set(list_it, EINA_TRUE);
             break;
          }
     }
   if (!elm_list_selected_items_get(li))
     elm_list_item_selected_set(def_it, EINA_TRUE);
   evas_object_smart_callback_add(li, "selected", _theme_sel, NULL);
   evas_object_smart_callback_add(li, "unselected", _theme_sel, NULL);
   elm_theme_name_available_list_free(list);

   elm_list_go(li);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pd, 0.9, 0.9);
   elm_table_pack(tb, pd, 0, 0, 1, 1);
   evas_object_show(pd);

   /* FIXME: not implemented yet
      bt = elm_button_add(win);
      evas_object_smart_callback_add(bt, "clicked", _theme_browse, win);
      elm_object_text_set(bt, "Browse...");
      evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(bt, 0.9, 0.9);
      elm_object_content_set(pd, bt);
      evas_object_show(bt);
    */
   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, pd, 0, 1, 1, 1);
   evas_object_show(pd);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Preview");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(pd, fr);
   evas_object_show(fr);

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, sc);
   evas_object_show(sc);

   sample = _sample_theme_new(win);
   elm_object_content_set(sc, sample);
   evas_object_show(sample);
   evas_object_data_set(win, "theme_preview", sample);

   /////////////////////////////////////////////
   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_table_pack(tb, sp, 0, 2, 1, 1);
   evas_object_show(sp);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(pd, 0.5, 0.5);
   elm_table_pack(tb, pd, 0, 3, 1, 1);
   evas_object_show(pd);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _theme_use, win);
   elm_object_text_set(bt, "Use Theme");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_object_content_set(pd, bt);
   evas_object_show(bt);

   evas_object_data_set(win, "themes", tb);
   elm_naviframe_item_simple_push(naviframe, tb);
}

static void
_font_preview_update(Evas_Object *win)
{
   Evas_Font_Size sz;
   char *name;

   if (!fndata.cur_font)
     return;

   name = elm_font_fontconfig_name_get(fndata.cur_font, fndata.cur_style);
   sz = fndata.cur_size;

   if (sz < 0)
     sz = (-sz * 10) / 100;
   if (sz == 0)
     sz = 10;

   edje_object_text_class_set(evas_object_data_get(win, "font_preview"),
                              "font_preview", name, sz);

   elm_font_fontconfig_name_free(name);
}

static void
_font_classes_list_sel(void *data   EINA_UNUSED,
                       Evas_Object *obj,
                       void        *event_info)
{
   const Eina_List *f_names_items, *l;
   Elm_Text_Class_Data *tc_data;
   Evas_Object *f_names_list;
   Elm_Object_Item *list_it;

   tc_data = elm_object_item_data_get(event_info);

   f_names_list = evas_object_data_get(elm_object_top_widget_get(obj),
                                       "font_names_list");
   ELM_LIST_ENABLE(f_names_list);

   f_names_items = elm_list_items_get(f_names_list);

   EINA_LIST_FOREACH(f_names_items, l, list_it)
     {
        const char *s;

        s = elm_object_item_text_get(list_it);

        if (tc_data->font && !strcmp(s, tc_data->font))
          {
             elm_list_item_selected_set(list_it, EINA_TRUE);
             elm_list_item_show(list_it);
             break;
          }
     }
}

static void
_font_names_list_sel(void *data   EINA_UNUSED,
                     Evas_Object *obj,
                     void        *event_info)
{
   Evas_Object *style_list, *cls_list, *win;
   const char *style, *sel_font;
   Elm_Text_Class_Data *tc_data;
   Elm_Object_Item *list_it, *fc_list_it;
   Elm_Font_Properties *efp;
   const Eina_List *l;

   list_it = event_info;
   sel_font = elm_object_item_text_get(list_it);

   win = elm_object_top_widget_get(obj);
   style_list = evas_object_data_get(win, "font_styles_list");
   cls_list = evas_object_data_get(win, "font_classes_list");

   fc_list_it = elm_list_selected_item_get(cls_list);
   if (!fc_list_it) return;  /* should not happen, fonts list disabled in
                         * this case */

   eina_stringshare_replace(&fndata.cur_font, sel_font);

   tc_data = elm_object_item_data_get(fc_list_it);
   if (tc_data->font) eina_stringshare_del(tc_data->font);
   if (fndata.cur_font) tc_data->font = eina_stringshare_ref(fndata.cur_font);

   /* load styles list */
   efp = eina_hash_find(fndata.font_hash, sel_font);

   ELM_LIST_ENABLE(style_list);
   elm_list_clear(style_list);

   evas_event_freeze(evas_object_evas_get(style_list));
   edje_freeze();

   list_it = NULL;

   EINA_LIST_FOREACH(efp->styles, l, style)
     {
        Elm_Object_Item *i;

        i = elm_list_item_append(style_list, style, NULL, NULL,
                                 _font_styles_list_sel, NULL);

        if (tc_data->style && (!strcmp(style, tc_data->style)))
          list_it = i;
     }

   elm_list_go(style_list);

   edje_thaw();
   evas_event_thaw(evas_object_evas_get(style_list));

   if (list_it)
     elm_list_item_selected_set(list_it, EINA_TRUE);

   _font_preview_update(win);
}

static void
_font_styles_list_sel(void *data   EINA_UNUSED,
                      Evas_Object *obj,
                      void        *event_info)
{
   Evas_Object *fc_list, *fs_list, *win;
   Elm_Text_Class_Data *tc_data;
   const Eina_List *l;
   Elm_Object_Item *list_it;

   win = elm_object_top_widget_get(obj);
   fc_list = evas_object_data_get(win, "font_classes_list");
   fs_list = evas_object_data_get(win, "font_sizes_list");

   list_it = elm_list_selected_item_get(fc_list);
   if (!list_it) return;  /* should not happen */

   eina_stringshare_replace(&fndata.cur_style,
                            elm_object_item_text_get(event_info));
   ELM_LIST_ENABLE(fs_list);

   tc_data = elm_object_item_data_get(list_it);
   eina_stringshare_del(tc_data->style);
   tc_data->style = eina_stringshare_ref(fndata.cur_style);

   evas_event_freeze(evas_object_evas_get(fs_list));
   edje_freeze();

   EINA_LIST_FOREACH(elm_list_items_get(fs_list), l, list_it)
     {
        Elm_Font_Size_Data *sdata;

        sdata = elm_object_item_data_get(list_it);
        elm_list_item_selected_set(l->data, EINA_FALSE);

        if (tc_data->size == sdata->size)
          {
             elm_list_item_selected_set(list_it, EINA_TRUE);
             elm_list_item_show(list_it);
             break;
          }
     }

   edje_thaw();
   evas_event_thaw(evas_object_evas_get(fs_list));

   _font_preview_update(win);
}

static void
_font_sizes_list_sel(void *data       EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   Elm_Text_Class_Data *tc_data;
   Evas_Object *fc_list, *win;
   Elm_Font_Size_Data *sd;
   Elm_Object_Item *list_it;

   win = elm_object_top_widget_get(obj);
   fc_list = evas_object_data_get(win, "font_classes_list");

   list_it = elm_list_selected_item_get(fc_list);
   if (!list_it) return;  /* should not happen */

   sd = elm_object_item_data_get(event_info);
   fndata.cur_size = sd->size;

   tc_data = elm_object_item_data_get(list_it);
   tc_data->size = fndata.cur_size;

   _font_preview_update(win);
}

static void
_fstyle_list_unselect_cb(void *data       EINA_UNUSED,
                         Evas_Object     *obj,
                         void *event_info EINA_UNUSED)
{
   Evas_Object *sizes_list, *win;

   if (elm_list_selected_item_get(obj)) return;

   win = elm_object_top_widget_get(obj);

   sizes_list = evas_object_data_get(win, "font_sizes_list");
   ELM_LIST_DISABLE(sizes_list);
}

static void
_fc_list_unselect_cb(void *data       EINA_UNUSED,
                     Evas_Object     *obj,
                     void *event_info EINA_UNUSED)
{
   Evas_Object *font_names_list, *styles_list, *sizes_list, *win;

   if (elm_list_selected_item_get(obj)) return;

   win = elm_object_top_widget_get(obj);

   font_names_list = evas_object_data_get(win, "font_names_list");
   ELM_LIST_DISABLE(font_names_list);

   styles_list = evas_object_data_get(win, "font_styles_list");
   ELM_LIST_DISABLE(styles_list);

   sizes_list = evas_object_data_get(win, "font_sizes_list");
   ELM_LIST_DISABLE(sizes_list);
}

static void
_font_classes_list_load(Evas_Object *li)
{
   Elm_Text_Class_Data *tc_data;
   Eina_List *l;
   Evas *evas;

   evas = evas_object_evas_get(li);
   evas_event_freeze(evas);
   edje_freeze();

   EINA_LIST_FOREACH(fndata.text_classes, l, tc_data)
     elm_list_item_append(li, tc_data->desc, NULL, NULL,
                          _font_classes_list_sel, tc_data);

   evas_object_smart_callback_add(li, "unselected", _fc_list_unselect_cb,
                                  NULL);

   elm_list_go(li);
   edje_thaw();
   evas_event_thaw(evas);
}

static void
_fonts_data_fill(Evas *evas)
{
   const Eina_List *fo_list, *l;
   Eina_List *text_classes;
   Elm_Text_Class_Data *tc_data;
   Elm_Font_Size_Data *sd;
   Elm_Font_Overlay *efo;
   Eina_List *evas_fonts;
   Elm_Text_Class *etc;
   int i;

   evas_fonts = evas_font_available_list(evas);
   fndata.font_hash = elm_font_available_hash_add(evas_fonts);
   evas_font_available_list_free(evas, evas_fonts);

   text_classes = elm_config_text_classes_list_get();
   fo_list = elm_config_font_overlay_list_get();

   EINA_LIST_FOREACH(text_classes, l, etc)
     {
        const Eina_List *ll;

        tc_data = calloc(1, sizeof(*tc_data));
        if (!tc_data) continue;

        tc_data->name = eina_stringshare_add(etc->name);
        tc_data->desc = eina_stringshare_add(etc->desc);

        EINA_LIST_FOREACH(fo_list, ll, efo)
          {
             if (strcmp(tc_data->name, efo->text_class))
               continue;

             if (efo->font)
               {
                  Elm_Font_Properties *efp;

                  efp = elm_font_properties_get(efo->font);
                  if (efp)
                    {
                       tc_data->font = eina_stringshare_add(efp->name);
                       /* we're sure we recorded with only 1 style selected */
                       tc_data->style = eina_stringshare_add(efp->styles->data);
                       elm_font_properties_free(efp);
                    }
               }
             tc_data->size = efo->size;
          }

        fndata.text_classes = eina_list_append(fndata.text_classes, tc_data);
     }

   elm_config_text_classes_list_free(text_classes);

   /* FIXME: hinting later */
   /* fndata.hinting = e_config->font_hinting; */

   sd = calloc(1, sizeof(Elm_Font_Size_Data));
   sd->size_str = eina_stringshare_add("Tiny");
   sd->size = -50;
   fndata.font_scale_list = eina_list_append(fndata.font_scale_list, sd);

   sd = calloc(1, sizeof(Elm_Font_Size_Data));
   sd->size_str = eina_stringshare_add("Small");
   sd->size = -80;
   fndata.font_scale_list = eina_list_append(fndata.font_scale_list, sd);

   sd = calloc(1, sizeof(Elm_Font_Size_Data));
   sd->size_str = eina_stringshare_add("Normal");
   sd->size = -100;
   fndata.font_scale_list = eina_list_append(fndata.font_scale_list, sd);

   sd = calloc(1, sizeof(Elm_Font_Size_Data));
   sd->size_str = eina_stringshare_add("Big");
   sd->size = -150;
   fndata.font_scale_list = eina_list_append(fndata.font_scale_list, sd);

   sd = calloc(1, sizeof(Elm_Font_Size_Data));
   sd->size_str = eina_stringshare_add("Really Big");
   sd->size = -190;
   fndata.font_scale_list = eina_list_append(fndata.font_scale_list, sd);

   sd = calloc(1, sizeof(Elm_Font_Size_Data));
   sd->size_str = eina_stringshare_add("Huge");
   sd->size = -250;
   fndata.font_scale_list = eina_list_append(fndata.font_scale_list, sd);

   for (i = 5; i < 51; i++)
     {
        char str[16];

        str[0] = 0;
        snprintf(str, sizeof(str), "%d pixels", i);

        sd = calloc(1, sizeof(Elm_Font_Size_Data));
        sd->size_str = eina_stringshare_add(str);
        sd->size = i;
        fndata.font_px_list = eina_list_append(fndata.font_px_list, sd);
     }
}

static int
_font_sort_cb(const void *data1,
              const void *data2)
{
   if (!data1) return 1;
   if (!data2) return -1;
   return strcmp(data1, data2);
}

static void
_font_sizes_list_load(Evas_Object *size_list)
{
   Elm_Font_Size_Data *size_data;
   Eina_List *l;
   Evas *evas;

   evas = evas_object_evas_get(size_list);
   evas_event_freeze(evas);
   edje_freeze();

   EINA_LIST_FOREACH(fndata.font_scale_list, l, size_data)
     elm_list_item_append(size_list, size_data->size_str, NULL, NULL,
                          _font_sizes_list_sel, size_data);

   EINA_LIST_FOREACH(fndata.font_px_list, l, size_data)
     elm_list_item_append(size_list, size_data->size_str, NULL, NULL,
                          _font_sizes_list_sel, size_data);

   elm_list_go(size_list);
   edje_thaw();
   evas_event_thaw(evas);
}

static void
_fnames_list_unselect_cb(void *data       EINA_UNUSED,
                         Evas_Object     *obj,
                         void *event_info EINA_UNUSED)
{
   Evas_Object *styles_list, *sizes_list, *win;

   if (elm_list_selected_item_get(obj)) return;

   win = elm_object_top_widget_get(obj);

   styles_list = evas_object_data_get(win, "font_styles_list");
   ELM_LIST_DISABLE(styles_list);

   sizes_list = evas_object_data_get(win, "font_sizes_list");
   ELM_LIST_DISABLE(sizes_list);
}

static Eina_Bool
_font_list_fill(const Eina_Hash *hash EINA_UNUSED,
                const void *key       EINA_UNUSED,
                void                 *data,
                void                 *fdata)
{
   Elm_Font_Properties *efp;
   Eina_List **flist;

   flist = fdata;
   efp = data;

   *flist = eina_list_append(*flist, efp->name);

   return EINA_TRUE;
}

static void
_font_names_list_load(Evas_Object *flist)
{
   Eina_List *l, *names_list = NULL;
   const char *font;
   Evas *evas;

   evas = evas_object_evas_get(flist);
   evas_event_freeze(evas);
   edje_freeze();

   eina_hash_foreach(fndata.font_hash, _font_list_fill, &names_list);
   names_list = eina_list_sort(names_list, eina_list_count(names_list),
                               _font_sort_cb);

   EINA_LIST_FOREACH(names_list, l, font)
     elm_list_item_append(flist, font, NULL, NULL, _font_names_list_sel, NULL);

   eina_list_free(names_list);

   evas_object_smart_callback_add(flist, "unselected",
                                  _fnames_list_unselect_cb, NULL);

   elm_list_go(flist);

   edje_thaw();
   evas_event_thaw(evas);
}

static void
_status_config_fonts(Evas_Object *win,
                     Evas_Object *naviframe)
{
   Evas_Object *base, *fr, *li, *rc, *preview, *sp, *pd, *bt, *bx;
   char buf[PATH_MAX];

   _fonts_data_fill(evas_object_evas_get(win));

   base = elm_table_add(win);
   evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(base, EVAS_HINT_FILL, EVAS_HINT_FILL);

   /* yeah, dummy rectangles are ugly as hell, but no good way yet of
      fixing sizes */
   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(rc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(rc, 70, 170);
   elm_table_pack(base, rc, 1, 0, 1, 1);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Font Class");
   elm_table_pack(base, fr, 0, 0, 1, 2);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr);

   /* FIXME: no multiselection for this list, for now */
   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, li);
   evas_object_show(li);

   evas_object_data_set(win, "font_classes_list", li);
   _font_classes_list_load(li);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Font");
   elm_table_pack(base, fr, 1, 0, 1, 2);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, li);
   evas_object_show(li);

   evas_object_data_set(win, "font_names_list", li);
   _font_names_list_load(li);
   ELM_LIST_DISABLE(li);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(rc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(rc, 70, 130);
   elm_table_pack(base, rc, 1, 2, 1, 1);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Style");
   elm_table_pack(base, fr, 0, 2, 1, 1);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr);
   ELM_LIST_DISABLE(li);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, li);
   evas_object_show(li);

   evas_object_data_set(win, "font_styles_list", li);
   evas_object_smart_callback_add(li, "unselected", _fstyle_list_unselect_cb,
                                  NULL);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Size");
   elm_table_pack(base, fr, 1, 2, 1, 1);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, li);
   evas_object_show(li);

   evas_object_data_set(win, "font_sizes_list", li);
   _font_sizes_list_load(li);
   ELM_LIST_DISABLE(li);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(base, bx, 0, 3, 2, 1);
   evas_object_show(bx);

   /* FIXME: what to do here? dedicated widget? some new entry API set? */
   snprintf(buf, sizeof(buf), "%s/objects/font_preview.edj", elm_app_data_dir_get());
   preview = elm_layout_add(win);
   elm_layout_file_set(preview, buf, "font_preview");
   elm_object_part_text_set(preview, "elm.text", "Preview Text — 我真的会写中文");
   evas_object_size_hint_weight_set(preview, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(preview, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(preview);

   elm_box_pack_end(bx, preview);
   evas_object_data_set(win, "font_preview", elm_layout_edje_get(preview));

   /////////////////////////////////////////////
   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_table_pack(base, sp, 0, 4, 2, 1);
   evas_object_show(sp);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(pd, 0.5, 0.5);
   elm_table_pack(base, pd, 0, 5, 2, 1);
   evas_object_show(pd);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bx);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _font_overlay_set_all, win);
   elm_object_text_set(bt, "Set to All");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _font_overlay_reset, win);
   elm_object_text_set(bt, "Reset");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _font_overlay_reset_all, win);
   elm_object_text_set(bt, "Reset All");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _font_overlay_change, win);
   elm_object_text_set(bt, "Apply");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   elm_object_content_set(pd, bx);

   evas_object_data_set(win, "fonts", base);

   elm_naviframe_item_simple_push(naviframe, base);
}

static void
_engines_list_item_del_cb(void            *data,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   free(data);
}

static void
_profiles_list_item_del_cb(void            *data,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   free(data);
}

static void
_profiles_list_selected_cb(void            *data,
                           Evas_Object     *obj,
                           void *event_info EINA_UNUSED)
{
   const char *cur_profile = NULL;
   const char *sel_profile, *pdir;
   const char *prof_name;
   char buf[PATH_MAX];
   Evas_Object *en;
#ifdef ELM_EFREET
   Efreet_Desktop *desk = NULL;
#endif

   sel_profile = data;
   if (!sel_profile)
     return;

   cur_profile = elm_config_profile_get();

   pdir = elm_config_profile_dir_get(sel_profile, EINA_TRUE);
   if (!pdir) pdir = elm_config_profile_dir_get(sel_profile, EINA_FALSE);
#ifdef ELM_EFREET
   snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
   desk = efreet_desktop_new(buf);
   if ((desk) && (desk->name)) prof_name = desk->name;
   else
     {
        if (desk) efreet_desktop_free(desk);
        if (pdir) elm_config_profile_dir_free(pdir);

        pdir = elm_config_profile_dir_get(sel_profile, EINA_FALSE);
        snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
        desk = efreet_desktop_new(buf);
        if ((desk) && (desk->name)) prof_name = desk->name;
        else prof_name = cur_profile;
     }
#endif

   if (!pdir)
     elm_object_disabled_set(evas_object_data_get(obj, "prof_reset_btn"),
                             EINA_TRUE);
   else
     {
        elm_object_disabled_set(evas_object_data_get(obj, "prof_del_btn"),
                                EINA_TRUE);
        elm_object_disabled_set(evas_object_data_get(obj, "prof_reset_btn"),
                                EINA_FALSE);
        elm_object_disabled_set(evas_object_data_get(obj, "prof_use_btn"),
                                EINA_FALSE);
        elm_config_profile_dir_free(pdir);
     }

   snprintf(buf, sizeof(buf), "<hilight>Selected profile: %s</><br/>",
            prof_name);
   elm_object_text_set(evas_object_data_get(obj, "prof_name_lbl"), buf);

   en = evas_object_data_get(obj, "prof_desc_entry");
#ifdef ELM_EFREET
   if (desk) elm_object_text_set(en, desk->comment);
   else
#endif
   elm_object_text_set(en, "Unknown");

#ifdef ELM_EFREET
   if (desk) efreet_desktop_free(desk);
#endif
}

static void
_profiles_list_fill(Evas_Object *l_widget,
                    Eina_List   *p_names)
{
   const char *cur_profile = NULL;
   const char *profile;
   void *sel_it = NULL;
   Eina_List *l;

   if (!p_names)
     return;

   elm_list_clear(l_widget);

   cur_profile = elm_config_profile_get();

   EINA_LIST_FOREACH(p_names, l, profile)
     {
#ifdef ELM_EFREET
        Efreet_Desktop *desk = NULL;
#endif
        const char *label, *ext, *pdir;
        char buf[PATH_MAX];
        Elm_Object_Item *list_it;
        Evas_Object *ic;

        pdir = elm_config_profile_dir_get(profile, EINA_TRUE);
        if (!pdir) pdir = elm_config_profile_dir_get(profile, EINA_FALSE);

#ifdef ELM_EFREET
        snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
        desk = efreet_desktop_new(buf);
        if ((desk) && (desk->name)) label = desk->name;
        else
          {
             if (desk) efreet_desktop_free(desk);
             if (pdir) elm_config_profile_dir_free(pdir);

             pdir = elm_config_profile_dir_get(profile, EINA_FALSE);
             snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
             desk = efreet_desktop_new(buf);
             if ((desk) && (desk->name)) label = desk->name;
             else label = profile;
          }
#endif

        buf[0] = 0;
        if (pdir) snprintf(buf, sizeof(buf), "%s/icon.edj", pdir);
#ifdef ELM_EFREET
        if ((desk) && (desk->icon) && (pdir))
          snprintf(buf, sizeof(buf), "%s/%s", pdir, desk->icon);
#endif
        ic = elm_icon_add(l_widget);
        ext = strrchr(buf, '.');
        if (ext)
          {
             if (!strcmp(ext, ".edj")) elm_image_file_set(ic, buf, "icon");
             else elm_image_file_set(ic, buf, NULL);
          }

        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL,
                                         1, 1);
        evas_object_show(ic);

        list_it = elm_list_item_append(l_widget, label, ic, NULL,
                                       _profiles_list_selected_cb,
                                       strdup(profile));
        elm_object_item_del_cb_set(list_it, _profiles_list_item_del_cb);
        if (cur_profile && !strcmp(profile, cur_profile))
          sel_it = list_it;

        elm_config_profile_dir_free(pdir);

#ifdef ELM_EFREET
        if (desk) efreet_desktop_free(desk);
#endif
     }

   if (sel_it) elm_list_item_selected_set(sel_it, EINA_TRUE);
   elm_list_go(l_widget);
}

static void
_profiles_list_unselect_cb(void *data       EINA_UNUSED,
                           Evas_Object     *obj,
                           void *event_info EINA_UNUSED)
{
   if (elm_list_selected_item_get(obj)) return;
   elm_object_disabled_set(evas_object_data_get(obj, "prof_del_btn"),
                           EINA_TRUE);
   elm_object_disabled_set(evas_object_data_get(obj, "prof_reset_btn"),
                           EINA_TRUE);
   elm_object_disabled_set(evas_object_data_get(obj, "prof_use_btn"),
                           EINA_TRUE);
}

static void
_status_config_profiles(Evas_Object *win,
                        Evas_Object *naviframe)
{
   Evas_Object *li, *bx, *fr_bx1, *fr_bx2, *btn_bx, *fr, *lb, *en, *sp, *pd,
   *bt;
   Eina_List *profs;
   Evas *evas;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   fr_bx1 = elm_box_add(win);
   evas_object_size_hint_weight_set(fr_bx1, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr_bx1, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_homogeneous_set(fr_bx1, EINA_TRUE);
   evas_object_show(fr_bx1);

   fr_bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(fr_bx2, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr_bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr_bx2);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Available Profiles");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, fr_bx1);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(li, "unselected", _profiles_list_unselect_cb,
                                  NULL);

   profs = elm_config_profile_list_get();

   evas_object_show(li);
   elm_box_pack_end(fr_bx2, li);

   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);

   evas_object_show(lb);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_editable_set(en, EINA_FALSE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);

   evas_object_data_set(li, "prof_name_lbl", lb);
   evas_object_data_set(li, "prof_desc_entry", en);

   elm_box_pack_end(fr_bx2, lb);
   elm_box_pack_end(fr_bx1, fr_bx2);
   elm_box_pack_end(fr_bx1, en);

   /////////////////////////////////////////////
   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 0.0, 0.0);
   evas_object_size_hint_align_set(pd, 0.5, 0.5);
   elm_box_pack_end(bx, pd);
   evas_object_show(pd);

   btn_bx = elm_box_add(win);
   elm_box_horizontal_set(btn_bx, EINA_TRUE);
   evas_object_size_hint_weight_set(btn_bx, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn_bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(btn_bx);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _profile_use, li);
   elm_object_text_set(bt, "Use");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(btn_bx, bt);
   evas_object_show(bt);

   evas_object_data_set(li, "prof_use_btn", bt);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _btn_todo, NULL); /* TODO */
   elm_object_text_set(bt, "Delete");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(btn_bx, bt);
   evas_object_show(bt);

   evas_object_data_set(li, "prof_del_btn", bt);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _profile_reset, li);
   elm_object_text_set(bt, "Reset");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(btn_bx, bt);
   evas_object_show(bt);

   evas_object_data_set(li, "prof_reset_btn", bt);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _btn_todo, NULL); /* TODO */
   elm_object_text_set(bt, "Add new");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(btn_bx, bt);
   evas_object_show(bt);

   elm_object_content_set(pd, btn_bx);

   evas = evas_object_evas_get(li);
   evas_event_freeze(evas);
   edje_freeze();

   _profiles_list_fill(li, profs);

   elm_config_profile_list_free(profs);

   edje_thaw();
   evas_event_thaw(evas);

   evas_object_data_set(win, "profiles", bx);
   elm_naviframe_item_simple_push(naviframe, bx);
}

static void
_status_config_scrolling_bounce(Evas_Object *win, Evas_Object *box)
{
   Evas_Object *fr, *bx, *ck, *pd, *lb, *sl;

   fr = elm_frame_add(box);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(fr, "Bounce");
   elm_box_pack_end(box, fr);
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   /* Enable Scroll Bounce */
   CHECK_ADD("Enable scroll bounce",
             "Set whether scrollers should bounce<br/>"
             "when they reach their viewport's edge<br/>"
             "during a scroll",
             sb_change, NULL);
   evas_object_data_set(win, "scroll_bounce_check", ck);
   elm_check_state_set(ck, elm_config_scroll_bounce_enabled_get());

   /* Scroll bounce friction */
   LABEL_FRAME_ADD("<hilight>Scroll bounce friction</>");

   sl = elm_slider_add(bx);
   elm_object_tooltip_text_set(sl, "This is the amount of inertia a <br/>"
                                   "scroller will impose at bounce animations");
   evas_object_data_set(win, "bounce_friction_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_indicator_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.0, 4.0);
   elm_slider_value_set(sl, elm_config_scroll_bounce_friction_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", bf_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", bf_change, NULL);
}

static void
_status_config_scrolling_acceleration(Evas_Object *win, Evas_Object *box)
{
   Evas_Object *fr, *bx, *sl, *pd, *lb;

   fr = elm_frame_add(box);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(fr, "Acceleration");
   elm_box_pack_end(box, fr);
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   /* Thumb scroll acceleration threshold */
   LABEL_FRAME_ADD("<hilight>Thumb scroll acceleration threshold</>");

   sl = elm_slider_add(bx);
   elm_object_tooltip_text_set(sl, "This is the minimum speed of mouse <br/>"
                                   "cursor movement which will accelerate<br/>"
                                   "scrolling velocity after a<br/>"
                                   "mouse up event (pixels/second)");
   evas_object_data_set(win, "ts_acceleration_threshold_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f pixels/s");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 10.0, 5000.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_acceleration_threshold_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsat_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsat_change, NULL);

   /* Thumb scroll acceleration time limit */
   LABEL_FRAME_ADD("<hilight>Thumb scroll acceleration time limit</>");

   sl = elm_slider_add(bx);
   elm_object_tooltip_text_set(sl, "This is the time limit for<br/>"
                                   "accelerating velocity<br/>");
   evas_object_data_set(win, "ts_acceleration_time_limit_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.1f");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_min_max_set(sl, 0.0, 15.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_acceleration_time_limit_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsatl_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsatl_change, NULL);

   /* Thumb scroll acceleration weight */
   LABEL_FRAME_ADD("<hilight>Thumb scroll acceleration weight</>");

   sl = elm_slider_add(bx);
   elm_object_tooltip_text_set(sl, "This is the weight for acceleration");
   evas_object_data_set(win, "ts_acceleration_weight_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.1f");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_min_max_set(sl, 1.0, 10.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_acceleration_weight_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsaw_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsaw_change, NULL);
}

static void
_status_config_scrolling(Evas_Object *win,
                         Evas_Object *naviframe)
{
   Evas_Object *lb, *pd, *bx, *sl, *sp, *ck, *sc;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   evas_object_show(sc);
   elm_object_content_set(sc, bx);

   /* Bounce */
   _status_config_scrolling_bounce(win, bx);

   /* Acceleration */
   _status_config_scrolling_acceleration(win, bx);

   /* Enable thumb scroll */
   CHECK_ADD("Enable thumb scroll",
             "Set whether scrollers should be<br/>"
             "draggable from any point in their views",
             ts_change, NULL);
   evas_object_data_set(win, "thumbscroll_check", ck);
   elm_check_state_set(ck, elm_config_scroll_thumbscroll_enabled_get());

   /* Thumb scroll threadhold */
   LABEL_FRAME_ADD("<hilight>Thumb scroll threshold</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the number of pixels one should<br/>"
                                   "travel while dragging a scroller's view<br/>"
                                   "to actually trigger scrolling");
   evas_object_data_set(win, "ts_threshold_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f pixels");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 8.0, 50.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_threshold_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tst_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tst_change, NULL);

   /* Thumb scroll hold threashold */
   LABEL_FRAME_ADD("<hilight>Thumb scroll hold threshold</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the number of pixels the range<br/>"
                                   "which can be scrolled, while the scroller<br/>"
                                   "is holed");
   evas_object_data_set(win, "ts_hold_threshold_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f pixels");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 4.0, 500.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_hold_threshold_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsht_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsht_change, NULL);

   /* Thumb scroll momentum threshold */
   LABEL_FRAME_ADD("<hilight>Thumb scroll momentum threshold</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the minimum speed of mouse <br/>"
                                   "cursor movement which will trigger<br/>"
                                   "list self scrolling animation after a<br/>"
                                   "mouse up event (pixels/second)");
   evas_object_data_set(win, "ts_momentum_threshold_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f pixels/s");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 10.0, 200.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_momentum_threshold_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsmt_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsmt_change,
                                  NULL);

   /* Thumb scroll flick distance tolerance */
   LABEL_FRAME_ADD("<hilight>Thumb scroll flick distance tolerance</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the number of pixels the maximum<br/>"
                                   "distance which can be flicked. If it is<br/>"
                                   "flicked more than this, the flick distance<br/>"
                                   "is same with maximum distance");
   evas_object_data_set(win, "ts_flick_distance_tolerance_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f pixels");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 100.0, 3000.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_flick_distance_tolerance_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsfdt_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsfdt_change, NULL);

   /* Thumb scroll friction */
   LABEL_FRAME_ADD("<hilight>Thumb scroll friction</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the amount of inertia a<br/>"
                                   "scroller will impose at self scrolling<br/>"
                                   "animations");
   evas_object_data_set(win, "ts_friction_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.1f");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_min_max_set(sl, 0.1, 10.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_friction_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsf_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsf_change, NULL);

   /* Thumb scroll min friction */
   LABEL_FRAME_ADD("<hilight>Thumb scroll min friction</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the min amount of inertia a<br/>"
                                   "scroller will impose at self scrolling<br/>"
                                   "animations");
   evas_object_data_set(win, "ts_min_friction_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.1f");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_min_max_set(sl, 0.1, 10.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_min_friction_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsmf_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsmf_change, NULL);

   /* Thumb scroll friction standard */
   LABEL_FRAME_ADD("<hilight>Thumb scroll friction standard</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the standard velocity of the scroller."
                                   "<br/>The scroll animation time is same<br/>"
                                   "with thumbscroll friction, if the velocity"
                                   "<br/>is same with standard velocity.");
   evas_object_data_set(win, "ts_friction_standard_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f pixel/s");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 10.0, 5000.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_friction_standard_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsfs_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsfs_change, NULL);

   /* Thumb scroll border friction */
   LABEL_FRAME_ADD("<hilight>Thumb scroll border friction</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the amount of lag between your<br/>"
                                   "actual mouse cursor dragging movement<br/>"
                                   "and a scroller's view movement itself,<br/>"
                                   "while pushing it into bounce state<br/>"
                                   "manually");
   evas_object_data_set(win, "ts_border_friction_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_indicator_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.0, 1.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_border_friction_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tsbf_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tsbf_change, NULL);

   /* Thumb scroll sensitivity friction */
   LABEL_FRAME_ADD("<hilight>Thumb scroll sensitivity friction</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the sensitivity amount which<br/>"
                                   "is be multiplied by the length of mouse<br/>"
                                   "dragging.");
   evas_object_data_set(win, "ts_sensitivity_friction_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_indicator_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.1, 1.0);
   elm_slider_value_set(sl, elm_config_scroll_thumbscroll_sensitivity_friction_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", tssf_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", tssf_change, NULL);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   /* Page scroll friction */
   LABEL_FRAME_ADD("<hilight>Page scroll friction</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the amount of inertia a<br/>"
                                   "paged scroller will impose at<br/>"
                                   "page fitting animations");
   evas_object_data_set(win, "page_scroll_friction_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.1f");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_min_max_set(sl, 0.0, 5.0);
   elm_slider_value_set(sl, elm_config_scroll_page_scroll_friction_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", ps_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", ps_change, NULL);

   /* Bring in scroll friction */
   LABEL_FRAME_ADD("<hilight>Bring in scroll friction</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the amount of inertia a<br/>"
                                   "scroller will impose at region bring<br/>"
                                   "animations");
   evas_object_data_set(win, "bring_in_scroll_friction_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.1f");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_min_max_set(sl, 0.0, 5.0);
   elm_slider_value_set(sl, elm_config_scroll_bring_in_scroll_friction_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", bis_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", bis_change, NULL);

   /* Zoom scroll friction */
   LABEL_FRAME_ADD("<hilight>Zoom scroll friction</>");

   sl = elm_slider_add(win);
   elm_object_tooltip_text_set(sl, "This is the amount of inertia<br/>"
                                   "scrollers will impose at animations<br/>"
                                   "triggered by Elementary widgets'<br/>"
                                   "zooming API");
   evas_object_data_set(win, "zoom_scroll_friction_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.1f");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_min_max_set(sl, 0.0, 5.0);
   elm_slider_value_set(sl, elm_config_scroll_zoom_friction_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", zf_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", zf_change, NULL);

   evas_object_data_set(win, "scrolling", sc);

   elm_naviframe_item_simple_push(naviframe, sc);
}

static char *
_engine_name_prettify(const char *engine)
{
   char *ret, *ptr;

   ret = strdup(engine);
   ret[0] -= 0x20;

   while ((ptr = strpbrk(ret, "_")))
     {
        *ptr = ' ';
     }

   return ret;
}

/* FIXME! ideally, we would trim elm_config.c's _elm_engines list at
   build time, making a getter for is as in ecore-evas. */
static Eina_Bool
_elm_engine_supported(const char *engine)
{
   const char *engines[] = {
      "software_x11",
      "fb",
      "directfb",
      "software_16_x11",
      "software_8_x11",
      "xrender_x11",
      "opengl_x11",
      "software_gdi",
      "software_16_wince_gdi",
      "sdl",
      "software_16_sdl",
      "opengl_sdl",
      "ews",
      "opengl_cocoa",
      "psl1ght",
      "wayland_shm",
      "wayland_egl",
      NULL
   };

   unsigned int i;

   for (i = 0; engines[i]; i++)
     {
#define ENGINE_COMPARE(name) (!strcmp(engines[i], name))
        if (ENGINE_COMPARE(engine))
          return EINA_TRUE;
#undef ENGINE_COMPARE
     }

   return EINA_FALSE;
}

static void
_engines_list_fill(Evas_Object *l_widget,
                   Eina_List   *e_names)
{
   const char *engine, *cur_engine;
   void *sel_it = NULL;
   Eina_List *l;

   if (!e_names)
     return;

   cur_engine = elm_config_engine_get();

   EINA_LIST_FOREACH(e_names, l, engine)
     {
        const char *label;
        Elm_Object_Item *list_it;

        if (!_elm_engine_supported(engine))
          continue;

        label = _engine_name_prettify(engine);

        list_it = elm_list_item_append(l_widget, label, NULL, NULL, NULL,
                                  strdup(engine));
        elm_object_item_del_cb_set(list_it, _engines_list_item_del_cb);
        free((void *)label);

        if (!strcmp(cur_engine, engine))
          sel_it = list_it;
     }

   if (sel_it) elm_list_item_selected_set(sel_it, EINA_TRUE);
   elm_list_go(l_widget);
}

static void
_status_config_rendering(Evas_Object *win,
                         Evas_Object *naviframe)
{
   Evas_Object *li, *bx, *fr, *sp, *pd, *bt;
   Eina_List *engines;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Available Engines");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   li = elm_list_add(win);
   elm_object_content_set(fr, li);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);

   engines = ecore_evas_engines_get();
   _engines_list_fill(li, engines);
   ecore_evas_engines_free(engines);

   evas_object_show(li);
   evas_object_data_set(win, "engines_list", li);

   /////////////////////////////////////////////
   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 0.0, 0.0);
   evas_object_size_hint_align_set(pd, 0.5, 0.5);
   elm_box_pack_end(bx, pd);
   evas_object_show(pd);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _engine_use, li);
   elm_object_text_set(bt, "Use Engine");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_object_content_set(pd, bt);
   evas_object_show(bt);

   evas_object_data_set(win, "rendering", bx);
   elm_naviframe_item_simple_push(naviframe, bx);
}

static void
_status_config_caches(Evas_Object *win,
                      Evas_Object *naviframe)
{
   Evas_Object *lb, *pd, *bx, *sl, *sp, *ck;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);

   LABEL_FRAME_ADD("<hilight>Cache Flush Interval (8 ticks per second)</>");

   sl = elm_slider_add(win);
   evas_object_data_set(win, "cache_flush_interval_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f ticks");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 8.0, 4096.0);
   elm_slider_value_set(sl, elm_config_cache_flush_interval_get());
   elm_object_disabled_set(sl, !elm_config_cache_flush_enabled_get());

   CHECK_ADD("Enable Flushing", "Enable Flushing", cf_enable, sl);
   elm_check_state_set(ck, elm_config_cache_flush_enabled_get());

   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", cf_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", cf_change, NULL);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   LABEL_FRAME_ADD("<hilight>Font Cache Size</>");

   sl = elm_slider_add(win);
   evas_object_data_set(win, "font_cache_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.1f MB");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_min_max_set(sl, 0.0, 4.0);
   elm_slider_value_set(sl, (double)elm_config_cache_font_cache_size_get() / 1024.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", fc_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", fc_change, NULL);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   LABEL_FRAME_ADD("<hilight>Image Cache Size</>");

   sl = elm_slider_add(win);
   evas_object_data_set(win, "image_cache_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f MB");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 0, 32);
   elm_slider_value_set(sl, (double)elm_config_cache_image_cache_size_get() / 1024.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", ic_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", ic_change, NULL);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   LABEL_FRAME_ADD("<hilight>Number of Edje Files to Cache</>");

   sl = elm_slider_add(win);
   evas_object_data_set(win, "edje_file_cache_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f files");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 0, 32);
   elm_slider_value_set(sl, elm_config_cache_edje_file_cache_size_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", efc_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", efc_change, NULL);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   LABEL_FRAME_ADD("<hilight>Number of Edje Collections to Cache</>");

   sl = elm_slider_add(win);
   evas_object_data_set(win, "edje_collection_cache_slider", sl);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f collections");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 0, 128);
   elm_slider_value_set(sl, elm_config_cache_edje_collection_cache_size_get());
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", ecc_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", ecc_change, NULL);

   evas_object_data_set(win, "caches", bx);

   elm_naviframe_item_simple_push(naviframe, bx);
}

static void
_status_config_full(Evas_Object *win,
                    Evas_Object *bx0)
{
   Evas_Object *tb, *naviframe;
   Elm_Object_Item *tb_sizing, *tb_it;

   tb = elm_toolbar_add(win);
   elm_toolbar_select_mode_set(tb, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_toolbar_menu_parent_set(tb, win);
   elm_toolbar_homogeneous_set(tb, EINA_FALSE);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   tb_sizing = elm_toolbar_item_append(tb, "zoom-fit-best", "Sizing",
                                    _cf_sizing, win);
   elm_toolbar_item_priority_set(tb_sizing, 100);

   tb_it = elm_toolbar_item_append(tb, "preferences-desktop-theme", "Theme",
                                _cf_themes, win);
   elm_toolbar_item_priority_set(tb_it, 90);

   elm_toolbar_item_append(tb, "preferences-desktop-font", "Fonts",
                           _cf_fonts, win);

   tb_it = elm_toolbar_item_append(tb, "system-users", "Profiles",
                                _cf_profiles, win);
   elm_toolbar_item_priority_set(tb_it, 90);

   elm_toolbar_item_append(tb, "system-run", "Scrolling", _cf_scrolling, win);
   elm_toolbar_item_append(tb, "video-display", "Rendering",
                           _cf_rendering, win);
   elm_toolbar_item_append(tb, "appointment-new", "Caches", _cf_caches, win);
   elm_toolbar_item_append(tb, "sound", "Audio", _cf_audio, win);
   elm_toolbar_item_append(tb, NULL, "Focus", _cf_focus, win);
   elm_toolbar_item_append(tb, NULL, "Etc", _cf_etc, win);

   elm_box_pack_end(bx0, tb);
   evas_object_show(tb);

   naviframe = elm_naviframe_add(win);
   evas_object_size_hint_align_set(naviframe, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(naviframe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_data_set(win, "naviframe", naviframe);

   _status_config_themes(win, naviframe);
   _status_config_fonts(win, naviframe);
   _status_config_profiles(win, naviframe);
   _status_config_rendering(win, naviframe);
   _status_config_scrolling(win, naviframe);
   _status_config_caches(win, naviframe);
   _status_config_audio(win, naviframe);
   _status_config_focus(win, naviframe);
   _status_config_etc(win, naviframe);
   _status_config_sizing(win, naviframe); // Note: call this at the end.

   // FIXME uncomment after flip style fix, please
   //elm_object_style_set(naviframe, "flip");
   elm_toolbar_item_selected_set(tb_sizing, EINA_TRUE);
   elm_box_pack_end(bx0, naviframe);
   evas_object_show(naviframe);
}

static void
win_create(void)
{
   Evas_Object *win, *bx0;

   win = elm_win_util_standard_add("main", "Elementary Config");
   if (!win) exit(1);
   ecore_event_handler_add(ELM_EVENT_CONFIG_ALL_CHANGED, _config_all_changed,
                           win);
   evas_object_smart_callback_add(win, "delete,request", config_exit, NULL);

   bx0 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx0, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx0);
   evas_object_show(bx0);

   if (!interactive) _status_basic(win, bx0);
   else if (0)
     _status_config(win, bx0);
   else _status_config_full(win, bx0);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

static Eina_Bool
_exit_timer(void *data EINA_UNUSED)
{
   elm_exit();
   return ECORE_CALLBACK_CANCEL;
}

/* this is your elementary main function - it MUST be called IMMEDIATELY
 * after elm_init() and MUST be passed argc and argv, and MUST be called
 * elm_main and not be static - must be a visible symbol with EAPI infront */
EAPI_MAIN int
elm_main(int    argc,
         char **argv)
{
   int i;

   elm_app_info_set(elm_main, "elementary", "images/logo.png");
   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);

   for (i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-h"))
          {
             printf("Usage:\n"
                    "  -h                This help\n"
                    "  -q                Quiet mode (dont show window)\n"
                    "  -t THEME          Set theme to THEME (ELM_THEME spec)\n"
                    "  -f SIZE           Set finger size to SIZE pixels\n"
                    "  -s SCALE          Set scale factor to SCALE\n"
                    );
          }
        else if (!strcmp(argv[i], "-q"))
          {
             quiet = 1;
             interactive = 0;
          }
        else if ((!strcmp(argv[i], "-t")) && (i < argc - 1))
          {
             i++;
             theme_set = argv[i];
             interactive = 0;
          }
        else if ((!strcmp(argv[i], "-f")) && (i < argc - 1))
          {
             i++;
             finger_size_set = argv[i];
             interactive = 0;
          }
        else if ((!strcmp(argv[i], "-s")) && (i < argc - 1))
          {
             i++;
             scale_set = argv[i];
             interactive = 0;
          }
     }
   /* put here any init code specific to this app like parsing args, etc. */
   if (!quiet)
     {
#ifdef ELM_EFREET
        elm_need_efreet();
#endif
        win_create(); /* create main window */
        if (!interactive)
          ecore_timer_add(2.0, _exit_timer, NULL);
     }
   if (!interactive)
     {
        if (theme_set) elm_theme_set(NULL, theme_set);
        if (finger_size_set) elm_config_finger_size_set(atoi(finger_size_set));
        if (scale_set)  elm_config_scale_set(atof(scale_set));

        elm_config_all_flush();

        if (quiet) elm_exit();
     }
   elm_run(); /* and run the program now and handle all events, etc. */
   /* if the mainloop that elm_run() runs exists, we exit the app */
   elm_shutdown(); /* clean up and shut down */
   /* exit code */
   return 0;
}
/* All elementary apps should use this. Put it right after elm_main() */
ELM_MAIN()
