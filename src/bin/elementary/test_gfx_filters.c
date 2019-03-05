#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_colorselector.eo.h"

static const int default_font_size = 48;

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
   { "fire.gif", "logo" },
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

static const Filter_Image images_anim[] = {
   { "fire.gif", "logo" },
   { NULL, NULL },
};

/* builtin filter examples */
static const Filter templates[] = {
   { "Custom", NULL, NULL },
   { "BLUR",
     "blur { 15, color = 'darkblue' }", NULL },
   { "Simple blend",
     "blend { color = 'darkblue' }", NULL },
   { "Black shadow",
     "if not myColor then myColor = color('yellow') end\n"
     "blur { 6, ox = 2, oy = 2, color = 'black' }\n"
     "blend { color = myColor }", NULL },
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
     "local len = state.scale * 10\n\n"
     "padding_set { len * 2 }\n"
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
   { "Text on fire",
     "-- set padding to 15 pixels max\n"
     "padding_set { 15 }\n\n"
     "-- our buffer: 1 source (fire gif) and 3 internal buffers\n"
     "logo = buffer { src = 'logo' }\n"
     "a = buffer { 'alpha' }\n"
     "b = buffer { 'rgba' }\n"
     "c = buffer { 'alpha' }\n\n"
     "-- create a massive blur: a\n"
     "grow { 5, dst = a }\n"
     "blur { 20, ox = -5, oy = -3, src = a, dst = a }\n\n"
     "-- draw fire using a as a mask\n"
     "mask { logo, a, fillmode = 'stretch_y_repeat_x' }\n\n"
     "-- create a displacement map: blur the fire a lot\n"
     "blend { src = logo, dst = b, fillmode = 'stretch' }\n"
     "blur { 30, src = logo, dst = b, src = b }\n"
     "displace { map = b, dst = c }\n\n"
     "-- draw the moving text\n"
     "blur { 4, src = c, color = '#ac180280' }\n"
     "blur { 2, src = c, color = '#dea80080' }\n"
     "mask { a, c }", images_anim }
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
   elm_spinner_wrap_set(obj, 1);
}

static void
_filter_apply(Eo *win, const char *code, const char *name)
{
   Eo *text, *tb;

   text = efl_key_wref_get(win, "text");
   tb = efl_key_wref_get(win, "textblock");

   efl_gfx_filter_program_set(text, code, name);
   efl_gfx_filter_program_set(tb, code, "main");
}

static void
_spinner_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win = data;
   Eo *text, *code, *spinner, *tb;
   int k;

   text = efl_key_wref_get(win, "text");
   tb = efl_key_wref_get(win, "textblock");
   code = efl_key_wref_get(win, "code");
   spinner = efl_key_wref_get(win, "spinner");
   k = (int) round(elm_spinner_value_get(spinner));
   if (k > 0)
     {
        const Filter *f = &templates[k];

        _filter_apply(win, f->code, f->name);
        efl_text_set(code, f->code);

        if (f->images)
          {
             for (size_t j = 0; f->images[j].path; j++)
               {
                  const char *name = f->images[j].src_name;
                  Eo *source_obj = efl_name_find(win, name);
                  efl_gfx_filter_source_set(text, name, source_obj);
                  efl_gfx_filter_source_set(tb, name, source_obj);
               }
          }
     }
}

static Eina_Value
_code_changed(void *data, const Eina_Value v,
              const Eina_Future *dead EINA_UNUSED)
{
   Eo *win = data;
   Eo *code, *spinner;
   Eina_Strbuf *buf = NULL;
   const char *source;

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
   _filter_apply(win, source, elm_spinner_special_value_get(spinner, 0));
   eina_strbuf_free(buf);

   return v;
}

static void
_code_changed_hack(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Future *f;

   f = eina_future_then(efl_loop_job(efl_loop_get(data)),
                        _code_changed, data, NULL);
   efl_future_then(data, f);
}

