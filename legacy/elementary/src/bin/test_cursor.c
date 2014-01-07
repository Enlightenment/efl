#include <Elementary_Cursor.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

// XXX: show all type of cursors in the elementary_test. this needs to be the first test

typedef struct _Testitem
{
   Elm_Object_Item *item;
   const char *path;
   int mode;
   int onoff;
} Testitem;

static Elm_Gengrid_Item_Class gic;

char *
grd_lbl_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   const Testitem *ti = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Photo %s", ti->path);
   return strdup(buf);
}

Evas_Object *
grd_content_get(void *data, Evas_Object *obj, const char *part)
{
   const Testitem *ti = data;
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *icon = elm_bg_add(obj);
        elm_bg_file_set(icon, ti->path, NULL);
        evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL,
                                         1, 1);
        evas_object_show(icon);
        return icon;
     }
   return NULL;
}

static Elm_Genlist_Item_Class itct;

static void
glt_exp(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   Evas_Object *gl = elm_object_item_widget_get(glit);
   int val = (int)(uintptr_t) elm_object_item_data_get(glit);
   Elm_Object_Item *glit1, *glit2, *glit3;

   val *= 10;
   glit1 = elm_genlist_item_append(gl, &itct, (void *)(uintptr_t) (val + 1), glit,
                                   ELM_GENLIST_ITEM_NONE, NULL, NULL);
   glit2 = elm_genlist_item_append(gl, &itct, (void *)(uintptr_t) (val + 2), glit,
                                   ELM_GENLIST_ITEM_NONE, NULL, NULL);
   glit3 = elm_genlist_item_append(gl, &itct, (void *)(uintptr_t) (val + 3), glit,
                                   ELM_GENLIST_ITEM_TREE, NULL, NULL);

   elm_genlist_item_cursor_set(glit1, ELM_CURSOR_HAND2);
   elm_genlist_item_cursor_set(glit2, ELM_CURSOR_HAND2);
   elm_genlist_item_cursor_set(glit3, ELM_CURSOR_HAND1);

}

static void
glt_con(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   elm_genlist_item_subitems_clear(glit);
}

static void
glt_exp_req(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   elm_genlist_item_expanded_set(glit, EINA_TRUE);
}

static void
glt_con_req(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   elm_genlist_item_expanded_set(glit, EINA_FALSE);
}

char *
glt_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item mode %i", (int)(uintptr_t)data);
   return strdup(buf);
}

void
test_cursor(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *bx, *bt, *list, *entry, *ck;
   Elm_Object_Item *lit;

   win = elm_win_add(NULL, "cursor", ELM_WIN_BASIC);
   elm_win_title_set(win, "Cursor");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);
   elm_object_cursor_set(bg, ELM_CURSOR_CIRCLE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ck = elm_clock_add(win);
   elm_object_cursor_set(ck, ELM_CURSOR_CLOCK);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   bt = elm_button_add(win);
   elm_object_cursor_set(bt, ELM_CURSOR_COFFEE_MUG);
   elm_object_text_set(bt, "Coffee Mug");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_cursor_set(bt, ELM_CURSOR_CLOCK);
   elm_object_text_set(bt, "Cursor unset");
   elm_object_cursor_unset(bt);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   list = elm_list_add(win);
   elm_box_pack_end(bx, list);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_cursor_set(list, ELM_CURSOR_WATCH);
   lit = elm_list_item_append(list, "watch over list | hand1", NULL, NULL, NULL,
                        NULL);
   elm_object_item_cursor_set(lit, ELM_CURSOR_HAND1);
   lit = elm_list_item_append(list, "watch over list | hand2", NULL, NULL, NULL,
                        NULL);
   elm_object_item_cursor_set(lit, ELM_CURSOR_HAND2);
   elm_list_go(list);
   evas_object_show(list);

   entry = elm_entry_add(win);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   elm_object_text_set(entry, "Xterm cursor");
   elm_entry_single_line_set(entry, EINA_TRUE);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, entry);
   evas_object_show(entry);
   elm_object_cursor_set(entry, ELM_CURSOR_XTERM);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

