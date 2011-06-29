#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

/* Hint:
 * In this example some calls to elm_genlist_item_append() are used which give the
 * value of an int as 'item data' and 'func data' after casting into (void*). For
 * sure this works only on architectures where sizeof(int)==sizeof(void*).
 * For real world usage you should hold a data structure or value in your
 * application and then give only a pointer to this data as data pointer.
 */

typedef struct _Testitem
{
   Elm_Genlist_Item *item;
   int mode;
   int onoff;
} Testitem;


static Elm_Genlist_Item_Class itc1;
char *gl_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(long)data);
   return strdup(buf);
}

Evas_Object *gl_icon_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic = elm_icon_add(obj);
   if (!strcmp(part, "elm.swallow.end"))
     snprintf(buf, sizeof(buf), "%s/images/bubble.png", PACKAGE_DATA_DIR);
   else
     snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}
Eina_Bool gl_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}
void gl_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

static void
gl_sel(void *data, Evas_Object *obj, void *event_info)
{
   printf("sel item data [%p] on genlist obj [%p], item pointer [%p]\n", data, obj, event_info);
}

static void
_move(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Object *gl = data;
   Evas_Event_Mouse_Move *ev = event_info;
   int where = 0;
   Elm_Genlist_Item *gli;
   gli = elm_genlist_at_xy_item_get(gl, ev->cur.canvas.x, ev->cur.canvas.y, &where);
   if (gli)
     printf("over %p, where %i\n", elm_genlist_item_data_get(gli), where);
   else
     printf("over none, where %i\n", where);
}

static void
_bt50_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_genlist_item_bring_in(data);
}

static void
_bt1500_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_genlist_item_middle_bring_in(data);
}

static void
_gl_selected(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("selected: %p\n", event_info);
}

static void
_gl_double_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("double clicked: %p\n", event_info);
}

static void
_gl_longpress(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("longpress %p\n", event_info);
}

void
test_genlist(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *bt_50, *bt_1500, *bx;
   Evas_Object *over;
   Elm_Genlist_Item *gli;
   int i;

   win = elm_win_add(NULL, "genlist", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_smart_callback_add(gl, "selected", _gl_selected, NULL);
   evas_object_smart_callback_add(gl, "clicked,double", _gl_double_clicked, NULL);
   evas_object_smart_callback_add(gl, "longpressed", _gl_longpress, NULL);
   // FIXME: This causes genlist to resize the horiz axis very slowly :(
   // Reenable this and resize the window horizontally, then try to resize it back
   //elm_genlist_horizontal_mode_set(gl, ELM_LIST_LIMIT);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, gl);
   evas_object_show(gl);

   over = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(over, 0, 0, 0, 0);
   evas_object_event_callback_add(over, EVAS_CALLBACK_MOUSE_MOVE, _move, gl);
   evas_object_repeat_events_set(over, 1);
   evas_object_show(over);
   evas_object_size_hint_weight_set(over, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, over);

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   bt_50 = elm_button_add(win);
   elm_object_text_set(bt_50, "Go to 50");
   evas_object_show(bt_50);
   elm_box_pack_end(bx, bt_50);

   bt_1500 = elm_button_add(win);
   elm_object_text_set(bt_1500, "Go to 1500");
   evas_object_show(bt_1500);
   elm_box_pack_end(bx, bt_1500);

   for (i = 0; i < 2000; i++)
     {
        gli = elm_genlist_item_append(gl, &itc1,
                                      (void *)(long)i/* item data */,
                                      NULL/* parent */,
                                      ELM_GENLIST_ITEM_NONE,
                                      gl_sel/* func */,
                                      (void *)(long)(i * 10)/* func data */);
        if (i == 50)
          evas_object_smart_callback_add(bt_50, "clicked", _bt50_cb, gli);
        else if (i == 1500)
          evas_object_smart_callback_add(bt_1500, "clicked", _bt1500_cb, gli);
     }
   evas_object_resize(win, 480, 800);
   evas_object_show(win);
}

/*************/

static void
my_gl_clear(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *gl = data;
   elm_genlist_clear(gl);
}

static void
my_gl_add(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *gl = data;
   static int i = 0;

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   elm_genlist_item_append(gl, &itc1,
                           (void *)(long)i/* item data */,
                           NULL/* parent */,
                           ELM_GENLIST_ITEM_NONE,
                           gl_sel/* func */,
                           (void *)(long)(i * 10)/* func data */);
   i++;
}

static void
my_gl_insert_before(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *gl = data;
   static int i = 0;
   Elm_Genlist_Item *gli_selected;

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   gli_selected = elm_genlist_selected_item_get(gl);
   if(!gli_selected)
     {
        printf("no item selected\n");
        return ;
     }

   elm_genlist_item_insert_before(gl, &itc1,
                                  (void *)(long)i/* item data */,
                                  NULL/* parent */,
                                  gli_selected /* item before */,
                                  ELM_GENLIST_ITEM_NONE,
                                  gl_sel/* func */,
                                  (void *)(long)(i * 10)/* func data */);
   i++;
}

static void
my_gl_insert_after(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *gl = data;
   static int i = 0;
   Elm_Genlist_Item *gli_selected;

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   gli_selected = elm_genlist_selected_item_get(gl);
   if(!gli_selected)
     {
        printf("no item selected\n");
        return ;
     }

   elm_genlist_item_insert_after(gl, &itc1,
                                 (void *)(long)i/* item data */,
                                 NULL/* parent */,
                                 gli_selected /* item after */,
                                 ELM_GENLIST_ITEM_NONE,
                                 gl_sel/* func */,
                                 (void *)(long)(i * 10)/* func data */);
   i++;
}

static void
my_gl_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli = elm_genlist_selected_item_get(gl);
   if (!gli)
     {
        printf("no item selected\n");
        return;
     }
   elm_genlist_item_del(gli);
}

