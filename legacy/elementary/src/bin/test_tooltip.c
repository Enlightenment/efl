#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct _Testitem
{
   Elm_Gengrid_Item *item;
   const char       *path;
   int               mode;
   int               onoff;
} Testitem;

static Elm_Gengrid_Item_Class gic;

char *
grdt_lbl_get(void            *data,
             Evas_Object *obj __UNUSED__,
             const char *part __UNUSED__)
{
   const Testitem *ti = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Photo %s", ti->path);
   return strdup(buf);
}

Evas_Object *
grdt_icon_get(void        *data,
              Evas_Object *obj,
              const char  *part)
{
   const Testitem *ti = data;
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *icon = elm_bg_add(obj);
        elm_bg_file_set(icon, ti->path, NULL);
        evas_object_size_hint_aspect_set(icon,
                                         EVAS_ASPECT_CONTROL_VERTICAL,
                                         1, 1);
        evas_object_show(icon);
        return icon;
     }
   return NULL;
}

static Elm_Genlist_Item_Class itct;

static void
gltt_exp(void *data       __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void            *event_info)
{
   Elm_Genlist_Item *it = event_info;
   Evas_Object *gl = elm_genlist_item_genlist_get(it);
   int val = (int)(long)elm_genlist_item_data_get(it);
   Elm_Genlist_Item *it1, *it2, *it3;

   val *= 10;
   it1 = elm_genlist_item_append(gl, &itct, (void *)(long)(val + 1), it,
                                 ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it2 = elm_genlist_item_append(gl, &itct, (void *)(long)(val + 2), it,
                                 ELM_GENLIST_ITEM_NONE, NULL, NULL);
   it3 = elm_genlist_item_append(gl, &itct, (void *)(long)(val + 3), it,
                                 ELM_GENLIST_ITEM_SUBITEMS, NULL, NULL);

   elm_genlist_item_tooltip_text_set(it1, "Testing A");
   elm_genlist_item_tooltip_text_set(it2, "Testing B");
   elm_genlist_item_tooltip_text_set(it3, "Testing C");
}

static void
gltt_con(void *data       __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void            *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static void
gltt_exp_req(void *data       __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void            *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 1);
}

static void
gltt_con_req(void *data       __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void            *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 0);
}

char *
gltt_label_get(void            *data,
               Evas_Object *obj __UNUSED__,
               const char *part __UNUSED__)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item mode %i", (int)(long)data);
   return strdup(buf);
}

static Evas_Object *
_tt_item_icon(void *data   __UNUSED__,
              Evas_Object *obj __UNUSED__,
              Evas_Object *tt,
              void *item   __UNUSED__)
{
   Evas_Object *ic = elm_icon_add(tt);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_resize(ic, 64, 64);
   return ic;
}

static Evas_Object *
_tt_item_icon2(void *data   __UNUSED__,
              Evas_Object *obj __UNUSED__,
              Evas_Object *tt,
              void *item   __UNUSED__)
{
   Evas_Object *ic = elm_icon_add(tt);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   return ic;
}

static Evas_Object *
_tt_item_icon3(void *data   __UNUSED__,
              Evas_Object *obj __UNUSED__,
              Evas_Object *tt,
              void *item   __UNUSED__)
{
   Evas_Object *ic = elm_icon_add(tt);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/insanely_huge_test_image.jpg", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   return ic;
}

static Evas_Object *
_tt_item_label(void *data   __UNUSED__,
              Evas_Object *obj __UNUSED__,
              Evas_Object *tt,
              void *item   __UNUSED__)
{
   Evas_Object *l = elm_label_add(tt);
   elm_object_text_set(l, "Something useful here?<ps>"
                          "No probably not, but this is a super long label<ps>"
                          "which probably breaks on your system, now doesn't it?<ps>"
                          "Yeah, I thought so.");
   elm_label_line_wrap_set(l, ELM_WRAP_MIXED);
   return l;
}

static void
_tt_item_icon_del(void            *data,
                  Evas_Object *obj __UNUSED__,
                  void            *event_info)
{
   // test to check for del_cb behavior!
   printf("_tt_icon_del: data=%ld (== 456?), event_info=%p\n",
          (long)data, event_info);
}

