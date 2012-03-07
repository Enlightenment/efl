#include <Elementary.h>
#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
struct _Idx_Data_Type
{
   Evas_Object *id;  /* Pointer to Index */
   Elm_Object_Item *item; /* Item we use for search */
};
typedef struct _Idx_Data_Type Idx_Data_Type;

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   Idx_Data_Type dt;
};
typedef struct _api_data api_data;

struct _pagectrl_data
{
   Evas_Object *index;
   Evas_Object *sc;
   Evas_Coord recent_x;
   Evas_Coord page_width;
   int page_cnt;
   Eina_Bool drag_started : 1;
   Eina_Bool changed : 1;
};
typedef struct _pagectrl_data pagectrl_data;

enum _api_state
{
   INDEX_LEVEL_SET,
   INDEX_ACTIVE_SET,
   INDEX_APPEND_RELATIVE,
   INDEX_PREPEND,
   INDEX_ITEM_DEL,
   INDEX_ITEM_FIND,
   INDEX_CLEAR,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   Idx_Data_Type *d = &api->dt;
   switch(api->state)
     { /* Put all api-changes under switch */
      case INDEX_LEVEL_SET: /* 0 */
         elm_index_autohide_disabled_set(d->id, EINA_TRUE);
         elm_index_item_level_set(d->id, (elm_index_item_level_get(d->id) ? 0 : 1));
         break;

      case INDEX_ACTIVE_SET: /* 1 */
         elm_index_autohide_disabled_set(d->id, EINA_FALSE);
         break;

      case INDEX_APPEND_RELATIVE: /* 2 */
             elm_index_item_insert_after(d->id, elm_index_item_find(d->id, d->item), "W", NULL, d->item);
         break;

      case INDEX_PREPEND: /* 3 */
         elm_index_item_prepend(d->id, "D", NULL, d->item);
         break;

      case INDEX_ITEM_DEL: /* 4 */
         elm_object_item_del(elm_index_item_find(d->id, d->item));
         break;

      case INDEX_ITEM_FIND: /* 5 */
           {
              Elm_Object_Item *i = elm_index_item_find(d->id, d->item);
              if(i)
                {
                   printf("Item Find - Found Item.\n");
                   elm_object_item_del(i);
                }
           }
         break;

      case INDEX_CLEAR: /* 6 */
         elm_index_item_clear(d->id);
         break;

      case API_STATE_LAST:
         break;

      default:
         return;
     }
}

static void
_api_bt_clicked(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{  /* Will add here a SWITCH command containing code to modify test-object */
   /* in accordance a->state value. */
   api_data *a = data;
   char str[128];

   printf("clicked event on API Button: api_state=<%d>\n", a->state);
   set_api_state(a);
   a->state++;
   sprintf(str, "Next API function (%u)", a->state);
   elm_object_text_set(obj, str);
   elm_object_disabled_set(obj, a->state == API_STATE_LAST);
}

static Elm_Genlist_Item_Class itci;
char *gli_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[256];
   int j = (long)data;
   snprintf(buf, sizeof(buf), "%c%c",
            'A' + ((j >> 4) & 0xf),
            'a' + ((j     ) & 0xf)
            );
   return strdup(buf);
}

void
_index_delay_changed_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   // called on a change but delayed in case multiple changes happen in a
   // short timespan
   elm_genlist_item_top_bring_in(elm_object_item_data_get(event_info));
}

void
_index_changed_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   // this is calld on every change, no matter how often
   // elm_genlist_item_bring_in(event_info);
}

void
_index_selected_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   // called on final select
   elm_genlist_item_top_bring_in(elm_object_item_data_get(event_info));
}

static void
_cleanup_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

static void
id_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("Current Index : %s\n", elm_index_item_letter_get((const Elm_Object_Item *)event_info));
}

void
test_index(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bxx, *gl, *id, *bt;
   Elm_Object_Item *glit;
   int i, j;
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_add(NULL, "index", ELM_WIN_BASIC);
   elm_win_title_set(win, "Index");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bxx = elm_box_add(win);
   elm_win_resize_object_add(win, bxx);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bxx);

   gl = elm_genlist_add(win);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(gl);

   api->dt.id = id = elm_index_add(win);
   evas_object_size_hint_weight_set(id, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(id, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, id);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   elm_box_pack_end(bxx, gl);

   evas_object_show(id);

   itci.item_style     = "default";
   itci.func.text_get = gli_text_get;
   itci.func.content_get  = NULL;
   itci.func.state_get = NULL;
   itci.func.del       = NULL;

	Elm_Object_Item *it;
   j = 0;
   for (i = 0; i < 100; i++)
     {
        glit = elm_genlist_item_append(gl, &itci,
                                       (void *)(long)j/* item data */,
                                       NULL/* parent */,
                                       ELM_GENLIST_ITEM_NONE,
                                       NULL/* func */, NULL/* func data */);
        if (!(j & 0xf))
          {
             char buf[32];

             snprintf(buf, sizeof(buf), "%c", 'A' + ((j >> 4) & 0xf));
				 if (j == 96)
             it = elm_index_item_append(id, buf, id_cb, glit);
				 else
             elm_index_item_append(id, buf, id_cb, glit);
             if (*buf == 'G')  /* Just init dt->item later used in API test */
               api->dt.item = glit;
          }
        j += 2;
     }
   evas_object_smart_callback_add(id, "delay,changed", _index_delay_changed_cb, NULL);
   evas_object_smart_callback_add(id, "changed", _index_changed_cb, NULL);
   evas_object_smart_callback_add(id, "selected", _index_selected_cb, NULL);
   elm_index_level_go(id, 0);
	elm_index_item_selected_set(it, 1);
   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

/***********/

typedef struct _Test_Index2_Elements
{
   Evas_Object *entry, *lst, *id;
} Test_Index2_Elements;

void
test_index2_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   /* FIXME it won't be called if elm_test main window is closed */
   free(data);
}