static void
my_gl_disable(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli = elm_genlist_selected_item_get(gl);
   if (!gli)
     {
        printf("no item selected\n");
        return;
     }
   elm_genlist_item_disabled_set(gli, 1);
   elm_genlist_item_selected_set(gli, 0);
   elm_genlist_item_update(gli);
}

static void
my_gl_update_all(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *gl = data;
   int i = 0;
   Elm_Genlist_Item *it = elm_genlist_first_item_get(gl);
   while (it)
     {
        elm_genlist_item_update(it);
        printf("%i\n", i);
        i++;
        it = elm_genlist_item_next_get(it);
     }
}

static void
my_gl_first(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli = elm_genlist_first_item_get(gl);
   if (!gli) return;
   elm_genlist_item_show(gli);
   elm_genlist_item_selected_set(gli, 1);
}

static void
my_gl_last(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli = elm_genlist_last_item_get(gl);
   if (!gli) return;
   elm_genlist_item_show(gli);
   elm_genlist_item_selected_set(gli, 1);
}

static Eina_Bool
my_gl_flush_delay(void *data __UNUSED__)
{
   elm_all_flush();
   return ECORE_CALLBACK_CANCEL;
}

static void
my_gl_flush(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   ecore_timer_add(1.2, my_gl_flush_delay, NULL);
}

void
test_genlist2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bx3, *bt;
   Elm_Genlist_Item *gli[10];
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "genlist2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gl);

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   gli[0] = elm_genlist_item_append(gl, &itc1,
                                    (void *)1001/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                    (void *)1001/* func data */);
   gli[1] = elm_genlist_item_append(gl, &itc1,
                                    (void *)1002/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                    (void *)1002/* func data */);
   gli[2] = elm_genlist_item_append(gl, &itc1,
                                    (void *)1003/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                    (void *)1003/* func data */);
   gli[3] = elm_genlist_item_prepend(gl, &itc1,
                                     (void *)1004/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                     (void *)1004/* func data */);
   gli[4] = elm_genlist_item_prepend(gl, &itc1,
                                     (void *)1005/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                     (void *)1005/* func data */);
   gli[5] = elm_genlist_item_insert_before(gl, &itc1,
                                           (void *)1006/* item data */, NULL/* parent */, gli[2]/* rel */, ELM_GENLIST_ITEM_NONE,
                                           gl_sel/* func */, (void *)1006/* func data */);
   gli[6] = elm_genlist_item_insert_after(gl, &itc1,
                                          (void *)1007/* item data */, NULL/* parent */, gli[2]/* rel */, ELM_GENLIST_ITEM_NONE,
                                          gl_sel/* func */, (void *)1007/* func data */);

   elm_box_pack_end(bx, gl);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "/\\");
   evas_object_smart_callback_add(bt, "clicked", my_gl_first, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "\\/");
   evas_object_smart_callback_add(bt, "clicked", my_gl_last, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "#");
   evas_object_smart_callback_add(bt, "clicked", my_gl_disable, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "U");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update_all, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "X");
   evas_object_smart_callback_add(bt, "clicked", my_gl_clear, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "+");
   evas_object_smart_callback_add(bt, "clicked", my_gl_add, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "-");
   evas_object_smart_callback_add(bt, "clicked", my_gl_del, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx3 = elm_box_add(win);
   elm_box_horizontal_set(bx3, EINA_TRUE);
   elm_box_homogeneous_set(bx3, EINA_TRUE);
   evas_object_size_hint_weight_set(bx3, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "+ before");
   evas_object_smart_callback_add(bt, "clicked", my_gl_insert_before, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx3, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "+ after");
   evas_object_smart_callback_add(bt, "clicked", my_gl_insert_after, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx3, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Flush");
   evas_object_smart_callback_add(bt, "clicked", my_gl_flush, gl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx3, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx3);
   evas_object_show(bx3);


   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

/*************/

static Elm_Genlist_Item_Class itc2;
char *gl2_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   const Testitem *tit = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Item mode %i", tit->mode);
   return strdup(buf);
}
Evas_Object *gl2_icon_get(void *data, Evas_Object *obj, const char *part)
{
   const Testitem *tit = data;
   char buf[PATH_MAX];
   Evas_Object *ic = elm_icon_add(obj);
   if (!strcmp(part, "elm.swallow.icon"))
     {
        if (!(tit->mode & 0x3))
          snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
        else if ((tit->mode & 0x3) == 1)
          snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
        else if ((tit->mode & 0x3) == 2)
          snprintf(buf, sizeof(buf), "%s/images/panel_01.jpg", PACKAGE_DATA_DIR);
        else if ((tit->mode & 0x3) == 3)
          snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", PACKAGE_DATA_DIR);
        elm_icon_file_set(ic, buf, NULL);
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        if (!(tit->mode & 0x3))
          snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", PACKAGE_DATA_DIR);
        else if ((tit->mode & 0x3) == 1)
          snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
        else if ((tit->mode & 0x3) == 2)
          snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", PACKAGE_DATA_DIR);
        else if ((tit->mode & 0x3) == 3)
          snprintf(buf, sizeof(buf), "%s/images/sky_04.jpg", PACKAGE_DATA_DIR);
        elm_icon_file_set(ic, buf, NULL);
     }
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}
Eina_Bool gl2_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}
void gl2_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

