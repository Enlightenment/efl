#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct _Theme Theme;

struct _Theme
{
   const char *label;
   const char *name;
   const char *path;
   Eina_Bool in_search_path;
};

static Theme *tsel = NULL;
static Eina_List *themes = NULL;

static int quiet = 0;
static int interactive = 1;

static const char *theme_set = NULL;
static const char *finger_size_set = NULL;
static const char *scale_set = NULL;

static void
my_win_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_config_save();
   /* called when my_win_main is requested to be deleted */
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

static void
sc_round(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 10.0))) / 10.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
sc_change(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   double scale = elm_scale_get();
   double val = elm_slider_value_get(obj);

   if (scale == val) return;
   elm_scale_all_set(val);
}

static void
fs_round(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   double val = elm_slider_value_get(obj);
   double v;

   v = ((double)((int)(val * 5.0))) / 5.0;
   if (v != val) elm_slider_value_set(obj, v);
}

static void
fs_change(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   double scale = elm_scale_get();
   double val = elm_slider_value_get(obj);

   if (scale == val) return;
   elm_finger_size_all_set(val);
}

static void
_status_basic(Evas_Object *win, Evas_Object *bx0)
{
   Evas_Object *lb, *fr;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   elm_frame_label_set(fr, "Information");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);

   lb = elm_label_add(win);
   elm_label_label_set(lb,
                       "Applying configuration change"
                       );
   elm_frame_content_set(fr, lb);
   evas_object_show(lb);
}

static void
_status_config(Evas_Object *win, Evas_Object *bx0)
{
   Evas_Object *lb, *pd, *bx2, *fr, *sl, *sp;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_label_set(fr, "Sizing");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);

   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.5);

   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);

   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 1.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
   elm_label_label_set(lb,"<hilight>Scale</>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);

   sl = elm_slider_add(win);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_indicator_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.25, 5.0);
   elm_slider_value_set(sl, elm_scale_get());
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", sc_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", sc_change, NULL);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   evas_object_size_hint_weight_set(sp, 1.0, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx2, sp);
   evas_object_show(sp);

   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);

   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 1.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
   elm_label_label_set(lb, "<hilight>Finger Size</><br>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);

   sl = elm_slider_add(win);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 5, 200);
   elm_slider_value_set(sl, elm_finger_size_get());
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", fs_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", fs_change, NULL);

   // FIXME: add theme selector (basic mode and advanced for fallbacks)
   // FIXME: save config
   // FIXME: profile selector / creator etc.
   //
   elm_frame_content_set(fr, bx2);
   evas_object_show(bx2);
}

static void
_flip_to(Evas_Object *win, const char *name)
{
   Evas_Object *wid, *pager;
   wid = evas_object_data_get(win, name);
   pager = evas_object_data_get(win, "pager");
   elm_pager_content_promote(pager, wid);
}

static void
_cf_sizing(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _flip_to(data, "sizing");
}

static void
_cf_themes(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _flip_to(data, "themes");
}

static void
_cf_fonts(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _flip_to(data, "fonts");
}

static void
_cf_profiles(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _flip_to(data, "profiles");
}

static void
_cf_scrolling(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _flip_to(data, "scrolling");
}

static void
_cf_rendering(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _flip_to(data, "rendering");
}

static void
_cf_caches(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _flip_to(data, "caches");
}