int
test_index2_cmp(const void *data1, const void *data2)
{
   const char *label1, *label2;
   const Elm_Object_Item *li_it1 = data1;
   const Elm_Object_Item *li_it2 = data2;

   label1 = elm_object_item_text_get(li_it1);
   label2 = elm_object_item_text_get(li_it2);

   return strcasecmp(label1, label2);
}

int
test_index2_icmp(const void *data1, const void *data2)
{
   const char *label1, *label2;
   const Elm_Object_Item *index_it1 = data1;
   const Elm_Object_Item *index_it2 = data2;

   label1 = elm_index_item_letter_get(index_it1);
   label2 = elm_index_item_letter_get(index_it2);

   return strcasecmp(label1, label2);
}

void
test_index2_it_add(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Test_Index2_Elements *gui = data;
   Elm_Object_Item *list_it;
   const char *label;
   char letter[2];

   label = elm_object_text_get(gui->entry);
   snprintf(letter, sizeof(letter), "%c", label[0]);
   list_it = elm_list_item_sorted_insert(gui->lst, label, NULL, NULL, NULL,
                                         NULL, test_index2_cmp);
   elm_index_item_sorted_insert(gui->id, letter, NULL, list_it, test_index2_icmp,
                                test_index2_cmp);
   elm_list_go(gui->lst);
   /* FIXME it's not showing the recently added item */
   elm_list_item_show(list_it);
}

void
test_index2_it_del(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Test_Index2_Elements *gui = data;
   const char *label, *label_next;
   Elm_Object_Item *list_it, *list_it_next;
   Elm_Object_Item *iit;

   list_it = elm_list_selected_item_get(obj);
   list_it_next = elm_list_item_next(list_it);

   if (!list_it_next)
     {
        iit = elm_index_item_find(gui->id, list_it);
        if (iit) elm_object_item_del(iit);
        elm_object_item_del(list_it);
        return;
     }

   label = elm_object_item_text_get(list_it);
   label_next = elm_object_item_text_get(list_it_next);

   iit = elm_index_item_find(gui->id, list_it);
   if (label[0] == label_next[0])
     elm_object_item_data_set(iit, list_it_next);
   else
     elm_object_item_del(iit);

   elm_object_item_del(list_it);
}

void
test_index2_id_changed(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   elm_list_item_show(elm_object_item_data_get(event_info));
}

