#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_colorselector.eo.h"

static void
_cb_size_radio_changed(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *o_bg = data;
   int size;
   size = elm_radio_value_get((Evas_Object *)obj);
   elm_bg_load_size_set(o_bg, size, size);
}

static void
_cb_radio_changed(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *o_bg = data;

   elm_bg_option_set(o_bg, elm_radio_value_get((Evas_Object *)obj));
}

static void
_cb_overlay_changed(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *o_bg = data;

   if (elm_check_state_get(obj))
     {
        Evas_Object *parent, *over;
        char buff[PATH_MAX];

        snprintf(buff, sizeof(buff), "%s/objects/test.edj", elm_app_data_dir_get());
        parent = elm_object_parent_widget_get(o_bg);
        over = edje_object_add(evas_object_evas_get(parent));
        edje_object_file_set(over, buff, "bg_overlay");
        elm_object_part_content_set(o_bg, "overlay", over);
     }
   else
     elm_object_part_content_set(o_bg, "overlay", NULL);
}

static void
_cb_color_changed(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *o_bg = data;
   double val = 0.0;

   val = elm_spinner_value_get(obj);
   if (EINA_DBL_EQ(val, 1.0))
     elm_bg_color_set(o_bg, 255, 255, 255);
   else if (EINA_DBL_EQ(val, 2.0))
     elm_bg_color_set(o_bg, 255, 0, 0);
   else if (EINA_DBL_EQ(val, 3.0))
     elm_bg_color_set(o_bg, 0, 0, 255);
   else if (EINA_DBL_EQ(val, 4.0))
     elm_bg_color_set(o_bg, 0, 255, 0);
}

void
test_bg_plain(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg;

   win = elm_win_add(NULL, "bg-plain", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Plain");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   /* allow bg to expand in x & y */
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   /* set size hints. a minimum size for the bg. this should propagate back
    * to the window thus limiting its size based off the bg as the bg is one
    * of the window's resize objects. */
   evas_object_size_hint_min_set(bg, 160, 160);
   /* and set a maximum size. not needed very often. normally used together
    * with evas_object_size_hint_min_set() at the same size to make a
    * window not resizable */
   evas_object_size_hint_max_set(bg, 640, 640);
   /* and now just resize the window to a size you want. normally widgets
    * will determine the initial size though */
   evas_object_resize(win, 320, 320);
   /* and show the window */
   evas_object_show(win);
}

void
test_bg_image(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg;
   Evas_Object *box, *hbox, *o_bg;
   Evas_Object *rd, *rdg;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "bg-image", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Image");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   o_bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_bg_file_set(o_bg, buf, NULL);
   evas_object_size_hint_weight_set(o_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o_bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, o_bg);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 50);
   elm_object_text_set(rd, "50 x 50");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_size_radio_changed, o_bg);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   rdg = rd;

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 100);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "100 x 100");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_size_radio_changed, o_bg);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 200);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "200 x 200");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_size_radio_changed, o_bg);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);

   elm_radio_value_set(rdg, 200);

   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   evas_object_show(o_bg);
   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

void
test_bg_options(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg;
   Evas_Object *box, *hbox, *o_bg;
   Evas_Object *rd, *rdg;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "bg-options", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Options");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   o_bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_bg_file_set(o_bg, buf, NULL);
   evas_object_size_hint_weight_set(o_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o_bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, o_bg);
   evas_object_show(o_bg);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, ELM_BG_OPTION_CENTER);
   elm_object_text_set(rd, "Center");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_radio_changed, o_bg);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   rdg = rd;

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, ELM_BG_OPTION_SCALE);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "Scale");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_radio_changed, o_bg);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, ELM_BG_OPTION_STRETCH);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "Stretch");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_radio_changed, o_bg);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, ELM_BG_OPTION_TILE);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "Tile");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_radio_changed, o_bg);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);

   elm_radio_value_set(rdg, ELM_BG_OPTION_SCALE);

   rd = elm_check_add(win);
   elm_object_text_set(rd, "Show Overlay");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_overlay_changed, o_bg);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);

   /* color choices ... this is ghetto, but we don't have a 'colorpicker'
    * widget yet :( */
   rd = elm_spinner_add(win);
   elm_object_tooltip_text_set(rd, "The background's part being affected<br/>"
                                   "here may be seen only if you enlarge<br/>"
                                   "the window and mark the 'Center' radio.");
   elm_object_style_set(rd, "vertical");
   elm_spinner_min_max_set(rd, 1, 4);
   elm_spinner_label_format_set(rd, "%.0f");
   elm_spinner_editable_set(rd, EINA_FALSE);
   elm_spinner_special_value_add(rd, 1, "White");
   elm_spinner_special_value_add(rd, 2, "Red");
   elm_spinner_special_value_add(rd, 3, "Blue");
   elm_spinner_special_value_add(rd, 4, "Green");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_color_changed, o_bg);
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);

   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
