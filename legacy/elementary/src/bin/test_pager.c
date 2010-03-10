#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
typedef struct _Pginfo Pginfo;

struct _Pginfo
{
   Evas_Object *pager, *pg1, *pg2, *pg3;
};

static void
my_pager_1(void *data, Evas_Object *obj, void *event_info)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg2);
}

static void
my_pager_2(void *data, Evas_Object *obj, void *event_info)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg3);
}

static void
my_pager_3(void *data, Evas_Object *obj, void *event_info)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg1);
}

static void
my_pager_pop(void *data, Evas_Object *obj, void *event_info)
{
   Pginfo *info = data;
   elm_pager_content_pop(info->pager);
}

void
test_pager(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *pg, *bx, *lb, *bt;
   static Pginfo info;

   win = elm_win_add(NULL, "pager", ELM_WIN_BASIC);
   elm_win_title_set(win, "Pager");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   pg = elm_pager_add(win);
   elm_win_resize_object_add(win, pg);
   evas_object_show(pg);

   info.pager = pg;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   lb = elm_label_add(win);
   elm_label_label_set(lb,
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
		       "show and hide of pages.");
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

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
   elm_label_label_set(lb,
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
   elm_label_label_set(lb,
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

   evas_object_show(win);
}
#endif