static void
_text_resize(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Size2D sz;

   sz = efl_gfx_entity_size_get(ev->object);
   efl_gfx_hint_size_min_set(ev->object, sz);
}

static void
_textblock_resize(void *data EINA_UNUSED, const Efl_Event *ev)
{
   int w = 0, h = 0, l = 0, r = 0, t = 0, b = 0;

   evas_object_textblock_size_native_get(ev->object, &w, &h);
   evas_object_textblock_style_insets_get(ev->object, &l, &r, &t, &b);
   efl_gfx_hint_size_min_set(ev->object, EINA_SIZE2D(w + l + r, h + t + b));
}

static void
_img_click(void *data, const Efl_Event *ev)
{
   Eina_Strbuf *buf;
   Eo *win = data;
   Eo *img = ev->object;
   Eo *code, *text, *tb;
   const char *name;

   name = efl_name_get(img);
   if (!name) return;

   code = efl_key_wref_get(win, "code");
   text = efl_key_wref_get(win, "text");
   tb = efl_key_wref_get(win, "textblock");

   buf = eina_strbuf_new();
   eina_strbuf_append_printf(buf, "%s = buffer { src = '%s' }\n", name, name);

   eina_strbuf_append(buf, efl_text_get(code));
   efl_text_set(code, eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);

   efl_gfx_filter_source_set(text, name, ev->object);
   efl_gfx_filter_source_set(tb, name, ev->object);
}

static void
_font_size_change(void *data, const Efl_Event *ev)
{
   Eo *win = data;
   Eo *text;
   const char *font;

   text = efl_key_wref_get(win, "text");
   efl_text_font_get(text, &font, NULL);
   efl_text_font_set(text, font, elm_spinner_value_get(ev->object));
}

static void
_flip_click(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *flip = efl_key_wref_get(win, "flip");

   /* FIXME: The flip APIs don't make sense for N items (front!?) */
   efl_ui_flip_go(flip, EFL_UI_FLIP_MODE_CROSS_FADE);
}

static void
_colsel_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Eo *win = data;
   Eo *colsel = obj;
   Eo *text, *tb;
   int r = 0, g = 0, b = 0, a = 255;
   char buf[64];

   text = efl_key_wref_get(win, "text");
   tb = efl_key_wref_get(win, "textblock");
   elm_colorselector_color_get(colsel, &r, &g, &b, &a);
   sprintf(buf, "color(%d, %d, %d, %d)", r, g, b, a);

   efl_gfx_filter_data_set(text, efl_name_get(colsel), buf, 1);
   efl_gfx_filter_data_set(tb, efl_name_get(colsel), buf, 1);
}

