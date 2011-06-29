#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
typedef struct _Pginfo Pginfo;

struct _Pginfo
{
    Evas_Object *win, *pager, *pg1, *pg2, *pg3, *pg4;
};

static void
my_pager_1(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg2);
}

static void
my_pager_2(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg3);
}

static void
my_pager_3(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg4);
}


static void
my_pager_4(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg1);
}


static void
my_pager_pop(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Pginfo *info = data;
   elm_pager_content_pop(info->pager);
}

struct style_tuple {
     const char *label;
     const char *name;
};
static const struct style_tuple styles[] = {
       { "Default", "default"},
       { "Slide", "slide"},
       { "Slide Invisible", "slide_invisible"},
       { "Fade", "fade"},
       { "Fade Translucide", "fade_translucide"},
       { "Fade Invisible", "fade_invisible"},
       { "Flip", "flip"},
};

static void
_style(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *pg = data;
   elm_object_style_set(pg, styles[elm_radio_value_get(obj)].name);
}

void
test_pager(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *pg, *bx, *lb, *bt;
   static Pginfo info;

   win = elm_win_add(NULL, "pager", ELM_WIN_BASIC);
   elm_win_title_set(win, "Pager");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, 1);
   info.win = win;

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   pg = elm_pager_add(win);
   evas_object_size_hint_weight_set(pg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, pg);
   evas_object_show(pg);

   info.pager = pg;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
		       "This is page 1 in a pager stack.<br>"
		       "<br>"
		       "So what is a pager stack? It is a stack<br>"
		       "of pages that hold widgets in it. The<br>"
		       "pages can be pushed and popped on and<br>"
		       "off the stack, activated and otherwise<br>"
		       "activated if already in the stack<br>"
		       "(activated means promoted to the top of<br>"
		       "the stack).<br>"
		       "<br>"
		       "The theme may define the animation how<br>"
		       "show and hide of pages. Select one theme style:");
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   unsigned int i = 0;
   Evas_Object *rdg = NULL, *rd;
   for (i = 0; i < (sizeof(styles) / sizeof(struct style_tuple)); i++)
     {
        rd = elm_radio_add(win);
        elm_radio_label_set(rd, styles[i].label);
        elm_radio_state_value_set(rd, i);
        if (rdg)
          elm_radio_group_add(rd, rdg);
        else
          rdg = rd;
        evas_object_smart_callback_add(rd, "changed", _style, pg);
        elm_box_pack_end(bx, rd);
        evas_object_show(rd);
     }
   elm_radio_value_set(rd, 0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Flip to 2");
   evas_object_smart_callback_add(bt, "clicked", my_pager_1, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Popme");
   evas_object_smart_callback_add(bt, "clicked", my_pager_pop, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_pager_content_push(pg, bx);
   info.pg1 = bx;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
		       "This is page 2 in a pager stack.<br>"
		       "<br>"
		       "This is just like the previous page in<br>"
		       "the pager stack."
		       );
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Flip to 3");
   evas_object_smart_callback_add(bt, "clicked", my_pager_2, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Popme");
   evas_object_smart_callback_add(bt, "clicked", my_pager_pop, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_pager_content_push(pg, bx);
   info.pg2 = bx;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
		       "This is page 3 in a pager stack.<br>"
		       "<br>"
		       "This is just like the previous page in<br>"
		       "the pager stack.");
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Flip to 1");
   evas_object_smart_callback_add(bt, "clicked", my_pager_3, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Popme");
   evas_object_smart_callback_add(bt, "clicked", my_pager_pop, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_pager_content_push(pg, bx);
   info.pg3 = bx;

   Evas_Object *ly, *ly2;
   char buf[4096];

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(ly, buf, "test/layout");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(ly);

   ly2 = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(ly2, buf, "layout2");
   evas_object_size_hint_weight_set(ly2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //elm_win_resize_object_add(win, ly2);
   evas_object_show(ly2);




   bt = elm_button_add(win);
   elm_button_label_set(bt, "Flip to 1");
   evas_object_smart_callback_add(bt, "clicked", my_pager_4, &info);
   elm_layout_content_set(ly2, "element1", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Popme");
   evas_object_smart_callback_add(bt, "clicked", my_pager_pop, &info);
   evas_object_show(bt);
   elm_layout_content_set(ly2, "element2", bt);

   elm_layout_content_set(ly, "swallow", ly2);
   evas_object_show(ly);

   elm_pager_content_push(pg, ly);
   info.pg4 = ly2;


   evas_object_show(win);
}

static void
my_pager_push(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Pginfo *info = data;
   Evas_Object *bx, *bt, *lb;
   static int count = 2;
   char buf[32];

   bx = elm_box_add(info->win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   lb = elm_label_add(info->win);
   snprintf(buf, sizeof(buf), "This is page %d in the slide pager<br><br>", count++);
   elm_object_text_set(lb, buf);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   bt = elm_button_add(info->win);
   elm_button_label_set(bt, "Push a new page");
   evas_object_smart_callback_add(bt, "clicked", my_pager_push, info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(info->win);
   elm_button_label_set(bt, "Go back (pop)");
   evas_object_smart_callback_add(bt, "clicked", my_pager_pop, info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_pager_content_push(info->pager, bx);
}

void
test_pager_slide(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *pg, *bx, *lb, *bt;
   static Pginfo info;

   win = elm_win_add(NULL, "pager", ELM_WIN_BASIC);
   elm_win_title_set(win, "Pager Slide");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, 1);
   info.win = win;

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   pg = elm_pager_add(win);
   elm_win_resize_object_add(win, pg);
   elm_object_style_set(pg, "slide");
   evas_object_show(pg);
   info.pager = pg;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
		       "This is page 1 in a slide pager.<br>"
		       "<br>"
		       "The slide pager style is useful for browsing<br>"
		       "a hierarchy of objects, as it makes clear<br>"
		       "the direction of the browse.<br>"
		       "This is the 'slide' style, also available<br>"
		       "a fully transparent style named 'slide_invisble'.<br>"
		       "<br>");
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Push a new page");
   evas_object_smart_callback_add(bt, "clicked", my_pager_push, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Go back (pop)");
   evas_object_smart_callback_add(bt, "clicked", my_pager_pop, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_pager_content_push(pg, bx);

   evas_object_show(win);
}
#endif
