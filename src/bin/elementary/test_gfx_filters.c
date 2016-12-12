#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

const int default_font_size = 48;

typedef struct _Filter_Image
{
   const char *path;
   const char *src_name;
} Filter_Image;

typedef struct _Filter
{
   const char *name;
   const char *code;
   const Filter_Image *images;
} Filter;

static const Filter_Image images[] = {
   { "plant_01.jpg", "plant" },
   { "rock_01.jpg", "rock" },
   { "rock_02.jpg", "roll" },
   { "sky_01.jpg", "sky" },
   { "sky_04.jpg", "cloud" },
   { "wood_01.jpg", "wood" },
   { "icon_00.png", "love" },
   { NULL, NULL }
};

static const Filter_Image images_wood[] = {
   { "wood_01.jpg", "wood" },
   { NULL, NULL },
};

static const Filter_Image images_cloud[] = {
   { "sky_04.jpg", "cloud" },
   { NULL, NULL },
};

/* builtin filter examples */
static const Filter templates[] = {
   { "Custom", NULL, NULL },
   { "Black shadow",
     "blur { 6, ox = 2, oy = 2, color = 'black' }\n"
     "blend {}", NULL },
   { "Fire glow",
     "a = buffer { 'alpha' }\n"
     "grow { 10, dst = a }\n"
     "blur { 15, oy = -4, ox = 1, src = a, color = '#900' }\n"
     "blur { 5, oy = -1, color = 'orange' }\n"
     "blur { 2, color = 'yellow' }\n"
     "grow { -2 }\n", NULL },
   { "Outer glow",
     "inv = function (x) return 255 - x end\n\n"
     "a = buffer {'alpha'}\n"
     "curve { inv, dst = a }\n\n"
     "b = buffer { 'alpha' }\n"
     "grow { 3, dst = b }\n\n"
     "c = buffer {}\n"
     "blur { 6, color = 'blue', dst = c, src = b }\n"
     "blur { 2, color = 'cyan', dst = c, src = b }\n"
     "mask { a, c }", NULL },
   { "Inset light",
     "inv = function (x) return 255 - x end\n\n"
     "a = buffer {'alpha'}\n"
     "curve { inv, dst = a }\n\n"
     "b = buffer {}\n"
     "blur { 2, ox = 1, oy = 1, src = a, dst = b, color = 'yellow' }\n\n"
     "blend { color = 'black' }\n"
     "mask { input, b }\n", NULL },
   { "Rocking it",
     "padding_set { 40 }\n"
     "local len = state.scale * 10\n\n"
     "a = buffer { 'alpha' }\n"
     "grow { len / 2, dst = a }\n"
     "blur { len * 2, oy = len, ox = len / 2, color = 'black', src = a }\n"
     "for i = len,0,-1 do\n"
     "  c = 128 / len * (len - i)\n"
     "  blur { 2, oy = i, ox = i/2, color = color { c * 0.8, c * 0.9, c } }\n"
     "end\n"
     "blend {}\n", NULL },
   { "Bumping on",
     "a = buffer { 'alpha' }\n"
     "grow { 2, dst = a }\n"
     "blur { 4, src = a, dst = a }\n\n"
     "bump { a, color = '#333', specular = 10, compensate = true, depth = 15 }\n", NULL },
   { "Modern art",
     "local inv = function (x) return 255 - x end\n\n"
     "a = buffer {}\n"
     "b = buffer {'alpha'}\n"
     "blend { dst = a, ox = 6, oy = -6, color = '#0f0' }\n"
     "blend { dst = a, ox = -6, oy = 6, color = '#f00' }\n"
     "blend { dst = a, ox = -3, oy = -3, color = '#f8f' }\n"
     "blend { dst = a, ox = 3, oy = 3, color = '#00f' }\n"
     "curve { inv, dst = b }\n"
     "blur { 20, color = '#fffa' }\n"
     "mask { a, b }\n", NULL },
   { "Forestry",
     "wood = buffer { src = 'wood' }\n"
     "mask { wood, fillmode = 'stretch' }", images_wood },
   { "Displaced cloud",
     "cloud = buffer { src = 'cloud' }\n"
     "displace { cloud, intensity = 10, fillmode = 'stretch' }", images_cloud },
};


static void
_spinner_fill(Eo *obj)
{
   const Filter *f;
   size_t k;

   for (k = 0; k < EINA_C_ARRAY_LENGTH(templates); k++)
     {
        f = &templates[k];
        elm_spinner_special_value_add(obj, (double) k, f->name);
     }
   elm_spinner_editable_set(obj, EINA_FALSE);
   elm_spinner_min_max_set(obj, 0, k - 1);
   elm_obj_spinner_wrap_set(obj, 1);
}

