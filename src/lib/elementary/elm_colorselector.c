#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_COMPONENT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_UI_FOCUS_COMPOSITION_PROTECTED
#define EFL_UI_FOCUS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_colorselector.h"
#include "elm_widget_spinner.h"

#define MY_CLASS ELM_COLORSELECTOR_CLASS

#define MY_CLASS_NAME "Elm_Colorselector"
#define MY_CLASS_NAME_LEGACY "elm_colorselector"

#define BASE_STEP       360.0
#define HUE_STEP        360.0
#define SAT_STEP        128.0
#define LIG_STEP        256.0
#define ALP_STEP        256.0
#define DEFAULT_HOR_PAD 10
#define DEFAULT_VER_PAD 10

static const char SIG_CHANGED[] = "changed";
static const char SIG_CHANGED_USER[] = "changed,user";
static const char SIG_COLOR_ITEM_SELECTED[] = "color,item,selected";
static const char SIG_COLOR_ITEM_LONGPRESSED[] = "color,item,longpressed";
static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_COLOR_ITEM_SELECTED, ""},
   {SIG_COLOR_ITEM_LONGPRESSED, ""},
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {SIG_CHANGED_USER, ""},
   {NULL, NULL}
};

static const Elm_Color_Name _color_name[] = {
   {{0, 0, 0, 255},       "black"},
   {{0, 0, 128, 255},     "navy"},
   {{0, 0, 139, 255},     "dark blue"},
   {{0, 0, 205, 255},     "medium blue"},
   {{0, 0, 255, 255},     "blue"},
   {{0, 100, 0, 255},     "dark green"},
   {{0, 128, 0, 255},     "green"},
   {{0, 128, 128, 255},   "teal"},
   {{0, 139, 139, 255},   "dark cyan"},
   {{0, 191, 255, 255},   "deep sky blue"},
   {{0, 206, 209, 255},   "dark turquoise"},
   {{0, 250, 154, 255},   "medium spring green"},
   {{0, 255, 0, 255},     "lime"},
   {{0, 255, 127, 255},   "spring green"},
   {{0, 255, 255, 255},   "aqua"},
   {{0, 255, 255, 255},   "cyan"},
   {{25, 25, 112, 255},   "midnight blue"},
   {{30, 144, 255, 255},  "dodger blue"},
   {{32, 178, 170, 255},  "light sea green"},
   {{34, 139, 34, 255},   "forest green"},
   {{46, 139, 87, 255},   "sea green"},
   {{47, 79, 79, 255},    "dark slate grey"},
   {{50, 205, 50, 255},   "lime green"},
   {{60, 179, 113, 255},  "medium sea green"},
   {{64, 224, 208, 255},  "turquoise"},
   {{65, 105, 225, 255},  "royal blue"},
   {{70, 130, 180, 255},  "steel blue"},
   {{72, 61, 139, 255},   "dark slate blue"},
   {{72, 209, 204, 255},  "medium turquoise"},
   {{75, 0, 130, 255},    "indigo"},
   {{85, 107, 47, 255},   "dark olive green"},
   {{95, 158, 160, 255},  "cadet blue"},
   {{100, 149, 237, 255}, "corn flower blue"},
   {{102, 205, 170, 255}, "medium aquamarine"},
   {{105, 105, 105, 255}, "dim grey"},
   {{106, 90, 205, 255},  "slate blue"},
   {{107, 142, 35, 255},  "olive drab"},
   {{112, 128, 144, 255}, "slate grey"},
   {{119, 136, 153, 255}, "light slate grey"},
   {{123, 104, 238, 255}, "medium slate blue"},
   {{124, 252, 0, 255},   "lawn green"},
   {{127, 255, 0, 255},   "chartreuse"},
   {{127, 255, 212, 255}, "aquamarine"},
   {{128, 0, 0, 255},     "maroon"},
   {{128, 0, 128, 255},   "purple"},
   {{128, 128, 0, 255},   "olive"},
   {{128, 128, 128, 255}, "grey"},
   {{135, 206, 235, 255}, "sky blue"},
   {{135, 206, 250, 255}, "light sky blue"},
   {{138, 43, 226, 255},  "blue violet"},
   {{139, 0, 0, 255},     "dark red"},
   {{139, 0, 139, 255},   "dark magenta"},
   {{139, 69, 19, 255},   "saddle brown"},
   {{143, 188, 143, 255}, "dark sea green"},
   {{144, 238, 144, 255}, "light green"},
   {{147, 112, 219, 255}, "medium purple"},
   {{148, 0, 211, 255},   "dark violet"},
   {{152, 251, 152, 255}, "pale green"},
   {{153, 50, 204, 255},  "dark orchid"},
   {{154, 205, 50, 255},  "yellow green"},
   {{160, 82, 45, 255},   "sienna"},
   {{165, 42, 42, 255},   "brown"},
   {{169, 169, 169, 255}, "dark grey"},
   {{173, 216, 230, 255}, "light blue"},
   {{173, 255, 47, 255},  "green yellow"},
   {{175, 238, 238, 255}, "pale turquoise"},
   {{176, 196, 222, 255}, "light steel blue"},
   {{176, 224, 230, 255}, "powder blue"},
   {{178, 34, 34, 255},   "fire brick"},
   {{184, 134, 11, 255},  "dark golden rod"},
   {{186, 85, 211, 255},  "medium orchid"},
   {{188, 143, 143, 255}, "rosy brown"},
   {{189, 183, 107, 255}, "dark khaki"},
   {{192, 192, 192, 255}, "silver"},
   {{199, 21, 133, 255},  "medium violet red"},
   {{205, 92, 92, 255},   "indian red"},
   {{205, 133, 63, 255},  "peru"},
   {{210, 105, 30, 255},  "chocolate"},
   {{210, 180, 140, 255}, "tan"},
   {{211, 211, 211, 255}, "light grey"},
   {{216, 191, 216, 255}, "thistle"},
   {{218, 112, 214, 255}, "orchid"},
   {{218, 165, 32, 255},  "golden rod"},
   {{219, 112, 147, 255}, "pale violet red"},
   {{220, 20, 60, 255},   "crimson"},
   {{220, 220, 220, 255}, "gainsboro"},
   {{221, 160, 221, 255}, "plum"},
   {{222, 184, 135, 255}, "burlywood"},
   {{224, 255, 255, 255}, "light cyan"},
   {{230, 230, 250, 255}, "lavender"},
   {{233, 150, 122, 255}, "dark salmon"},
   {{238, 130, 238, 255}, "violet"},
   {{238, 232, 170, 255}, "pale golden rod"},
   {{240, 128, 128, 255}, "light coral"},
   {{240, 230, 140, 255}, "khaki"},
   {{240, 248, 255, 255}, "alice blue"},
   {{240, 255, 255, 255}, "azure"},
   {{240, 255, 240, 255}, "honey dew"},
   {{244, 164, 96, 255},  "sandy brown"},
   {{245, 222, 179, 255}, "wheat"},
   {{245, 245, 220, 255}, "beige"},
   {{245, 245, 245, 255}, "white smoke"},
   {{245, 255, 250, 255}, "mint cream"},
   {{248, 248, 255, 255}, "ghost white"},
   {{250, 235, 215, 255}, "antique white"},
   {{250, 128, 114, 255}, "salmon"},
   {{250, 250, 210, 255}, "light golden rod yellow"},
   {{250, 240, 230, 255}, "linen"},
   {{253, 245, 230, 255}, "old lace"},
   {{255, 0, 0, 255},     "red"},
   {{255, 0, 255, 255},   "fuchsia"},
   {{255, 0, 255, 255},   "magenta"},
   {{255, 20, 147, 255},  "deep pink"},
   {{255, 69, 0, 255},    "orange red"},
   {{255, 99, 71, 255},   "tomato"},
   {{255, 105, 180, 255}, "hot pink"},
   {{255, 127, 80, 255},  "coral"},
   {{255, 140, 0, 255},   "dark orange"},
   {{255, 160, 122, 255}, "light salmon"},
   {{255, 165, 0, 255},   "orange"},
   {{255, 182, 193, 255}, "light pink "},
   {{255, 192, 203, 255}, "pink"},
   {{255, 215, 0, 255},   "gold"},
   {{255, 218, 185, 255}, "peach puff"},
   {{255, 222, 173, 255}, "navajo white"},
   {{255, 228, 181, 255}, "moccasin"},
   {{255, 228, 196, 255}, "bisque"},
   {{255, 228, 225, 255}, "misty rose"},
   {{255, 235, 205, 255}, "blanched almond"},
   {{255, 239, 213, 255}, "papaya whip"},
   {{255, 240, 245, 255}, "lavender blush"},
   {{255, 245, 238, 255}, "sea shell"},
   {{255, 248, 220, 255}, "corn silk"},
   {{255, 250, 205, 255}, "lemon chiffon"},
   {{255, 250, 240, 255}, "floral white"},
   {{255, 255, 0, 255},   "yellow"},
   {{255, 255, 224, 255}, "light yellow"},
   {{255, 255, 240, 255}, "ivory"},
   {{255, 250, 250, 255}, "snow"},
   {{255, 255, 255, 255}, "white"}
};

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"activate", _key_action_activate},
   {NULL, NULL}
};

static void
_flush_color_children(Eo *obj, Elm_Colorselector_Data *pd)
{
   Eina_List *items = NULL;

   if (pd->mode == ELM_COLORSELECTOR_ALL) {
      items = eina_list_append(items, pd->picker);
      items = eina_list_merge(items, eina_list_clone(pd->items));
      items = eina_list_append(items, pd->col_bars_area);
   } else if (pd->mode == ELM_COLORSELECTOR_BOTH) {
      items = eina_list_merge(items, eina_list_clone(pd->items));
      items = eina_list_append(items, pd->col_bars_area);
   } else if (pd->mode == ELM_COLORSELECTOR_COMPONENTS) {
      items = eina_list_append(items, pd->col_bars_area);
   } else if (pd->mode == ELM_COLORSELECTOR_PALETTE) {
      items = eina_list_merge(items, eina_list_clone(pd->items));
   } else if (pd->mode == ELM_COLORSELECTOR_PICKER) {
      items = eina_list_append(items, pd->picker);
   }
   efl_ui_focus_composition_elements_set(obj, items);
}

enum Palette_Box_Direction
{
   PALETTE_BOX_UP,
   PALETTE_BOX_DOWN
};

static const char *
_get_color_name(unsigned int R, unsigned int G, unsigned int B, unsigned int A)
{
   unsigned int low, mid, high;
   unsigned int r, g, b;

   if (A != 255) return NULL;
   low = 0;
   high = EINA_C_ARRAY_LENGTH(_color_name) - 1;
   while (low <= high)
     {
        mid = (low + high) / 2;

        r = _color_name[mid].color.r;
        g = _color_name[mid].color.g;
        b = _color_name[mid].color.b;

        if (r == R && g == G && B == b)
          return _color_name[mid].name;

        if ((r < R) || ((r == R) && (g < G)) || ((r == R) && (g == G) && b < B))
          low = mid + 1;
        else
          high = mid - 1;
      }
    return NULL;
}

static void
_items_del(Elm_Colorselector_Data *sd)
{
   Elm_Object_Item *item;

   if (!sd->items) return;

   EINA_LIST_FREE(sd->items, item)
     {
        efl_del (item);
     }

   sd->items = NULL;
   sd->selected = NULL;
   sd->focus_items = NULL;
}

