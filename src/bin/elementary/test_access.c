#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_cleanup_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_config_access_set(EINA_FALSE);
}

typedef struct _Item_Data
{
   Elm_Object_Item *item;
   int index;
} Item_Data;

static Elm_Genlist_Item_Class *itc1, *itc2;
char *gl_access_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[256];
   Item_Data *id = data;
   snprintf(buf, sizeof(buf), "Item # %d", id->index);
   return strdup(buf);
}

Evas_Object *gl_access_content_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *bt;

   if (!strcmp(part, "elm.swallow.end"))
     {
        bt = elm_button_add(obj);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_layout_text_set(bt, NULL, "OK");
     }
   else
     {
        bt = elm_icon_add(obj);
        snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
        elm_image_file_set(bt, buf, NULL);
        evas_object_size_hint_aspect_set(bt, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
     }

   return bt;
}

static void
gl_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   free(data);
}

static Elm_Gengrid_Item_Class *gic;

Evas_Object *
grid_access_content_get(void *data, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic;
   const Item_Data *id = data;

   if (!strcmp(part, "elm.swallow.icon"))
     {
        ic = elm_icon_add(obj);
        elm_object_scale_set(ic, 0.5);
        snprintf(buf, sizeof(buf), "%s/images/icon_%02i.png", elm_app_data_dir_get(), (id->index % 4));
        elm_image_file_set(ic, buf, NULL);
        elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
        evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(ic, 0.5, 0.5);
        evas_object_show(ic);
        return ic;
     }
   return NULL;
}

Evas_Object *gl_access_content_full_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part)
{
   int i;
   Evas_Object *grid;

   if (strcmp(part, "elm.swallow.content")) return NULL;

   grid = elm_gengrid_add(obj);
   elm_gengrid_item_size_set(grid, ELM_SCALE_SIZE(100), ELM_SCALE_SIZE(100));
   elm_gengrid_horizontal_set(grid, EINA_FALSE);
   elm_gengrid_reorder_mode_set(grid, EINA_TRUE);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);

   gic = elm_gengrid_item_class_new();
   gic->item_style = "default";
   gic->func.text_get = NULL;
   gic->func.content_get = grid_access_content_get;
   gic->func.state_get = NULL;
   gic->func.del = gl_del;

   for (i = 0; i < 4; i++)
     {
        Item_Data *id = calloc(1, sizeof(Item_Data));
        id->index = i;

        elm_gengrid_item_append(grid, gic, id, NULL, NULL);
     }
   elm_gengrid_item_class_free(gic);

   evas_object_size_hint_min_set(grid,
                                 ELM_SCALE_SIZE(300), ELM_SCALE_SIZE(150));
   return grid;
}

static void _realized(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *ei)
{
   Evas_Object *content, *bt;
   Eina_List *items;
   items = NULL;

   if (!ei) return;
   Elm_Object_Item *item = ei;

   const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(item);

   if (!strcmp(itc->item_style, "full"))
     {
        /* unregister item itself */
        elm_object_item_access_unregister(item);

        /* convey highlight to its content */
        content = elm_object_item_part_content_get(item, "elm.swallow.content");
        if (!content) return;

        items = eina_list_append(items, content);
     }
   else
     {
        bt = elm_object_item_part_content_get(item, "elm.swallow.end");
        if (!bt) return;

        items = eina_list_append(items, bt);
     }

   elm_object_item_access_order_set(item, items);
}

void
test_access(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int i;
   Evas_Object *win, *bx, *gl;

   win = elm_win_util_standard_add("access", "Access");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, NULL);

   elm_config_access_set(EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_homogeneous_set(bx, EINA_TRUE);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, gl);
   evas_object_show(gl);

   evas_object_smart_callback_add(gl, "realized", _realized, NULL);

   itc1 = elm_genlist_item_class_new();
   itc1->item_style     = "default";
   itc1->func.text_get = gl_access_text_get;
   itc1->func.content_get  = gl_access_content_get;
   itc1->func.state_get = NULL;
   itc1->func.del       = gl_del;

   itc2 = elm_genlist_item_class_new();
   itc2->item_style     = "full";
   itc2->func.text_get = NULL;
   itc2->func.content_get  = gl_access_content_full_get;
   itc2->func.state_get = NULL;
   itc2->func.del       = gl_del;

   for (i = 1; i < 10; i++)
     {
        Item_Data *id = calloc(1, sizeof(Item_Data));
        id->index = i;

        if (i % 4)
          {
             elm_genlist_item_append(
               gl,
               itc1,
               id,
               NULL/* parent */,
               ELM_GENLIST_ITEM_NONE,
               NULL/* func */,
               NULL/* func data */
               );
          }
        else
          {
             elm_genlist_item_append(
               gl,
               itc2,
               id,
               NULL/* parent */,
               ELM_GENLIST_ITEM_NONE,
               NULL/* func */,
               NULL/* func data */
               );
          }
     }

   elm_genlist_item_class_free(itc1);
   elm_genlist_item_class_free(itc2);
   evas_object_resize(win, 500, 400);
   evas_object_show(win);
}