_colorsel_cb(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Efl_Ui_Win *win = data;
   int r, g, b, a;

   // Solid color API
   elm_colorselector_color_get(obj, &r, &g, &b, &a);
   efl_gfx_color_set(efl_part(win, "background"), r, g, b, a);
   efl_gfx_color_get(efl_part(win, "background"), &r, &g, &b, &a);
   printf("bg color: %d %d %d %d\n", r, g, b, a);
   fflush(stdout);
}

static void
_file_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Win *win = data;
   const char *f, *k;

   // File API
   efl_file_get(efl_part(win, "background"), &f, &k);
   if (f)
     {
        efl_file_set(efl_part(win, "background"), NULL, NULL);
     }
   else
     {
        efl_file_get(ev->object, &f, &k);
        efl_file_set(efl_part(win, "background"), f, k);
     }
}

static void
_image_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Win *win = data, *o;
   const char *f, *k;

   // Content API
   if (efl_content_get(efl_part(win, "background")))
     efl_content_set(efl_part(win, "background"), NULL);
   else
     {
        efl_file_get(ev->object, &f, &k);
        o = efl_add(EFL_UI_IMAGE_CLASS, win,
                    efl_gfx_image_scale_type_set(efl_added, EFL_GFX_IMAGE_SCALE_TYPE_FIT_OUTSIDE),
                    efl_file_set(efl_added, f, k)
                    );
        efl_content_set(efl_part(win, "background"), o);
     }
}

void
test_bg_window(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *cs;
   char buf[PATH_MAX];

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Bg EOAPI (Efl.Ui.Win)"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE),
                 efl_ui_win_alpha_set(efl_added, 1));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                 efl_gfx_size_hint_weight_set(efl_added, 1, 1),
                 efl_content_set(win, efl_added));

   cs = elm_colorselector_add(win);
   elm_colorselector_mode_set(cs, ELM_COLORSELECTOR_PALETTE);
   elm_colorselector_palette_color_add(cs, 64, 64, 64, 255);
   elm_colorselector_palette_color_add(cs, 255, 128, 128, 255);
   elm_colorselector_palette_color_add(cs, 0, 64, 64, 64);
   elm_colorselector_palette_color_add(cs, 0, 0, 0, 0);
   evas_object_smart_callback_add(cs, "color,item,selected", _colorsel_cb, win);
   evas_object_size_hint_align_set(cs, 0.5, 0.5);
   evas_object_show(cs);
   efl_pack(box, cs);

   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   efl_add(EFL_UI_IMAGE_CLASS, win,
           efl_file_set(efl_added, buf, NULL),
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(64, 64)),
           efl_gfx_size_hint_align_set(efl_added, 0.5, 0.5),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _file_cb, win),
           efl_pack(box, efl_added));

   snprintf(buf, sizeof(buf), "%s/images/sky_04.jpg", elm_app_data_dir_get());
   efl_add(EFL_UI_IMAGE_CLASS, win,
           efl_file_set(efl_added, buf, NULL),
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(64, 64)),
           efl_gfx_size_hint_align_set(efl_added, 0.5, 0.5),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _image_cb, win),
           efl_pack(box, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 200));
}

static void
_cb_radio_changed_scale_type(void *data, const Efl_Event *ev)
{
   Evas_Object *o_bg = data;

   efl_gfx_image_scale_type_set(o_bg, efl_ui_nstate_value_get(ev->object));
}

static void
_cb_check_changed_scale_type(void *data, const Efl_Event *ev)
{
   Evas_Object *o_bg = data;
   int r, g, b, a;

   if (efl_ui_check_selected_get(ev->object))
     efl_gfx_color_set(o_bg, 255, 128, 128, 255);
   else
     efl_gfx_color_set(o_bg, 0, 0, 0, 0);

   efl_gfx_color_get(o_bg, &r, &g, &b, &a);
   printf("bg color: %d %d %d %d\n", r, g, b, a);
   printf("bg hex color code: %s\n", efl_gfx_color_code_get(o_bg));
   fflush(stdout);
}