static void
_tt_text_replace(void *data       __UNUSED__,
                 Evas_Object     *obj,
                 void *event_info __UNUSED__)
{
   static int count = 0;
   char buf[64];
   snprintf(buf, sizeof(buf), "count=%d", count);
   count++;
   elm_object_tooltip_text_set(obj, buf);
}

static void
_tt_timer_del(void *data       __UNUSED__,
              Evas *e          __UNUSED__,
              Evas_Object     *obj,
              void *event_info __UNUSED__)
{
   Ecore_Timer *timer = evas_object_data_del(obj, "test-timer");
   if (!timer) return;
   ecore_timer_del(timer);
}

static Eina_Bool
_tt_text_replace_timer_cb(void *data)
{
   _tt_text_replace(NULL, data, NULL);
   return EINA_TRUE;
}

static void
_tt_text_replace_timed(void *data       __UNUSED__,
                       Evas_Object     *obj,
                       void *event_info __UNUSED__)
{
   Ecore_Timer *timer = evas_object_data_get(obj, "test-timer");
   if (timer)
     {
        ecore_timer_del(timer);
        evas_object_data_del(obj, "test-timer");
        elm_object_text_set(obj, "Simple text tooltip, click to start"
                                  " changed timed");
        return;
     }

   timer = ecore_timer_add(1.5, _tt_text_replace_timer_cb, obj);
   evas_object_data_set(obj, "test-timer", timer);
   elm_object_text_set(obj, "Simple text tooltip, click to stop changed"
                             " timed");
}

static Evas_Object *
_tt_icon(void *data   __UNUSED__,
         Evas_Object *obj __UNUSED__,
         Evas_Object *tt)
{
   Evas_Object *ic = elm_icon_add(tt);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_resize(ic, 64, 64);
   return ic;
}

static Evas_Object *
_tt_icon2(void *data   __UNUSED__,
          Evas_Object *obj __UNUSED__,
          Evas_Object *tt)
{
   Evas_Object *ic = elm_icon_add(tt);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/icon_00.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_resize(ic, 64, 64);
   return ic;
}

static void
_tt_icon_del(void            *data,
             Evas_Object *obj __UNUSED__,
             void            *event_info)
{
   // test to check for del_cb behavior!
   printf("_tt_icon_del: data=%ld (== 123?), event_info=%p\n",
          (long)data, event_info);
}

static Eina_Bool
_tt_icon_replace_timer_cb(void *data)
{
   static int current = 0;

   elm_object_tooltip_content_cb_set
     (data, current ? _tt_icon2 : _tt_icon, NULL, NULL);

   current = !current;
   return EINA_TRUE;
}

static void
_tt_icon_replace_timed(void *data       __UNUSED__,
                       Evas_Object     *obj,
                       void *event_info __UNUSED__)
{
   Ecore_Timer *timer = evas_object_data_get(obj, "test-timer");
   if (timer)
     {
        ecore_timer_del(timer);
        evas_object_data_del(obj, "test-timer");
        elm_object_text_set(obj, "Icon tooltip, click to start changed"
                                  " timed");
        return;
     }

   timer = ecore_timer_add(1.5, _tt_icon_replace_timer_cb, obj);
   evas_object_data_set(obj, "test-timer", timer);
   elm_object_text_set(obj, "Icon tooltip, click to stop changed timed");
}

static Eina_Bool
_tt_style_replace_timer_cb(void *data)
{
   static int current = 0;
   elm_object_tooltip_style_set(data, current ? NULL : "transparent");
   current = !current;
   return EINA_TRUE;
}

static void
_tt_style_replace_timed(void *data       __UNUSED__,
                        Evas_Object     *obj,
                        void *event_info __UNUSED__)
{
   Ecore_Timer *timer = evas_object_data_get(obj, "test-timer");
   if (timer)
     {
        ecore_timer_del(timer);
        evas_object_data_del(obj, "test-timer");
        elm_object_text_set(obj, "Icon tooltip style, click to start"
                                  " changed timed");
        return;
     }

   timer = ecore_timer_add(1.5, _tt_style_replace_timer_cb, obj);
   evas_object_data_set(obj, "test-timer", timer);
   elm_object_text_set(obj, "Icon tooltip style, click to stop changed"
                             " timed");
}