static void
_spinner_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *text, *code, *spinner;
   int k;

   text = efl_key_wref_get(win, "text");
   code = efl_key_wref_get(win, "code");
   spinner = efl_key_wref_get(win, "spinner");
   k = (int) round(elm_spinner_value_get(spinner));
   if (k > 0)
     {
        const Filter *f = &templates[k];

        efl_gfx_filter_program_set(text, f->code, f->name);
        efl_text_set(code, f->code);

        if (f->images)
          {
             for (size_t j = 0; f->images[j].path; j++)
               {
                  const char *name = f->images[j].src_name;
                  Eo *source_obj = efl_name_find(win, name);
                  efl_gfx_filter_source_set(text, name, source_obj);
               }
          }
     }
}

static void
_code_changed(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *text, *code, *spinner;
   Eina_Strbuf *buf = NULL;
   const char *source;

   text = efl_key_wref_get(win, "text");
   code = efl_key_wref_get(win, "code");
   spinner = efl_key_wref_get(win, "spinner");

   source = efl_text_get(code);
   {
      // Fix up paragrah separators from the UTF-8 input. :(
      const char *paragraph = "\u2029";

      if (source && strstr(source, paragraph))
        {
           buf = eina_strbuf_new();
           eina_strbuf_append(buf, source);
           eina_strbuf_replace_all(buf, paragraph, "\n");
           source = eina_strbuf_string_get(buf);
        }
   }

   elm_spinner_value_set(spinner, 0);
   efl_gfx_filter_program_set(text, source, elm_spinner_special_value_get(spinner, 0));
   eina_strbuf_free(buf);
}

static void
_code_changed_hack(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Future *p = efl_loop_job(efl_loop_get(data), NULL);
   efl_future_then(p, _code_changed, NULL, NULL, data);
}

static void
_text_resize(void *data EINA_UNUSED, const Efl_Event *ev)
{
   int w = 0, h = 0;

   efl_gfx_size_get(ev->object, &w, &h);
   efl_gfx_size_hint_min_set(ev->object, w, h);
}

static Evas_Object *
_img_tooltip(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, Evas_Object *tt)
{
   return efl_add(EFL_UI_TEXT_CLASS, tt, efl_text_set(efl_added, efl_name_get(tt)));
}

static void
_img_click(void *data, const Efl_Event *ev)
{
   Eina_Strbuf *buf;
   Eo *win = data;
   Eo *img = ev->object;
   Eo *code, *text;
   const char *name;

   name = efl_name_get(img);
   if (!name) return;

   code = efl_key_wref_get(win, "code");
   text = efl_key_wref_get(win, "text");

   buf = eina_strbuf_new();
   eina_strbuf_append_printf(buf, "%s = buffer { src = '%s' }\n", name, name);

   eina_strbuf_append(buf, efl_text_get(code));
   efl_text_set(code, eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);

   efl_gfx_filter_source_set(text, name, ev->object);
}

static void
_font_size_change(void *data, const Efl_Event *ev)
{
   Eo *win = data;
   Eo *text;
   const char *font;

   text = efl_key_wref_get(win, "text");
   efl_text_properties_font_get(text, &font, NULL);
   efl_text_properties_font_set(text, font, elm_spinner_value_get(ev->object));
}