const char *
_elm_theme_current_get(const char *theme_search_order)
{
   const char *ret;
   const char *p;

   if (!theme_search_order)
     return NULL;

   for (p = theme_search_order; ; p++)
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
_profile_change_do(Evas_Object *win, const char *profile)
{
   const char *curr_theme, *curr_engine;
   const Eina_List *l_items, *l;
   Elm_List_Item *it;
   Elm_Theme *th;
   double scale;
   int fs;

   elm_profile_all_set(profile);

   scale = elm_scale_get();
   fs = elm_finger_size_get();

   /* gotta update root windows' atoms */
   elm_scale_all_set(scale);
   elm_slider_value_set(evas_object_data_get(win, "scale_slider"), scale);
   elm_finger_size_all_set(fs);
   elm_slider_value_set(evas_object_data_get(win, "fs_slider"), fs);

   curr_theme = _elm_theme_current_get(elm_theme_get(NULL));
   elm_theme_all_set(curr_theme);

   th = elm_theme_new();
   elm_theme_set(th, curr_theme);
   elm_object_theme_set(evas_object_data_get(win, "theme_preview"), th);
   elm_theme_free(th);
   eina_stringshare_del(curr_theme);

   curr_engine = elm_engine_current_get();
   l_items = elm_list_items_get(evas_object_data_get(win, "engines_list"));
   EINA_LIST_FOREACH(l_items, l, it)
     {
       if (!strcmp(elm_list_item_data_get(it), curr_engine))
         {
           elm_list_item_selected_set(it, EINA_TRUE);
           break;
         }
     }
}

static void
_engine_use(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *li;
   const char *selection;

   li = data;
   selection = elm_list_item_data_get(elm_list_selected_item_get(li));

   if (!strcmp(elm_engine_current_get(), selection))
     return;

   elm_engine_set(selection);
   elm_config_save(); /* make sure new engine has its data dir */
}

static void
_profile_use(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *li;
   const char *selection;

   li = data;
   selection = elm_list_item_data_get(elm_list_selected_item_get(li));

   if (!strcmp(elm_profile_current_get(), selection))
     return;

   elm_config_save(); /* dump config into old profile's data dir */
   elm_profile_set(selection); /* just here to update info for getters below */

   _profile_change_do(elm_object_top_widget_get(li), selection);
   elm_config_save(); /* make sure new profile has its data dir */
}

static void
_btn_todo(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("To be done!\n");
}

static void
_profile_reset(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   const char *selection, *curr;
   const char *pdir;
   Evas_Object *li;

   li = data;
   selection = elm_list_item_data_get(elm_list_selected_item_get(li));
   curr = elm_profile_current_get();

   pdir = elm_profile_dir_get(selection, EINA_TRUE);
   if (!pdir)
     return;

   elm_config_save(); /* dump config into old profile's data dir */

   ecore_file_recursive_rm(pdir);
   free((void *)pdir);

   elm_config_reload();

   _profile_change_do(elm_object_top_widget_get(li), curr);
   elm_config_save(); /* make sure new profile has its data dir */
}

static void
_theme_use(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   const char *defth;
   char *newth;
   Theme *t = tsel;

   if (!t) return;
   defth = elm_theme_get(NULL);
   newth = malloc(strlen(defth) + 1 + strlen(t->name) + 1);
   if (newth)
     {
        char *rest;

        newth[0] = 0;
        rest = strchr(defth, ':');
        if (!rest)
          strcpy(newth, t->name);
        else
          {
             strcpy(newth, t->name);
             strcat(newth, rest);
          }
        elm_theme_all_set(newth);
        free(newth);
     }
}

static void
_theme_sel(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Theme *t = data;
   Evas_Object *win = elm_object_top_widget_get(obj);
   Evas_Object *sample = evas_object_data_get(win, "theme_preview");
   Elm_Theme *th;
   const char *defth;
   char *newth;

   tsel = t;
   defth = elm_theme_get(NULL);
   newth = malloc(strlen(defth) + 1 + strlen(t->name) + 1);
   th = elm_theme_new();
   if (newth)
     {
        char *rest;

        newth[0] = 0;
        rest = strchr(defth, ':');
        if (!rest)
          strcpy(newth, t->name);
        else
          {
             strcpy(newth, t->name);
             strcat(newth, rest);
          }
        elm_theme_set(th, newth);
        free(newth);
     }
   elm_object_theme_set(sample, th);
   elm_theme_free(th);
}

/*static void
  _theme_browse(void *data, Evas_Object *obj, void *event_info)
  {
  printf("not implemented\n");
  }*/

static void
_status_config_sizing(Evas_Object *win, Evas_Object *pager)
{
   Evas_Object *lb, *pd, *bx2, *sl, *sp;

   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.5);

   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);

   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 1.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
   elm_label_label_set(lb,"<hilight>Scale</>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);

   sl = elm_slider_add(win);
   evas_object_data_set(win, "scale_slider", sl);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_indicator_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.25, 5.0);
   elm_slider_value_set(sl, elm_scale_get());
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", sc_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", sc_change, NULL);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   evas_object_size_hint_weight_set(sp, 1.0, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx2, sp);
   evas_object_show(sp);

   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);

   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 1.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);
   elm_label_label_set(lb, "<hilight>Finger Size</><br>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);

   sl = elm_slider_add(win);
   evas_object_data_set(win, "fs_slider", sl);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   elm_slider_span_size_set(sl, 120);
   elm_slider_unit_format_set(sl, "%1.0f");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_min_max_set(sl, 5, 200);
   elm_slider_value_set(sl, elm_finger_size_get());
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", fs_round, NULL);
   evas_object_smart_callback_add(sl, "delay,changed", fs_change, NULL);

   evas_object_data_set(win, "sizing", bx2);

   elm_pager_content_push(pager, bx2);
}