static void
my_gl_update(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Testitem *tit = data;
   tit->mode++;
   elm_genlist_item_update(tit->item);
}

void
test_genlist3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bt;
   static Testitem tit[3];

   win = elm_win_add(NULL, "genlist3", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist 3");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gl);

   itc2.item_style     = "default";
   itc2.func.label_get = gl2_label_get;
   itc2.func.icon_get  = gl2_icon_get;
   itc2.func.state_get = gl2_state_get;
   itc2.func.del       = gl2_del;

   tit[0].mode = 0;
   tit[0].item = elm_genlist_item_append(gl, &itc2,
                                         &(tit[0])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                         NULL/* func data */);
   tit[1].mode = 1;
   tit[1].item = elm_genlist_item_append(gl, &itc2,
                                         &(tit[1])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                         NULL/* func data */);
   tit[2].mode = 2;
   tit[2].item = elm_genlist_item_append(gl, &itc2,
                                         &(tit[2])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                         NULL/* func data */);

   elm_box_pack_end(bx, gl);
   evas_object_show(bx);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[1]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[0]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[2]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[1]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[3]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[2]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

/*************/

static void
my_gl_item_check_changed(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Testitem *tit = data;
   tit->onoff = elm_check_state_get(obj);
   printf("item %p onoff = %i\n", tit, tit->onoff);
}

static Elm_Genlist_Item_Class itc3;
char *gl3_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   const Testitem *tit = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Item mode %i", tit->mode);
   return strdup(buf);
}
Evas_Object *gl3_icon_get(void *data, Evas_Object *obj, const char *part)
{
   const Testitem *tit = data;
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *lb;

        lb = elm_label_add(obj);
        elm_label_line_wrap_set(lb, 1);
        elm_label_wrap_width_set(lb, 201);
        elm_object_text_set(lb, "ashj ascjscjh n asjkl hcjlh ls hzshnn zjh sh zxjcjsnd h dfw sdv edev efe fwefvv vsd cvs ws wf  fvwf wd fwe f  we wef we wfe rfwewef wfv wswf wefg sdfws w wsdcfwcf wsc vdv  sdsd sdcd cv wsc sdcv wsc d sdcdcsd sdcdsc wdvd sdcsd wscxcv wssvd sd");
        return lb;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *ck;
        ck = elm_check_add(obj);
        evas_object_propagate_events_set(ck, EINA_FALSE);
        elm_check_state_set(ck, tit->onoff);
        evas_object_smart_callback_add(ck, "changed", my_gl_item_check_changed, data);
        return ck;
     }
   return NULL;
}
Eina_Bool gl3_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}
void gl3_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

void
test_genlist4(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bt;
   static Testitem tit[3];

   win = elm_win_add(NULL, "genlist4", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist 4");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   elm_genlist_height_for_width_mode_set(gl, EINA_TRUE);
   elm_genlist_multi_select_set(gl, 1);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gl);

   itc3.item_style     = "default";
   itc3.func.label_get = gl3_label_get;
   itc3.func.icon_get  = gl3_icon_get;
   itc3.func.state_get = gl3_state_get;
   itc3.func.del       = gl3_del;

   tit[0].mode = 0;
   tit[0].item = elm_genlist_item_append(gl, &itc3,
                                         &(tit[0])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                         NULL/* func data */);
   tit[1].mode = 1;
   tit[1].item = elm_genlist_item_append(gl, &itc3,
                                         &(tit[1])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                         NULL/* func data */);
   tit[2].mode = 2;
   tit[2].item = elm_genlist_item_append(gl, &itc3,
                                         &(tit[2])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                         NULL/* func data */);

   elm_box_pack_end(bx, gl);
   evas_object_show(bx);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[1]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[0]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[2]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[1]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[3]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[2]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}


/*************/
static void
my_gl_item_check_changed2(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Testitem *tit = data;
   tit->onoff = elm_check_state_get(obj);
   printf("item %p onoff = %i\n", tit, tit->onoff);
}