static void
_color_with_saturation(Elm_Colorselector_Data *sd, int *sr, int *sg, int *sb)
{
   if (sd->er > 127)
     *sr = 127 + (int)((double)(sd->er - 127) * sd->s);
   else
     *sr = 127 - (int)((double)(127 - sd->er) * sd->s);

   if (sd->eg > 127)
     *sg = 127 + (int)((double)(sd->eg - 127) * sd->s);
   else
     *sg = 127 - (int)((double)(127 - sd->eg) * sd->s);

   if (sd->eb > 127)
     *sb = 127 + (int)((double)(sd->eb - 127) * sd->s);
   else
     *sb = 127 - (int)((double)(127 - sd->eb) * sd->s);
}

static void
_color_with_lightness(Elm_Colorselector_Data *sd, int *lr, int *lg, int *lb)
{
   if (sd->l > 0.5)
     {
        *lr = sd->er + (int)((double)(255 - sd->er) * (sd->l - 0.5) * 2.0);
        *lg = sd->eg + (int)((double)(255 - sd->eg) * (sd->l - 0.5) * 2.0);
        *lb = sd->eb + (int)((double)(255 - sd->eb) * (sd->l - 0.5) * 2.0);
     }
   else if (sd->l < 0.5)
     {
        *lr = (double)sd->er * sd->l * 2.0;
        *lg = (double)sd->eg * sd->l * 2.0;
        *lb = (double)sd->eb * sd->l * 2.0;
     }
   else
     {
        *lr = sd->er;
        *lg = sd->eg;
        *lb = sd->eb;
     }
}

static void
_color_picker_init(Elm_Colorselector_Data *sd)
{
   unsigned int *pixels;
   unsigned int *copy;
   int color;
   int x, y, w, h;

   if (!evas_object_data_get(sd->spinners[0], "_changed"))
     {
        elm_spinner_value_set(sd->spinners[0], sd->r);
     }
   if (!evas_object_data_get(sd->spinners[1], "_changed"))
     {
        elm_spinner_value_set(sd->spinners[1], sd->g);
     }
   if (!evas_object_data_get(sd->spinners[2], "_changed"))
     {
        elm_spinner_value_set(sd->spinners[2], sd->b);
     }
   if (!evas_object_data_get(sd->spinners[3], "_changed"))
     {
        elm_spinner_value_set(sd->spinners[3], sd->a);
     }

   color = (sd->a << 24) |
     (((sd->r * sd->a) / 255) << 16) |
     (((sd->g * sd->a) / 255) << 8) |
     (((sd->b * sd->a) / 255));

   if (sd->a == 255)
     evas_object_image_alpha_set(sd->picker_display, EINA_FALSE);
   else
     evas_object_image_alpha_set(sd->picker_display, EINA_TRUE);

   pixels = evas_object_image_data_get(sd->picker_display, EINA_TRUE);
   copy = pixels;
   for (y = 0; y < 17; y++)
     {
        for (x = 0; x < 17; x++)
          {
             *(pixels++) = color;
          }
     }
   evas_object_image_data_set(sd->picker_display, copy);
   evas_object_geometry_get(sd->picker_display, NULL, NULL, &w, &h);
   evas_object_image_data_update_add(sd->picker_display, 0, 0, w, h);
}

static void
_rgb_to_hsl(Elm_Colorselector_Data *sd)
{
   double r2, g2, b2;
   double v, m, vm;
   double r, g, b;

   r = sd->r;
   g = sd->g;
   b = sd->b;

   r /= 255.0;
   g /= 255.0;
   b /= 255.0;

   v = (r > g) ? r : g;
   v = (v > b) ? v : b;

   m = (r < g) ? r : g;
   m = (m < b) ? m : b;

   sd->h = 0.0;
   sd->s = 0.0;
   sd->l = 0.0;

   sd->l = (m + v) / 2.0;

   if (sd->l <= 0.0) return;

   vm = v - m;
   sd->s = vm;

   if (sd->s > 0.0) sd->s /= (sd->l <= 0.5) ? (v + m) : (2.0 - v - m);
   else return;

   r2 = (v - r) / vm;
   g2 = (v - g) / vm;
   b2 = (v - b) / vm;

   if (r == v) sd->h = (g == m ? 5.0 + b2 : 1.0 - g2);
   else if (g == v)
     sd->h = (b == m ? 1.0 + r2 : 3.0 - b2);
   else sd->h = (r == m ? 3.0 + g2 : 5.0 - r2);

   sd->h *= 60.0;
}

static Eina_Bool
_hsl_to_rgb(Elm_Colorselector_Data *sd)
{
   double sv, vsf, f, p, q, t, v;
   double r = 0, g = 0, b = 0;
   double _h, _s, _l;
   int i = 0;

   _h = sd->h;
   _s = sd->s;
   _l = sd->l;

   if (_s == 0.0) r = g = b = _l;
   else
     {
        if (_h == 360.0) _h = 0.0;
        _h /= 60.0;

        v = (_l <= 0.5) ? (_l * (1.0 + _s)) : (_l + _s - (_l * _s));
        p = _l + _l - v;

        if (v) sv = (v - p) / v;
        else sv = 0;

        i = (int)_h;
        f = _h - i;

        vsf = v * sv * f;

        t = p + vsf;
        q = v - vsf;

        switch (i)
          {
           case 0:
             r = v;
             g = t;
             b = p;
             break;

           case 1:
             r = q;
             g = v;
             b = p;
             break;

           case 2:
             r = p;
             g = v;
             b = t;
             break;

           case 3:
             r = p;
             g = q;
             b = v;
             break;

           case 4:
             r = t;
             g = p;
             b = v;
             break;

           case 5:
             r = v;
             g = p;
             b = q;
             break;
          }
     }
   i = (int)(r * 255.0);
   f = (r * 255.0) - i;
   r = (f <= 0.5) ? i : (i + 1);

   i = (int)(g * 255.0);
   f = (g * 255.0) - i;
   g = (f <= 0.5) ? i : (i + 1);

   i = (int)(b * 255.0);
   f = (b * 255.0) - i;
   b = (f <= 0.5) ? i : (i + 1);

   if (sd->r == r && sd->g == g && sd->b == b) return EINA_FALSE;

   sd->r = r;
   sd->g = g;
   sd->b = b;

   return EINA_TRUE;
}

static void
_update_ergb(Elm_Colorselector_Data *sd, double x)
{
   double one_six = 1.0 / 6.0;

   if (x < one_six)
     {
        sd->er = 255;
        sd->eg = (255.0 * x * 6.0);
        sd->eb = 0;
     }
   else if (x < 2 * one_six)
     {
        sd->er = 255 - (int)(255.0 * (x - one_six) * 6.0);
        sd->eg = 255;
        sd->eb = 0;
     }
   else if (x < 3 * one_six)
     {
        sd->er = 0;
        sd->eg = 255;
        sd->eb = (int)(255.0 * (x - (2.0 * one_six)) * 6.0);
     }
   else if (x < 4 * one_six)
     {
        sd->er = 0;
        sd->eg = 255 - (int)(255.0 * (x - (3.0 * one_six)) * 6.0);
        sd->eb = 255;
     }
   else if (x < 5 * one_six)
     {
        sd->er = 255.0 * (x - (4.0 * one_six)) * 6.0;
        sd->eg = 0;
        sd->eb = 255;
     }
   else
     {
        sd->er = 255;
        sd->eg = 0;
        sd->eb = 255 - (int)(255.0 * (x - (5.0 * one_six)) * 6.0);
     }
}

static void
_update_colorbars(Elm_Colorselector_Data *sd)
{
   int r, g, b;
   evas_object_color_set
     (sd->cb_data[0]->arrow, sd->er, sd->eg, sd->eb, 255);
   evas_object_color_set
     (sd->cb_data[1]->bg_rect, sd->er, sd->eg, sd->eb, 255);
   evas_object_color_set
     (sd->cb_data[2]->bg_rect, sd->er, sd->eg, sd->eb, 255);
   evas_object_color_set
     (sd->cb_data[3]->bar, sd->er, sd->eg, sd->eb, 255);

   _color_with_saturation(sd, &r, &g, &b);
   evas_object_color_set(sd->cb_data[1]->arrow, r, g, b, 255);

   _color_with_lightness(sd, &r, &g, &b);
   evas_object_color_set(sd->cb_data[2]->arrow, r, g, b, 255);

   evas_object_color_set(sd->cb_data[3]->arrow,
                         (sd->er * sd->a) / 255,
                         (sd->eg * sd->a) / 255,
                         (sd->eb * sd->a) / 255,
                         sd->a);
}

static void
_update_hsla_from_colorbar(Evas_Object *obj, Color_Type type, double x)
{
   int ta;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   switch (type)
     {
      case HUE:
        sd->h = 360.0 * x;
        _update_ergb(sd, x);
        break;

      case SATURATION:
        sd->s = 1.0 - x;
        break;

      case LIGHTNESS:
        sd->l = x;
        break;

      case ALPHA:
        ta = 255.0 * x;
        if (ta == sd->a) return;
        sd->a = ta;
        break;

      default:
        return;
     }

   if (type != ALPHA && !_hsl_to_rgb(sd)) return;

   _update_colorbars(sd);
   if ((sd->mode == ELM_COLORSELECTOR_ALL) || (sd->mode == ELM_COLORSELECTOR_PICKER))
     _color_picker_init(sd);
   efl_event_callback_legacy_call(obj, ELM_COLORSELECTOR_EVENT_CHANGED, NULL);
   efl_event_callback_legacy_call(obj, ELM_COLORSELECTOR_EVENT_CHANGED_USER, NULL);
}

static void
_colors_set(Evas_Object *obj,
            int r,
            int g,
            int b,
            int a,
            Eina_Bool mode_change)
{
   double x, y;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if ((sd->r == r) && (sd->g == g) && (sd->b == b) && (sd->a == a) && !mode_change)
     return;

   sd->r = r;
   sd->g = g;
   sd->b = b;
   sd->a = a;

   if ((sd->mode == ELM_COLORSELECTOR_ALL) || (sd->mode == ELM_COLORSELECTOR_COMPONENTS)
      || (sd->mode == ELM_COLORSELECTOR_BOTH))
     {
        _rgb_to_hsl(sd);

        edje_object_part_drag_value_get
          (sd->cb_data[0]->colorbar, "elm.arrow", &x, &y);
        x = sd->h / 360.0;
        edje_object_part_drag_value_set
          (sd->cb_data[0]->colorbar, "elm.arrow", x, y);
        _update_ergb(sd, x);

        edje_object_part_drag_value_get
          (sd->cb_data[1]->colorbar, "elm.arrow", &x, &y);
        x = 1.0 - sd->s;
        edje_object_part_drag_value_set
          (sd->cb_data[1]->colorbar, "elm.arrow", x, y);

        edje_object_part_drag_value_get
          (sd->cb_data[2]->colorbar, "elm.arrow", &x, &y);
        x = sd->l;
        edje_object_part_drag_value_set(sd->cb_data[2]->colorbar, "elm.arrow", x, y);

        edje_object_part_drag_value_get
          (sd->cb_data[3]->colorbar, "elm.arrow", &x, &y);
        x = sd->a / 255.0;
        edje_object_part_drag_value_set
          (sd->cb_data[3]->colorbar, "elm.arrow", x, y);

        _update_colorbars(sd);
     }
   if ((sd->mode == ELM_COLORSELECTOR_ALL) || (sd->mode == ELM_COLORSELECTOR_PICKER))
     _color_picker_init(sd);
   if (!mode_change)
     efl_event_callback_legacy_call(obj, ELM_COLORSELECTOR_EVENT_CHANGED, NULL);
}