static Evas_Object *
_sample_theme_new(Evas_Object *win)
{
   Evas_Object *base, *bg, *bt, *ck, *rd, *rdg, *sl, *fr, *li, *rc, *sp;

   base = elm_table_add(win);
   evas_object_size_hint_weight_set(base, 1.0, 1.0);
   evas_object_size_hint_align_set(base, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(base, bg, 0, 0, 2, 5);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button");
   elm_table_pack(base, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   ck = elm_check_add(win);
   elm_check_label_set(ck, "Check");
   elm_table_pack(base, ck, 0, 1, 1, 1);
   evas_object_show(ck);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 0);
   elm_radio_label_set(rd, "Radio 1");
   elm_table_pack(base, rd, 1, 0, 1, 1);
   evas_object_show(rd);
   rdg = rd;

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 1);
   elm_radio_label_set(rd, "Radio 2");
   elm_radio_group_add(rd, rdg);
   elm_table_pack(base, rd, 1, 1, 1, 1);
   evas_object_show(rd);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   evas_object_size_hint_weight_set(sp, 1.0, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_table_pack(base, sp, 0, 2, 2, 1);
   evas_object_show(sp);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Slider");
   elm_slider_span_size_set(sl, 120);
   elm_slider_min_max_set(sl, 1, 10);
   elm_slider_value_set(sl, 4);
   evas_object_size_hint_weight_set(sl, 1.0, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(base, sl, 0, 3, 2, 1);
   evas_object_show(sl);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Frame");
   elm_table_pack(base, fr, 0, 4, 2, 1);
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, 1.0, 1.0);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_content_set(fr, li);
   evas_object_show(li);

   elm_list_item_append(li, "List Item 1", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Second Item", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Third Item", NULL, NULL,  NULL, NULL);
   elm_list_go(li);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rc, 120, 120);
   elm_table_pack(base, rc, 0, 4, 2, 1);

   return base;
}

static void
_status_config_themes(Evas_Object *win, Evas_Object *pager)
{
   Evas_Object *tb, *rc, *sc, *sp, *li, *pd, *fr, *bt, *sample;
   Eina_List *list, *l;
   char *th, *s, *ext;

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 1.0, 1.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rc, 240, 120);
   elm_table_pack(tb, rc, 0, 0, 1, 1);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rc, 240, 240);
   elm_table_pack(tb, rc, 0, 1, 1, 1);

   /////////////////////////////////////////////

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 1.0, 1.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, pd, 0, 0, 1, 1);
   evas_object_show(pd);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, 1.0, 1.0);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_content_set(pd, li);
   evas_object_show(li);

   list = elm_theme_name_available_list_new();
   EINA_LIST_FOREACH(list, l, th)
     {
        Theme *t;

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
        elm_list_item_append(li, t->label, NULL, NULL,  _theme_sel, t);
     }
   elm_theme_name_available_list_free(list);

   elm_list_go(li);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 1.0, 1.0);
   evas_object_size_hint_align_set(pd, 0.9, 0.9);
   elm_table_pack(tb, pd, 0, 0, 1, 1);
   evas_object_show(pd);

   /* FIXME: not implemented yet
      bt = elm_button_add(win);
      evas_object_smart_callback_add(bt, "clicked", _theme_browse, win);
      elm_button_label_set(bt, "Browse...");
      evas_object_size_hint_weight_set(bt, 1.0, 1.0);
      evas_object_size_hint_align_set(bt, 0.9, 0.9);
      elm_frame_content_set(pd, bt);
      evas_object_show(bt);
   */
   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 1.0, 0.0);
   evas_object_size_hint_align_set(pd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, pd, 0, 1, 1, 1);
   evas_object_show(pd);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Preview");
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_content_set(pd, fr);
   evas_object_show(fr);

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, 0, 0);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_content_set(fr, sc);
   evas_object_show(sc);

   sample = _sample_theme_new(win);
   elm_scroller_content_set(sc, sample);
   evas_object_show(sample);
   evas_object_data_set(win, "theme_preview", sample);

   /////////////////////////////////////////////
   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   evas_object_size_hint_weight_set(sp, 1.0, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   elm_table_pack(tb, sp, 0, 2, 1, 1);
   evas_object_show(sp);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   evas_object_size_hint_weight_set(pd, 0.0, 0.0);
   evas_object_size_hint_align_set(pd, 0.5, 0.5);
   elm_table_pack(tb, pd, 0, 3, 1, 1);
   evas_object_show(pd);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _theme_use, win);
   elm_button_label_set(bt, "Use Theme");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_frame_content_set(pd, bt);
   evas_object_show(bt);

   evas_object_data_set(win, "themes", tb);
   elm_pager_content_push(pager, tb);
}