static Elm_Genlist_Item_Class itc5;
char *gl5_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part)
{
   const Testitem *tit = data;
   char buf[256];
   if (!strcmp(part, "elm.text"))
     {
        snprintf(buf, sizeof(buf), "Item mode %i", tit->mode);
     }
   else if (!strcmp(part, "elm.text.sub"))
     {
        snprintf(buf, sizeof(buf), "%i bottles on the wall", tit->mode);
     }
   return strdup(buf);
}
Evas_Object *gl5_icon_get(void *data, Evas_Object *obj, const char *part)
{
   const Testitem *tit = data;
   char buf[PATH_MAX];
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *bx = elm_box_add(obj);
        Evas_Object *ic;
        elm_box_horizontal_set(bx, EINA_TRUE);
        ic = elm_icon_add(obj);
        snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
        elm_icon_file_set(ic, buf, NULL);
        elm_icon_scale_set(ic, 0, 0);
        evas_object_show(ic);
        elm_box_pack_end(bx, ic);
        ic = elm_icon_add(obj);
        elm_icon_file_set(ic, buf, NULL);
        elm_icon_scale_set(ic, 0, 0);
        evas_object_show(ic);
        elm_box_pack_end(bx, ic);
        elm_box_horizontal_set(bx, EINA_TRUE);
        evas_object_show(bx);
        return bx;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *ck;
        ck = elm_check_add(obj);
        evas_object_propagate_events_set(ck, EINA_FALSE);
        elm_check_state_set(ck, tit->onoff);
        evas_object_smart_callback_add(ck, "changed", my_gl_item_check_changed2, data);
        evas_object_show(ck);
        return ck;
     }
   return NULL;
}
Eina_Bool gl5_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}
void gl5_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

static void
item_drag_up(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("drag up\n");
}

static void
item_drag_down(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("drag down\n");
}

static void
item_drag_left(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("drag left\n");
}

static void
item_drag_right(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("drag right\n");
}

static void
scroll_top(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("Top edge!\n");
}

static void
scroll_bottom(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("Bottom edge!\n");
}

static void
scroll_left(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("Left edge!\n");
}

static void
scroll_right(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("Right edge!\n");
}

static void
item_drag(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("drag\n");
}

static void
item_drag_stop(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("drag stop\n");
}

static void
item_longpress(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("longpress\n");
}

void
test_genlist5(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bt;
   static Testitem tit[3];

   win = elm_win_add(NULL, "genlist5", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist 5");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   elm_genlist_always_select_mode_set(gl, 1);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gl);
   itc5.item_style     = "double_label";
   itc5.func.label_get = gl5_label_get;
   itc5.func.icon_get  = gl5_icon_get;
   itc5.func.state_get = gl5_state_get;
   itc5.func.del       = gl5_del;

   tit[0].mode = 0;
   tit[0].item = elm_genlist_item_append(gl, &itc5,
                                         &(tit[0])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                         NULL/* func data */);
   tit[1].mode = 1;
   tit[1].item = elm_genlist_item_append(gl, &itc5,
                                         &(tit[1])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                         NULL/* func data */);
   tit[2].mode = 2;
   tit[2].item = elm_genlist_item_append(gl, &itc5,
                                         &(tit[2])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
                                         NULL/* func data */);

   elm_box_pack_end(bx, gl);
   evas_object_show(bx);

   evas_object_smart_callback_add(gl, "drag,start,up", item_drag_up, NULL);
   evas_object_smart_callback_add(gl, "drag,start,down", item_drag_down, NULL);
   evas_object_smart_callback_add(gl, "drag,start,left", item_drag_left, NULL);
   evas_object_smart_callback_add(gl, "drag,start,right", item_drag_right, NULL);
   evas_object_smart_callback_add(gl, "scroll,edge,top", scroll_top, NULL);
   evas_object_smart_callback_add(gl, "scroll,edge,bottom", scroll_bottom, NULL);
   evas_object_smart_callback_add(gl, "scroll,edge,left", scroll_left, NULL);
   evas_object_smart_callback_add(gl, "scroll,edge,right", scroll_right, NULL);
   evas_object_smart_callback_add(gl, "drag", item_drag, NULL);
   evas_object_smart_callback_add(gl, "drag,stop", item_drag_stop, NULL);
   evas_object_smart_callback_add(gl, "longpressed", item_longpress, NULL);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[1]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[0]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[2]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[1]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[3]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[2]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

/*************/

static Elm_Genlist_Item_Class itc4;

static void
gl4_sel(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = (Elm_Genlist_Item *)event_info;
   int depth = 0;

   depth = elm_genlist_item_expanded_depth_get(it);
   printf("expanded depth for selected item is %d\n", depth);

}
static void
gl4_exp(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   Evas_Object *gl = elm_genlist_item_genlist_get(it);
   int val = (int)(long)elm_genlist_item_data_get(it);
   val *= 10;
   elm_genlist_item_append(gl, &itc4,
                           (void *)(long)(val + 1)/* item data */, it/* parent */,
                           ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc4,
                           (void *)(long)(val + 2)/* item data */, it/* parent */,
                           ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc4,
                           (void *)(long)(val + 3)/* item data */, it/* parent */,
                           ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
                           NULL/* func data */);
}
static void
gl4_con(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static void
gl4_exp_req(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 1);
}
static void
gl4_con_req(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 0);
}

char *gl4_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item mode %i", (int)(long)data);
   return strdup(buf);
}
Evas_Object *gl4_icon_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *ic = elm_icon_add(obj);
        snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
        elm_icon_file_set(ic, buf, NULL);
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
        evas_object_show(ic);
        return ic;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *ck;
        ck = elm_check_add(obj);
        evas_object_propagate_events_set(ck, EINA_FALSE);
        evas_object_show(ck);
        return ck;
     }
   return NULL;
}
Eina_Bool gl4_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}
void gl4_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