static void
_unselect_selected_item(Elm_Colorselector_Data *sd)
{
   Eo *eo_temp_item;

   if (sd->selected)
     {
        eo_temp_item = eina_list_data_get(sd->selected);
        elm_obj_color_item_selected_set(eo_temp_item, EINA_FALSE);
     }
}

static void
_spinner_changed_cb(void *data, const Efl_Event *event)
{
   Elm_Colorselector_Data *sd = data;
   Evas_Object *parent;
   int i, v;

   _unselect_selected_item(sd);
   for (i = 0; i < 4 && sd->spinners[i] != event->object; i++);

   parent = evas_object_data_get(event->object, "parent");
   v = elm_spinner_value_get(event->object);
   evas_object_data_set(event->object, "_changed", event->object);

   switch (i)
     {
      case 0:
         _colors_set(parent, v, sd->g, sd->b, sd->a, EINA_FALSE);
         break;
      case 1:
         _colors_set(parent, sd->r, v, sd->b, sd->a, EINA_FALSE);
         break;
      case 2:
         _colors_set(parent, sd->r, sd->g, v, sd->a, EINA_FALSE);
         break;
      case 3:
         _colors_set(parent, sd->r, sd->g, sd->b, v, EINA_FALSE);
         break;
     }
   evas_object_data_del(event->object, "_changed");
   efl_event_callback_legacy_call(parent, ELM_COLORSELECTOR_EVENT_CHANGED_USER, NULL);
}

#ifdef HAVE_ELEMENTARY_X
static Eina_Bool _mouse_grab_pixels(void *data, int type EINA_UNUSED, void *event EINA_UNUSED);
static Eina_Bool _key_up_cb(void *data, int type EINA_UNUSED, void *event EINA_UNUSED);
static Eina_Bool _mouse_up_cb(void *data, int type EINA_UNUSED, void *event EINA_UNUSED);

static Ecore_X_Window
_x11_elm_widget_xwin_get(const Evas_Object *obj)
{
   Evas_Object *top;
   Ecore_X_Window xwin = 0;

   top = elm_widget_top_get(obj);
   if (!top) top = elm_widget_top_get(elm_widget_parent_widget_get(obj));
   if (top) xwin = elm_win_xwindow_get(top);
   if (!xwin)
     {
        Ecore_Evas *ee;
        Evas *evas = evas_object_evas_get(obj);
        if (!evas) return 0;
        ee = ecore_evas_ecore_evas_get(evas);
        if (!ee) return 0;
        xwin = _elm_ee_xwin_get(ee);
     }
   return xwin;
}

static void
_start_grab_pick_cb(void *data, const Efl_Event *event)
{
   Evas_Object *o = data;

   ELM_COLORSELECTOR_DATA_GET(o, sd);

   elm_object_disabled_set(event->object, EINA_TRUE);

   sd->grab.mouse_motion = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _mouse_grab_pixels, o);
   sd->grab.key_up = ecore_event_handler_add(ECORE_EVENT_KEY_UP, _key_up_cb, o);
   sd->grab.mouse_up = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, _mouse_up_cb, o);

   ecore_x_keyboard_grab(sd->grab.xroot);
   ecore_x_pointer_grab(sd->grab.xroot);
}

static Eina_Bool
_key_up_cb(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *o = data;

   /* key => cancel */
   ELM_COLORSELECTOR_DATA_GET(o, sd);

   elm_object_disabled_set(sd->button, EINA_FALSE);
   ecore_x_keyboard_ungrab();
   ecore_x_pointer_ungrab();
   ELM_SAFE_FREE(sd->grab.mouse_motion, ecore_event_handler_del);
   ELM_SAFE_FREE(sd->grab.key_up, ecore_event_handler_del);
   ELM_SAFE_FREE(sd->grab.mouse_up, ecore_event_handler_del);

   return EINA_TRUE;
}

static Eina_Bool
_mouse_up_cb(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   const unsigned int *pixels;
   Evas_Object *o = data;
   int r, g, b;

   /* mouse up => check it */
   ELM_COLORSELECTOR_DATA_GET(o, sd);

   elm_object_disabled_set(sd->button, EINA_FALSE);
   ecore_x_keyboard_ungrab();
   ecore_x_pointer_ungrab();
   ELM_SAFE_FREE(sd->grab.mouse_motion, ecore_event_handler_del);
   ELM_SAFE_FREE(sd->grab.key_up, ecore_event_handler_del);
   ELM_SAFE_FREE(sd->grab.mouse_up, ecore_event_handler_del);

   pixels = evas_object_image_data_get(sd->picker_display, EINA_FALSE);
   r = (pixels[17 * 8 + 8] >> 16) & 0xFF;
   g = (pixels[17 * 8 + 8] >> 8) & 0xFF;
   b = pixels[17 * 8 + 8] & 0xFF;

   _unselect_selected_item(sd);
   _colors_set(o, r, g, b, 0xFF, EINA_FALSE);
   efl_event_callback_legacy_call(o, ELM_COLORSELECTOR_EVENT_CHANGED_USER, NULL);

   return EINA_TRUE;
}

static Eina_Bool
_mouse_grab_pixels(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *obj = data;
   Ecore_X_Visual visual;
   Ecore_X_Display *display;
   Ecore_X_Screen *scr;
   Ecore_X_Image *img;
   Ecore_X_Window xwin;
   int *src;
   int bpl = 0, rows = 0, bpp = 0;
   int x, y, w, h;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (sd->grab.in) return EINA_TRUE;

   xwin = _x11_elm_widget_xwin_get(obj);
   sd->grab.xroot = ecore_x_window_root_get(xwin);
   ecore_x_pointer_xy_get(sd->grab.xroot, &x, &y);

   if (x < 8) x += 8;
   if (y < 8) y += 8;

   if (sd->grab.x == x && sd->grab.y == y) return EINA_TRUE;
   sd->grab.x = x;
   sd->grab.y = y;

   evas_object_image_alpha_set(sd->picker_display, EINA_FALSE);

   display = ecore_x_display_get();
   scr = ecore_x_default_screen_get();
   visual = ecore_x_default_visual_get(display, scr);
   img = ecore_x_image_new(17, 17, visual, ecore_x_window_depth_get(sd->grab.xroot));

   if (!img)
     {
        ERR("Failed to get an image from ecore_x_image_new()");
        return EINA_TRUE;
     }

   ecore_x_image_get(img, sd->grab.xroot, x - 8, y - 8, 0, 0, 17, 17);
   src = ecore_x_image_data_get(img, &bpl, &rows, &bpp);
   if (!ecore_x_image_is_argb32_get(img))
     {
        Ecore_X_Colormap colormap;
        unsigned int *pixels;

        colormap = ecore_x_default_colormap_get(display, scr);
        pixels = evas_object_image_data_get(sd->picker_display, EINA_TRUE);
        ecore_x_image_to_argb_convert(src, bpp, bpl, colormap, visual,
                                      0, 0, 17, 17,
                                      pixels, (17 * sizeof(int)), 0, 0);
     }
   else
     {
        evas_object_image_data_copy_set(sd->picker_display, src);
     }

   ecore_x_image_free(img);

   evas_object_geometry_get(sd->picker_display, NULL, NULL, &w, &h);
   evas_object_image_data_update_add(sd->picker_display, 0, 0, w, h);

   return EINA_TRUE;
}
#endif

static void
_mouse_in_canvas(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *obj = data;
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   sd->grab.in = EINA_TRUE;
}

static void
_mouse_out_canvas(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *obj = data;
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   sd->grab.in = EINA_FALSE;
}

static void
_create_colorpicker(Evas_Object *obj)
{
   Evas_Object *ed;
   Evas_Object *im;
   Evas_Object *spinner;
   Evas_Object *bx;
   Eina_Stringshare *style;
   int i;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);
   if (sd->picker) return;
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin;

   xwin = _x11_elm_widget_xwin_get(obj);
   if (xwin)
     {
        sd->grab.xroot = ecore_x_window_root_get(xwin);
        ecore_x_input_raw_select(sd->grab.xroot);
     }
