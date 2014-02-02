#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define BUTTON_TEXT_SET(BT, TEXT) \
   elm_object_text_set((BT), (TEXT)); \
   elm_object_tooltip_text_set((BT), (TEXT)); \
   elm_object_tooltip_window_mode_set((BT), EINA_TRUE)

static const char *img1 = PACKAGE_DATA_DIR "/images/logo.png";
static const char *img2 = PACKAGE_DATA_DIR "/images/plant_01.jpg";
static const char *img3 = PACKAGE_DATA_DIR "/images/rock_01.jpg";
static const char *img4 = PACKAGE_DATA_DIR "/images/rock_02.jpg";
static const char *img5 = PACKAGE_DATA_DIR "/images/sky_01.jpg";
static const char *img6 = PACKAGE_DATA_DIR "/images/sky_02.jpg";
static const char *img7 = PACKAGE_DATA_DIR "/images/sky_03.jpg";

static char *
gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(uintptr_t)data);
   return strdup(buf);
}

static Evas_Object *
gl_content_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic = elm_icon_add(obj);
   if (!strcmp(part, "elm.swallow.end"))
     snprintf(buf, sizeof(buf), "%s/images/bubble.png", elm_app_data_dir_get());
   else
     snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static Elm_Genlist_Item_Class itc =
{
   .version = ELM_GENLIST_ITEM_CLASS_VERSION,
   .item_style = "default",
   .func =
     {
        .text_get = gl_text_get,
        .content_get  = gl_content_get,
        .state_get = NULL,
        .del       = NULL
     }
};

static Evas_Object *
_content_image_new(Evas_Object *parent, const char *img)
{
   Evas_Object *ic;

   ic = elm_icon_add(parent);
   elm_image_file_set(ic, img, NULL);
   elm_image_fill_outside_set(ic, EINA_FALSE);
   return ic;
}

static void
_gl_selected(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("selected: %p\n", event_info);
}

static void
_gl_double_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("double clicked: %p\n", event_info);
}

static void
_gl_longpress(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("longpress %p\n", event_info);
}

static void
_content_list_populate_cb(Evas_Object *gl)
{
   int i;
   for (i = 0; i < 10; i++)
     elm_genlist_item_append(gl, &itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
   if (elm_genlist_items_count(gl) < 500)
     ecore_job_add((Ecore_Cb)_content_list_populate_cb, gl);
}

static Evas_Object *
_content_list_new(Evas_Object *parent)
{
   Evas_Object *gl;

   gl = elm_genlist_add(parent);
   evas_object_smart_callback_add(gl, "selected", _gl_selected, NULL);
   evas_object_smart_callback_add(gl, "clicked,double", _gl_double_clicked, NULL);
   evas_object_smart_callback_add(gl, "longpressed", _gl_longpress, NULL);
   // FIXME: This causes genlist to resize the horiz axis very slowly :(
   // Reenable this and resize the window horizontally, then try to resize it back
   //elm_genlist_mode_set(gl, ELM_LIST_LIMIT);

   ecore_job_add((Ecore_Cb)_content_list_populate_cb, gl);
   return gl;
}

static void
_navi_pop(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_naviframe_item_pop(data);
}

static void
_navi_it_del(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_object_item_del(data);
}

static void
_title_visible(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_naviframe_item_title_enabled_set(data,
                                        !elm_naviframe_item_title_enabled_get(data),
                                        EINA_TRUE);
}

static void
_promote(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_naviframe_item_promote(data);
}

static void
_page7(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *bt2, *content, *nf = data;
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt, "Page 6");

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Page 1");
   evas_object_smart_callback_add(bt2, "clicked", _promote,
                                  evas_object_data_get(nf, "page1"));
   content = _content_image_new(nf, img6);
   it = elm_naviframe_item_push(nf, "Page 7", bt, bt2, content, "overlap");
   elm_object_item_part_text_set(it, "subtitle", "Overlap style!");

   evas_object_smart_callback_add(bt, "clicked", _navi_pop, nf);
}