static void
_unimplemented(Evas_Object *win, Evas_Object *pager, const char *name)
{
   Evas_Object *lb, *pd, *bx2;

   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.5);

   pd = elm_frame_add(win);
   evas_object_size_hint_weight_set(pd, 0.0, 0.0);
   evas_object_size_hint_align_set(pd, 0.5, 0.5);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx2, pd);
   evas_object_show(pd);

   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, 0.0, 0.0);
   evas_object_size_hint_align_set(lb, 0.5, 0.5);
   elm_label_label_set(lb,"<hilight>Not implemented yet</>");
   elm_frame_content_set(pd, lb);
   evas_object_show(lb);

   evas_object_data_set(win, name, bx2);
   elm_pager_content_push(pager, bx2);
}

static void
_status_config_fonts(Evas_Object *win, Evas_Object *pager)
{
   _unimplemented(win, pager, "fonts");
}

static void
_engines_list_item_del_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

static void
_profiles_list_item_del_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

static void
_profiles_list_selected_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   const char *cur_profile = NULL;
   const char *sel_profile, *pdir;
   Eina_Bool cur_selected;
   const char *prof_name;
   char buf[PATH_MAX];
   Evas_Object *en;
#ifdef ELM_EFREET
   Efreet_Desktop *desk = NULL;
#endif

   sel_profile = data;
   if (!sel_profile)
     return;

   cur_profile = elm_profile_current_get();
   cur_selected = !strcmp(cur_profile, sel_profile);

   pdir = elm_profile_dir_get(sel_profile, EINA_FALSE);
#ifdef ELM_EFREET
   snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
   desk = efreet_desktop_new(buf);
   if ((desk) && (desk->name)) prof_name = desk->name;
   else
#endif
     prof_name = cur_profile;

   if (!pdir)
       elm_object_disabled_set(evas_object_data_get(obj, "prof_reset_btn"),
                               cur_selected);
   else
     {
       elm_object_disabled_set(evas_object_data_get(obj, "prof_del_btn"),
                               EINA_TRUE);
       elm_profile_dir_free(pdir);
     }

   snprintf(buf, sizeof(buf), "<hilight>Selected profile: %s</><br>",
            prof_name);
   elm_label_label_set(evas_object_data_get(obj, "prof_name_lbl"), buf);

   en = evas_object_data_get(obj, "prof_desc_entry");
#ifdef ELM_EFREET
   if (desk) elm_scrolled_entry_entry_set(en, desk->comment);
   else
#endif
     elm_scrolled_entry_entry_set(en, "Unknown");

#ifdef ELM_EFREET
   if (desk) efreet_desktop_free(desk);
#endif
}

static void
_profiles_list_fill(Evas_Object *l_widget, Eina_List *p_names)
{
   const char *cur_profile = NULL;
   const char *profile;
   void *sel_it = NULL;
   Eina_List *l;

   if (!p_names)
     return;

   elm_list_clear(l_widget);

   cur_profile = elm_profile_current_get();

   EINA_LIST_FOREACH(p_names, l, profile)
     {
#ifdef ELM_EFREET
       Efreet_Desktop *desk = NULL;
#endif
        const char *label, *ext, *pdir;
        char buf[PATH_MAX];
        Elm_List_Item *it;
        Evas_Object *ic;

        pdir = elm_profile_dir_get(profile, EINA_FALSE);
        label = profile;

#ifdef ELM_EFREET
        snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
        desk = efreet_desktop_new(buf);
        if ((desk) && (desk->name)) label = desk->name;
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
             if (!strcmp(ext, ".edj")) elm_icon_file_set(ic, buf, "icon");
             else elm_icon_file_set(ic, buf, NULL);
          }

        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL,
                                         1, 1);
        evas_object_show(ic);

        it = elm_list_item_append(l_widget, label, ic, NULL,
                                  _profiles_list_selected_cb, strdup(profile));
        elm_list_item_del_cb_set(it, _profiles_list_item_del_cb);
        if (cur_profile && !strcmp(profile, cur_profile))
          sel_it = it;

        elm_profile_dir_free(pdir);