#endif
   /* setup the color picker */
   sd->picker = elm_layout_add(obj);
   if (!elm_layout_theme_set(sd->picker, "colorselector", "picker/base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   evas_object_size_hint_weight_set(sd->picker, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sd->picker, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_widget_sub_object_add(obj, sd->picker);

   bx = elm_box_add(sd->picker);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set(sd->picker, "elm.swallow.picker", bx);
   evas_object_show(bx);

   ed = edje_object_add(evas_object_evas_get(sd->picker));
   elm_widget_theme_object_set(obj, ed, "colorselector", "picker", elm_widget_style_get(obj));
   evas_object_size_hint_weight_set(ed, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ed, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ed);
   evas_object_show(ed);

   style = eina_stringshare_printf("colorselector/%s", elm_widget_style_get(obj));
#ifdef HAVE_ELEMENTARY_X
   if (xwin)
     {
        sd->button = elm_button_add(sd->picker);
        elm_object_style_set(sd->button, style);
        elm_object_text_set(sd->button, E_("Pick a color"));
        efl_event_callback_add
              (sd->button, EFL_UI_EVENT_CLICKED, _start_grab_pick_cb, obj);
        elm_box_pack_end(bx, sd->button);
        evas_object_show(sd->button);
     }
#endif

   im = evas_object_image_add(evas_object_evas_get(sd->picker));
   evas_object_size_hint_aspect_set(im, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
   evas_object_image_smooth_scale_set(im, EINA_FALSE);
   evas_object_image_colorspace_set(im, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_size_set(im, 17, 17);
   evas_object_image_alpha_set(im, EINA_TRUE);
   evas_object_image_filled_set(im, EINA_TRUE);
   edje_object_part_swallow(ed, "elm.picker", im);
   elm_widget_sub_object_add(obj, im);

   sd->picker_display = im;

   for (i = 0; i < 4; i++)
     {
        spinner = elm_spinner_add(sd->picker);
        elm_object_style_set(spinner, style);
        evas_object_data_set(spinner, "parent", obj);
        efl_event_callback_add
              (spinner, ELM_SPINNER_EVENT_CHANGED, _spinner_changed_cb, sd);
        elm_spinner_editable_set(spinner, EINA_TRUE);
        elm_spinner_interval_set(spinner, 0.1);
        elm_spinner_min_max_set(spinner, 0, 255);
        evas_object_size_hint_weight_set(spinner, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(spinner, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_show(spinner);

        sd->spinners[i] = spinner;
     }

   elm_layout_content_set(sd->picker, "elm.swallow.red", sd->spinners[0]);
   elm_layout_text_set(sd->picker, "elm.label.red", E_("R:"));
   elm_layout_content_set(sd->picker, "elm.swallow.green", sd->spinners[1]);
   elm_layout_text_set(sd->picker, "elm.label.green", E_("G:"));
   elm_layout_content_set(sd->picker, "elm.swallow.blue", sd->spinners[2]);
   elm_layout_text_set(sd->picker, "elm.label.blue", E_("B:"));
   elm_layout_content_set(sd->picker, "elm.swallow.alpha", sd->spinners[3]);
   elm_layout_text_set(sd->picker, "elm.label.alpha", E_("A:"));

   evas_event_callback_add(evas_object_evas_get(obj), EVAS_CALLBACK_CANVAS_FOCUS_IN, _mouse_in_canvas, obj);
   evas_event_callback_add(evas_object_evas_get(obj), EVAS_CALLBACK_CANVAS_FOCUS_OUT, _mouse_out_canvas, obj);

   eina_stringshare_del(style);

   _color_picker_init(sd);
}

static void
_arrow_cb(void *data,
          Evas_Object *obj,
          const char *emission EINA_UNUSED,
          const char *source EINA_UNUSED)
{
   Color_Bar_Data *cb_data = data;
   double x, y;
   ELM_COLORSELECTOR_DATA_GET(cb_data->parent, sd);

   _unselect_selected_item(sd);
   edje_object_part_drag_value_get(obj, "elm.arrow", &x, &y);
   _update_hsla_from_colorbar(cb_data->parent, cb_data->color_type, x);
}

static void
_colorbar_arrow_set(Color_Bar_Data *cb_data, int mouse_x)
{
   Evas_Coord x, y, w, h;
   double arrow_x = 0, arrow_y;
   ELM_COLORSELECTOR_DATA_GET(cb_data->parent, sd);

   evas_object_geometry_get(cb_data->bar, &x, &y, &w, &h);
   edje_object_part_drag_value_get
     (cb_data->colorbar, "elm.arrow", &arrow_x, &arrow_y);

   if (w > 0) arrow_x = (double)(mouse_x - x) / (double)w;
   if (arrow_x > 1) arrow_x = 1;
   else if (arrow_x < 0) arrow_x = 0;
   else _unselect_selected_item(sd);

   edje_object_part_drag_value_set
     (cb_data->colorbar, "elm.arrow", arrow_x, arrow_y);

   _update_hsla_from_colorbar(cb_data->parent, cb_data->color_type, arrow_x);
   sd->sel_color_type = cb_data->color_type;
   sd->focused = ELM_COLORSELECTOR_COMPONENTS;
}

static void
_colorbar_down_cb(void *data,
                  Evas *e EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Color_Bar_Data *cb_data = data;

   _colorbar_arrow_set(cb_data, ev->canvas.x);
}

static void
_colorbar_move_cb(void *data,
                  Evas *e EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Color_Bar_Data *cb_data = data;

   if (!ev->buttons)
     return;

   _colorbar_arrow_set(cb_data, ev->cur.canvas.x);
}

static void
_button_clicked_cb(void *data, const Efl_Event *event)
{
   Color_Bar_Data *cb_data = data;
   double x, y, step;
   ELM_COLORSELECTOR_DATA_GET(cb_data->parent, sd);

   if (event->object == cb_data->rbt) step = 1.0;
   else step = -1.0;

   edje_object_part_drag_value_get(cb_data->colorbar, "elm.arrow", &x, &y);

   switch (cb_data->color_type)
     {
      case HUE:
        x += step / HUE_STEP;
        break;

      case SATURATION:
        x += step / SAT_STEP;
        break;

      case LIGHTNESS:
        x += step / LIG_STEP;
        break;

      case ALPHA:
        x += step / ALP_STEP;
        break;

      default:
        break;
     }

   if (x > 1.0) x = 1.0;
   else if (x < 0.0) x = 0.0;
   else _unselect_selected_item(sd);

   edje_object_part_drag_value_set(cb_data->colorbar, "elm.arrow", x, y);
   _update_hsla_from_colorbar(cb_data->parent, cb_data->color_type, x);
   sd->sel_color_type = cb_data->color_type;
   sd->focused = ELM_COLORSELECTOR_COMPONENTS;
}

static void
_button_repeat_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Color_Bar_Data *cb_data = data;
   double x, y, step;

   if (event->object == cb_data->rbt) step = 1.0 / BASE_STEP;
   else step = -1.0 / BASE_STEP;

   edje_object_part_drag_value_get(cb_data->colorbar, "elm.arrow", &x, &y);
   x += step;

   if (x > 1.0) x = 1.0;
   else if (x < 0.0) x = 0.0;

   edje_object_part_drag_value_set(cb_data->colorbar, "elm.arrow", x, y);
   _update_hsla_from_colorbar(cb_data->parent, cb_data->color_type, x);
}

static void
_access_colorbar_register(Evas_Object *obj,
                          Color_Bar_Data *cd,
                          const char* part)
{
   Evas_Object *ao;
   Elm_Access_Info *ai;
   const char* colorbar_type = NULL;

   ao = _elm_access_edje_object_part_object_register(obj, cd->colorbar, part);
   ai = _elm_access_info_get(ao);

   switch (cd->color_type)
     {
      case HUE:
        colorbar_type = "hue color bar";
        break;

      case SATURATION:
        colorbar_type = "saturation color bar";
        break;

      case LIGHTNESS:
        colorbar_type = "lightness color bar";
        break;

      case ALPHA:
        colorbar_type = "alpha color bar";
        break;

      default:
        break;
     }

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, colorbar_type);

   // this will be used in focus_next();
   cd->access_obj = ao;
}

static void
_color_bars_add(Evas_Object *obj)
{
   char colorbar_name[128];
   char colorbar_s[128];
   char buf[1024];
   int i = 0;
   Evas *e;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   e = evas_object_evas_get(obj);

   for (i = 0; i < 4; i++)
     {
        if (!sd->cb_data[i]) sd->cb_data[i] = ELM_NEW(Color_Bar_Data);
        sd->cb_data[i]->parent = obj;

        switch (i)
          {
           case 0:
             sd->cb_data[i]->color_type = HUE;
             break;

           case 1:
             sd->cb_data[i]->color_type = SATURATION;
             break;

           case 2:
             sd->cb_data[i]->color_type = LIGHTNESS;
             break;

           case 3:
             sd->cb_data[i]->color_type = ALPHA;
             break;
          }

        /* load colorbar area */
        if (!sd->cb_data[i]->colorbar) sd->cb_data[i]->colorbar = edje_object_add(e);
        elm_widget_theme_object_set
          (obj, sd->cb_data[i]->colorbar, "colorselector", "base",
          elm_widget_style_get(obj));
        snprintf(colorbar_name, sizeof(colorbar_name), "colorbar_%d", i);
        snprintf(colorbar_s, sizeof(colorbar_s), "elm.colorbar_%d", i);
        edje_object_signal_callback_add
          (sd->cb_data[i]->colorbar, "drag", "*", _arrow_cb, sd->cb_data[i]);
        elm_object_part_content_set(sd->col_bars_area,
                                    colorbar_s, sd->cb_data[i]->colorbar);

        /* load colorbar image */
        if (!sd->cb_data[i]->bar) sd->cb_data[i]->bar = edje_object_add(e);
        snprintf(buf, sizeof(buf), "%s/%s", colorbar_name,
                 elm_widget_style_get(obj));
        elm_widget_theme_object_set
          (obj, sd->cb_data[i]->bar, "colorselector", "image", buf);
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.bar", sd->cb_data[i]->bar);
        elm_widget_sub_object_add(sd->col_bars_area, sd->cb_data[i]->bar);

        /* provide expanded touch area */
        if (!sd->cb_data[i]->touch_area) sd->cb_data[i]->touch_area = evas_object_rectangle_add(e);
        evas_object_color_set(sd->cb_data[i]->touch_area, 0, 0, 0, 0);
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.arrow_bg",
          sd->cb_data[i]->touch_area);
        evas_object_event_callback_add
          (sd->cb_data[i]->touch_area, EVAS_CALLBACK_MOUSE_DOWN, _colorbar_down_cb, sd->cb_data[i]);
        evas_object_event_callback_add
          (sd->cb_data[i]->touch_area, EVAS_CALLBACK_MOUSE_MOVE, _colorbar_move_cb, sd->cb_data[i]);
        elm_widget_sub_object_add(sd->col_bars_area, sd->cb_data[i]->touch_area);

        // ACCESS
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
           _access_colorbar_register(obj, sd->cb_data[i], "elm.arrow_bg_access");

        /* load background rectangle of the colorbar. used for
           changing color of the opacity bar */
        if ((i == 1) || (i == 2))
          {
             if (!sd->cb_data[i]->bg_rect) sd->cb_data[i]->bg_rect = evas_object_rectangle_add(e);
             evas_object_color_set
               (sd->cb_data[i]->bg_rect, sd->er, sd->eg, sd->eb, 255);
             edje_object_part_swallow
               (sd->cb_data[i]->colorbar, "elm.bar_bg",
               sd->cb_data[i]->bg_rect);

             elm_widget_sub_object_add(sd->col_bars_area, sd->cb_data[i]->bg_rect);
          }
        if (i == 3)
          {
             if (!sd->cb_data[i]->bg_rect) sd->cb_data[i]->bg_rect = edje_object_add(e);
             snprintf(buf, sizeof(buf), "%s/%s", colorbar_name,
                      elm_widget_style_get(obj));
             elm_widget_theme_object_set
               (obj, sd->cb_data[i]->bg_rect, "colorselector", "bg_image",
               buf);
             edje_object_part_swallow
               (sd->cb_data[i]->colorbar, "elm.bar_bg",
               sd->cb_data[i]->bg_rect);
             elm_widget_sub_object_add(obj, sd->cb_data[i]->bg_rect);
             evas_object_color_set
               (sd->cb_data[i]->bar, sd->er, sd->eg, sd->eb, 255);
          }

        /* load arrow image, pointing the colorbar */
        if (!sd->cb_data[i]->arrow) sd->cb_data[i]->arrow = edje_object_add(e);
        elm_widget_theme_object_set
          (obj, sd->cb_data[i]->arrow, "colorselector", "arrow",
          elm_widget_style_get(obj));
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.arrow_icon",
          sd->cb_data[i]->arrow);
        elm_widget_sub_object_add(sd->col_bars_area, sd->cb_data[i]->arrow);

        if (i == 2)
          evas_object_color_set(sd->cb_data[i]->arrow, 0, 0, 0, 255);
        else
          evas_object_color_set
            (sd->cb_data[i]->arrow, sd->er, sd->eg, sd->eb, 255);

        /* load left button */
        if (!sd->cb_data[i]->lbt) sd->cb_data[i]->lbt = elm_button_add(sd->col_bars_area);
        snprintf(buf, sizeof(buf), "colorselector/left/%s",
                 elm_widget_style_get(obj));
        elm_object_style_set(sd->cb_data[i]->lbt, buf);
        elm_widget_sub_object_add(sd->col_bars_area, sd->cb_data[i]->lbt);
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.l_button", sd->cb_data[i]->lbt);
        efl_event_callback_add
          (sd->cb_data[i]->lbt, EFL_UI_EVENT_CLICKED, _button_clicked_cb, sd->cb_data[i]);
        elm_button_autorepeat_set(sd->cb_data[i]->lbt, EINA_TRUE);
        elm_button_autorepeat_initial_timeout_set
          (sd->cb_data[i]->lbt, _elm_config->longpress_timeout);
        elm_button_autorepeat_gap_timeout_set
          (sd->cb_data[i]->lbt, (1.0 / _elm_config->fps));
        efl_event_callback_add
          (sd->cb_data[i]->lbt, EFL_UI_EVENT_REPEATED, _button_repeat_cb, sd->cb_data[i]);

        /* load right button */
        if (!sd->cb_data[i]->rbt) sd->cb_data[i]->rbt = elm_button_add(sd->col_bars_area);
        snprintf(buf, sizeof(buf), "colorselector/right/%s",
                 elm_widget_style_get(obj));
        elm_object_style_set(sd->cb_data[i]->rbt, buf);
        elm_widget_sub_object_add(sd->col_bars_area, sd->cb_data[i]->rbt);
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.r_button", sd->cb_data[i]->rbt);
        efl_event_callback_add
          (sd->cb_data[i]->rbt, EFL_UI_EVENT_CLICKED, _button_clicked_cb, sd->cb_data[i]);
        elm_button_autorepeat_set(sd->cb_data[i]->rbt, EINA_TRUE);
        elm_button_autorepeat_initial_timeout_set
          (sd->cb_data[i]->rbt, _elm_config->longpress_timeout);
        elm_button_autorepeat_gap_timeout_set
          (sd->cb_data[i]->rbt, (1.0 / _elm_config->fps));
        efl_event_callback_add
          (sd->cb_data[i]->rbt, EFL_UI_EVENT_REPEATED, _button_repeat_cb, sd->cb_data[i]);
     }
}

EOLIAN static Efl_Ui_Theme_Apply_Result
_elm_colorselector_efl_ui_widget_theme_apply(Eo *obj, Elm_Colorselector_Data *sd)
{
   int i;
   Eina_List *elist;
   Elm_Object_Item *eo_item;
   const char *hpadstr, *vpadstr;
   Eina_Stringshare *style;
   unsigned int h_pad = DEFAULT_HOR_PAD;
   unsigned int v_pad = DEFAULT_VER_PAD;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_RESULT_FAIL);
   Efl_Ui_Theme_Apply_Result int_ret = EFL_UI_THEME_APPLY_RESULT_FAIL;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   if ((sd->mode == ELM_COLORSELECTOR_PALETTE) ||
       (sd->mode == ELM_COLORSELECTOR_ALL) ||
       (sd->mode == ELM_COLORSELECTOR_BOTH))
     {
        double scale;

        hpadstr = edje_object_data_get
            (wd->resize_obj, "horizontal_pad");
        if (hpadstr) h_pad = atoi(hpadstr);
        vpadstr = edje_object_data_get
            (wd->resize_obj, "vertical_pad");
        if (vpadstr) v_pad = atoi(vpadstr);

        scale = efl_gfx_entity_scale_get(obj) * elm_config_scale_get() / edje_object_base_scale_get(wd->resize_obj);
        efl_pack_padding_set(sd->palette_box, h_pad * scale, v_pad * scale, 0);

        EINA_LIST_FOREACH(sd->items, elist, eo_item)
          {
             ELM_COLOR_ITEM_DATA_GET(eo_item, item);
             if (!elm_layout_theme_set
                 (VIEW(item), "colorselector", "item", elm_widget_style_get(obj)))
               CRI("Failed to set layout!");

             elm_widget_theme_object_set
               (obj, item->color_obj, "colorselector", "item/color",
               elm_widget_style_get(obj));
          }
     }

   if ((sd->mode == ELM_COLORSELECTOR_COMPONENTS) ||
       (sd->mode == ELM_COLORSELECTOR_ALL) ||
       (sd->mode == ELM_COLORSELECTOR_BOTH))
     {
        if (!sd->col_bars_area) return EINA_FALSE;

        if (!elm_layout_theme_set(sd->col_bars_area, "colorselector", "bg",
                                  elm_widget_style_get(obj)))
          CRI("Failed to set layout!");

        for (i = 0; i < 4; i++)
          {
             if (sd->cb_data[i])
               {
                  ELM_SAFE_FREE(sd->cb_data[i]->colorbar, evas_object_del);
                  ELM_SAFE_FREE(sd->cb_data[i]->bar, evas_object_del);
                  ELM_SAFE_FREE(sd->cb_data[i]->lbt, evas_object_del);
                  ELM_SAFE_FREE(sd->cb_data[i]->rbt, evas_object_del);
                  if (i != 0)
                    ELM_SAFE_FREE(sd->cb_data[i]->bg_rect, evas_object_del);
                  ELM_SAFE_FREE(sd->cb_data[i]->arrow, evas_object_del);
                  ELM_SAFE_FREE(sd->cb_data[i]->touch_area, evas_object_del);
               }
          }
        _color_bars_add(obj);
        elm_colorselector_color_set(obj, sd->r, sd->g, sd->b, sd->a);
     }

   if ((sd->mode == ELM_COLORSELECTOR_PICKER) ||
       (sd->mode == ELM_COLORSELECTOR_ALL))
     {
        if (!elm_layout_theme_set(sd->picker, "colorselector", "picker/base",
                                  elm_widget_style_get(obj)))
          CRI("Failed to set layout!");

        style = eina_stringshare_printf("colorselector/%s", elm_widget_style_get(obj));
     #ifdef HAVE_ELEMENTARY_X
        elm_object_style_set(sd->button, style);
     #endif
        for (i = 0; i < 4; i++)
          elm_object_style_set(sd->spinners[i], style);
        eina_stringshare_del(style);
     }

   elm_layout_sizing_eval(obj);
   return int_ret;
}