void
test_genlist6(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bt;

   win = elm_win_add(NULL, "genlist-tree", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist Tree");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gl);

   itc4.item_style     = "default";
   itc4.func.label_get = gl4_label_get;
   itc4.func.icon_get  = gl4_icon_get;
   itc4.func.state_get = gl4_state_get;
   itc4.func.del       = gl4_del;

   elm_genlist_item_append(gl, &itc4,
                           (void *)1/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc4,
                           (void *)2/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc4,
                           (void *)3/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
                           NULL/* func data */);

   evas_object_smart_callback_add(gl, "expand,request", gl4_exp_req, gl);
   evas_object_smart_callback_add(gl, "contract,request", gl4_con_req, gl);
   evas_object_smart_callback_add(gl, "expanded", gl4_exp, gl);
   evas_object_smart_callback_add(gl, "contracted", gl4_con, gl);

   elm_box_pack_end(bx, gl);
   evas_object_show(bx);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[1]");
   //   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[0]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[2]");
   //   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[1]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "[3]");
   //   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[2]));
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

/*************/

struct genlist7_data
{
   Evas_Object *win, *pager;
};

static Elm_Genlist_Item_Class itc7;
static void
gl_sel7(void *data, Evas_Object *obj, void *event_info)
{
   if (!event_info) return;
   elm_genlist_item_item_class_update(event_info, &itc7);
   printf("sel item data [%p] on genlist obj [%p], item pointer [%p], new item style [%s] \n", data, obj, event_info, itc7.item_style);
}

static void
test_genlist7_back_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   struct genlist7_data *info = data;
   if (!info) return;

   elm_pager_content_pop(info->pager);
}

static void
test_genlist7_swipe(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   struct genlist7_data *info = data;
   Evas_Object *box, *entry, *button;
   char item_data[] = "Just a simple test";

   if ((!event_info) || (!data)) return;

   box = elm_box_add(info->win);
   elm_box_homogeneous_set(box, EINA_FALSE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(box);

   entry = elm_entry_add(info->win);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   elm_entry_editable_set(entry, EINA_FALSE);
   elm_entry_entry_set(entry, item_data);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(entry);

   button = elm_button_add(info->win);
   elm_object_text_set(button, "back");
   evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0);
   evas_object_smart_callback_add(button, "clicked", test_genlist7_back_cb,
                                  info);
   evas_object_show(button);

   elm_box_pack_start(box, entry);
   elm_box_pack_end(box, button);

   elm_pager_content_push(info->pager, box);
}

void
test_genlist7(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *pager;
   static struct genlist7_data info;
   static Testitem tit[3];

   win = elm_win_add(NULL, "genlist7", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist 7");
   elm_win_autodel_set(win, 1);
   info.win = win;

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   pager = elm_pager_add(win);
   elm_win_resize_object_add(win, pager);
   evas_object_size_hint_weight_set(pager, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pager, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(pager);
   info.pager = pager;

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(gl, "swipe", test_genlist7_swipe, &info);
   evas_object_show(gl);
   elm_pager_content_push(pager, gl);

   itc2.item_style     = "default";
   itc2.func.label_get = gl2_label_get;
   itc2.func.icon_get  = gl2_icon_get;
   itc2.func.state_get = gl2_state_get;
   itc2.func.del       = gl2_del;

   itc7.item_style     = "double_label";
   itc7.func.label_get = gl5_label_get;
   itc7.func.icon_get  = gl5_icon_get;
   itc7.func.state_get = gl5_state_get;
   itc7.func.del       = gl5_del;

   tit[0].mode = 0;
   tit[0].item = elm_genlist_item_append(gl, &itc2,
                                         &(tit[0])/* item data */, NULL/* parent */,
                                         ELM_GENLIST_ITEM_NONE, gl_sel7/* func */,
                                         NULL/* func data */);
   tit[1].mode = 1;
   tit[1].item = elm_genlist_item_append(gl, &itc2,
                                         &(tit[1])/* item data */, NULL/* parent */,
                                         ELM_GENLIST_ITEM_NONE, gl_sel7/* func */,
                                         NULL/* func data */);
   tit[2].mode = 2;
   tit[2].item = elm_genlist_item_append(gl, &itc2,
                                         &(tit[2])/* item data */, NULL/* parent */,
                                         ELM_GENLIST_ITEM_NONE, gl_sel7/* func */,
                                         NULL/* func data */);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

/*************/

static Elm_Genlist_Item_Class itc_group;
char *gl8_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Group Index # %i (Item # %i)",  (int)((long)data / 10), (int)(long)data);
   return strdup(buf);
}

static void
_bt_show_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_genlist_item_top_show(data);
   //elm_genlist_item_show(data);
   //elm_genlist_item_middle_show(data);
}
static void
_bt_bring_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_genlist_item_top_bring_in(data);
   //elm_genlist_item_bring_in(data);
   //elm_genlist_item_middle_bring_in(data);
}

void
test_genlist8(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *bt[8], *bx, *bx2, *bx3;
   Elm_Genlist_Item *gli = NULL, *git = NULL;
   int i, bt_count, bt_num;

   win = elm_win_add(NULL, "genlist-group", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist Group");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_smart_callback_add(gl, "selected", _gl_selected, NULL);
   evas_object_smart_callback_add(gl, "clicked,double", _gl_double_clicked, NULL);
   evas_object_smart_callback_add(gl, "longpressed", _gl_longpress, NULL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, gl);
   evas_object_show(gl);

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   itc_group.item_style     = "group_index";
   itc_group.func.label_get = gl8_label_get;
   itc_group.func.icon_get  = NULL;
   itc_group.func.state_get = NULL;
   itc_group.func.del       = gl_del;

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt_num = 0;
   bt[bt_num] = elm_button_add(win);
   elm_object_text_set(bt[bt_num], "Show 0");
   evas_object_size_hint_align_set(bt[bt_num], EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt[bt_num], EVAS_HINT_EXPAND, 0.0);
   evas_object_show(bt[bt_num]);
   elm_box_pack_end(bx2, bt[bt_num]);

   bt[++bt_num] = elm_button_add(win);
   elm_object_text_set(bt[bt_num], "Show 26");
   evas_object_size_hint_align_set(bt[bt_num], EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt[bt_num], EVAS_HINT_EXPAND, 0.0);
   evas_object_show(bt[bt_num]);
   elm_box_pack_end(bx2, bt[bt_num]);

   bt[++bt_num] = elm_button_add(win);
   elm_object_text_set(bt[bt_num], "Show 101");
   evas_object_size_hint_align_set(bt[bt_num], EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt[bt_num], EVAS_HINT_EXPAND, 0.0);
   evas_object_show(bt[bt_num]);
   elm_box_pack_end(bx2, bt[bt_num]);

   bt[++bt_num] = elm_button_add(win);
   elm_object_text_set(bt[bt_num], "Show 480");
   evas_object_size_hint_align_set(bt[bt_num], EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt[bt_num], EVAS_HINT_EXPAND, 0.0);
   evas_object_show(bt[bt_num]);
   elm_box_pack_end(bx2, bt[bt_num]);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx3 = elm_box_add(win);
   elm_box_horizontal_set(bx3, EINA_TRUE);
   elm_box_homogeneous_set(bx3, EINA_TRUE);
   evas_object_size_hint_weight_set(bx3, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt[++bt_num] = elm_button_add(win);
   elm_object_text_set(bt[bt_num], "Bring top 0");
   evas_object_size_hint_align_set(bt[bt_num], EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt[bt_num], EVAS_HINT_EXPAND, 0.0);
   evas_object_show(bt[bt_num]);
   elm_box_pack_end(bx3, bt[bt_num]);

   bt[++bt_num] = elm_button_add(win);
   elm_object_text_set(bt[bt_num], "Bring top 31");
   evas_object_size_hint_align_set(bt[bt_num], EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt[bt_num], EVAS_HINT_EXPAND, 0.0);
   evas_object_show(bt[bt_num]);
   elm_box_pack_end(bx3, bt[bt_num]);

   bt[++bt_num] = elm_button_add(win);
   elm_object_text_set(bt[bt_num], "Bring top 239");
   evas_object_size_hint_align_set(bt[bt_num], EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt[bt_num], EVAS_HINT_EXPAND, 0.0);
   evas_object_show(bt[bt_num]);
   elm_box_pack_end(bx3, bt[bt_num]);

   bt[++bt_num] = elm_button_add(win);
   elm_object_text_set(bt[bt_num], "Bring top 477");
   evas_object_size_hint_align_set(bt[bt_num], EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt[bt_num], EVAS_HINT_EXPAND, 0.0);
   evas_object_show(bt[bt_num]);
   elm_box_pack_end(bx3, bt[bt_num]);

   elm_box_pack_end(bx, bx3);
   evas_object_show(bx3);

   bt_count = 0;
   for (i = 0; i < 500; i++)
     {
        if (!(i % 10))
          {
             gli = git = elm_genlist_item_append(gl, &itc_group,
                                                 (void *)(long)i/* item data */,
                                                 NULL/* parent */,
                                                 ELM_GENLIST_ITEM_GROUP,
                                                 gl_sel/* func */,
                                                 (void *)(long)(i * 10)/* func data */);
             elm_genlist_item_display_only_set(git, EINA_TRUE);
          }
        else if (git)
          {
             gli = elm_genlist_item_append(gl, &itc1,
                                           (void *)(long)i/* item data */,
                                           git/* parent */,
                                           ELM_GENLIST_ITEM_NONE,
                                           gl_sel/* func */,
                                           (void *)(long)(i * 10)/* func data */);
          }
        switch (i)
          {
           case 0:
              evas_object_smart_callback_add(bt[0], "clicked", _bt_show_cb, gli);
              evas_object_smart_callback_add(bt[4], "clicked", _bt_bring_cb, gli);
              break;
           case 26:
              evas_object_smart_callback_add(bt[1], "clicked", _bt_show_cb, gli);
              break;
           case 31:
              evas_object_smart_callback_add(bt[5], "clicked", _bt_bring_cb, gli);
              break;
           case 101:
              evas_object_smart_callback_add(bt[2], "clicked", _bt_show_cb, gli);
              break;
           case 239:
              evas_object_smart_callback_add(bt[6], "clicked", _bt_bring_cb, gli);
              break;
           case 477:
              evas_object_smart_callback_add(bt[7], "clicked", _bt_bring_cb, gli);
              break;
           case 480:
              evas_object_smart_callback_add(bt[3], "clicked", _bt_show_cb, gli);
              break;
          }
     }

   evas_object_resize(win, 480, 800);
   evas_object_show(win);
}

/*************/

static void
gl9_exp(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   Evas_Object *gl = elm_genlist_item_genlist_get(it);
   int val = (int)(long)elm_genlist_item_data_get(it);
   val *= 10;
   elm_genlist_item_append(gl, &itc1,
                           (void *)(long)(val + 1)/* item data */, it/* parent */,
                           ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc1,
                           (void *)(long)(val + 2)/* item data */, it/* parent */,
                           ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc1,
                           (void *)(long)(val + 3)/* item data */, it/* parent */,
                           ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
                           NULL/* func data */);
}
static void
gl9_con(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static void
gl9_exp_req(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, EINA_TRUE);
}
static void
gl9_con_req(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, EINA_FALSE);
}

void
test_genlist9(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *bx;
   Elm_Genlist_Item *git;

   win = elm_win_add(NULL, "genlist-group-tree", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist Group Tree");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_smart_callback_add(gl, "selected", _gl_selected, NULL);
   evas_object_smart_callback_add(gl, "clicked,double", _gl_double_clicked, NULL);
   evas_object_smart_callback_add(gl, "longpressed", _gl_longpress, NULL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, gl);
   evas_object_show(gl);

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   itc_group.item_style     = "group_index";
   itc_group.func.label_get = gl8_label_get;
   itc_group.func.icon_get  = NULL;
   itc_group.func.state_get = NULL;
   itc_group.func.del       = gl_del;

   git = elm_genlist_item_append(gl, &itc_group,
                                 (void *)0/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_GROUP, gl4_sel/* func */,
                                 NULL/* func data */);
   elm_genlist_item_display_only_set(git, EINA_TRUE);
   elm_genlist_item_append(gl, &itc1,
                           (void *)1/* item data */, git/* parent */, ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc1,
                           (void *)2/* item data */, git/* parent */, ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc1,
                           (void *)3/* item data */, git/* parent */, ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
                           NULL/* func data */);
   git = elm_genlist_item_append(gl, &itc_group,
                                 (void *)4/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_GROUP, gl4_sel/* func */,
                                 NULL/* func data */);
   elm_genlist_item_display_only_set(git, EINA_TRUE);
   elm_genlist_item_append(gl, &itc1,
                           (void *)5/* item data */, git/* parent */, ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc1,
                           (void *)6/* item data */, git/* parent */, ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
                           NULL/* func data */);
   elm_genlist_item_append(gl, &itc1,
                           (void *)7/* item data */, git/* parent */, ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
                           NULL/* func data */);

   evas_object_smart_callback_add(gl, "expand,request", gl9_exp_req, gl);
   evas_object_smart_callback_add(gl, "contract,request", gl9_con_req, gl);
   evas_object_smart_callback_add(gl, "expanded", gl9_exp, gl);
   evas_object_smart_callback_add(gl, "contracted", gl9_con, gl);

   evas_object_resize(win, 480, 800);
   evas_object_show(win);
}

/*************/

static Elm_Genlist_Item_Class itc10;
static char *mode_type[] = { "slide", "rotate" };
char *gl10_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part)
{
   char buf[256];
   if (!strcmp(part, "elm.text.mode"))
     snprintf(buf, sizeof(buf), "Mode # %i", (int)(long)data);
   else
     snprintf(buf, sizeof(buf), "Item # %i", (int)(long)data);
   return strdup(buf);
}

Evas_Object *gl10_icon_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic = elm_icon_add(obj);
   if (!strcmp(part, "elm.swallow.end"))
     snprintf(buf, sizeof(buf), "%s/images/bubble.png", PACKAGE_DATA_DIR);
   else
     snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}
static void
_gl_sel10(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if (!data) return;
   int v = elm_radio_value_get(data);
   if (v == 1)
     elm_genlist_item_mode_set(event_info, mode_type[v], EINA_TRUE);
}

static void
_my_gl_mode_right(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   if (!data) return;
   int v = elm_radio_value_get(data);
   if (v == 0)
     elm_genlist_item_mode_set(event_info, mode_type[v], EINA_TRUE);
}

static void
_my_gl_mode_left(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   if (!data) return;
   int v = elm_radio_value_get(data);
   if (v == 0)
     elm_genlist_item_mode_set(event_info, mode_type[v], EINA_FALSE);
}

static void
_my_gl_mode_cancel(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   fprintf(stderr, "drag\n");
   if (!data) return;
   int v = elm_radio_value_get(data);
   Elm_Genlist_Item *it = (Elm_Genlist_Item *)elm_genlist_mode_item_get(obj);
   if (it)
     elm_genlist_item_mode_set(it, mode_type[v], EINA_FALSE);
}

void
test_genlist10(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *fr, *lb, *bx, *bx2, *bx3, *rd, *rdg, *gl;
   int i;

   win = elm_win_add(NULL, "genlist10", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist Mode");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Mode Type");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx2 = elm_box_add(win);
   elm_frame_content_set(fr, bx2);
   evas_object_show(bx2);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "Sweep genlist items to the right.<br>"
                       "Test this by changing Mode Type to Slide or Rotate.");
   elm_box_pack_end(bx2, lb);
   evas_object_show(lb);

   bx3 = elm_box_add(win);
   elm_box_horizontal_set(bx3, EINA_TRUE);
   elm_box_pack_end(bx2, bx3);
   evas_object_show(bx3);

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_radio_state_value_set(rd, 0);
   elm_radio_label_set(rd, "Slide  ");
   evas_object_show(rd);
   elm_box_pack_end(bx3, rd);
   rdg = rd;

   rd = elm_radio_add(win);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_radio_state_value_set(rd, 1);
   elm_radio_label_set(rd, "Rotate");
   elm_radio_group_add(rd, rdg);
   evas_object_show(rd);
   elm_box_pack_end(bx3, rd);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(gl, "drag,start,right", _my_gl_mode_right, rdg);
   evas_object_smart_callback_add(gl, "drag,start,left", _my_gl_mode_left, rdg);
   evas_object_smart_callback_add(gl, "drag,start,up", _my_gl_mode_cancel, rdg);
   evas_object_smart_callback_add(gl, "drag,start,down", _my_gl_mode_cancel, rdg);
   evas_object_show(gl);

   itc10.item_style     = "default";
   itc10.func.label_get = gl10_label_get;
   itc10.func.icon_get  = gl10_icon_get;
   itc10.func.state_get = gl_state_get;
   itc10.func.del       = gl_del;
   itc10.mode_item_style = "mode";

   for (i = 0; i < 50; i++)
     elm_genlist_item_append(gl,
                             &itc10,
                             (void *)(1000 + i)/* item data */,
                             NULL/* parent */,
                             ELM_GENLIST_ITEM_NONE/* flags */,
                             _gl_sel10/* func */,
                             rdg/* func data */);

   elm_box_pack_end(bx, gl);

   evas_object_resize(win, 520, 520);
   evas_object_show(win);
}