#ifdef ELM_EFREET
       if (desk) efreet_desktop_free(desk);
#endif
     }

   if (sel_it) elm_list_item_selected_set(sel_it, EINA_TRUE);
   elm_list_go(l_widget);
}

static void
_status_config_profiles(Evas_Object *win, Evas_Object *pager)
{
   Evas_Object *li, *bx, *fr_bx1, *fr_bx2, *btn_bx, *fr, *lb, *en, *sp, *pd,
      *bt;
   Eina_List *profs;
   Evas *evas;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   fr_bx1 = elm_box_add(win);
   evas_object_size_hint_weight_set(fr_bx1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr_bx1, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_homogenous_set(fr_bx1, EINA_TRUE);
   evas_object_show(fr_bx1);

   fr_bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(fr_bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr_bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr_bx2);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Available Profiles");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_frame_content_set(fr, fr_bx1);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);

   profs = elm_profile_list_get();

   evas_object_show(li);
   elm_box_pack_end(fr_bx2, li);

   lb = elm_label_add(win);
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, 0.5);

   evas_object_show(lb);

   en = elm_scrolled_entry_add(win);
   elm_scrolled_entry_editable_set(en, EINA_FALSE);
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
   evas_object_size_hint_weight_set(btn_bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn_bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(btn_bx);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _profile_use, li);
   elm_button_label_set(bt, "Use");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(btn_bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _btn_todo, NULL); /* TODO */
   elm_button_label_set(bt, "Delete");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(btn_bx, bt);
   evas_object_show(bt);

   evas_object_data_set(li, "prof_del_btn", bt);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _profile_reset, li);
   elm_button_label_set(bt, "Reset");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(btn_bx, bt);
   evas_object_show(bt);

   evas_object_data_set(li, "prof_reset_btn", bt);

   bt = elm_button_add(win);
   evas_object_smart_callback_add(bt, "clicked", _btn_todo, NULL); /* TODO */
   elm_button_label_set(bt, "Add new");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(btn_bx, bt);
   evas_object_show(bt);

   elm_frame_content_set(pd, btn_bx);

   evas = evas_object_evas_get(li);
   evas_event_freeze(evas);
   edje_freeze();

   _profiles_list_fill(li, profs);

   elm_profile_list_free(profs);

   edje_thaw();
   evas_event_thaw(evas);

   evas_object_data_set(win, "profiles", bx);
   elm_pager_content_push(pager, bx);
}

