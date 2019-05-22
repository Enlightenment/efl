#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include <Efl_Ui.h>


static Evas_Object *
_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
   const int size = ELM_SCALE_SIZE(48);

   Evas_Object *content, *ly;
   int num = (int)(uintptr_t)data;
   char buf[PATH_MAX], bufimg[PATH_MAX], buftxt[256];

   snprintf(buf, sizeof(buf), "%s/objects/test_masking.edj", elm_app_data_dir_get());

   ly = elm_layout_add(obj);
   elm_layout_file_set(ly, buf, "masking");
   if (!strcmp(part, "elm.swallow.icon"))
     {
        content = elm_icon_add(ly);
        //elm_image_async_open_set(icon, 1);
        snprintf(bufimg, sizeof(bufimg), "%s/images/%s",
                 elm_app_data_dir_get(), (num&1) ? "sky_01.jpg" : "rock_01.jpg");
        elm_image_file_set(content, bufimg, NULL);
        evas_object_size_hint_min_set(content, size, size);
        evas_object_size_hint_max_set(content, size, size);
     }
   else // if (!strcmp(part, "elm.swallow.end"))
     {
        content = elm_layout_add(obj);
        elm_layout_file_set(content, buf, "text");
        sprintf(buftxt, "# %d #", num);
        elm_layout_text_set(content, "text", buftxt);
     }
   elm_object_part_content_set(ly, "content", content);

   return ly;
}

static char *
_gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[64];
   sprintf(buf, "Genlist item %u", (unsigned)(uintptr_t)data);
   return strdup(buf);
}

static Eina_Bool
_gl_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_gl_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED)
{

}

static void
_toggle_mask(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *ly = data;

   if (!efl_key_data_get(ly, "unclipped"))
     {
        elm_layout_signal_emit(ly, "unclip", "elm_test");
        efl_key_data_set(ly, "unclipped", "yup");
     }
   else
     {
        elm_layout_signal_emit(ly, "clip", "elm_test");
        efl_key_data_set(ly, "unclipped", NULL);
     }
}

static void
_toggle_map(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *ly = data;

   if (!efl_gfx_mapping_has(ly))
     {
        efl_gfx_mapping_zoom(ly, 0.8, 0.8, NULL, 0.5, 0.5);
        efl_gfx_mapping_rotate(ly, 45, NULL, 0.5, 0.5);
     }
   else efl_gfx_mapping_reset(ly);
}

static void
_rotate_win(void *data, const Efl_Event *ev EINA_UNUSED)
{
   //Efl_Gfx_Orientation orient;
   Eo *win = data;

   // FIXME: This is not implemented???
   //orient = efl_gfx_orientation_get(win);
   //efl_gfx_orientation_set(win, (orient + 90) % 360);
   elm_win_rotation_set(win, (elm_win_rotation_get(win) + 90) % 360);
}

void
test_evas_mask(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *o, *gl, *ly, *box2;
   Elm_Genlist_Item_Class *itc;
   char buf[PATH_MAX];

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Evas masking demo"),
                 efl_ui_win_autodel_set(efl_added, 1));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL));
   efl_content_set(win, box);

   // FIXME: No API to set background as "tile" :(
   snprintf(buf, sizeof(buf), "%s/images/pm_fill.png", elm_app_data_dir_get());
   efl_file_simple_load(efl_part(win, "background"), buf, NULL);

   // FIXME: layout EO API
   snprintf(buf, sizeof(buf), "%s/objects/test_masking.edj", elm_app_data_dir_get());
   ly = efl_add(EFL_UI_LAYOUT_CLASS, win,
                efl_file_set(efl_added, buf),
                efl_file_key_set(efl_added, "masking"));
   efl_pack(box, ly);

   // FIXME: No genlist in EO API
   o = gl = elm_genlist_add(win);
   elm_genlist_homogeneous_set(gl, 1);
   efl_gfx_hint_align_set(o, -1, -1);
   efl_gfx_hint_weight_set(o, 1, 1);

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.content_get = _gl_content_get;
   itc->func.text_get = _gl_text_get;
   itc->func.state_get = _gl_state_get;
   itc->func.del = _gl_del;

   for (int i = 0; i < 64; i++)
     {
        elm_genlist_item_append(gl, itc,
                                (void *)(uintptr_t)i,
                                NULL, // parent
                                ELM_GENLIST_ITEM_NONE,
                                NULL, // func
                                NULL); // data
     }

   elm_genlist_item_class_free(itc);
   efl_content_set(efl_part(ly, "content"), gl);

   box2 = efl_add(EFL_UI_BOX_CLASS, win,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_LTR),
                  efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
                  efl_pack(box, efl_added));

   // FIXME: button EO API
   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Toggle mask"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _toggle_mask, ly),
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_pack(box2, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Toggle map"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _toggle_map, ly),
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_pack(box2, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Rotate Window"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _rotate_win, win),
           efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
           efl_pack(box2, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 600));
}