/*************/

static void
_reorder_tg_changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   elm_genlist_reorder_mode_set(data, elm_toggle_state_get(obj));
}

/**
 * gl_moved is called after an item was reordered.
 * This is only called when reorder mode is enabled.
 *
 * @param obj          :  the genlist object.
 * @param item         :  the moved item.
 * @param rel_item     :  the relative item.
 * @param move_after   :  whether or not the rel_item is after item.
 *
 * If the move_after is true,
 *  the item(*item) had been moved after the given relative item(*rel_item) in list.
 * If the move_after is false,
 *  the item(*item) had been moved before the given relative item(*rel_item) in list.
 *
 */
static void gl_moved(Evas_Object *obj __UNUSED__, Elm_Genlist_Item *item __UNUSED__, Elm_Genlist_Item *rel_item __UNUSED__, Eina_Bool move_after __UNUSED__)
{
   // if needed, add application logic.
}

void
test_genlist11(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *fr, *lb, *bx, *tg, *gl;
   int i;

   win = elm_win_add(NULL, "genlist-reorder-mode", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist Reorder Mode");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fr = elm_frame_add(win);

   elm_frame_label_set(fr, "Reorder Mode");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "Enable reorder mode if you want to move item.<br>"
                       "Then long press and drag item.");
   elm_frame_content_set(fr, lb);
   evas_object_show(lb);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gl);

   tg = elm_toggle_add(win);
   elm_object_text_set(tg, "Reorder Mode:");
   elm_toggle_state_set(tg, elm_mirrored_get());
   evas_object_smart_callback_add(tg, "changed", _reorder_tg_changed_cb, gl);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;
   itc1.func.moved     = gl_moved;

   for (i = 0; i < 50; i++)
     elm_genlist_item_append(gl,
                             &itc1,
                             (void *)(1 + i)/* item data */,
                             NULL/* parent */,
                             ELM_GENLIST_ITEM_NONE/* flags */,
                             NULL/* func */,
                             NULL/* func data */);

   elm_box_pack_end(bx, gl);

   evas_object_resize(win, 400, 500);
   evas_object_show(win);
}
#endif