void
test_cursor2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *o, *grid, *gl;
   Elm_Object_Item *glit1, *glit2, *glit3;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *lit;
   char buf[PATH_MAX];
   static Testitem ti[144];
   int i, n;
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

   win = elm_win_util_standard_add("cursor2", "Cursor 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   o = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(o, EINA_FALSE);
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.0);
   tb_it = elm_toolbar_item_append(o, NULL, "Bogosity", NULL, NULL);
   elm_object_item_cursor_set(tb_it, ELM_CURSOR_BOGOSITY);
   tb_it = elm_toolbar_item_append(o, NULL, "Unset", NULL, NULL);
   elm_object_item_cursor_set(tb_it, ELM_CURSOR_BOGOSITY);
   elm_object_item_cursor_unset(tb_it);
   tb_it = elm_toolbar_item_append(o, NULL, "Xterm", NULL, NULL);
   elm_object_item_cursor_set(tb_it, ELM_CURSOR_XTERM);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_list_add(win);
   elm_box_pack_end(bx, o);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   lit = elm_list_item_append(o, "cursor bogosity", NULL, NULL,  NULL, NULL);
   elm_object_item_cursor_set(lit, ELM_CURSOR_BOGOSITY);
   lit = elm_list_item_append(o, "cursor unset", NULL, NULL,  NULL, NULL);
   elm_object_item_cursor_set(lit, ELM_CURSOR_BOGOSITY);
   elm_object_item_cursor_unset(lit);
   lit = elm_list_item_append(o, "cursor xterm", NULL, NULL,  NULL, NULL);
   elm_object_item_cursor_set(lit, ELM_CURSOR_XTERM);
   elm_list_go(o);
   evas_object_show(o);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gl);

   itct.item_style     = "default";
   itct.func.text_get = glt_text_get;

   glit1 = elm_genlist_item_append(gl, &itct, (void *) 1, NULL,
                                   ELM_GENLIST_ITEM_TREE, NULL, NULL);
   glit2 = elm_genlist_item_append(gl, &itct, (void *) 2, NULL,
                                   ELM_GENLIST_ITEM_TREE, NULL, NULL);
   glit3 = elm_genlist_item_append(gl, &itct, (void *) 3, NULL,
                                   ELM_GENLIST_ITEM_NONE, NULL, NULL);

   elm_genlist_item_cursor_set(glit1, ELM_CURSOR_HAND1);
   elm_genlist_item_cursor_set(glit2, ELM_CURSOR_HAND1);
   elm_genlist_item_cursor_set(glit3, ELM_CURSOR_CROSS);

   evas_object_smart_callback_add(gl, "expand,request", glt_exp_req, gl);
   evas_object_smart_callback_add(gl, "contract,request", glt_con_req, gl);
   evas_object_smart_callback_add(gl, "expanded", glt_exp, gl);
   evas_object_smart_callback_add(gl, "contracted", glt_con, gl);

   elm_box_pack_end(bx, gl);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 130, 130);
   elm_gengrid_horizontal_set(grid, EINA_FALSE);
   elm_gengrid_multi_select_set(grid, EINA_TRUE);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);

   gic.item_style = "default";
   gic.func.text_get = grd_lbl_get;
   gic.func.content_get = grd_content_get;

   n = 0;
   for (i = 0; i < 3 * 3; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
        n = (n + 1) % 9;
        ti[i].mode = i;
        ti[i].path = eina_stringshare_add(buf);
        ti[i].item = elm_gengrid_item_append(grid, &gic, &(ti[i]), NULL, NULL);
        if (n % 2)
           elm_gengrid_item_cursor_set(ti[i].item, ELM_CURSOR_HAND1);
        else
           elm_gengrid_item_cursor_set(ti[i].item, ELM_CURSOR_CLOCK);
        if (!(i % 5))
           elm_gengrid_item_selected_set(ti[i].item, EINA_TRUE);
     }
   elm_box_pack_end(bx, grid);
   evas_object_show(grid);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

