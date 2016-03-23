#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   void *inwin;
};
typedef struct _api_data api_data;

enum _api_state
{
   CONTENT_UNSET,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   Evas_Object *t;
   switch(api->state)
     {
      case CONTENT_UNSET:
         t = elm_win_inwin_content_unset(api->inwin);
         evas_object_del(t);
         t = elm_label_add(elm_object_parent_widget_get(api->inwin));
         elm_object_text_set(t, "Content was unset.<br>DONE!");
         elm_win_inwin_content_set(api->inwin, t);
         evas_object_show(t);

      default:
         return;
     }
}

static void
_api_bt_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
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

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   free(data);
}

void
test_inwin(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *inwin, *lb, *bxx, *bt;
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("inwin", "InWin");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   inwin = elm_win_inwin_add(win);
   api->inwin = inwin;
   evas_object_show(inwin);

   bxx = elm_box_add(inwin);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bxx);

   bt = elm_button_add(inwin);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "This is an \"inwin\" - a window in a<br/>"
                       "window. This is handy for quick popups<br/>"
                       "you want centered, taking over the window<br/>"
                       "until dismissed somehow. Unlike hovers they<br/>"
                       "don't hover over their target.");
   elm_box_pack_end(bxx, lb);
   elm_win_inwin_content_set(inwin, bxx);
   evas_object_show(lb);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);
}

void
test_inwin2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *inwin, *lb;

   win = elm_win_util_standard_add("inwin2", "InWin 2");
   elm_win_autodel_set(win, EINA_TRUE);

   inwin = elm_win_inwin_add(win);
   elm_object_style_set(inwin, "minimal_vertical");
   evas_object_show(inwin);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "This is an \"inwin\" - a window in a<br/>"
                       "window. This is handy for quick popups<br/>"
                       "you want centered, taking over the window<br/>"
                       "until dismissed somehow. Unlike hovers they<br/>"
                       "don't hover over their target.<br/>"
                       "<br/>"
                       "This \"minimal_vertical\" inwin style compacts<br/>"
                       "itself vertically to the size of its contents<br/> "
                       "minimum size.");
   elm_win_inwin_content_set(inwin, lb);
   evas_object_show(lb);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);
}
