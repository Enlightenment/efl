#include <Elementary.h>
#include <Elementary_Cursor.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct _Testitem
{
   Elm_Gengrid_Item *item;
   const char *path;
   int mode;
   int onoff;
} Testitem;

static Elm_Gengrid_Item_Class gic;

char *
grd_lbl_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   const Testitem *ti = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Photo %s", ti->path);
   return strdup(buf);
}

Evas_Object *
grd_icon_get(void *data, Evas_Object *obj, const char *part)
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
glt_exp(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
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

   elm_genlist_item_cursor_set(it1, ELM_CURSOR_HAND2);
   elm_genlist_item_cursor_set(it2, ELM_CURSOR_HAND2);
   elm_genlist_item_cursor_set(it3, ELM_CURSOR_HAND1);

}

static void
glt_con(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static void
glt_exp_req(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 1);
}

static void
glt_con_req(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 0);
}

char *
glt_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item mode %i", (int)(long)data);
   return strdup(buf);
}

void
test_cursor(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt, *list, *entry, *ck;
   Elm_List_Item *lit;

   win = elm_win_add(NULL, "cursor", ELM_WIN_BASIC);
   elm_win_title_set(win, "Cursor");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
   elm_button_label_set(bt, "Coffee Mug");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_cursor_set(bt, ELM_CURSOR_CLOCK);
   elm_button_label_set(bt, "Cursor unset");
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
   elm_list_item_cursor_set(lit, ELM_CURSOR_HAND1);
   lit = elm_list_item_append(list, "watch over list | hand2", NULL, NULL, NULL,
                        NULL);
   elm_list_item_cursor_set(lit, ELM_CURSOR_HAND2);
   elm_list_go(list);
   evas_object_show(list);

   entry = elm_entry_add(win);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   elm_entry_entry_set(entry, "Xterm cursor");
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
test_cursor2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *o, *grid, *gl;
   Elm_Genlist_Item *it1, *it2, *it3;
   Elm_Toolbar_Item *tit;
   Elm_List_Item *lit;
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

   win = elm_win_add(NULL, "cursor2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Cursor 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   o = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(o, 0);
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.0);
   tit = elm_toolbar_item_append(o, NULL, "Bogosity", NULL, NULL);
   elm_toolbar_item_cursor_set(tit, ELM_CURSOR_BOGOSITY);
   tit = elm_toolbar_item_append(o, NULL, "Unset", NULL, NULL);
   elm_toolbar_item_cursor_set(tit, ELM_CURSOR_BOGOSITY);
   elm_toolbar_item_cursor_unset(tit);
   tit = elm_toolbar_item_append(o, NULL, "Xterm", NULL, NULL);
   elm_toolbar_item_cursor_set(tit, ELM_CURSOR_XTERM);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_list_add(win);
   elm_box_pack_end(bx, o);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   lit = elm_list_item_append(o, "cursor bogosity", NULL, NULL,  NULL, NULL);
   elm_list_item_cursor_set(lit, ELM_CURSOR_BOGOSITY);
   lit = elm_list_item_append(o, "cursor unset", NULL, NULL,  NULL, NULL);
   elm_list_item_cursor_set(lit, ELM_CURSOR_BOGOSITY);
   elm_list_item_cursor_unset(lit);
   lit = elm_list_item_append(o, "cursor xterm", NULL, NULL,  NULL, NULL);
   elm_list_item_cursor_set(lit, ELM_CURSOR_XTERM);
   elm_list_go(o);
   evas_object_show(o);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gl);

   itct.item_style     = "default";
   itct.func.label_get = glt_label_get;

   it1 = elm_genlist_item_append(gl, &itct, (void *) 1, NULL,
                                 ELM_GENLIST_ITEM_SUBITEMS, NULL, NULL);
   it2 = elm_genlist_item_append(gl, &itct, (void *) 2, NULL,
                                 ELM_GENLIST_ITEM_SUBITEMS, NULL, NULL);
   it3 = elm_genlist_item_append(gl, &itct, (void *) 3, NULL,
                                 ELM_GENLIST_ITEM_NONE, NULL, NULL);

   elm_genlist_item_cursor_set(it1, ELM_CURSOR_HAND1);
   elm_genlist_item_cursor_set(it2, ELM_CURSOR_HAND1);
   elm_genlist_item_cursor_set(it3, ELM_CURSOR_CROSS);

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
   gic.func.label_get = grd_lbl_get;
   gic.func.icon_get = grd_icon_get;

   n = 0;
   for (i = 0; i < 3 * 3; i++)
     {
        snprintf(buf, sizeof(buf), "%s/images/%s", PACKAGE_DATA_DIR, img[n]);
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
test_cursor3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *o;
   Elm_List_Item *lit;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "cursor3", ELM_WIN_BASIC);
   elm_win_title_set(win, "Cursor 3");
   elm_win_autodel_set(win, 1);

   snprintf(buf, sizeof(buf), "%s/objects/cursors.edj", PACKAGE_DATA_DIR);
   elm_theme_extension_add(NULL, buf);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   o = elm_button_add(win);
   elm_object_cursor_set(o, ELM_CURSOR_HAND1);
   elm_object_cursor_engine_only_set(o, EINA_FALSE);
   elm_button_label_set(o, "hand1");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, ELM_CURSOR_HAND2);
   elm_button_label_set(o, "hand2 x");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, ELM_CURSOR_HAND2);
   elm_object_cursor_engine_only_set(o, EINA_FALSE);
   elm_button_label_set(o, "hand2");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand3");
   elm_object_cursor_engine_only_set(o, EINA_FALSE);
   elm_button_label_set(o, "hand3");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand3");
   elm_object_cursor_engine_only_set(o, EINA_FALSE);
   elm_object_cursor_style_set(o, "transparent");
   elm_button_label_set(o, "hand3 transparent");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand3");
   elm_object_cursor_engine_only_set(o, EINA_FALSE);
   elm_object_cursor_unset(o);
   elm_button_label_set(o, "unset");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand4");
   elm_button_label_set(o, "not existent");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   elm_cursor_engine_only_set(0);
   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand2");
   elm_button_label_set(o, "hand 2 engine only config false");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   elm_cursor_engine_only_set(1);
   o = elm_button_add(win);
   elm_object_cursor_set(o, "hand2");
   elm_button_label_set(o, "hand 2 engine only config true");
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_list_add(win);
   elm_box_pack_end(bx, o);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   lit = elm_list_item_append(o, "cursor hand2 x", NULL, NULL,  NULL, NULL);
   elm_list_item_cursor_set(lit, ELM_CURSOR_HAND2);
   lit = elm_list_item_append(o, "cursor hand2", NULL, NULL,  NULL, NULL);
   elm_list_item_cursor_set(lit, ELM_CURSOR_HAND2);
   elm_list_item_cursor_engine_only_set(lit, EINA_FALSE);
   lit = elm_list_item_append(o, "cursor hand3", NULL, NULL,  NULL, NULL);
   elm_list_item_cursor_set(lit, "hand3");
   elm_list_item_cursor_engine_only_set(lit, EINA_FALSE);
   lit = elm_list_item_append(o, "cursor hand3 transparent", NULL, NULL,
                              NULL, NULL);
   elm_list_item_cursor_set(lit, "hand3");
   elm_list_item_cursor_style_set(lit, "transparent");
   elm_list_item_cursor_engine_only_set(lit, EINA_FALSE);
   elm_list_go(o);
   evas_object_show(o);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

#endif