void
test_cursor3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *o;
   Elm_Object_Item *lit;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("cursor3", "Cursor 3");
   elm_win_autodel_set(win, EINA_TRUE);

   snprintf(buf, sizeof(buf), "%s/objects/cursors.edj", elm_app_data_dir_get());
   elm_theme_extension_add(NULL, buf);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   o = elm_button_add(win);
   elm_object_cursor_set(o, ELM_CURSOR_HAND1);
   elm_object_cursor_theme_search_enabled_set(o, EINA_TRUE);
   elm_object_text_set(o, "hand1");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, ELM_CURSOR_HAND2);
   elm_object_text_set(o, "hand2 x");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, ELM_CURSOR_HAND2);
   elm_object_cursor_theme_search_enabled_set(o, EINA_TRUE);
   elm_object_text_set(o, "hand2");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand3");
   elm_object_cursor_theme_search_enabled_set(o, EINA_TRUE);
   elm_object_text_set(o, "hand3");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand3");
   elm_object_cursor_theme_search_enabled_set(o, EINA_TRUE);
   elm_object_cursor_style_set(o, "transparent");
   elm_object_text_set(o, "hand3 transparent");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand3");
   elm_object_cursor_theme_search_enabled_set(o, EINA_TRUE);
   elm_object_cursor_unset(o);
   elm_object_text_set(o, "unset");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand4");
   elm_object_text_set(o, "not existent");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   elm_config_cursor_engine_only_set(EINA_FALSE);
   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand2");
   elm_object_text_set(o, "hand 2 engine only config false");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   elm_config_cursor_engine_only_set(EINA_TRUE);
   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand2");
   elm_object_text_set(o, "hand 2 engine only config true");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_list_add(win);
   elm_box_pack_end(bx, o);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   lit = elm_list_item_append(o, "cursor hand2 x", NULL, NULL,  NULL, NULL);
   elm_object_item_cursor_set(lit, ELM_CURSOR_HAND2);
   lit = elm_list_item_append(o, "cursor hand2", NULL, NULL,  NULL, NULL);
   elm_object_item_cursor_set(lit, ELM_CURSOR_HAND2);
   elm_object_item_cursor_engine_only_set(lit, EINA_FALSE);
   lit = elm_list_item_append(o, "cursor hand3", NULL, NULL,  NULL, NULL);
   elm_object_item_cursor_set(lit, "hand3");
   elm_object_item_cursor_engine_only_set(lit, EINA_FALSE);
   lit = elm_list_item_append(o, "cursor hand3 transparent", NULL, NULL,
                              NULL, NULL);
   elm_object_item_cursor_set(lit, "hand3");
   elm_object_item_cursor_style_set(lit, "transparent");
   elm_object_item_cursor_engine_only_set(lit, EINA_FALSE);
   elm_list_go(o);
   evas_object_show(o);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

void
test_cursor4(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *ly, *bt;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("cursor layout", "Cursor Layout");
   elm_win_autodel_set(win, EINA_TRUE);

   snprintf(buf, sizeof(buf), "%s/objects/cursors.edj", elm_app_data_dir_get());

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ly = elm_layout_add(win);
   elm_layout_file_set(ly, buf, "test/layout/events");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_cursor_set(ly, ELM_CURSOR_HAND1);
   elm_box_pack_end(bx, ly);
   evas_object_show(ly);

   ly = elm_layout_add(win);
   elm_layout_file_set(ly, buf, "test/layout/noevents");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_cursor_set(ly, ELM_CURSOR_XTERM);
   elm_layout_part_cursor_set(ly, "top-left", ELM_CURSOR_CROSS);
   elm_layout_part_cursor_set(ly, "bottom-left", ELM_CURSOR_PIRATE);
   elm_box_pack_end(bx, ly);
   evas_object_show(ly);

   ly = elm_layout_add(win);
   elm_layout_file_set(ly, buf, "test/layout/parts2");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_part_cursor_set(ly, "top-left", ELM_CURSOR_CROSS);
   elm_layout_part_cursor_set(ly, "bottom-left", ELM_CURSOR_PIRATE);
   elm_box_pack_end(bx, ly);
   evas_object_show(ly);

   ly = elm_layout_add(win);
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_file_set(ly, buf, "test/layout/swallow");
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Cursor over obj");
   elm_object_part_content_set(ly, "swallow1", bt);
   elm_object_cursor_set(bt, ELM_CURSOR_PIRATE);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Cursor over part");
   elm_object_part_content_set(ly, "swallow2", bt);
   elm_layout_part_cursor_set(ly, "swallow2", ELM_CURSOR_PIRATE);
   elm_box_pack_end(bx, ly);
   evas_object_show(ly);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