static void
_sub_obj_size_hints_set(Evas_Object *sobj,
                        int timesw,
                        int timesh)
{
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(timesw, &minw, timesh, &minh);
   if (!efl_isa(sobj, EFL_CANVAS_LAYOUT_CLASS)) return;
   edje_object_size_min_restricted_calc(sobj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(sobj, minw, minh);
   evas_object_size_hint_max_set(sobj, -1, -1);
}

static void
_item_sizing_eval(Elm_Color_Item_Data *item)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Object *edje;

   if (!item) return;
   edje = elm_layout_edje_get(VIEW(item));
   if (!edje) return;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(edje, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(VIEW(item), minw, minh);
}

/* fix size hints of color palette items, so that the box gets it */
static void
_palette_sizing_eval(Elm_Colorselector_Data *sd)
{
   Eina_List *elist;
   Elm_Object_Item *eo_item;

   EINA_LIST_FOREACH(sd->items, elist, eo_item)
     {
        ELM_COLOR_ITEM_DATA_GET(eo_item, item);
        _item_sizing_eval(item);
     }
}

static void
_component_sizing_eval(Elm_Colorselector_Data *sd)
{
   Evas_Coord minw = -1, minh = -1;
   int i;

   for (i = 0; i < 4; i++)
     {
        if (sd->cb_data[i])
          {
             if (sd->cb_data[i]->bg_rect)
               _sub_obj_size_hints_set(sd->cb_data[i]->bg_rect, 1, 1);

             _sub_obj_size_hints_set(sd->cb_data[i]->bar, 1, 1);
             _sub_obj_size_hints_set(sd->cb_data[i]->rbt, 1, 1);
             _sub_obj_size_hints_set(sd->cb_data[i]->lbt, 1, 1);
             _sub_obj_size_hints_set(sd->cb_data[i]->colorbar, 4, 1);
          }
     }

   edje_object_size_min_restricted_calc
     (elm_layout_edje_get(sd->col_bars_area), &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(sd->col_bars_area, minw, minh);
}

static void
_full_sizing_eval(Elm_Colorselector_Data *sd)
{
   _palette_sizing_eval(sd);
   _component_sizing_eval(sd);
}

static void
_picker_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   evas_object_size_hint_combined_min_get(sd->picker, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

EOLIAN static void
_elm_colorselector_elm_layout_sizing_eval(Eo *obj, Elm_Colorselector_Data *sd)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   if (!efl_finalized_get(obj)) return; //not constructed yet

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);

   switch (sd->mode)
     {
      case ELM_COLORSELECTOR_PALETTE:
        _palette_sizing_eval(sd);
        break;

      case ELM_COLORSELECTOR_COMPONENTS:
        _component_sizing_eval(sd);
        break;

      case ELM_COLORSELECTOR_BOTH:
        _full_sizing_eval(sd);
        break;

      case ELM_COLORSELECTOR_PICKER:
        _picker_sizing_eval(obj);
        break;

      case ELM_COLORSELECTOR_ALL:
        _full_sizing_eval(sd);
        break;

      default:
        return;
     }

   efl_pack_layout_request(sd->palette_box);
   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_on_resize(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
           Evas_Object *obj, void *event_info EINA_UNUSED)
{
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if ((sd->mode == ELM_COLORSELECTOR_PALETTE) ||
       (sd->mode == ELM_COLORSELECTOR_BOTH))
     {
        Evas_Coord w = 0, h = 0, minw = -1;

        evas_object_size_hint_combined_min_get(obj, &minw, NULL);
        evas_object_geometry_get(obj, NULL, NULL, &w, &h);
        if ((w != sd->_w) && (w < minw))
          elm_layout_sizing_eval(obj);
        sd->_w = w;
        sd->_h = h;
     }
}

static Eina_Bool
_on_color_long_press(void *data)
{
   Elm_Color_Item_Data *item = (Elm_Color_Item_Data *)data;

   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   sd->longpress_timer = NULL;

   efl_event_callback_legacy_call
     (WIDGET(item), ELM_COLORSELECTOR_EVENT_COLOR_ITEM_LONGPRESSED, EO_OBJ(item));

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_color_pressed(void *data,
                  Evas *e EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info)
{
   Elm_Color_Item_Data *item = (Elm_Color_Item_Data *)data;
   Evas_Event_Mouse_Down *ev = event_info;

   if (!item) return;

   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   if (ev->button != 1) return;

   ecore_timer_del(sd->longpress_timer);
   sd->longpress_timer = ecore_timer_add
       (_elm_config->longpress_timeout, _on_color_long_press, data);

   item->still_in = EINA_TRUE;
}

static void
_on_color_moved(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   Elm_Color_Item_Data *item = (Elm_Color_Item_Data *)data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord x = 0, y = 0, w = 0, h = 0;

   if (!item) return;

   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   evas_object_geometry_get(item->color_obj, &x, &y, &w, &h);

   if (ELM_RECTS_POINT_OUT(x, y, w, h, ev->cur.canvas.x, ev->cur.canvas.y))
     {
        ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
        item->still_in = EINA_FALSE;
     }
}

static void
_on_color_released(void *data,
                   Evas *e EINA_UNUSED,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info)
{
   Elm_Color_Item_Data *item = (Elm_Color_Item_Data *)data;
   Eina_List *l;
   Elm_Object_Item *eo_temp_item;
   Evas_Event_Mouse_Down *ev = event_info;

   if (!item) return;

   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   if (ev->button != 1) return;
   ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);

   if ((ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) || !item->still_in)
     return;

   elm_object_signal_emit(VIEW(item), "elm,state,selected", "elm");
   elm_colorselector_color_set(WIDGET(item), item->color->r, item->color->g,
                               item->color->b, item->color->a);
   efl_event_callback_legacy_call
     (WIDGET(item), ELM_COLORSELECTOR_EVENT_COLOR_ITEM_SELECTED, EO_OBJ(item));

   eo_temp_item = eina_list_data_get(sd->selected);
   if (eo_temp_item && (eo_temp_item != EO_OBJ(item)))
     {
        ELM_COLOR_ITEM_DATA_GET(eo_temp_item, temp_item);
        elm_object_signal_emit(VIEW(temp_item), "elm,state,unselected", "elm");
     }

   EINA_LIST_FOREACH(sd->items, l, eo_temp_item)
     {
        ELM_COLOR_ITEM_DATA_GET(eo_temp_item, temp_item);
        if (item == temp_item) sd->selected = l;
     }
   sd->focused = ELM_COLORSELECTOR_PALETTE;
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   char *ret;
   Eina_Strbuf *buf;
   const char *color_name = NULL;
   int r = 0, g = 0, b = 0, a = 0;

   Elm_Color_Item_Data *it = data;
   ELM_COLORSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);

   elm_colorselector_palette_item_color_get(EO_OBJ(it), &r, &g, &b, &a);

   buf = eina_strbuf_new();
   color_name = _get_color_name(r, g, b, a);
   if (color_name)
     eina_strbuf_append_printf(buf, "%s", color_name);
   else
     eina_strbuf_append_printf(buf, "red %d, green %d, blue %d, alpha %d", r, g, b, a);
   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_access_activate_cb(void *data EINA_UNUSED,
                    Evas_Object *part_obj EINA_UNUSED,
                    Elm_Object_Item *eo_item)
{
   ELM_COLOR_ITEM_DATA_GET(eo_item, item);
   elm_object_item_signal_emit(eo_item, "elm,state,selected", "elm");
   _on_color_released(item, NULL, NULL, NULL);
}

EOLIAN static Evas_Object*
_elm_color_item_elm_widget_item_access_register(Eo *eo_it, Elm_Color_Item_Data *it)
{
   Elm_Access_Info *ai;

   Evas_Object *res;
   res = elm_wdg_item_access_register(efl_super(eo_it, ELM_COLOR_ITEM_CLASS));

   ai = _elm_access_info_get(it->base->access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("color selector palette item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
   _elm_access_activate_callback_set(ai, _access_activate_cb,  EO_OBJ(it));

   return res;
}

static void
_item_resize(void *data EINA_UNUSED,
             Evas *e EINA_UNUSED,
             Evas_Object *obj,
             void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_color_item_elm_widget_item_signal_emit(Eo *eo_it EINA_UNUSED,
                                            Elm_Color_Item_Data *it,
                                            const char *emission,
                                            const char *source)
{
   elm_object_signal_emit(VIEW(it), emission, source);
}

static Eina_Bool
_item_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   Eina_List *l;
   ELM_COLOR_ITEM_DATA_GET(obj, item);
   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   elm_object_signal_emit(VIEW(item), "elm,state,selected", "elm");
   if (_elm_config->atspi_mode)
     efl_access_state_changed_signal_emit(obj,
                                                              EFL_ACCESS_STATE_CHECKED,
                                                              EINA_TRUE);
   elm_colorselector_color_set(WIDGET(item), item->color->r, item->color->g,
                               item->color->b, item->color->a);
   evas_object_smart_callback_call(WIDGET(item), SIG_COLOR_ITEM_SELECTED,
                                   EO_OBJ(item));

   Eo *eo_temp_item = eina_list_data_get(sd->selected);
   if (eo_temp_item && (eo_temp_item != EO_OBJ(item)))
     {
        ELM_COLOR_ITEM_DATA_GET(eo_temp_item, temp_item);
        elm_object_signal_emit(VIEW(temp_item), "elm,state,unselected", "elm");
     }

   EINA_LIST_FOREACH(sd->items, l, eo_temp_item)
     {
        ELM_COLOR_ITEM_DATA_GET(eo_temp_item, temp_item);
        if (item == temp_item) sd->selected = l;
     }
   sd->focused = ELM_COLORSELECTOR_PALETTE;

   return EINA_TRUE;
}

EOLIAN static Eo *
_elm_color_item_efl_object_constructor(Eo *eo_item, Elm_Color_Item_Data *item)
{
   eo_item = efl_constructor(efl_super(eo_item, ELM_COLOR_ITEM_CLASS));
   item->base = efl_data_scope_get(eo_item, ELM_WIDGET_ITEM_CLASS);

   Evas_Object *obj;
   obj = efl_parent_get(eo_item);
   WIDGET(item) = obj;
   VIEW_SET(item, elm_layout_add(obj));
   if (!elm_layout_theme_set
       (VIEW(item), "colorselector", "item", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   evas_object_size_hint_weight_set
     (VIEW(item), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add
     (VIEW(item), EVAS_CALLBACK_RESIZE, _item_resize, NULL);

   item->color_obj = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set
     (obj, item->color_obj, "colorselector", "item/color",
     elm_widget_style_get(obj));
   evas_object_size_hint_weight_set
     (item->color_obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
     (item->color_obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add
     (item->color_obj, EVAS_CALLBACK_MOUSE_DOWN, _on_color_pressed, item);
   evas_object_event_callback_add
     (item->color_obj, EVAS_CALLBACK_MOUSE_MOVE, _on_color_moved, item);
   evas_object_event_callback_add
     (item->color_obj, EVAS_CALLBACK_MOUSE_UP, _on_color_released, item);
   if (!elm_layout_content_set(VIEW(item), "elm.swallow.color_obj", item->color_obj))
     elm_layout_content_set(VIEW(item), "color_obj", item->color_obj);

   _item_sizing_eval(item);
   evas_object_show(VIEW(item));

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     elm_wdg_item_access_register(eo_item);

   efl_access_object_role_set(eo_item, EFL_ACCESS_ROLE_RADIO_BUTTON);

   return eo_item;
}

EOLIAN static void
_elm_color_item_efl_object_destructor(Eo *obj, Elm_Color_Item_Data *item)
{
   free(item->color);
   efl_destructor(efl_super(obj, ELM_COLOR_ITEM_CLASS));
}

static void
_colors_remove(Evas_Object *obj)
{
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   _items_del(sd);
   _elm_config_colors_free(sd->palette_name);
}

static void
_colors_save(Evas_Object *obj)
{
   Eina_List *elist;
   Elm_Object_Item *eo_item;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   _elm_config_colors_free(sd->palette_name);
   EINA_LIST_FOREACH(sd->items, elist, eo_item)
     {
        ELM_COLOR_ITEM_DATA_GET(eo_item, item);
        _elm_config_color_set(sd->palette_name, item->color->r, item->color->g,
              item->color->b, item->color->a);
     }
}

static void
_palette_colors_load(Evas_Object *obj)
{
   Eina_List *elist;
   Eo *eo_item;
   Eina_List *color_list;
   Elm_Color_RGBA *color;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   color_list = _elm_config_color_list_get(sd->palette_name);
   if (!color_list) return;

   EINA_LIST_FOREACH(color_list, elist, color)
     {
        eo_item = efl_add(ELM_COLOR_ITEM_CLASS, obj);
        if (!eo_item) return;

        ELM_COLOR_ITEM_DATA_GET(eo_item, item);

        item->color = ELM_NEW(Elm_Color_RGBA);
        if (!item->color) return;
        item->color->r = color->r;
        item->color->g = color->g;
        item->color->b = color->b;
        item->color->a = color->a;

        efl_pack(sd->palette_box, VIEW(item));
        evas_object_color_set(item->color_obj,
                              (item->color->r * item->color->a) / 255,
                              (item->color->g * item->color->a) / 255,
                              (item->color->b * item->color->a) / 255,
                              item->color->a);

        sd->items = eina_list_append(sd->items, eo_item);
     }

   sd->config_load = EINA_TRUE;
}

static inline void
_palette_box_prepare(Eo *o)
{
   efl_ui_direction_set(o, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   efl_gfx_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_gfx_entity_visible_set(o, EINA_FALSE);
}

static void
_create_colorpalette(Evas_Object *obj)
{
   const char *hpadstr, *vpadstr;
   unsigned int h_pad = DEFAULT_HOR_PAD;
   unsigned int v_pad = DEFAULT_VER_PAD;
   double scale;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->palette_box) return;
   if (elm_widget_is_legacy(obj))
     {
        sd->palette_box = elm_legacy_add(EFL_UI_BOX_FLOW_CLASS, obj);
     }
   else
     {
        sd->palette_box = efl_add(EFL_UI_BOX_FLOW_CLASS, obj);
     }
   _palette_box_prepare(sd->palette_box);

   hpadstr = edje_object_data_get(wd->resize_obj, "horizontal_pad");
   if (hpadstr) h_pad = atoi(hpadstr);
   vpadstr = edje_object_data_get(wd->resize_obj, "vertical_pad");
   if (vpadstr) v_pad = atoi(vpadstr);

   scale = efl_gfx_entity_scale_get(obj) * elm_config_scale_get() / edje_object_base_scale_get(wd->resize_obj);
   efl_pack_padding_set(sd->palette_box, h_pad * scale, v_pad * scale, 0);
   efl_pack_align_set(sd->palette_box, 0.0, 0.0);
   if (!elm_layout_content_set(obj, "elm.palette", sd->palette_box))
     elm_layout_content_set(obj, "palette", sd->palette_box);
   sd->palette_name = eina_stringshare_add("default");
   _palette_colors_load(obj);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _on_resize, NULL);
}

static void
_create_colorcomponents(Evas_Object *obj)
{
   ELM_COLORSELECTOR_DATA_GET(obj, sd);
   if (sd->col_bars_area) return;
   sd->col_bars_area = elm_layout_add(obj);
   if (!elm_layout_theme_set
       (sd->col_bars_area, "colorselector", "bg", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   if (!elm_layout_content_set(obj, "elm.selector", sd->col_bars_area))
     elm_layout_content_set(obj, "selector", sd->col_bars_area);
   _hsl_to_rgb(sd);
   _color_bars_add(obj);
}

EOLIAN static void
_elm_colorselector_efl_canvas_group_group_add(Eo *obj, Elm_Colorselector_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set
       (obj, "colorselector", "palette", elm_object_style_get(obj)))
     CRI("Failed to set layout!");

   _create_colorpalette(obj);

   _create_colorcomponents(obj);

   elm_layout_signal_emit(obj, "elm,state,both", "elm");

   priv->mode = ELM_COLORSELECTOR_BOTH;
   priv->focused = ELM_COLORSELECTOR_PALETTE;
   priv->sel_color_type = HUE;
   priv->selected = NULL;
   priv->focus_items = NULL;
   priv->er = 255;
   priv->eg = 0;
   priv->eb = 0;
   priv->h = 0.0;
   priv->s = 1.0;
   priv->l = 0.0;
   priv->a = 255;
   priv->grab.x = -1;
   priv->grab.y = -1;
#ifdef HAVE_ELEMENTARY_X
   priv->grab.xroot = -1;
#endif
   priv->grab.in = EINA_TRUE;
   elm_layout_sizing_eval(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   _flush_color_children(obj, priv);
}

EOLIAN static void
_elm_colorselector_efl_canvas_group_group_del(Eo *obj, Elm_Colorselector_Data *sd)
{
   int i = 0;
   void *tmp[4];

   evas_event_callback_del_full(evas_object_evas_get(obj), EVAS_CALLBACK_CANVAS_FOCUS_IN, _mouse_in_canvas, obj);
   evas_event_callback_del_full(evas_object_evas_get(obj), EVAS_CALLBACK_CANVAS_FOCUS_OUT, _mouse_out_canvas, obj);

   ecore_timer_del(sd->longpress_timer);
   eina_stringshare_del(sd->palette_name);

#ifdef HAVE_ELEMENTARY_X
   ecore_event_handler_del(sd->grab.mouse_motion);
   ecore_event_handler_del(sd->grab.mouse_up);
   ecore_event_handler_del(sd->grab.key_up);
#endif

   // We created the items with efl_add, they will be dead after this.
   sd->items = eina_list_free(sd->items);
   sd->selected = NULL;
   sd->focus_items = NULL;

   /* This cb_data are used during the destruction process of base.del */
   for (i = 0; i < 4; i++)
     tmp[i] = sd->cb_data[i];
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
   for (i = 0; i < 4; i++)
     free(tmp[i]);
}

static Eina_List*
_palette_box_vertical_item_get(Eina_List* ref_item, enum Palette_Box_Direction dir)
{
   Evas_Coord basex, basey, x, y, dx, min_dx;
   Elm_Object_Item *eo_item;
   Eina_List* l;
   Eina_List* res = NULL;
   Eina_List* (*dir_func)(const Eina_List*);

   if (!ref_item) return NULL;

   switch (dir)
     {
        case PALETTE_BOX_UP:
          dir_func = eina_list_prev;
          break;
        case PALETTE_BOX_DOWN:
          dir_func = eina_list_next;
          break;
        default:
          return NULL;
     }

   eo_item = eina_list_data_get(ref_item);
   ELM_COLOR_ITEM_DATA_GET(eo_item, item);
   evas_object_geometry_get(VIEW(item), &basex, &basey, NULL, NULL);

   for (l = ref_item; l; l = dir_func(l))
     {
        eo_item = eina_list_data_get(l);
        item = efl_data_scope_get(eo_item, ELM_COLOR_ITEM_CLASS);
        evas_object_geometry_get(VIEW(item), &x, &y, NULL, NULL);
        if (basey != y) break;
     }

   basey = y;
   min_dx = -1;

   for (; l; l = dir_func(l))
     {
        eo_item = eina_list_data_get(l);
        item = efl_data_scope_get(eo_item, ELM_COLOR_ITEM_CLASS);
        evas_object_geometry_get(VIEW(item), &x, &y, NULL, NULL);
        if (basey != y) break;

        dx = abs(x - basex);
        if (dx < min_dx || min_dx < 0)
          {
             min_dx = dx;
             res = l;
          }
        else
          {
             break;
          }
      }

   return res;
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   ELM_COLORSELECTOR_DATA_GET(obj, sd);
   Elm_Object_Item *eo_item = NULL;
   Eina_List *cl = NULL;
   char colorbar_s[128];
   const char *dir = params;

   if (!sd->selected) sd->selected = sd->items;
   if (!sd->focus_items) sd->focus_items = sd->items;
   _elm_widget_focus_auto_show(obj);
   if (!strcmp(dir, "left"))
     {
        if (sd->focused == ELM_COLORSELECTOR_PALETTE && sd->selected)
          {
             cl = eina_list_prev(sd->focus_items);
          }
        else if (sd->focused == ELM_COLORSELECTOR_COMPONENTS)
          {
             Efl_Event event = {};
             event.object = sd->cb_data[sd->sel_color_type]->lbt;
             _button_clicked_cb(sd->cb_data[sd->sel_color_type], &event);
          }
        else return EINA_FALSE;
     }
   else if (!strcmp(dir, "right"))
     {
        if (sd->focused == ELM_COLORSELECTOR_PALETTE && sd->selected)
          {
             cl = eina_list_next(sd->focus_items);
          }
        else if (sd->focused == ELM_COLORSELECTOR_COMPONENTS)
          {
             Efl_Event event = {};
             event.object = sd->cb_data[sd->sel_color_type]->rbt;
             _button_clicked_cb(sd->cb_data[sd->sel_color_type], &event);
          }
        else return EINA_FALSE;
     }
   else if (!strcmp(dir, "up"))
     {
        if (sd->focused == ELM_COLORSELECTOR_COMPONENTS)
          {
             sd->sel_color_type = sd->sel_color_type - 1;
             if (sd->sel_color_type < HUE)
               {
                  if (sd->mode == ELM_COLORSELECTOR_BOTH)
                    {
                       sd->focused = ELM_COLORSELECTOR_PALETTE;
                       /*when focus is shifted to palette start from
                        * first item*/
                       sd->selected = sd->items;
                       cl = sd->selected;
                    }
                  else
                    {
                       sd->sel_color_type = HUE;
                       return EINA_FALSE;
                    }
               }
          }
        else if (sd->focused == ELM_COLORSELECTOR_PALETTE)
          {
             cl = _palette_box_vertical_item_get(sd->focus_items, PALETTE_BOX_UP);
             if (!cl) cl = sd->focus_items;
          }
     }
   else if (!strcmp(dir, "down"))
     {
        if (sd->focused == ELM_COLORSELECTOR_PALETTE)
          {
             cl = _palette_box_vertical_item_get(sd->focus_items, PALETTE_BOX_DOWN);
             if (sd->mode == ELM_COLORSELECTOR_BOTH && !cl)
               {
                  sd->focused = ELM_COLORSELECTOR_COMPONENTS;
                  /*when focus is shifted to component start from
                   * first color type*/
                  sd->sel_color_type = HUE;
               }
          }
        else if (sd->focused == ELM_COLORSELECTOR_COMPONENTS)
          {
             snprintf(colorbar_s, sizeof(colorbar_s), "elm.colorbar_%d",
                      (sd->sel_color_type + 1));
             /*Append color type only if next color bar is available*/
             if (elm_object_part_content_get(sd->col_bars_area, colorbar_s))
               sd->sel_color_type = sd->sel_color_type + 1;
             else return EINA_FALSE;
          }
     }
   else return EINA_FALSE;

   if (cl)
     {
        eo_item = eina_list_data_get(cl);
        elm_object_item_focus_set(eo_item, EINA_TRUE);
     }
   else if (!cl && (sd->focused == ELM_COLORSELECTOR_PALETTE))
     return EINA_FALSE;
   else if (!cl && (sd->focused == ELM_COLORSELECTOR_COMPONENTS))
      _elm_widget_focus_highlight_start(obj);

   return EINA_TRUE;
}


static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (!sd->selected) sd->selected = sd->items;
   if (!sd->focus_items) sd->focus_items = sd->items;
   if (sd->focused == ELM_COLORSELECTOR_PALETTE)
     {
        Elm_Object_Item *eo_item = NULL;
        eo_item = eina_list_data_get(sd->focus_items);
        elm_obj_color_item_selected_set(eo_item, EINA_TRUE);

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool _elm_colorselector_smart_focus_next_enable = EINA_FALSE;

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   Eina_List *l;
   Elm_Object_Item *eo_it;
   int i = 0;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if ((sd->mode == ELM_COLORSELECTOR_PALETTE) ||
       (sd->mode == ELM_COLORSELECTOR_ALL) ||
       (sd->mode == ELM_COLORSELECTOR_BOTH))
     {
        if (is_access)
          {
             EINA_LIST_FOREACH(sd->items, l, eo_it)
               elm_wdg_item_access_register(eo_it);
          }
        else
          {
             EINA_LIST_FOREACH(sd->items, l, eo_it)
               elm_wdg_item_access_unregister(eo_it);
          }
     }
   if ((sd->mode == ELM_COLORSELECTOR_COMPONENTS) ||
       (sd->mode == ELM_COLORSELECTOR_ALL) ||
       (sd->mode == ELM_COLORSELECTOR_BOTH))
     {
        for (i = 0; i < 4; i++)
          {
             if (is_access)
               _access_colorbar_register(obj, sd->cb_data[i],
                                         "elm.arrow_bg_access");
             else
               _elm_access_edje_object_part_object_unregister(obj, sd->cb_data[i]->colorbar,
                                                              "elm.arrow_bg_access");
          }
     }
}

EOLIAN static Eina_Rect
_elm_colorselector_efl_ui_widget_focus_highlight_geometry_get(const Eo *obj EINA_UNUSED, Elm_Colorselector_Data *sd)
{
   if (sd->focused_item && (sd->focused == ELM_COLORSELECTOR_PALETTE))
     {
       ELM_COLOR_ITEM_DATA_GET(sd->focused_item, focus_it);
       return efl_gfx_entity_geometry_get(VIEW(focus_it));
     }
   else if(sd->focused == ELM_COLORSELECTOR_COMPONENTS)
     return efl_gfx_entity_geometry_get(sd->cb_data[sd->sel_color_type]->colorbar);

   return efl_gfx_entity_geometry_get(obj);
}

EOLIAN static void
_elm_colorselector_efl_ui_widget_on_access_update(Eo *obj, Elm_Colorselector_Data *_pd EINA_UNUSED, Eina_Bool acs)
{
   _elm_colorselector_smart_focus_next_enable = acs;
   _access_obj_process(obj, _elm_colorselector_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_colorselector_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_colorselector_efl_object_constructor(Eo *obj, Elm_Colorselector_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_COLOR_CHOOSER);
   legacy_child_focus_handle(obj);

   return obj;
}

EOLIAN static void
_elm_colorselector_picked_color_set(Eo *obj, Elm_Colorselector_Data *_pd EINA_UNUSED, int r, int g, int b, int a)
{
   _colors_set(obj, r, g, b, a, EINA_FALSE);
}

EOLIAN static void
_elm_colorselector_picked_color_get(const Eo *obj EINA_UNUSED, Elm_Colorselector_Data *sd, int *r, int *g, int *b, int *a)
{
   if (r) *r = sd->r;
   if (g) *g = sd->g;
   if (b) *b = sd->b;
   if (a) *a = sd->a;
}

EOLIAN static void
_elm_colorselector_mode_set(Eo *obj, Elm_Colorselector_Data *sd, Elm_Colorselector_Mode mode)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Object *o;

   if (sd->mode == mode) return;
   sd->mode = mode;

   o = elm_layout_content_unset(obj, "elm.selector");
   if (!o) o = elm_layout_content_unset(obj, "selector");
   if (o) evas_object_hide(o);

   o = elm_layout_content_unset(obj, "elm.palette");
   if (!o) o = elm_layout_content_unset(obj, "palette");
   if (o) evas_object_hide(o);

   o = elm_layout_content_unset(obj, "elm.picker");
   if (!o) o = elm_layout_content_unset(obj, "picker");
   if (o) evas_object_hide(o);

   switch (sd->mode)
     {
      case ELM_COLORSELECTOR_PALETTE:
        if (!elm_layout_content_set(obj, "elm.palette", sd->palette_box))
          elm_layout_content_set(obj, "palette", sd->palette_box);
        elm_layout_signal_emit(obj, "elm,state,palette", "elm");
        sd->focused = ELM_COLORSELECTOR_PALETTE;
        break;

      case ELM_COLORSELECTOR_COMPONENTS:
        if (!elm_layout_content_set(obj, "elm.selector", sd->col_bars_area))
          elm_layout_content_set(obj, "selector", sd->col_bars_area);
        elm_layout_signal_emit(obj, "elm,state,components", "elm");
        sd->focused = ELM_COLORSELECTOR_COMPONENTS;
        sd->sel_color_type = HUE;
        _unselect_selected_item(sd);
        break;

      case ELM_COLORSELECTOR_BOTH:
        if (!elm_layout_content_set(obj, "elm.palette", sd->palette_box))
          elm_layout_content_set(obj, "palette", sd->palette_box);
        if (!elm_layout_content_set(obj, "elm.selector", sd->col_bars_area))
          elm_layout_content_set(obj, "selector", sd->col_bars_area);
        elm_layout_signal_emit(obj, "elm,state,both", "elm");
        sd->focused = ELM_COLORSELECTOR_PALETTE;
        break;

      case ELM_COLORSELECTOR_PICKER:
        _create_colorpicker(obj);
        if (!elm_layout_content_set(obj, "elm.picker", sd->picker))
          elm_layout_content_set(obj, "picker", sd->picker);
        elm_layout_signal_emit(obj, "elm,state,picker", "elm");
        sd->focused = ELM_COLORSELECTOR_PICKER;
        _unselect_selected_item(sd);
        break;

      case ELM_COLORSELECTOR_ALL:
        _create_colorpicker(obj);
        if (!elm_layout_content_set(obj, "elm.palette", sd->palette_box))
          elm_layout_content_set(obj, "palette", sd->palette_box);
        if (!elm_layout_content_set(obj, "elm.selector", sd->col_bars_area))
          elm_layout_content_set(obj, "selector", sd->col_bars_area);
        if (!elm_layout_content_set(obj, "elm.picker", sd->picker))
          elm_layout_content_set(obj, "picker", sd->picker);
        elm_layout_signal_emit(obj, "elm,state,all", "elm");
        sd->focused = ELM_COLORSELECTOR_PALETTE;
        break;

      default:
        return;
     }

   edje_object_message_signal_process(wd->resize_obj);

   _colors_set(obj, sd->r, sd->g, sd->b, sd->a, EINA_TRUE);
   elm_layout_sizing_eval(obj);
   _flush_color_children(obj, sd);
}

EOLIAN static Elm_Colorselector_Mode
_elm_colorselector_mode_get(const Eo *obj EINA_UNUSED, Elm_Colorselector_Data *sd)
{
   return sd->mode;
}

EAPI void
elm_colorselector_palette_item_color_get(const Elm_Object_Item *it,
                                         int *r,
                                         int *g,
                                         int *b,
                                         int *a)
{
   elm_obj_color_item_color_get(it, r, g, b, a);
}

EOLIAN static void
_elm_color_item_color_get(const Eo *eo_item EINA_UNUSED,
                          Elm_Color_Item_Data *item,
                          int *r,
                          int *g,
                          int *b,
                          int *a)
{
    if (r) *r = item->color->r;
    if (g) *g = item->color->g;
    if (b) *b = item->color->b;
    if (a) *a = item->color->a;
}

EAPI void
elm_colorselector_palette_item_color_set(Elm_Object_Item *it,
                                         int r,
                                         int g,
                                         int b,
                                         int a)
{
   elm_obj_color_item_color_set(it, r, g, b, a);
}

EOLIAN static void
_elm_color_item_color_set(Eo *eo_item EINA_UNUSED,
                          Elm_Color_Item_Data *item,
                          int r,
                          int g,
                          int b,
                          int a)
{
   item->color->r = r;
   item->color->g = g;
   item->color->b = b;
   item->color->a = a;
   evas_object_color_set(item->color_obj,
                         (item->color->r * item->color->a) / 255,
                         (item->color->g * item->color->a) / 255,
                         (item->color->b * item->color->a) / 255,
                         item->color->a);
   _colors_save(WIDGET(item));
}

EOLIAN static Elm_Object_Item*
_elm_colorselector_palette_color_add(Eo *obj, Elm_Colorselector_Data *sd, int r, int g, int b, int a)
{
   Eo *eo_item;

   if (sd->config_load)
     {
        _items_del(sd);
        sd->config_load = EINA_FALSE;
     }
   eo_item = efl_add(ELM_COLOR_ITEM_CLASS, obj);
   if (!eo_item) return NULL;

   ELM_COLOR_ITEM_DATA_GET(eo_item, item);
   item->color = ELM_NEW(Elm_Color_RGBA);
   if (!item->color) return NULL;

   item->color->r = r;
   item->color->g = g;
   item->color->b = b;
   item->color->a = a;

   efl_pack(sd->palette_box, VIEW(item));
   evas_object_color_set(item->color_obj,
                         (item->color->r * item->color->a) / 255,
                         (item->color->g * item->color->a) / 255,
                         (item->color->b * item->color->a) / 255,
                         item->color->a);

   sd->items = eina_list_append(sd->items, eo_item);

   elm_layout_sizing_eval(obj);

   return eo_item;
}

EOLIAN static void
_elm_colorselector_palette_clear(Eo *obj, Elm_Colorselector_Data *sd)
{

   _colors_remove(obj);
   if (sd->mode == ELM_COLORSELECTOR_BOTH)
     sd->focused = ELM_COLORSELECTOR_COMPONENTS;
}

EOLIAN static const Eina_List*
_elm_colorselector_palette_items_get(const Eo *obj EINA_UNUSED, Elm_Colorselector_Data *sd)
{
   return sd->items;
}

EAPI void
elm_colorselector_palette_item_selected_set(Elm_Object_Item *it,
                              Eina_Bool selected)
{
   elm_obj_color_item_selected_set(it, selected);
}

EOLIAN static void
_elm_color_item_selected_set(Eo *eo_item,
                             Elm_Color_Item_Data *item,
                             Eina_Bool selected)
{
   Eo *eo_temp_item;
   Eina_List *l;

   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   eo_temp_item = eina_list_data_get(sd->selected);
   if (eo_item == eo_temp_item)
     {
        if (!selected)
          {
             elm_object_signal_emit(VIEW(item), "elm,state,unselected", "elm");
             sd->selected = NULL;
          }

        return;
     }

   if (selected)
     {
        elm_object_signal_emit(VIEW(item), "elm,state,selected", "elm");
        elm_colorselector_color_set(WIDGET(item), item->color->r, item->color->g,
                                    item->color->b, item->color->a);
        if (eo_temp_item)
          {
             ELM_COLOR_ITEM_DATA_GET(eo_temp_item, temp_item);
             elm_object_signal_emit(VIEW(temp_item), "elm,state,unselected", "elm");
          }

        EINA_LIST_FOREACH(sd->items, l, eo_temp_item)
          if (eo_item == eo_temp_item) sd->selected = l;

        elm_object_signal_emit(VIEW(item), "elm,anim,activate", "elm");
        efl_event_callback_legacy_call
          (WIDGET(item), ELM_COLORSELECTOR_EVENT_COLOR_ITEM_SELECTED, eo_item);
     }
}

EAPI Eina_Bool
elm_colorselector_palette_item_selected_get(const Elm_Object_Item *it)
{
   return elm_obj_color_item_selected_get(it);
}

EOLIAN static Eina_Bool
_elm_color_item_selected_get(const Eo *eo_item EINA_UNUSED, Elm_Color_Item_Data *item)
{
   Eo *eo_temp_item;

   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   eo_temp_item = eina_list_data_get(sd->selected);
   if (eo_item == eo_temp_item) return EINA_TRUE;
   else return EINA_FALSE;
}

EOLIAN static Elm_Object_Item*
_elm_colorselector_palette_selected_item_get(const Eo *obj EINA_UNUSED, Elm_Colorselector_Data *sd)
{
   return eina_list_data_get(sd->selected);
}

EOLIAN static void
_elm_colorselector_palette_name_set(Eo *obj, Elm_Colorselector_Data *sd, const char *palette_name)
{
   EINA_SAFETY_ON_NULL_RETURN(palette_name);

   if (!strcmp(sd->palette_name, palette_name)) return;

   _items_del(sd);
   eina_stringshare_replace(&sd->palette_name, palette_name);
   _palette_colors_load(obj);
}

EOLIAN static const char*
_elm_colorselector_palette_name_get(const Eo *obj EINA_UNUSED, Elm_Colorselector_Data *sd)
{
   return sd->palette_name;
}

static void
_elm_colorselector_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
      _elm_colorselector_smart_focus_next_enable = EINA_TRUE;
}

EOLIAN static const Efl_Access_Action_Data*
_elm_colorselector_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Elm_Colorselector_Data *sd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "move,left", "move", "left", _key_action_move},
          { "move,right", "move", "right", _key_action_move},
          { "move,up", "move", "up", _key_action_move},
          { "move,down", "move", "down", _key_action_move},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

EOLIAN static Eina_List*
_elm_colorselector_efl_access_object_access_children_get(const Eo *obj EINA_UNUSED, Elm_Colorselector_Data *sd)
{
   Eina_List *ret = NULL;

   ret = efl_access_object_access_children_get(efl_super(obj, ELM_COLORSELECTOR_CLASS));
   // filter - out box contiainer
   ret = eina_list_remove(ret, sd->palette_box);
   // append items as colorselector children
   ret = eina_list_merge(ret, eina_list_clone(sd->items));

   return ret;
}

EOLIAN static Efl_Access_State_Set
_elm_color_item_efl_access_object_state_set_get(const Eo *obj, Elm_Color_Item_Data *sd EINA_UNUSED)
{
   Efl_Access_State_Set ret;
   Eina_Bool sel;

   ret = efl_access_object_state_set_get(efl_super(obj, ELM_COLOR_ITEM_CLASS));

   sel = elm_obj_color_item_selected_get(obj);

   if (sel)
     STATE_TYPE_SET(ret, EFL_ACCESS_STATE_CHECKED);

   return ret;
}

EOLIAN static void
_elm_color_item_elm_widget_item_item_focus_set(Eo *eo_it, Elm_Color_Item_Data *it, Eina_Bool focused)
{
   Evas_Object *obj = WIDGET(it);
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (focused)
     {
        if (eo_it != sd->focused_item)
          sd->focused_item = eo_it;

        Eina_List *l;
        Eo *eo_temp_item;
        EINA_LIST_FOREACH(sd->items, l, eo_temp_item)
          if (eo_it == eo_temp_item) sd->focus_items = l;
     }
   else
     {
        if (!efl_ui_focus_object_focus_get(obj))
          return;
        sd->focused_item = NULL;
     }

   _elm_widget_focus_highlight_start(obj);
}

EOLIAN static Eina_Bool
_elm_color_item_elm_widget_item_item_focus_get(const Eo *eo_it, Elm_Color_Item_Data *it)
{
   Evas_Object *obj = WIDGET(it);
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (eo_it == sd->focused_item)
     return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static const Efl_Access_Action_Data*
_elm_color_item_efl_access_widget_action_elm_actions_get(const Eo *eo_it EINA_UNUSED, Elm_Color_Item_Data *it EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "activate", "activate", NULL, _item_action_activate},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

EOLIAN static const char*
_elm_color_item_efl_access_object_i18n_name_get(const Eo *eo_it, Elm_Color_Item_Data *it)
{
   Eina_Strbuf *buf;
   const char *color_name = NULL;
   const char *name;
   char *accessible_name;

   name = efl_access_object_i18n_name_get(efl_super(eo_it, ELM_COLOR_ITEM_CLASS));
   if (name) return name;

   buf = eina_strbuf_new();
   color_name = _get_color_name(it->color->r, it->color->g, it->color->b, it->color->a);
   if (color_name)
     eina_strbuf_append_printf(buf, "%s", color_name);
   else
     eina_strbuf_append_printf(buf, "red %d, green %d, blue %d, alpha %d",
                               it->color->r, it->color->g, it->color->b, it->color->a);
   accessible_name = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);

   eina_stringshare_del(it->base->accessible_name);
   it->base->accessible_name = eina_stringshare_add(accessible_name);
   free(accessible_name);
   return it->base->accessible_name;
}

EOLIAN static Eina_Rect
_elm_color_item_efl_ui_focus_object_focus_geometry_get(const Eo *obj EINA_UNUSED, Elm_Color_Item_Data *pd)
{
   return efl_gfx_entity_geometry_get(pd->color_obj);
}

EOLIAN static void
_elm_color_item_efl_ui_focus_object_focus_set(Eo *obj, Elm_Color_Item_Data *pd, Eina_Bool focus)
{
   efl_ui_focus_object_focus_set(efl_super(obj, ELM_COLOR_ITEM_CLASS), focus);
   evas_object_focus_set(pd->color_obj, focus);
   elm_object_item_focus_set(obj, focus);
}

EOLIAN static Efl_Ui_Focus_Object*
_elm_color_item_efl_ui_focus_object_focus_parent_get(const Eo *obj EINA_UNUSED, Elm_Color_Item_Data *pd)
{
   return WIDGET(pd);
}

EOLIAN static Efl_Ui_Focus_Manager*
_elm_color_item_efl_ui_focus_object_focus_manager_get(const Eo *obj, Elm_Color_Item_Data *pd)
{
   return efl_ui_focus_object_focus_manager_get(WIDGET(pd));
}


/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(elm_colorselector, Elm_Colorselector_Data)

/* Internal EO APIs and hidden overrides */

#define ELM_COLORSELECTOR_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_colorselector), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_colorselector)

#include "elm_colorselector.eo.c"
#include "elm_color_item.eo.c"