static void
_status_config_scrolling(Evas_Object *win, Evas_Object *pager)
{
   _unimplemented(win, pager, "scrolling");
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
_engines_list_fill(Evas_Object *l_widget, Eina_List *e_names)
{
   const char *engine, *cur_engine;
   void *sel_it = NULL;
   Eina_List *l;

   if (!e_names)
     return;

   cur_engine = elm_engine_current_get();

   EINA_LIST_FOREACH(e_names, l, engine)
     {
        const char *label;
        Elm_List_Item *it;

        if (!_elm_engine_supported(engine))
          continue;

        label = _engine_name_prettify(engine);

        it = elm_list_item_append(l_widget, label, NULL, NULL, NULL,
                                  strdup(engine));
        elm_list_item_del_cb_set(it, _engines_list_item_del_cb);
        free((void *)label);

        if (!strcmp(cur_engine, engine))
          sel_it = it;
     }

   if (sel_it) elm_list_item_selected_set(sel_it, EINA_TRUE);
   elm_list_go(l_widget);
}

static void
_status_config_rendering(Evas_Object *win, Evas_Object *pager)
{
   Evas_Object *li, *bx, *fr, *sp, *pd, *bt;
   Eina_List *engines;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Available Engines");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   li = elm_list_add(win);
   elm_frame_content_set(fr, li);
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
   elm_button_label_set(bt, "Use Engine");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_frame_content_set(pd, bt);
   evas_object_show(bt);

   evas_object_data_set(win, "rendering", bx);
   elm_pager_content_push(pager, bx);
}

static void
_status_config_caches(Evas_Object *win, Evas_Object *pager)
{
   _unimplemented(win, pager, "caches");
}

static void
_status_config_full(Evas_Object *win, Evas_Object *bx0)
{
   Evas_Object *tb, *pager;
   Elm_Toolbar_Item *sizing, *it;

   tb = elm_toolbar_add(win);
   elm_toolbar_menu_parent_set(tb, win);
   elm_toolbar_homogenous_set(tb, 0);
   evas_object_size_hint_weight_set(tb, 1.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   sizing = elm_toolbar_item_append(tb, "zoom-fit-best", "Sizing",
                                    _cf_sizing, win);
   elm_toolbar_item_priority_set(sizing, 100);

   it = elm_toolbar_item_append(tb, "preferences-desktop-theme", "Theme",
                                _cf_themes, win);
   elm_toolbar_item_priority_set(it, 90);

   elm_toolbar_item_append(tb, "preferences-desktop-font", "Fonts",
                           _cf_fonts, win);

   it = elm_toolbar_item_append(tb, "system-users", "Profiles",
                                _cf_profiles, win);
   elm_toolbar_item_priority_set(it, 90);

   elm_toolbar_item_append(tb, "system-run", "Scrolling", _cf_scrolling, win);
   elm_toolbar_item_append(tb, "video-display", "Rendering",
                           _cf_rendering, win);
   elm_toolbar_item_append(tb, "appointment-new", "Caches", _cf_caches, win);

   elm_box_pack_end(bx0, tb);
   evas_object_show(tb);

   pager = elm_pager_add(win);
   evas_object_size_hint_align_set(pager, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(pager, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_data_set(win, "pager", pager);

   _status_config_themes(win, pager);
   _status_config_fonts(win, pager);
   _status_config_profiles(win, pager);
   _status_config_rendering(win, pager);
   _status_config_scrolling(win, pager);
   _status_config_caches(win, pager);
   _status_config_sizing(win, pager);

   // FIXME uncomment after flip style fix, please
   //elm_object_style_set(pager, "flip");
   elm_toolbar_item_selected_set(sizing, EINA_TRUE);
   elm_box_pack_end(bx0, pager);
   evas_object_show(pager);
}

static void
status_win(void)
{
   Evas_Object *win, *bg, *bx0;

   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_title_set(win, "Elementary Config");

   evas_object_smart_callback_add(win, "delete,request", my_win_del, NULL);
   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx0 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx0, 1.0, 1.0);
   elm_win_resize_object_add(win, bx0);
   evas_object_show(bx0);

   if (!interactive) _status_basic(win, bx0);
   else if (0) _status_config(win, bx0);
   else _status_config_full(win, bx0);

   evas_object_show(win);
}

static Eina_Bool
_exit_timer(void *data __UNUSED__)
{
   elm_exit();
   return ECORE_CALLBACK_CANCEL;
}

/* this is your elementary main function - it MUST be called IMMEDIATELY
 * after elm_init() and MUST be passed argc and argv, and MUST be called
 * elm_main and not be static - must be a visible symbol with EAPI infront */
EAPI int
elm_main(int argc, char **argv)
{
   int i;

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
        status_win(); /* create main window */
        if (!interactive)
          ecore_timer_add(2.0, _exit_timer, NULL);
     }
   if (!interactive)
     {
        if (theme_set)
          {
             elm_theme_all_set(theme_set);
          }
        if (finger_size_set)
          {
             elm_finger_size_all_set(atoi(finger_size_set));
          }
        if (scale_set)
          {
             elm_scale_all_set(atof(scale_set));
          }
        if (quiet)
          {
             elm_exit();
          }
     }
   elm_run(); /* and run the program now and handle all events, etc. */
   /* if the mainloop that elm_run() runs exists, we exit the app */
   elm_shutdown(); /* clean up and shut down */
   /* exit code */
   return 0;
}
#endif
/* All emelentary apps should use this. Put it right after elm_main() */
ELM_MAIN()
