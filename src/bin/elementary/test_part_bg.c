#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define MAX_NUM_OF_CONTENT 18

const Efl_Class *content_class[MAX_NUM_OF_CONTENT];


static void
_reset_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Evas_Object *radio = data;
   Evas_Object *target;

   radio = efl_ui_radio_selected_object_get(radio);
   target = evas_object_data_get(radio, "data");

   efl_gfx_color_set(efl_part(target, "background"), 0, 0, 0, 0);
   efl_file_simple_load(efl_part(target, "background"), NULL, NULL);
}

static void
_color_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Evas_Object *radio = data;
   Evas_Object *target;
   static Eina_Bool i;

   radio = efl_ui_radio_selected_object_get(radio);
   target = evas_object_data_get(radio, "data");
   i ^= EINA_TRUE;
   efl_gfx_color_set(efl_part(target, "background"), (i) ? 255 : 0, (i) ? 0 : 255, 0, 255);
}

static void
_scale_type_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Evas_Object *radio = data;
   Evas_Object *target;
   Efl_Gfx_Image_Scale_Type type;
   char buf[PATH_MAX];

   radio = efl_ui_radio_selected_object_get(radio);
   target = evas_object_data_get(radio, "data");

   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   efl_file_simple_load(efl_part(target, "background"), buf, NULL);
   type = efl_gfx_image_scale_type_get(efl_part(target, "background"));
   type = (type + 1) % 5;
   efl_gfx_image_scale_type_set(efl_part(target, "background"), type);
}

static Evas_Object *
_create_box_contents(Evas_Object *box)
{
   Evas_Object *hbox;
   Evas_Object *radio_group, *radio;
   Evas_Object *content;
   char buf[PATH_MAX];
   unsigned int i;

   hbox = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                  efl_gfx_hint_weight_set(efl_added, 1, 1),
                  efl_pack_end(box, efl_added));

   radio_group = radio = efl_add(EFL_UI_RADIO_CLASS, hbox,
                                 efl_pack_end(hbox, efl_added),
                                 efl_gfx_hint_weight_set(efl_added, 0, 0),
                                 efl_ui_radio_state_value_set(efl_added, 0));

   content = efl_add(EFL_UI_BOX_CLASS, hbox,
                     efl_pack_end(hbox, efl_added));
   evas_object_data_set(radio, "data", content);

   content = efl_add(EFL_UI_TEXT_CLASS, content,
                     efl_text_set(efl_added, "box"),
                     efl_text_interactive_editable_set(efl_added, EINA_FALSE),
                     efl_pack_end(content, efl_added));

   for (i = 0; i < MAX_NUM_OF_CONTENT; i++)
     {
        if (!content_class[i]) continue;

        hbox = efl_add(EFL_UI_BOX_CLASS, box,
                       efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                       efl_pack_end(box, efl_added));

        radio = efl_add(EFL_UI_RADIO_CLASS, hbox,
                        efl_ui_radio_group_add(efl_added, radio_group),
                        efl_pack_end(hbox, efl_added),
                        efl_ui_radio_state_value_set(efl_added, i + 1),
                        efl_gfx_hint_weight_set(efl_added, 0, 0));

        content = efl_add(content_class[i], hbox,
                          efl_pack_end(hbox, efl_added));

        if (efl_isa(content, efl_text_interface_get()))
          efl_text_set(content, "text");

        if (efl_isa(content, EFL_UI_IMAGE_CLASS))
          {
             snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
             efl_file_simple_load(content, buf, NULL);
          }

        evas_object_data_set(radio, "data", content);
     }

   efl_ui_nstate_value_set(radio_group, 0);

   return radio_group;
}

void
test_part_background(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win;
   Evas_Object *scr;
   Evas_Object *vbox, *hbox;
   Evas_Object *radio_group;
   Evas_Object *reset_btn, *color_btn, *scale_btn;

   content_class[0] = EFL_UI_CALENDAR_CLASS;
   content_class[1] = EFL_UI_SLIDER_CLASS;
   content_class[2] = EFL_UI_SLIDER_INTERVAL_CLASS;
   content_class[3] = EFL_UI_PROGRESSBAR_CLASS;
   content_class[4] = EFL_UI_CLOCK_CLASS;
   content_class[5] = EFL_UI_BUTTON_CLASS;
   content_class[6] = EFL_UI_CHECK_CLASS;
   content_class[7] = EFL_UI_RADIO_CLASS;
   content_class[8] = EFL_UI_TEXT_CLASS;
   content_class[9] = EFL_UI_FLIP_CLASS;
   content_class[10] = EFL_UI_PANES_CLASS;
   content_class[11] = EFL_UI_VIDEO_CLASS;
   content_class[12] = EFL_UI_BG_CLASS;
   content_class[13] = EFL_UI_IMAGE_CLASS;
   content_class[14] = EFL_UI_IMAGE_ZOOMABLE_CLASS;
   content_class[15] = EFL_UI_NSTATE_CLASS;
   content_class[16] = EFL_UI_SPIN_CLASS;
   content_class[17] = EFL_UI_SPIN_BUTTON_CLASS;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Widget Part Background"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   vbox = efl_add(EFL_UI_BOX_CLASS, win,
                  efl_gfx_hint_weight_set(efl_added, 1, 1),
                  efl_content_set(win, efl_added));

   hbox = efl_add(EFL_UI_BOX_CLASS, vbox,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                  efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
                  efl_pack_end(vbox, efl_added));

   reset_btn = efl_add(EFL_UI_BUTTON_CLASS, hbox,
                       efl_pack_end(hbox, efl_added),
                       efl_text_set(efl_added, "reset"));
   color_btn = efl_add(EFL_UI_BUTTON_CLASS, hbox,
                       efl_pack_end(hbox, efl_added),
                       efl_text_set(efl_added, "color"));
   scale_btn = efl_add(EFL_UI_BUTTON_CLASS, hbox,
                       efl_pack_end(hbox, efl_added),
                       efl_text_set(efl_added, "scale_type"));

   scr = elm_scroller_add(vbox);
   evas_object_show(scr);
   evas_object_size_hint_weight_set(scr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(scr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_pack_end(vbox, scr);

   vbox = efl_add(EFL_UI_BOX_CLASS, scr,
                  efl_gfx_hint_weight_set(efl_added, 1, 1),
                  efl_content_set(scr, efl_added));

   radio_group = _create_box_contents(vbox);

   efl_event_callback_add(reset_btn, EFL_UI_EVENT_CLICKED, _reset_cb, radio_group);
   efl_event_callback_add(color_btn, EFL_UI_EVENT_CLICKED, _color_cb, radio_group);
   efl_event_callback_add(scale_btn, EFL_UI_EVENT_CLICKED, _scale_type_cb, radio_group);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 200));
}