void
test_index2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *box, *bt;
   Test_Index2_Elements *gui;

   gui = malloc(sizeof(*gui));

   win = elm_win_add(NULL, "sorted-index-list", ELM_WIN_BASIC);
   elm_win_title_set(win, "Sorted Index and List");
   evas_object_smart_callback_add(win, "delete,request", test_index2_del, gui);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   gui->id = elm_index_add(win);
   evas_object_size_hint_weight_set(gui->id, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, gui->id);
   evas_object_smart_callback_add(gui->id, "delay,changed",
                                  test_index2_id_changed, NULL);
   evas_object_show(gui->id);

   gui->entry = elm_entry_add(win);
   elm_entry_scrollable_set(gui->entry, EINA_TRUE);
   elm_object_text_set(gui->entry, "Label");
   elm_entry_single_line_set(gui->entry, EINA_TRUE);
   evas_object_size_hint_weight_set(gui->entry, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(gui->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, gui->entry);
   evas_object_show(gui->entry);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Add");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, bt);
   evas_object_smart_callback_add(bt, "clicked", test_index2_it_add, gui);
   evas_object_show(bt);

   gui->lst = elm_list_add(win);
   elm_box_pack_end(box, gui->lst);
   evas_object_size_hint_weight_set(gui->lst, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(gui->lst, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(gui->lst, "selected", test_index2_it_del,
                                  gui);
   elm_list_go(gui->lst);
   evas_object_show(gui->lst);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}


static void
_drag_start(void *data , Evas_Object *obj, void *event_info __UNUSED__)
{
   pagectrl_data *page_data = data;
   Evas_Coord x, y, w, h;
   elm_scroller_region_get(obj, &x, &y, &w, &h);

   page_data->recent_x = x;
   page_data->drag_started = EINA_TRUE;
}

static void
_drag_stop(void *data , Evas_Object *obj, void *event_info __UNUSED__)
{
   pagectrl_data *page_data = data;
   Evas_Coord x, y, w, h;
   elm_scroller_region_get(obj, &x, &y, &w, &h);

   if ((page_data->recent_x == x) && (x != 0))
     elm_object_scroll_hold_push(obj);
}

static void
_anim_start(void *data , Evas_Object *obj, void *event_info __UNUSED__)
{
   pagectrl_data *page_data = data;
   Evas_Coord x, y, w, h;
   Elm_Object_Item *it;
   int idx;
   int threshold;
   Evas_Coord max_page_x;

   if (page_data->changed)
     {
        page_data->changed = EINA_FALSE;
        return;
     }

   if (!page_data->drag_started) return;
   max_page_x = (page_data->page_cnt - 1) * page_data->page_width;
   elm_scroller_region_get(obj, &x, &y, &w, &h);
   it = elm_index_selected_item_get(page_data->index, 0);
   if (!it) return;
   idx = (int) elm_object_item_data_get(it);
   if (x < 0) x = 0;
   if (x > max_page_x) x = max_page_x;

   threshold = page_data->page_width * 0.2;
   if (abs(page_data->recent_x - x) > threshold)
     {
        if (x > page_data->recent_x) idx++;
        else if ((x < page_data->recent_x) && (idx > 0)) idx--;
     }

   if (idx > page_data->page_cnt) idx = page_data->page_cnt;

   page_data->drag_started = EINA_FALSE;

   elm_scroller_region_bring_in(obj, page_data->page_width * idx, 0, w, h);

   it = elm_index_item_find(page_data->index, (void *) idx);
//   elm_index_item_selected_set(it, EINA_TRUE);
}

static void
_anim_stop(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   elm_object_scroll_hold_pop(obj);
}

static void
_index3_selected_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Coord x, y, w, h;
   Elm_Object_Item *it = event_info;
   int idx = (int) elm_object_item_data_get(it);
   pagectrl_data *page_data = data;

   evas_object_geometry_get(page_data->sc, &x, &y, &w, &h);
   page_data->changed = EINA_TRUE;

   elm_scroller_region_bring_in(page_data->sc, page_data->page_width * idx, 0,
                                w, h);
}

void
test_index3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *sc, *img, *id;
   char buf[PATH_MAX];
   api_data *api = calloc(1, sizeof(api_data));
   pagectrl_data *page_data = calloc(1, sizeof(pagectrl_data));
   Evas_Coord page_h;
   int i;
   Elm_Object_Item *it;
   page_data->page_width = 480;
   page_data->page_cnt = 3;

   win = elm_win_add(NULL, "index", ELM_WIN_BASIC);
   elm_win_title_set(win, "index - pagecontrol style");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   img = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/horz_scrollbar.jpg", elm_app_data_dir_get());
   elm_image_file_set(img, buf, NULL);
   elm_image_resizable_set(img, EINA_TRUE, EINA_FALSE);
   elm_image_object_size_get(img, NULL, &page_h);
   evas_object_show(img);

   page_data->sc = sc = elm_scroller_add(win);
   evas_object_smart_callback_add(sc, "scroll,drag,start", _drag_start,
                                  page_data);
   evas_object_smart_callback_add(sc, "scroll,drag,stop", _drag_stop,
                                  page_data);
   evas_object_smart_callback_add(sc, "scroll,anim,start", _anim_start,
                                  page_data);
   evas_object_smart_callback_add(sc, "scroll,anim,stop", _anim_stop,
                                  page_data);

   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_FALSE);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF,
                           ELM_SCROLLER_POLICY_OFF);
   elm_scroller_page_size_set(sc, page_data->page_width, page_h);
   elm_object_content_set(sc, img);
   elm_object_scroll_lock_y_set(sc, EINA_TRUE);
   elm_win_resize_object_add(win, sc);
   evas_object_show(sc);

   page_data->index = api->dt.id = id = elm_index_add(win);
   elm_index_horizontal_set(id, EINA_TRUE);
   elm_index_autohide_disabled_set(id, EINA_TRUE);
   elm_object_style_set(id, "pagecontrol");

   evas_object_smart_callback_add(id, "selected", _index3_selected_cb,
                                  page_data);
   evas_object_size_hint_weight_set(id, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(id, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(id);

   for(i = 0; i < page_data->page_cnt; i++)
     {
        snprintf(buf, sizeof(buf), "%c", '1' + i);
        if (i == 0)
          it = elm_index_item_append(id, buf, NULL, (void *) i);
        else
          elm_index_item_append(id, buf, NULL, (void *) i);
     }

   elm_index_level_go(id, 0);
   //elm_index_item_selected_set(it, EINA_TRUE);

   elm_win_resize_object_add(win, id);
   evas_object_resize(win, page_data->page_width, 585);
   evas_object_show(win);
}


#endif