static void
_tt_visible_lock_toggle(void *data       __UNUSED__,
                        Evas_Object     *obj,
                        void *event_info __UNUSED__)
{
   static int locked = 0;

   locked = !locked;
   if (locked)
     {
        elm_object_text_set(obj, "Locked tooltip visibility");
        elm_object_tooltip_text_set(obj, "This tooltip is locked"
                                         " visible,<br> click the button"
                                         " to unlock!");
        elm_object_tooltip_show(obj);
     }
   else
     {
        elm_object_text_set(obj, "Unlocked tooltip visibility");
        elm_object_tooltip_text_set(obj, "This tooltip is unlocked"
                                         " visible,<br> click the button"
                                         " to lock!");
        elm_object_tooltip_hide(obj);
     }
}

void
test_tooltip(void *data       __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *tb, *bt, *se, *lst;
   Elm_Toolbar_Item *ti;
   Elm_List_Item *li;

   win = elm_win_add(NULL, "tooltip", ELM_WIN_BASIC);
   elm_win_title_set(win, "Tooltip");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, 0);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   ti = elm_toolbar_item_append(tb, NULL, "Open", NULL, NULL);
   elm_toolbar_item_tooltip_text_set(ti, "Opens a file");

   ti = elm_toolbar_item_append(tb, NULL, "Icon", NULL, NULL);
   elm_toolbar_item_tooltip_content_cb_set
     (ti, _tt_item_icon, (void *)456L, _tt_item_icon_del);
   elm_toolbar_item_tooltip_style_set(ti, "transparent");

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Simple text tooltip");
   elm_object_tooltip_text_set(bt, "Simple text tooltip");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Simple text tooltip, click to change");
   elm_object_tooltip_text_set(bt, "Initial");
   evas_object_smart_callback_add(bt, "clicked", _tt_text_replace, NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Simple text tooltip, click to start"
                            " changed timed");
   elm_object_tooltip_text_set(bt, "Initial");
   evas_object_smart_callback_add(bt, "clicked", _tt_text_replace_timed,
                                  NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_event_callback_add(bt, EVAS_CALLBACK_DEL, _tt_timer_del,
                                  NULL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Icon tooltip");
   elm_object_tooltip_content_cb_set(bt, _tt_icon, (void *)123L,
                                     _tt_icon_del);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Icon tooltip, click to start changed timed");
   elm_object_tooltip_content_cb_set(bt, _tt_icon, NULL, NULL);
   evas_object_smart_callback_add(bt, "clicked", _tt_icon_replace_timed,
                                  NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_event_callback_add(bt, EVAS_CALLBACK_DEL, _tt_timer_del,
                                  NULL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Transparent Icon tooltip");
   elm_object_tooltip_content_cb_set(bt, _tt_icon, NULL, NULL);
   elm_object_tooltip_style_set(bt, "transparent");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Icon tooltip style, click to start changed"
                            " timed");
   elm_object_tooltip_content_cb_set(bt, _tt_icon, NULL, NULL);
   evas_object_smart_callback_add(bt, "clicked", _tt_style_replace_timed,
                                  NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_event_callback_add(bt, EVAS_CALLBACK_DEL, _tt_timer_del,
                                  NULL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Unlocked tooltip visibility");
   elm_object_tooltip_text_set(bt, "This tooltip is unlocked visible,<br>"
                                   " click the button to lock!");
   evas_object_smart_callback_add(bt, "clicked", _tt_visible_lock_toggle,
                                  NULL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   se = elm_entry_add(win);
   elm_entry_scrollable_set(se, EINA_TRUE);
   evas_object_size_hint_weight_set(se, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(se, EVAS_HINT_FILL, 0.5);
   elm_entry_scrollbar_policy_set(se, ELM_SCROLLER_POLICY_OFF,
                                           ELM_SCROLLER_POLICY_OFF);
   elm_entry_entry_set(se, "Hello, some scrolled entry here!");
   elm_object_tooltip_text_set(se, "Type something here!");
   elm_entry_single_line_set(se, 1);
   elm_box_pack_end(bx, se);
   evas_object_show(se);

   lst = elm_list_add(win);
   li = elm_list_item_append(lst, "Hello", NULL, NULL, NULL, NULL);
   elm_list_item_tooltip_content_cb_set(li, _tt_item_label, NULL, NULL);
   elm_list_item_tooltip_size_restrict_disable(li, EINA_TRUE);
   li = elm_list_item_append(lst, "Icon Tooltip", NULL, NULL, NULL, NULL);
   elm_list_item_tooltip_content_cb_set(li, _tt_item_icon, NULL, NULL);
   li = elm_list_item_append(lst, "Big Icon Tooltip", NULL, NULL, NULL, NULL);
   elm_list_item_tooltip_content_cb_set(li, _tt_item_icon2, NULL, NULL);
   elm_list_item_tooltip_style_set(li, "transparent");
   elm_list_item_tooltip_size_restrict_disable(li, EINA_TRUE);
   li = elm_list_item_append(lst, "Insanely Big Icon Tooltip", NULL, NULL, NULL, NULL);
   elm_list_item_tooltip_content_cb_set(li, _tt_item_icon3, NULL, NULL);
   elm_list_item_tooltip_style_set(li, "transparent");
   elm_list_item_tooltip_size_restrict_disable(li, EINA_TRUE);
   evas_object_size_hint_weight_set(lst, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(lst, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(lst, 100, 100);
   elm_list_go(lst);
   elm_box_pack_end(bx, lst);
   evas_object_show(lst);

   evas_object_resize(win, 320, 580);
   evas_object_show(win);
}

void
test_tooltip2(void *data       __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *grid, *gl;
   Elm_Genlist_Item *it1, *it2, *it3;
   static Testitem ti[144];
   int i, n;
   char buf[PATH_MAX];
   const char *img[9] =
   {
      "panel_01.jpg",
      "plant_01.jpg",
      "rock_01.jpg",
      "rock_02.jpg",
      "sky_01.jpg",
      "sky_02.jpg",
      "sky_03.jpg",
      "sky_04.jpg",
      "wood_01.jpg",
   };

   win = elm_win_add(NULL, "tooltip2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Tooltip 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 100, 100);
   elm_gengrid_horizontal_set(grid, EINA_FALSE);
   elm_gengrid_multi_select_set(grid, EINA_TRUE);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);

   gic.item_style = "default";
   gic.func.label_get = grdt_lbl_get;
   gic.func.icon_get = grdt_icon_get;

   n = 0;
   for (i = 0; i < 9; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", PACKAGE_DATA_DIR,
                 img[n]);
        n++;
        ti[i].mode = i;
        ti[i].path = eina_stringshare_add(buf);
        ti[i].item = elm_gengrid_item_append(grid, &gic, &(ti[i]), NULL,
                                             NULL);
        if (n % 2)
          elm_gengrid_item_tooltip_text_set(ti[i].item, "Testing X");
        else
          elm_gengrid_item_tooltip_text_set(ti[i].item, "Testing Y");
        if (!(i % 5))
          elm_gengrid_item_selected_set(ti[i].item, EINA_TRUE);
     }

   elm_box_pack_end(bx, grid);
   evas_object_show(grid);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);

   itct.item_style = "default";
   itct.func.label_get = gltt_label_get;

   it1 = elm_genlist_item_append(gl, &itct, (void *)1, NULL,
                                 ELM_GENLIST_ITEM_SUBITEMS, NULL, NULL);
   it2 = elm_genlist_item_append(gl, &itct, (void *)2, NULL,
                                 ELM_GENLIST_ITEM_SUBITEMS, NULL, NULL);
   it3 = elm_genlist_item_append(gl, &itct, (void *)3, NULL,
                                 ELM_GENLIST_ITEM_NONE, NULL, NULL);

   elm_genlist_item_tooltip_text_set(it1, "Testing 1");
   elm_genlist_item_tooltip_text_set(it2, "Testing 2");
   elm_genlist_item_tooltip_text_set(it3, "Testing 3");

   evas_object_smart_callback_add(gl, "expand,request", gltt_exp_req, gl);
   evas_object_smart_callback_add(gl, "contract,request", gltt_con_req,
                                  gl);
   evas_object_smart_callback_add(gl, "expanded", gltt_exp, gl);
   evas_object_smart_callback_add(gl, "contracted", gltt_con, gl);

   elm_box_pack_end(bx, gl);
   evas_object_show(gl);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

#endif