void
test_gfx_filters(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *box2, *o, *text = NULL, *spinner, *code, *split, *flip, *tb;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Gfx Filter Editor"),
                 efl_ui_win_autodel_set(efl_added, 1));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL));

   efl_content_set(win, box);

   {
      box2 = efl_add(EFL_UI_BOX_CLASS, win,
                     efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                     efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
                     efl_gfx_hint_align_set(efl_added, 0.5, 0.0),
                     efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE));
      efl_pack(box, box2);

      /* FIXME: Efl.Ui.Text doesn't work as expected. */
      o = efl_add(EFL_UI_TEXT_CLASS, win,
                  efl_gfx_hint_weight_set(efl_added, 0.0, 1.0),
                  efl_gfx_hint_align_set(efl_added, 1.0, 0.5));
      efl_text_set(o, "Filter:");
      efl_pack(box2, o);

      o = spinner = elm_spinner_add(win);
      efl_gfx_hint_weight_set(o, 1.0, 1.0);
      efl_gfx_hint_align_set(o, -1.0, 0.5);
      _spinner_fill(o);
      efl_pack(box2, o);
      evas_object_show(o);

      o = elm_spinner_add(win);
      efl_gfx_hint_weight_set(o, 0.0, 1.0);
      efl_gfx_hint_align_set(o, -1.0, 0.5);
      elm_spinner_min_max_set(o, 6, 120);
      elm_spinner_value_set(o, default_font_size);
      efl_event_callback_add(o, ELM_SPINNER_EVENT_DELAY_CHANGED, _font_size_change, win);
      efl_pack(box2, o);
      evas_object_show(o);

      o = efl_add(EFL_UI_BUTTON_CLASS, win,
                  efl_text_set(efl_added, "Flip"),
                  efl_gfx_hint_weight_set(efl_added, 0.0, 1.0),
                  efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                  efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _flip_click, win));
      efl_pack(box2, o);
   }

   {
      box2 = efl_add(EFL_UI_BOX_CLASS, win,
                     efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                     efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
                     efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_FALSE),
                     efl_pack_padding_set(efl_added, 5, 5, 1),
                     efl_gfx_hint_margin_set(efl_added, 5, 5, 5, 5),
                     efl_pack_align_set(efl_added, 0, 0.5));
      efl_pack(box, box2);

      for (size_t k = 0; k < EINA_C_ARRAY_LENGTH(images); k++)
        {
           Eina_Size2D size = { ELM_SCALE_SIZE(48), ELM_SCALE_SIZE(48) };
           char buf[PATH_MAX];

           if (!images[k].path) break;
           snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), images[k].path);
           o = efl_add(EFL_UI_IMAGE_CLASS, win,
                       efl_gfx_hint_weight_set(efl_added, 0.0, 0.0),
                       efl_gfx_hint_align_set(efl_added, 0.5, 0.5),
                       efl_gfx_hint_size_max_set(efl_added, size),
                       efl_gfx_hint_size_min_set(efl_added, size),
                       efl_file_set(efl_added, buf),
                       efl_name_set(efl_added, images[k].src_name),
                       elm_object_tooltip_text_set(efl_added, images[k].src_name));
           if (efl_player_playable_get(o))
             efl_player_play_set(o, 1);
           efl_event_callback_add(o, EFL_UI_EVENT_CLICKED, _img_click, win);
           efl_pack(box2, o);
        }

      Eina_Size2D maxsz = { -1, ELM_SCALE_SIZE(48 * 2) };
      const struct { int r, g, b, a; } colors[] = {
      { 255, 255, 255, 255 },
      { 0, 0, 0, 255 },
      { 64, 64, 64, 255 },
      { 128, 128, 128, 255 },
      { 196, 196, 196, 255 },
      { 255, 0, 0, 255 },
      { 0, 255, 0, 255 },
      { 0, 0, 255, 255 },
      { 255, 255, 0, 255 },
      { 255, 0, 255, 255 },
      { 0, 255, 255, 255 },
      { 0, 0, 0, 0 }
      };

      o = elm_colorselector_add(win);
      efl_gfx_hint_weight_set(o, 1.0, 0);
      efl_gfx_hint_align_set(o, -1.0, 0);
      elm_colorselector_mode_set(o, ELM_COLORSELECTOR_PALETTE);
      efl_gfx_hint_size_max_set(o, maxsz);
      efl_name_set(o, "myColor");
      elm_object_tooltip_text_set(o, "Pick a color to use as variable 'myColor'");
      evas_object_smart_callback_add(o, "changed", _colsel_cb, win);
      efl_pack(box2, o);
      evas_object_show(o);

      for (size_t k = 0; k < EINA_C_ARRAY_LENGTH(colors); k++)
        elm_colorselector_palette_color_add(o, colors[k].r, colors[k].g, colors[k].b, colors[k].a);
   }

   o = split = efl_add(EFL_UI_PANES_CLASS, win,
                       efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL));
   efl_pack(box, split);

   {
      flip = efl_add(EFL_UI_FLIP_CLASS, win);
      efl_content_set(efl_part(split, "first"), flip);

      box2 = efl_add(EFL_UI_BOX_STACK_CLASS, win,
                     efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                     efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
                     efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE),
                     efl_pack_align_set(efl_added, 0.5, 0.5));
      efl_pack(flip, box2);

      // Note: No TEXT object with EO APIs
      o = text = evas_object_text_add(evas_object_evas_get(win));
      efl_event_callback_add(o, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _text_resize, NULL);
      efl_text_font_set(o, "Sans:style=Bold", default_font_size);
      efl_gfx_entity_scale_set(text, elm_config_scale_get());
      efl_text_set(o, "EFL");
      efl_gfx_entity_visible_set(o, 1);
      efl_pack(box2, o);

      o = box2 = efl_add(EFL_UI_BOX_STACK_CLASS, win,
                         efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                         efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
                         efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE),
                         efl_pack_align_set(efl_added, 0.5, 0.5));
      efl_pack(flip, box2);

      const struct { const char *name, *code; } programs [] = {
        { "one", "blur { 5, color = 'darkblue' } blend {}" },
        { "two", "blur { 15, color = 'red' } blend {}" },
        { "main", "blend {}" }
      };

      const struct { const char *name, *value; int exec; } prg_data [] = {
        { "myColor", "color(255, 0, 255)", EINA_TRUE },
      };

      const char *tbtxt = "Classic <gfx_filter='one'>hello</> world!<br>"
                          "And <gfx_filter='two'>This filter over<br>"
                          "multiple lines</> :)<br/>"
                          "<gfx_filter='main'>Main filter</>";

      // Experimental textblock support
      o = tb = evas_object_textblock_add(evas_object_evas_get(win));
      efl_event_callback_add(o, EFL_CANVAS_TEXT_EVENT_STYLE_INSETS_CHANGED, _textblock_resize, NULL);
      efl_event_callback_add(o, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _textblock_resize, NULL);
      Evas_Textblock_Style *st = evas_textblock_style_new();
      evas_textblock_style_set(st, "DEFAULT='font=Sans font_size=20 color=#FFF wrap=word'");
      for (size_t k = 0; k < EINA_C_ARRAY_LENGTH(programs); k++)
        efl_gfx_filter_program_set(o, programs[k].code, programs[k].name);
      for (size_t k = 0; k < EINA_C_ARRAY_LENGTH(prg_data); k++)
        efl_gfx_filter_data_set(o, prg_data[k].name, prg_data[k].value, prg_data[k].exec);
      evas_object_textblock_style_set(o, st);
      evas_object_textblock_text_markup_set(o, tbtxt);
      efl_gfx_entity_scale_set(o, elm_config_scale_get());
      efl_pack(box2, o);
      evas_object_resize(o, 1, 1);
   }

   {
      const char *code_filter =
            "a = buffer { 'alpha' }"
            "grow { 2, dst = a } "
            "blur { 6, ox = 1, oy = 1, src = a, color = '#004' }"
            "blur { 3, ox = 1, oy = 1, color = 'black' }"
            "blend { color = 'lime' }";

      o = code = efl_add(EFL_UI_TEXT_EDITABLE_CLASS, win,
                         efl_ui_text_scrollable_set(efl_added, 1),
                         efl_text_multiline_set(efl_added, 1));
      efl_event_callback_add(o, EFL_UI_TEXT_EVENT_CHANGED_USER, _code_changed_hack, win);

      // Insert filter code inside style string: DEFAULT='blah blah <here>'
      efl_gfx_filter_program_set(o, code_filter, "code");
      efl_text_gfx_filter_set(o, "code");

      efl_content_set(efl_part(split, "second"), code);
   }

   efl_key_wref_set(win, "text", text);
   efl_key_wref_set(win, "textblock", tb);
   efl_key_wref_set(win, "code", code);
   efl_key_wref_set(win, "flip", flip);
   efl_key_wref_set(win, "spinner", spinner);
   evas_object_smart_callback_add(spinner, "changed", _spinner_cb, win);
   elm_spinner_value_set(spinner, 1.0);
   _spinner_cb(win, spinner, NULL);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(500,  600));
}