void
test_bg_scale_type(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win;
   Evas_Object *box, *hbox, *o_bg;
   Evas_Object *rd, *rdg;
   char buf[PATH_MAX];

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Bg EOAPI (Efl.Ui.Bg)"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE),
                 efl_ui_win_alpha_set(efl_added, EINA_FALSE));

   efl_add(EFL_UI_BG_CLASS, win,
           efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, EFL_GFX_SIZE_HINT_EXPAND),
           efl_content_set(win, efl_added));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
                 efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, EFL_GFX_SIZE_HINT_EXPAND),
                 efl_content_set(win, efl_added));

   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());

   o_bg = efl_add(EFL_UI_BG_CLASS, box,
                  efl_file_set(efl_added, buf, NULL),
                  efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, EFL_GFX_SIZE_HINT_EXPAND),
                  efl_gfx_size_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE),
                  efl_pack(box, efl_added));

   hbox = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                  efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, 0.0),
                  efl_gfx_size_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE),
                  efl_pack(box, efl_added));

   rdg = rd = efl_add(EFL_UI_RADIO_CLASS, hbox,
                efl_ui_radio_state_value_set(efl_added, EFL_GFX_IMAGE_SCALE_TYPE_FILL),
                efl_text_set(efl_added, "Fill"),
                efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, 0.0),
                efl_event_callback_add(efl_added, EFL_UI_RADIO_EVENT_CHANGED, _cb_radio_changed_scale_type, o_bg),
                efl_pack(hbox, efl_added));

   rd = efl_add(EFL_UI_RADIO_CLASS, hbox,
                efl_ui_radio_state_value_set(efl_added, EFL_GFX_IMAGE_SCALE_TYPE_FIT_INSIDE),
                efl_ui_radio_group_add(efl_added, rdg),
                efl_text_set(efl_added, "Fit Inside"),
                efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, 0.0),
                efl_event_callback_add(efl_added, EFL_UI_RADIO_EVENT_CHANGED, _cb_radio_changed_scale_type, o_bg),
                efl_pack(hbox, efl_added));


   rd = efl_add(EFL_UI_RADIO_CLASS, hbox,
                efl_ui_radio_state_value_set(efl_added, EFL_GFX_IMAGE_SCALE_TYPE_FIT_OUTSIDE),
                efl_ui_radio_group_add(efl_added, rdg),
                efl_text_set(efl_added, "Fit Outside"),
                efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, 0.0),
                efl_event_callback_add(efl_added, EFL_UI_RADIO_EVENT_CHANGED, _cb_radio_changed_scale_type, o_bg),
                efl_pack(hbox, efl_added));

   rd = efl_add(EFL_UI_RADIO_CLASS, hbox,
                efl_ui_radio_state_value_set(efl_added, EFL_GFX_IMAGE_SCALE_TYPE_NONE),
                efl_ui_radio_group_add(efl_added, rdg),
                efl_text_set(efl_added, "None"),
                efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, 0.0),
                efl_event_callback_add(efl_added, EFL_UI_RADIO_EVENT_CHANGED, _cb_radio_changed_scale_type, o_bg),
                efl_pack(hbox, efl_added));

   rd = efl_add(EFL_UI_RADIO_CLASS, hbox,
                efl_ui_radio_state_value_set(efl_added, EFL_GFX_IMAGE_SCALE_TYPE_TILE),
                efl_ui_radio_group_add(efl_added, rdg),
                efl_text_set(efl_added, "Tile"),
                efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, 0.0),
                efl_event_callback_add(efl_added, EFL_UI_RADIO_EVENT_CHANGED, _cb_radio_changed_scale_type, o_bg),
                efl_pack(hbox, efl_added));

   efl_ui_nstate_value_set(rdg, EFL_GFX_IMAGE_SCALE_TYPE_FILL);

   efl_add(EFL_UI_CHECK_CLASS, hbox,
                efl_text_set(efl_added, "Bg Color"),
                efl_gfx_size_hint_weight_set(efl_added, EFL_GFX_SIZE_HINT_EXPAND, 0.0),
                efl_event_callback_add(efl_added, EFL_UI_CHECK_EVENT_CHANGED, _cb_check_changed_scale_type, o_bg),
                efl_pack(hbox, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 200));
}