void
test_gfx_filters(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *box2, *o, *text, *spinner, *code, *split;

   win = efl_add(EFL_UI_WIN_STANDARD_CLASS, NULL,
                 efl_text_set(efl_added, "Gfx Filter Editor"),
                 efl_ui_win_autodel_set(efl_added, 1));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_orientation_set(efl_added, EFL_ORIENT_DOWN),
                 efl_gfx_size_hint_weight_set(efl_added, 1.0, 1.0));

   efl_content_set(win, box);

   {
      box2 = efl_add(EFL_UI_BOX_CLASS, win,
                     efl_orientation_set(efl_added, EFL_ORIENT_HORIZONTAL),
                     efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
                     efl_gfx_size_hint_align_set(efl_added, -1.0, 0.0),
                     efl_gfx_visible_set(efl_added, 1));
      efl_pack(box, box2);

      /* FIXME: Efl.Ui.Text doesn't work as expected. */
      o = efl_add(EFL_UI_TEXT_CLASS, win,
                  efl_gfx_size_hint_weight_set(efl_added, 0.0, 1.0),
                  efl_gfx_size_hint_align_set(efl_added, 1.0, 0.5),
                  efl_gfx_visible_set(efl_added, 1));
      efl_text_set(o, "Filter:");
      efl_pack(box2, o);

      o = spinner = efl_add(ELM_SPINNER_CLASS, win,
                            efl_gfx_size_hint_weight_set(efl_added, 1.0, 1.0),
                            efl_gfx_size_hint_align_set(efl_added, -1.0, 0.5),
                            efl_gfx_visible_set(efl_added, 1));
      _spinner_fill(o);
      efl_pack(box2, o);
      spinner = o;

      o = efl_add(ELM_SPINNER_CLASS, win,
                  efl_gfx_size_hint_weight_set(efl_added, 0.0, 1.0),
                  efl_gfx_size_hint_align_set(efl_added, -1.0, 0.5),
                  elm_spinner_min_max_set(efl_added, 6, 120),
                  elm_spinner_value_set(efl_added, default_font_size),
                  efl_event_callback_add(efl_added, ELM_SPINNER_EVENT_DELAY_CHANGED, _font_size_change, win),
                  efl_gfx_visible_set(efl_added, 1));

      efl_pack(box2, o);
   }

   {
      size_t k;

      box2 = efl_add(EFL_UI_BOX_CLASS, win,
                     efl_orientation_set(efl_added, EFL_ORIENT_HORIZONTAL),
                     efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
                     efl_gfx_size_hint_align_set(efl_added, 0.0, 0.5),
                     efl_pack_padding_set(efl_added, 5, 5, 1),
                     efl_gfx_size_hint_margin_set(efl_added, 5, 5, 5, 5),
                     efl_pack_align_set(efl_added, 0, 0.5));
      efl_pack(box, box2);

      for (k = 0; k < EINA_C_ARRAY_LENGTH(images); k++)
        {
           char buf[PATH_MAX];

           if (!images[k].path) break;
           snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), images[k].path);
           o = efl_add(EFL_UI_IMAGE_CLASS, win,
                       efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
                       efl_gfx_size_hint_align_set(efl_added, 0.5, 0.5),
                       efl_gfx_size_hint_max_set(efl_added, ELM_SCALE_SIZE(48), ELM_SCALE_SIZE(48)),
                       efl_gfx_size_hint_min_set(efl_added, ELM_SCALE_SIZE(48), ELM_SCALE_SIZE(48)),
                       efl_file_set(efl_added, buf, NULL),
                       efl_name_set(efl_added, images[k].src_name),
                       efl_gfx_visible_set(efl_added, 1));
           elm_object_tooltip_content_cb_set(o, _img_tooltip, NULL, NULL); // GRRRRR
           efl_event_callback_add(o, EFL_UI_EVENT_CLICKED, _img_click, win);
           efl_pack(box2, o);
        }

      efl_gfx_visible_set(box2, 1);
   }

   /* FIXME: No panes in EO APIs. */
   o = split = efl_add(ELM_PANES_CLASS, win,
                       efl_gfx_size_hint_weight_set(efl_added, 1.0, 1.0),
                       efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
                       efl_orientation_set(efl_added, EFL_ORIENT_HORIZONTAL),
                       efl_gfx_visible_set(efl_added, 1));
   efl_pack(box, split);

   {
      o = box2 = efl_add(EFL_UI_BOX_STACK_CLASS, win,
                         efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
                         efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
                         efl_pack_align_set(efl_added, 0.5, 0.5));
      efl_content_set(efl_part(split, "top"), box2);

      /* FIXME: No textblock support! TEXT is not part of EO public API. */
      o = text = evas_object_text_add(evas_object_evas_get(win));
      efl_event_callback_add(o, EFL_GFX_EVENT_RESIZE, _text_resize, NULL);
      efl_text_properties_font_set(o, "Sans:style=Bold", default_font_size);
      efl_canvas_object_scale_set(text, elm_config_scale_get());
      efl_text_set(o, "EFL");
      efl_gfx_visible_set(o, 1);
      efl_pack(box2, o);
   }

   {
      o = code = efl_add(EFL_UI_TEXT_EDITABLE_CLASS, win,
                         efl_gfx_size_hint_weight_set(efl_added, 1.0, 1.0),
                         efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
                         efl_ui_text_scrollable_set(efl_added, 1));
      efl_event_callback_add(o, EFL_UI_TEXT_EVENT_CHANGED_USER, _code_changed, win);

      // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
      efl_event_callback_add(o, EFL_EVENT_KEY_DOWN, _code_changed_hack, win);

      efl_content_set(efl_part(split, "bottom"), code);
   }

   efl_key_wref_set(win, "text", text);
   efl_key_wref_set(win, "code", code);
   efl_key_wref_set(win, "spinner", spinner);
   efl_event_callback_add(spinner, ELM_SPINNER_EVENT_CHANGED, _spinner_cb, win);
   elm_spinner_value_set(spinner, 1.0);
   _spinner_cb(win, NULL);

   efl_gfx_size_set(win, 500, 600);
   efl_gfx_visible_set(win, 1);
}