static void
_page6(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *bt2, *content, *nf = data;
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt, "Page 5");

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Page 7");
   evas_object_smart_callback_add(bt2, "clicked", _page7, nf);

   content = _content_image_new(nf, img7);
   it = elm_naviframe_item_push(nf, "Page 6", bt, bt2, content, "overlap");
   elm_object_item_part_text_set(it, "subtitle", "Overlap style!");

   evas_object_smart_callback_add(bt, "clicked", _navi_pop, nf);
}

static void
_page5(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *bt2, *content, *nf = data;
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt, "Page 4");

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Page 6");
   evas_object_smart_callback_add(bt2, "clicked", _page6, nf);

   content = _content_image_new(nf, img5);
   it = elm_naviframe_item_insert_after(nf,
                                        elm_naviframe_top_item_get(nf),
                                        "Page 5",
                                        bt,
                                        bt2,
                                        content,
                                        NULL);
   elm_object_item_part_text_set(it, "subtitle", "This page is inserted without transition");
   evas_object_smart_callback_add(bt, "clicked", _navi_it_del, it);
}

static void
_page4(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *ic, *content, *nf = data;
   char buf[PATH_MAX];
   Elm_Object_Item *it;

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/icon_right_arrow.png",
            elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bt = elm_button_add(nf);
   evas_object_smart_callback_add(bt, "clicked", _page5, nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_content_set(bt, "icon", ic);

   content = _content_image_new(nf, img4);

   it = elm_naviframe_item_push(nf,
                                "Page 4",
                                NULL,
                                bt,
                                content,
                                NULL);
   elm_object_item_part_text_set(it, "subtitle", "Title area visibility test");

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_object_item_part_content_set(it, "icon", ic);
   elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
   evas_object_smart_callback_add(content, "clicked", _title_visible, it);
}

static void
_page3(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *bt2, *ic, *content, *nf = data;
   char buf[PATH_MAX];
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt, "Prev");
   evas_object_smart_callback_add(bt, "clicked", _navi_pop, nf);

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Next");
   evas_object_smart_callback_add(bt2, "clicked", _page4, nf);

   content = _content_image_new(nf, img3);

   it = elm_naviframe_item_push(nf,
                                "Page 3",
                                bt,
                                bt2,
                                content,
                                NULL);
   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   elm_object_item_part_content_set(it, "icon", ic);
}

static void
_page2(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *content, *ic, *nf = data;
   char buf[PATH_MAX];
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bt, "clicked", _page3, nf);

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/icon_right_arrow.png",
            elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_object_part_content_set(bt, "icon", ic);

   content = _content_image_new(nf, img2);

   it = elm_naviframe_item_push(nf, "Page 2", NULL, bt, content,  NULL);
   elm_object_item_part_text_set(it, "subtitle", "Here is sub-title part!");
}

void
test_naviframe_complex(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *nf, *btn, *box, *content, *tb, *o;
   Elm_Object_Item *it;
   int x;

   win = elm_win_util_standard_add("naviframe", "Naviframe");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(box, EINA_TRUE);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   tb = elm_toolbar_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_toolbar_horizontal_set(tb, EINA_FALSE);
   elm_toolbar_select_mode_set(tb, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_box_pack_end(box, tb);
   evas_object_show(tb);

   nf = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(nf, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, nf);
   evas_object_show(nf);

   for (x = 0; x < 4; x++)
     {
        const char *label[] = { "View 1", "View 2", "View 3", "View 4" };
        o = elm_naviframe_add(win);
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

        btn = elm_button_add(o);
        evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(btn, "clicked", _page2, o);
        BUTTON_TEXT_SET(btn, "Next");
        evas_object_show(btn);

        if (x % 2)
          content = _content_image_new(o, img1);
        else
          content = _content_list_new(o);
        evas_object_data_set(o, "page1", elm_naviframe_item_push(o, "Page 1", NULL, btn, content, NULL));

        it = elm_naviframe_item_simple_push(nf, o);
        elm_toolbar_item_append(tb, NULL, label[x], _promote, it);
        evas_object_show(o);
     }

   elm_toolbar_item_selected_set(elm_toolbar_last_item_get(tb), EINA_TRUE);
   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