void
test_access2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int i, j, k;
   char buf[PATH_MAX];
   Evas_Object *win, *bx, *sc, *ly, *ly2, *ic;
   Evas_Object *ao, *to;

   win = elm_win_util_standard_add("access", "Access");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, NULL);

   elm_config_access_set(EINA_TRUE);

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_TRUE, EINA_FALSE);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);
   evas_object_show(sc);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_homogeneous_set(bx, EINA_TRUE);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_object_content_set(sc, bx);
   evas_object_show(bx);

   for (k = 0 ; k < 3; k++)
     {
        ly = elm_layout_add(win);
        snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
        elm_layout_file_set(ly, buf, "access_page");
        evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_show(ly);

        for (j = 0; j < 3; j++)
          {
             for (i = 0; i < 3; i++)
               {
                  ly2 = elm_layout_add(win);
                  snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
                  elm_layout_file_set(ly2, buf, "access_icon");
                  evas_object_size_hint_weight_set(ly2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(ly2, EVAS_HINT_FILL, EVAS_HINT_FILL);

                  ic = elm_icon_add(win);
                  elm_object_scale_set(ic, 0.5);
                  snprintf(buf, sizeof(buf), "%s/images/icon_%02i.png", elm_app_data_dir_get(), (i + (k * 3)));
                  elm_image_file_set(ic, buf, NULL);
                  elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
                  evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(ic, 0.5, 0.5);
                  elm_object_part_content_set(ly2, "slot", ic);
                  evas_object_show(ic);

                  snprintf(buf, sizeof(buf), "slot.%i.%i", i, j);
                  elm_object_part_content_set(ly, buf, ly2);
                  evas_object_show(ly2);

                  /* access */
                  edje_object_freeze(elm_layout_edje_get(ly2));
                  to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(ly2), "access");
                  edje_object_thaw(elm_layout_edje_get(ly2));
                  ao = elm_access_object_register(to, ly2);
                  elm_object_focus_custom_chain_append(ly2, ao, NULL);
               }
          }

        elm_box_pack_end(bx, ly);
     }

   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}

static Eina_Bool
_key_down_cb(void *data, int type EINA_UNUSED, void *ei)
{
   Elm_Access_Action_Info *a;
   Ecore_Event_Key *ev = ei;

   a = calloc(1, sizeof(Elm_Access_Action_Info));
   if (!a) return ECORE_CALLBACK_PASS_ON;

   if (ev && ev->key)
     {
        if (!strcmp(ev->key, "F1"))
          {
             a->highlight_cycle = EINA_TRUE;
             elm_access_action(data, ELM_ACCESS_ACTION_HIGHLIGHT_NEXT, a);
          }
     }
   free(a);

   return ECORE_CALLBACK_PASS_ON;
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   if (data) return strdup(data);
   return NULL;
}

void
test_access3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX];
   Evas_Object *win, *box, *lbl, *ly, *btn;
   Evas_Object *red_ao, *green_ao, *blue_ao, *black_ao, *to;

   win = elm_win_util_standard_add("access", "Access");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, NULL);

   elm_config_access_set(EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   lbl = elm_label_add(box);
   elm_object_text_set(lbl,
                       "Move a mouse pointer to any object and press F1 to "
                       "move access highlight to the next object.");
   evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lbl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, lbl);
   evas_object_show(lbl);

   ly = elm_layout_add(box);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "access_color_page");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ly);
   evas_object_show(ly);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "action");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(btn);

   elm_object_part_content_set(ly, "center", btn);

   edje_object_freeze(elm_layout_edje_get(ly));
   to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(ly), "red");
   edje_object_thaw(elm_layout_edje_get(ly));
   red_ao = elm_access_object_register(to, ly);
   elm_access_info_cb_set(red_ao, ELM_ACCESS_INFO, _access_info_cb, "red");
   elm_access_highlight_next_set(btn, ELM_HIGHLIGHT_DIR_NEXT, red_ao);

   edje_object_freeze(elm_layout_edje_get(ly));
   to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(ly), "green");
   edje_object_thaw(elm_layout_edje_get(ly));
   green_ao = elm_access_object_register(to, ly);
   elm_access_info_cb_set(green_ao, ELM_ACCESS_INFO, _access_info_cb, "green");
   elm_access_highlight_next_set(red_ao, ELM_HIGHLIGHT_DIR_NEXT, green_ao);

   edje_object_freeze(elm_layout_edje_get(ly));
   to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(ly), "blue");
   edje_object_thaw(elm_layout_edje_get(ly));
   blue_ao = elm_access_object_register(to, ly);
   elm_access_info_cb_set(blue_ao, ELM_ACCESS_INFO, _access_info_cb, "blue");
   elm_access_highlight_next_set(green_ao, ELM_HIGHLIGHT_DIR_NEXT, blue_ao);

   edje_object_freeze(elm_layout_edje_get(ly));
   to = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(ly), "black");
   edje_object_thaw(elm_layout_edje_get(ly));
   black_ao = elm_access_object_register(to, ly);
   elm_access_info_cb_set(black_ao, ELM_ACCESS_INFO, _access_info_cb, "black");
   elm_access_highlight_next_set(blue_ao, ELM_HIGHLIGHT_DIR_NEXT, black_ao);

   ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_down_cb, win);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}
