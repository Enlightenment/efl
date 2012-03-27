#include <Elementary.h>
#include "elm_priv.h"

#define BASE_STEP 360.0
#define HUE_STEP 360.0
#define SAT_STEP 128.0
#define LIG_STEP 256.0
#define ALP_STEP 256.0
#define DEFAULT_HOR_PAD 10
#define DEFAULT_VER_PAD 10

typedef enum _Color_Type
{
   HUE,
   SATURATION,
   LIGHTNESS,
   ALPHA
} Color_Type;

typedef struct _Colorselector_Data Colorselector_Data;
struct _Colorselector_Data
{
   Evas_Object *parent;
   Evas_Object *colorbar;
   Evas_Object *bar;
   Evas_Object *lbt;
   Evas_Object *rbt;
   Evas_Object *bg_rect;
   Evas_Object *arrow;
   Evas_Object *touch_area;
   Color_Type color_type;
};

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Color_Item Elm_Color_Item;
struct _Widget_Data
{
   Evas_Object *sel;
   Evas_Object *base;
   Evas_Object *box;
   Eina_List *items;
   Colorselector_Data *cp[4];
   Ecore_Timer *longpress_timer;
   const char *palette_name;
   Evas_Coord _x, _y, _w, _h;
   int r, g, b, a;
   int er, eg, eb;
   int sr, sg, sb;
   int lr, lg, lb;
   double h, s, l;
   Elm_Colorselector_Mode mode;
   Eina_Bool longpressed : 1;
   Eina_Bool config_load: 1;
};

struct _Elm_Color_Item
{
   ELM_WIDGET_ITEM;
   Evas_Object *color_obj;
   Elm_Color_RGBA *color;
};

static const char *widtype = NULL;

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _resize_cb(void *data, Evas *a, Evas_Object *obj, void *event_info);
static void _rgb_to_hsl(void *data);
static void _hsl_to_rgb(void *data);
static void _color_with_saturation(void *data);
static void _color_with_lightness(void *data);
static void _draw_rects(void *data, double x);
static void _arrow_cb(void *data, Evas_Object *obj, const char *emission,
                      const char *source);
static void _colorbar_cb(void *data, Evas *e, Evas_Object *obj,
                         void *event_info);
static void _left_button_clicked_cb(void *data, Evas_Object * obj,
                                    void *event_info);
static void _left_button_repeat_cb(void *data, Evas_Object * obj,
                                   void *event_info);
static void _right_button_clicked_cb(void *data, Evas_Object * obj,
                                     void *event_info);
static void _right_button_repeat_cb(void *data, Evas_Object * obj,
                                    void *event_info);
static void _add_colorbar(Evas_Object *obj);
static void _set_color(Evas_Object *obj, int r, int g, int b, int a);
static Elm_Color_Item *_item_new(Evas_Object *obj);
static void _item_sizing_eval(Elm_Color_Item *item);
static void _item_highlight(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _item_unhighlight(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool _long_press(void *data);
static void _remove_items(Widget_Data *wd);
static void _colors_remove(Evas_Object *obj);
static void _colors_save(Evas_Object *obj);
static void _colors_load_apply(Evas_Object *obj);

static const char SIG_CHANGED[] = "changed";
static const char SIG_COLOR_ITEM_SELECTED[] = "color,item,selected";
static const char SIG_COLOR_ITEM_LONGPRESSED[] = "color,item,longpressed";

static const Evas_Smart_Cb_Description _signals[] =
{
   {SIG_COLOR_ITEM_SELECTED, ""},
   {SIG_COLOR_ITEM_LONGPRESSED, ""},
   {SIG_CHANGED, ""},
   {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int i = 0;

   if (!wd) return;
   if (wd->longpress_timer) ecore_timer_del(wd->longpress_timer);
   if (wd->palette_name) eina_stringshare_del(wd->palette_name);
   _remove_items(wd);
   for (i = 0; i < 4; i++) free(wd->cp[i]);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *elist;
   Elm_Color_Item *item;
   int i;
   const char *hpadstr, *vpadstr;
   unsigned int h_pad = DEFAULT_HOR_PAD;
   unsigned int v_pad = DEFAULT_VER_PAD;

   if ((!wd) || (!wd->sel)) return;

   _elm_theme_object_set(obj, wd->base, "colorselector", "palette",
                         elm_widget_style_get(obj));
   _elm_theme_object_set(obj, wd->sel, "colorselector", "bg",
                         elm_widget_style_get(obj));
   hpadstr = edje_object_data_get(wd->base, "horizontal_pad");
   if (hpadstr) h_pad = atoi(hpadstr);
   vpadstr = edje_object_data_get(wd->base, "vertical_pad");
   if (vpadstr) v_pad = atoi(vpadstr);
   elm_box_padding_set(wd->box, (Evas_Coord)(h_pad * elm_widget_scale_get(obj) * _elm_config->scale),
                       (Evas_Coord)(v_pad * elm_widget_scale_get(obj) *_elm_config->scale));
   EINA_LIST_FOREACH(wd->items, elist, item)
     {
        elm_layout_theme_set(VIEW(item), "colorselector", "item", elm_widget_style_get(obj));
        _elm_theme_object_set(obj, item->color_obj, "colorselector", "item/color", elm_widget_style_get(obj));
     }
   for (i = 0; i < 4; i++)
     {
        evas_object_del(wd->cp[i]->colorbar);
        wd->cp[i]->colorbar = NULL;
        evas_object_del(wd->cp[i]->bar);
        wd->cp[i]->bar = NULL;
        evas_object_del(wd->cp[i]->lbt);
        wd->cp[i]->lbt = NULL;
        evas_object_del(wd->cp[i]->rbt);
        wd->cp[i]->rbt = NULL;
        if (i != 0)
          {
             evas_object_del(wd->cp[i]->bg_rect);
             wd->cp[i]->bg_rect = NULL;
          }
        evas_object_del(wd->cp[i]->arrow);
        wd->cp[i]->arrow = NULL;
        evas_object_del(wd->cp[i]->touch_area);
        wd->cp[i]->touch_area = NULL;
     }

   _add_colorbar(obj);
   elm_colorselector_color_set(obj, wd->r, wd->g, wd->b, wd->a);
   _sizing_eval(obj);
}

static void
_colorselector_set_size_hints(Evas_Object *obj, int timesw, int timesh)
{
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(timesw, &minw, timesh, &minh);
   edje_object_size_min_restricted_calc(obj, &minw, &minh,
                                        minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_item_sizing_eval(Elm_Color_Item *item)
{
   Evas_Coord minw = -1, minh = -1;

   if (!item) return;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(VIEW(item), &minw, &minh, minw,
                                        minh);
   evas_object_size_hint_min_set(VIEW(item), minw, minh);
}

static void _resize_cb(void *data, Evas *a __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_sizing_eval_palette(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *elist;
   Elm_Color_Item *item;
   Evas_Coord bw = 0, bh = 0;
   Evas_Coord w = 0, h = 0;
   if (!wd) return;

   EINA_LIST_FOREACH(wd->items, elist, item)
     {
        _item_sizing_eval(item);
     }
   evas_object_size_hint_min_get(wd->box, &bw, &bh);
   evas_object_size_hint_min_set(obj, bw, bh);
   evas_object_size_hint_max_set(obj, -1, -1);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < bw) w = bw;
   if (h < bh) h = bh;
   evas_object_resize(obj, w, h);
}

static void
_sizing_eval_selector(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord w = 0, h = 0;
   int i;

   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   for (i = 0; i < 4; i++)
     {
        if (wd->cp[i]->bg_rect)
          _colorselector_set_size_hints(wd->cp[i]->bg_rect, 1, 1);
        _colorselector_set_size_hints(wd->cp[i]->bar, 1, 1);
        _colorselector_set_size_hints(wd->cp[i]->rbt, 1, 1);
        _colorselector_set_size_hints(wd->cp[i]->lbt, 1, 1);

        _colorselector_set_size_hints(wd->cp[i]->colorbar, 4, 1);
     }

   elm_coords_finger_size_adjust(4, &minw, 4, &minh);
   edje_object_size_min_restricted_calc(wd->sel, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   evas_object_resize(obj, w, h);
}

static void
_sizing_eval_palette_selector(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord bw = 0, bh = 0;
   Evas_Coord w = 0, h = 0;
   int i;
   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   for (i = 0; i < 4; i++)
     {
        if (wd->cp[i]->bg_rect)
          _colorselector_set_size_hints(wd->cp[i]->bg_rect, 1, 1);
        _colorselector_set_size_hints(wd->cp[i]->bar, 1, 1);
        _colorselector_set_size_hints(wd->cp[i]->rbt, 1, 1);
        _colorselector_set_size_hints(wd->cp[i]->lbt, 1, 1);

        _colorselector_set_size_hints(wd->cp[i]->colorbar, 4, 1);
     }

   elm_coords_finger_size_adjust(4, &minw, 4, &minh);
   edje_object_size_min_restricted_calc(wd->sel, &minw, &minh, minw, minh);
   evas_object_size_hint_min_get(wd->box, &bw, &bh);
   evas_object_size_hint_min_set(obj, minw, minh+bh);
   evas_object_size_hint_max_set(obj, -1, -1);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < (minh+bh)) h = (minh+bh);
   evas_object_resize(obj, w, h);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   switch (wd->mode)
     {
        case ELM_COLORSELECTOR_PALETTE:
           _sizing_eval_palette(obj);
           break;
        case ELM_COLORSELECTOR_COMPONENTS:
           _sizing_eval_selector(obj);
           break;
        case ELM_COLORSELECTOR_BOTH:
           _sizing_eval_palette_selector(obj);
           break;
        default:
           break;
     }
}

static Eina_Bool
_long_press(void *data)
{
   Elm_Color_Item *item = (Elm_Color_Item *) data;
   Widget_Data *wd = elm_widget_data_get(WIDGET(item));
   if (!wd) return ECORE_CALLBACK_CANCEL;
   wd->longpress_timer = NULL;
   wd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(WIDGET(item), SIG_COLOR_ITEM_LONGPRESSED, item);
   return ECORE_CALLBACK_CANCEL;
}

static void
_item_highlight(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Color_Item *item = (Elm_Color_Item *) data;
   Evas_Event_Mouse_Down *ev = event_info;
   if (!item) return;
   Widget_Data *wd = elm_widget_data_get(WIDGET(item));
   if (!wd) return;
   if (ev->button != 1) return;
   elm_object_signal_emit(VIEW(item), "elm,state,selected", "elm");
   wd->longpressed = EINA_FALSE;
   if (wd->longpress_timer) ecore_timer_del(wd->longpress_timer);
   wd->longpress_timer = ecore_timer_add(_elm_config->longpress_timeout, _long_press, data);
}

static void
_item_unhighlight(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Color_Item *item = (Elm_Color_Item *) data;
   Evas_Event_Mouse_Down *ev = event_info;
   if (!item) return;
   Widget_Data *wd = elm_widget_data_get(WIDGET(item));
   if (!wd) return;
   if (ev->button != 1) return;
   if (wd->longpress_timer)
     {
        ecore_timer_del(wd->longpress_timer);
        wd->longpress_timer = NULL;
     }
   elm_object_signal_emit(VIEW(item), "elm,state,unselected", "elm");
   if (!wd->longpressed)
     {
        evas_object_smart_callback_call(WIDGET(item), SIG_COLOR_ITEM_SELECTED, item);
        elm_colorselector_color_set(WIDGET(item), item->color->r, item->color->g, item->color->b, item->color->a);
     }
}

static void
_remove_items(Widget_Data *wd)
{
   Elm_Color_Item *item;

   if (!wd->items) return;

   EINA_LIST_FREE(wd->items, item)
     {
        free(item->color);
        elm_widget_item_free(item);
     }

   wd->items = NULL;
}

static Elm_Color_Item*
_item_new(Evas_Object *obj)
{
   Elm_Color_Item *item;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   item = elm_widget_item_new(obj, Elm_Color_Item);
   if (!item) return NULL;

   VIEW(item) = elm_layout_add(obj);
   elm_layout_theme_set(VIEW(item), "colorselector", "item", elm_widget_style_get(obj));
   evas_object_size_hint_weight_set(VIEW(item), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
   item->color_obj = edje_object_add(evas_object_evas_get(obj));
   _elm_theme_object_set(obj, item->color_obj, "colorselector", "item/color", elm_widget_style_get(obj));
   evas_object_size_hint_weight_set(item->color_obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(item->color_obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add(item->color_obj, EVAS_CALLBACK_MOUSE_DOWN, _item_highlight, item);
   evas_object_event_callback_add(item->color_obj, EVAS_CALLBACK_MOUSE_UP, _item_unhighlight, item);
   elm_object_part_content_set(VIEW(item), "color_obj", item->color_obj);
   _item_sizing_eval(item);
   evas_object_show(VIEW(item));

   return item;
}

static void
_colors_remove(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   _remove_items(wd);
   _elm_config_colors_free(wd->palette_name);
}

static void _colors_save(Evas_Object *obj)
{
   Eina_List *elist;
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Color_Item *item;
   _elm_config_colors_free(wd->palette_name);
   EINA_LIST_FOREACH(wd->items, elist, item)
     {
        _elm_config_color_set(wd->palette_name, item->color->r, item->color->g,
                              item->color->b, item->color->a);
     }
}

static void
_colors_load_apply(Evas_Object *obj)
{
   Elm_Color_RGBA *color;
   Eina_List *elist;
   Eina_List *color_list;
   Elm_Color_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);
   color_list = _elm_config_color_list_get(wd->palette_name);
   if (!color_list) return;
   EINA_LIST_FOREACH(color_list, elist, color)
     {
        item = _item_new(obj);
        if (!item) return;
        item->color = ELM_NEW(Elm_Color_RGBA);
        if (!item->color) return;
        item->color->r = color->r;
        item->color->g = color->g;
        item->color->b = color->b;
        item->color->a = color->a;
        elm_box_pack_end(wd->box, VIEW(item));
        evas_object_color_set(item->color_obj, item->color->r, item->color->g,
                              item->color->b, item->color->a);
        wd->items = eina_list_append(wd->items, item);
        _sizing_eval_palette(obj);
     }
   wd->config_load = EINA_TRUE;
}

static void
_rgb_to_hsl(void *data)
{
   Widget_Data *wd = data;
   double r, g, b;
   double v, m, vm;
   double r2, g2, b2;

   r = wd->r;
   g = wd->g;
   b = wd->b;

   r /= 255.0;
   g /= 255.0;
   b /= 255.0;

   v = (r > g) ? r : g;
   v = (v > b) ? v : b;

   m = (r < g) ? r : g;
   m = (m < b) ? m : b;

   wd->h = 0.0;
   wd->s = 0.0;
   wd->l = 0.0;

   wd->l = (m + v) / 2.0;

   if (wd->l <= 0.0) return;

   vm = v - m;
   wd->s = vm;

   if (wd->s > 0.0) wd->s /= (wd->l <= 0.5) ? (v + m) : (2.0 - v - m);
   else return;

   r2 = (v - r) / vm;
   g2 = (v - g) / vm;
   b2 = (v - b) / vm;

   if (r == v) wd->h = (g == m ? 5.0 + b2 : 1.0 - g2);
   else if (g == v) wd->h = (b == m ? 1.0 + r2 : 3.0 - b2);
   else wd->h = (r == m ? 3.0 + g2 : 5.0 - r2);

   wd->h *= 60.0;
}

static void
_hsl_to_rgb(void *data)
{
   Widget_Data *wd = data;
   double r = 0, g = 0, b = 0;
   double _h, _s, _l;
   int i = 0;
   double sv, vsf, f, p, q, t, v;

   _h = wd->h;
   _s = wd->s;
   _l = wd->l;

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
   wd->r = (f <= 0.5) ? i : (i + 1);

   i = (int)(g * 255.0);
   f = (g * 255.0) - i;
   wd->g = (f <= 0.5) ? i : (i + 1);

   i = (int)(b * 255.0);
   f = (b * 255.0) - i;
   wd->b = (f <= 0.5) ? i : (i + 1);
}

static void
_color_with_saturation(void *data)
{
   Widget_Data *wd = data;

   if (wd->er > 127)
     wd->sr = 127 + (int)((double)(wd->er - 127) * wd->s);
   else
     wd->sr = 127 - (int)((double)(127 - wd->er) * wd->s);

   if (wd->eg > 127)
     wd->sg = 127 + (int)((double)(wd->eg - 127) * wd->s);
   else
     wd->sg = 127 - (int)((double)(127 - wd->eg) * wd->s);

   if (wd->eb > 127)
     wd->sb = 127 + (int)((double)(wd->eb - 127) * wd->s);
   else
     wd->sb = 127 - (int)((double)(127 - wd->eb) * wd->s);
}

static void
_color_with_lightness(void *data)
{
   Widget_Data *wd = data;

   if (wd->l > 0.5)
     {
        wd->lr = wd->er + (int)((double)(255 - wd->er) * (wd->l - 0.5) * 2.0);
        wd->lg = wd->eg + (int)((double)(255 - wd->eg) * (wd->l - 0.5) * 2.0);
        wd->lb = wd->eb + (int)((double)(255 - wd->eb) * (wd->l - 0.5) * 2.0);
     }
   else if (wd->l < 0.5)
     {
        wd->lr = (double)wd->er * wd->l * 2.0;
        wd->lg = (double)wd->eg * wd->l * 2.0;
        wd->lb = (double)wd->eb * wd->l * 2.0;
     }
   else
     {
        wd->lr = wd->er;
        wd->lg = wd->eg;
        wd->lb = wd->eb;
     }
}

static void
_draw_rects(void *data, double x)
{
   Colorselector_Data *cp = data;
   Widget_Data *wd = elm_widget_data_get(cp->parent);
   double one_six = 1.0 / 6.0;

   switch (cp->color_type)
     {
      case HUE:
         wd->h = 360.0 * x;

         if (x < one_six)
           {
              wd->er = 255;
              wd->eg = (255.0 * x * 6.0);
              wd->eb = 0;
           }
         else if (x < 2 * one_six)
           {
              wd->er = 255 - (int)(255.0 * (x - one_six) * 6.0);
              wd->eg = 255;
              wd->eb = 0;
           }
         else if (x < 3 * one_six)
           {
              wd->er = 0;
              wd->eg = 255;
              wd->eb = (int)(255.0 * (x - (2.0 * one_six)) * 6.0);
           }
         else if (x < 4 * one_six)
           {
              wd->er = 0;
              wd->eg = 255 - (int)(255.0 * (x - (3.0 * one_six)) * 6.0);
              wd->eb = 255;
           }
         else if (x < 5 * one_six)
           {
              wd->er = 255.0 * (x - (4.0 * one_six)) * 6.0;
              wd->eg = 0;
              wd->eb = 255;
           }
         else
           {
              wd->er = 255;
              wd->eg = 0;
              wd->eb = 255 - (int)(255.0 * (x - (5.0 * one_six)) * 6.0);
           }

         evas_object_color_set(wd->cp[0]->arrow, wd->er, wd->eg, wd->eb, 255);
         evas_object_color_set(wd->cp[1]->bg_rect, wd->er, wd->eg, wd->eb, 255);
         evas_object_color_set(wd->cp[2]->bg_rect, wd->er, wd->eg, wd->eb, 255);
         evas_object_color_set(wd->cp[3]->bar, wd->er, wd->eg, wd->eb, 255);

         _color_with_saturation(wd);
         evas_object_color_set(wd->cp[1]->arrow, wd->sr, wd->sg, wd->sb, 255);

         _color_with_lightness(wd);
         evas_object_color_set(wd->cp[2]->arrow, wd->lr, wd->lg, wd->lb, 255);

         evas_object_color_set(wd->cp[3]->arrow,
                               (wd->er * wd->a) / 255,
                               (wd->eg * wd->a) / 255,
                               (wd->eb * wd->a) / 255,
                               wd->a);
         break;
      case SATURATION:
         wd->s = 1.0 - x;
         _color_with_saturation(wd);
         evas_object_color_set(wd->cp[1]->arrow, wd->sr, wd->sg, wd->sb, 255);
         break;
      case LIGHTNESS:
         wd->l = x;
         _color_with_lightness(wd);
         evas_object_color_set(wd->cp[2]->arrow, wd->lr, wd->lg, wd->lb, 255);
         break;
      case ALPHA:
         wd->a = 255.0 * x;
         evas_object_color_set(wd->cp[3]->arrow,
                               (wd->er * wd->a) / 255,
                               (wd->eg * wd->a) / 255,
                               (wd->eb * wd->a) / 255,
                               wd->a);
         break;
      default:
         break;
     }
   _hsl_to_rgb(wd);
}

static void
_arrow_cb(void *data, Evas_Object *obj, const char *emission __UNUSED__,
          const char *source __UNUSED__)
{
   Colorselector_Data *cp = data;
   double x, y;

   edje_object_part_drag_value_get(obj, "elm.arrow", &x, &y);
   _draw_rects(data, x);
   evas_object_smart_callback_call(cp->parent, SIG_CHANGED, NULL);
}

static void
_colorbar_cb(void *data, Evas *e, Evas_Object *obj __UNUSED__, void *event_info)
{
   Colorselector_Data *cp = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w, h;
   double arrow_x = 0, arrow_y;

   evas_object_geometry_get(cp->bar, &x, &y, &w, &h);
   edje_object_part_drag_value_get(cp->colorbar, "elm.arrow",
                                   &arrow_x, &arrow_y);
   if (w > 0) arrow_x = (double)(ev->canvas.x - x) / (double)w;
   if (arrow_x > 1) arrow_x = 1;
   if (arrow_x < 0) arrow_x = 0;
   edje_object_part_drag_value_set(cp->colorbar, "elm.arrow", arrow_x, arrow_y);
   _draw_rects(data, arrow_x);
   evas_object_smart_callback_call(cp->parent, SIG_CHANGED, NULL);
   evas_event_feed_mouse_cancel(e, 0, NULL);
   evas_event_feed_mouse_down(e, 1, EVAS_BUTTON_NONE, 0, NULL);
}

static void
_left_button_clicked_cb(void *data, Evas_Object * obj __UNUSED__,
                        void *event_info __UNUSED__)
{
   Colorselector_Data *cp = data;
   double x, y;

   edje_object_signal_emit(cp->lbt, "elm,state,left,button,down",
                           "left_button");
   edje_object_part_drag_value_get(cp->colorbar, "elm.arrow", &x, &y);

   switch(cp->color_type)
     {
      case HUE :
         x -= 1.0 / HUE_STEP;
         break;
      case SATURATION :
         x -= 1.0 / SAT_STEP;
         break;
      case LIGHTNESS :
         x -= 1.0 / LIG_STEP;
         break;
      case ALPHA :
         x -= 1.0 / ALP_STEP;
         break;
      default :
         break;
     }

   if (x < 0.0) x = 0.0;

   edje_object_part_drag_value_set(cp->colorbar, "elm.arrow", x, y);
   _draw_rects(data, x);
   evas_object_smart_callback_call(cp->parent, SIG_CHANGED, NULL);
}

static void
_left_button_repeat_cb(void *data, Evas_Object * obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   Colorselector_Data *cp = data;
   double x, y;

   edje_object_part_drag_value_get(cp->colorbar, "elm.arrow", &x, &y);
   x -= 1.0 / BASE_STEP;
   if (x < 0.0) x = 0.0;
   edje_object_part_drag_value_set(cp->colorbar, "elm.arrow", x, y);
   _draw_rects(data, x);
   evas_object_smart_callback_call(cp->parent, SIG_CHANGED, NULL);

}

static void
_right_button_clicked_cb(void *data, Evas_Object * obj __UNUSED__,
                         void *event_info __UNUSED__)
{
   Colorselector_Data *cp = data;
   double x, y;

   edje_object_signal_emit(cp->rbt, "elm,state,right,button,down",
                           "right_button");
   edje_object_part_drag_value_get(cp->colorbar, "elm.arrow", &x, &y);

   switch(cp->color_type)
     {
      case HUE :
         x += 1.0 / HUE_STEP;
         break;
      case SATURATION :
         x += 1.0 / SAT_STEP;
         break;
      case LIGHTNESS :
         x += 1.0 / LIG_STEP;
         break;
      case ALPHA :
         x += 1.0 / ALP_STEP;
         break;
      default :
         break;
     }

   if (x > 1.0) x = 1.0;

   edje_object_part_drag_value_set(cp->colorbar, "elm.arrow", x, y);
   _draw_rects(data, x);
   evas_object_smart_callback_call(cp->parent, SIG_CHANGED, NULL);
}

static void
_right_button_repeat_cb(void *data, Evas_Object * obj __UNUSED__,
                        void *event_info __UNUSED__)
{
   Colorselector_Data *cp = data;
   double x, y;

   edje_object_part_drag_value_get(cp->colorbar, "elm.arrow", &x, &y);
   x += 1.0 / BASE_STEP;
   if (x > 1.0) x = 1.0;
   edje_object_part_drag_value_set(cp->colorbar, "elm.arrow", x, y);
   _draw_rects(data, x);
   evas_object_smart_callback_call(cp->parent, SIG_CHANGED, NULL);
}

static void
_add_colorbar(Evas_Object *obj)
{
   char colorbar_name[128];
   char colorbar_s[128];
   Widget_Data *wd;
   Evas *e;
   int i = 0;
   char buf[1024];

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   e = evas_object_evas_get(obj);

   for (i = 0; i < 4; i++)
     {
        wd->cp[i] = ELM_NEW(Colorselector_Data);
        wd->cp[i]->parent = obj;
        switch(i)
          {
           case 0 :
              wd->cp[i]->color_type = HUE;
              break;
           case 1 :
              wd->cp[i]->color_type = SATURATION;
              break;
           case 2 :
              wd->cp[i]->color_type = LIGHTNESS;
              break;
           case 3 :
              wd->cp[i]->color_type = ALPHA;
              break;
           default :
              break;
          }
        /* load colorbar area */
        wd->cp[i]->colorbar = edje_object_add(e);
        _elm_theme_object_set(obj, wd->cp[i]->colorbar, "colorselector", "base",
                              elm_widget_style_get(obj));
        snprintf(colorbar_name, sizeof(colorbar_name), "colorbar_%d", i);
        snprintf(colorbar_s, sizeof(colorbar_s), "elm.colorbar_%d", i);
        edje_object_signal_callback_add(wd->cp[i]->colorbar, "drag", "*",
                                        _arrow_cb, wd->cp[i]);
        edje_object_part_swallow(wd->sel, colorbar_s, wd->cp[i]->colorbar);
        elm_widget_sub_object_add(obj, wd->cp[i]->colorbar);

        /* load colorbar image */
        wd->cp[i]->bar = edje_object_add(e);
        snprintf(buf, sizeof(buf), "%s/%s", colorbar_name,
                 elm_widget_style_get(obj));
        _elm_theme_object_set(obj, wd->cp[i]->bar, "colorselector", "image",
                              buf);
        edje_object_part_swallow(wd->cp[i]->colorbar, "elm.bar",
                                 wd->cp[i]->bar);
        elm_widget_sub_object_add(obj, wd->cp[i]->bar);

        /* provide expanded touch area */
        wd->cp[i]->touch_area = evas_object_rectangle_add(e);
        evas_object_color_set(wd->cp[i]->touch_area, 0, 0, 0, 0);
        edje_object_part_swallow(wd->cp[i]->colorbar, "elm.arrow_bg",
                                 wd->cp[i]->touch_area);
        evas_object_event_callback_add(wd->cp[i]->touch_area,
                                       EVAS_CALLBACK_MOUSE_DOWN, _colorbar_cb,
                                       wd->cp[i]);
        elm_widget_sub_object_add(obj, wd->cp[i]->touch_area);

        /* load background rectangle of the colorbar. used for
           changing color of the opacity bar */
        if ((i == 1) || (i == 2))
          {
             wd->cp[i]->bg_rect = evas_object_rectangle_add(e);
             evas_object_color_set(wd->cp[i]->bg_rect, wd->er, wd->eg, wd->eb,
                                   255);
             edje_object_part_swallow(wd->cp[i]->colorbar, "elm.bar_bg",
                                      wd->cp[i]->bg_rect);

             elm_widget_sub_object_add(obj, wd->cp[i]->bg_rect);
          }
        if (i == 3)
          {
             wd->cp[i]->bg_rect = edje_object_add(e);
             snprintf(buf, sizeof(buf), "%s/%s", colorbar_name,
                      elm_widget_style_get(obj));
             _elm_theme_object_set(obj, wd->cp[i]->bg_rect, "colorselector",
                                   "bg_image", buf);
             edje_object_part_swallow(wd->cp[i]->colorbar, "elm.bar_bg",
                                      wd->cp[i]->bg_rect);
             elm_widget_sub_object_add(obj, wd->cp[i]->bg_rect);
             evas_object_color_set(wd->cp[i]->bar, wd->er, wd->eg, wd->eb, 255);
          }
        /* load arrow image, pointing the colorbar */
        wd->cp[i]->arrow = edje_object_add(e);
        _elm_theme_object_set(obj, wd->cp[i]->arrow, "colorselector", "arrow",
                              elm_widget_style_get(obj));
        edje_object_part_swallow(wd->cp[i]->colorbar, "elm.arrow_icon",
                                 wd->cp[i]->arrow);
        elm_widget_sub_object_add(obj, wd->cp[i]->arrow);
        if (i == 2)
          evas_object_color_set(wd->cp[i]->arrow, 0, 0, 0, 255);
        else
          evas_object_color_set(wd->cp[i]->arrow, wd->er, wd->eg, wd->eb, 255);

        /* load left button */
        wd->cp[i]->lbt = elm_button_add(obj);
        snprintf(buf, sizeof(buf), "colorselector/left/%s",
                 elm_widget_style_get(obj));
        elm_object_style_set(wd->cp[i]->lbt, buf);
        elm_widget_sub_object_add(obj, wd->cp[i]->lbt);
        edje_object_part_swallow(wd->cp[i]->colorbar, "elm.l_button",
                                 wd->cp[i]->lbt);
        evas_object_smart_callback_add(wd->cp[i]->lbt, "clicked",
                                       _left_button_clicked_cb, wd->cp[i]);
        elm_button_autorepeat_set(wd->cp[i]->lbt, EINA_TRUE);
        elm_button_autorepeat_initial_timeout_set(wd->cp[i]->lbt,
                                                  _elm_config->longpress_timeout);
        elm_button_autorepeat_gap_timeout_set(wd->cp[i]->lbt,
                                              (1.0 / _elm_config->fps));
        evas_object_smart_callback_add(wd->cp[i]->lbt, "repeated",
                                       _left_button_repeat_cb, wd->cp[i]);

        /* load right button */
        wd->cp[i]->rbt = elm_button_add(obj);
        snprintf(buf, sizeof(buf), "colorselector/right/%s",
                 elm_widget_style_get(obj));
        elm_object_style_set(wd->cp[i]->rbt, buf);
        elm_widget_sub_object_add(obj, wd->cp[i]->rbt);
        edje_object_part_swallow(wd->cp[i]->colorbar, "elm.r_button",
                                 wd->cp[i]->rbt);
        evas_object_smart_callback_add(wd->cp[i]->rbt, "clicked",
                                       _right_button_clicked_cb, wd->cp[i]);
        elm_button_autorepeat_set(wd->cp[i]->rbt, EINA_TRUE);
        elm_button_autorepeat_initial_timeout_set(wd->cp[i]->rbt,
                                                  _elm_config->longpress_timeout);
        elm_button_autorepeat_gap_timeout_set(wd->cp[i]->rbt,
                                              (1.0 / _elm_config->fps));
        evas_object_smart_callback_add(wd->cp[i]->rbt, "repeated",
                                       _right_button_repeat_cb, wd->cp[i]);
     }
}

static void
_set_color(Evas_Object *obj, int r, int g, int b, int a)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double x, y;

   wd->r = r;
   wd->g = g;
   wd->b = b;
   wd->a = a;

   _rgb_to_hsl(wd);

   edje_object_part_drag_value_get(wd->cp[0]->colorbar, "elm.arrow", &x, &y);
   x = wd->h / 360.0;
   edje_object_part_drag_value_set(wd->cp[0]->colorbar, "elm.arrow", x, y);
   _draw_rects(wd->cp[0], x);

   edje_object_part_drag_value_get(wd->cp[1]->colorbar, "elm.arrow", &x, &y);
   x = 1.0 - wd->s;
   edje_object_part_drag_value_set(wd->cp[1]->colorbar, "elm.arrow", x, y);
   _draw_rects(wd->cp[1], x);

   edje_object_part_drag_value_get(wd->cp[2]->colorbar, "elm.arrow", &x, &y);
   x = wd->l;
   edje_object_part_drag_value_set(wd->cp[2]->colorbar, "elm.arrow", x, y);
   _draw_rects(wd->cp[2], x);

   edje_object_part_drag_value_get(wd->cp[3]->colorbar, "elm.arrow", &x, &y);
   x = wd->a / 255.0;
   edje_object_part_drag_value_set(wd->cp[3]->colorbar, "elm.arrow", x, y);
   _draw_rects(wd->cp[3], x);
}

EAPI Evas_Object *
elm_colorselector_add(Evas_Object *parent)
{
   Evas_Object *obj = NULL;
   Widget_Data *wd = NULL;
   Evas *e;
   const char *hpadstr, *vpadstr;
   unsigned int h_pad = DEFAULT_HOR_PAD;
   unsigned int v_pad = DEFAULT_VER_PAD;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "colorselector");
   elm_widget_type_set(obj, "colorselector");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   /* load background edj */
   wd->base = edje_object_add(e);
   _elm_theme_object_set(obj, wd->base, "colorselector", "palette", "default");
   elm_widget_resize_object_set(obj, wd->base);
   evas_object_event_callback_add(wd->base, EVAS_CALLBACK_RESIZE,
                                  _resize_cb, obj);

   wd->box = elm_box_add(obj);
   elm_box_layout_set(wd->box, evas_object_box_layout_flow_horizontal,
                      NULL, NULL);
   elm_box_horizontal_set(wd->box, EINA_TRUE);
   evas_object_size_hint_weight_set(wd->box, EVAS_HINT_EXPAND,
                                    0);
   evas_object_size_hint_align_set(wd->box, EVAS_HINT_FILL, 0);
   elm_box_homogeneous_set(wd->box, EINA_TRUE);
   hpadstr = edje_object_data_get(wd->base, "horizontal_pad");
   if (hpadstr) h_pad = atoi(hpadstr);
   vpadstr = edje_object_data_get(wd->base, "vertical_pad");
   if (vpadstr) v_pad = atoi(vpadstr);
   elm_box_padding_set(wd->box, (Evas_Coord)(h_pad * elm_widget_scale_get(obj) * _elm_config->scale),
                       (Evas_Coord)(v_pad * elm_widget_scale_get(obj) *_elm_config->scale));
   elm_box_align_set(wd->box, 0.5, 0.5);
   elm_widget_sub_object_add(obj, wd->box);
   evas_object_show(wd->box);
   edje_object_part_swallow(wd->base, "palette", wd->box);
   wd->palette_name = eina_stringshare_add("default");
   _colors_load_apply(obj);

   /* load background edj */
   wd->sel = edje_object_add(e);
   _elm_theme_object_set(obj, wd->sel, "colorselector", "bg", "default");
   edje_object_part_swallow(wd->base, "selector", wd->sel);
   elm_widget_sub_object_add(obj, wd->sel);

   wd->mode = ELM_COLORSELECTOR_BOTH;
   wd->er = 255;
   wd->eg = 0;
   wd->eb = 0;
   wd->h = 0.0;
   wd->s = 1.0;
   wd->l = 0.0;
   wd->a = 255;

   _hsl_to_rgb(wd);
   _add_colorbar(obj);
   _sizing_eval(obj);

   return obj;
}

EAPI void
elm_colorselector_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   _set_color(obj, r, g, b, a);
}

EAPI void
elm_colorselector_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (r) *r = wd->r;
   if (g) *g = wd->g;
   if (b) *b = wd->b;
   if (a) *a = wd->a;
}

EAPI void
elm_colorselector_mode_set(Evas_Object *obj, Elm_Colorselector_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->mode == mode) return;
   wd->mode = mode;
   switch (wd->mode)
     {
        case ELM_COLORSELECTOR_PALETTE:
           if (edje_object_part_swallow_get(wd->base, "selector"))
             {
                edje_object_part_unswallow(wd->base, wd->sel);
                evas_object_hide(wd->sel);
             }
           if (!edje_object_part_swallow_get(wd->base, "palette"))
             {
                edje_object_part_swallow(wd->base, "palette", wd->box);
                evas_object_show(wd->box);
             }
           break;
        case ELM_COLORSELECTOR_COMPONENTS:
           if (edje_object_part_swallow_get(wd->base, "palette"))
             {
                edje_object_part_unswallow(wd->base, wd->box);
                evas_object_hide(wd->box);
             }
           if (!edje_object_part_swallow_get(wd->base, "selector"))
             {
                edje_object_part_swallow(wd->base, "selector", wd->sel);
                evas_object_show(wd->sel);
             }
           break;
        case ELM_COLORSELECTOR_BOTH:
           if (!edje_object_part_swallow_get(wd->base, "palette"))
             {
                edje_object_part_swallow(wd->base, "palette", wd->box);
                evas_object_show(wd->box);
             }
           if (!edje_object_part_swallow_get(wd->base, "selector"))
             {
                edje_object_part_swallow(wd->base, "selector", wd->sel);
                evas_object_show(wd->sel);
             }
           break;
        default:
           return;
     }
   _sizing_eval(obj);
}

EAPI Elm_Colorselector_Mode
elm_colorselector_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_COLORSELECTOR_BOTH;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_COLORSELECTOR_BOTH;
   return wd->mode;
}

EAPI void
elm_colorselector_palette_item_color_get(const Elm_Object_Item *it, int *r __UNUSED__, int *g __UNUSED__, int *b __UNUSED__, int*a __UNUSED__)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Color_Item *item;
   item = (Elm_Color_Item *) it;
   if (item)
     {
        if(r) *r = item->color->r;
        if(g) *g = item->color->g;
        if(b) *b = item->color->b;
        if(a) *a = item->color->a;
     }
}

EAPI void
elm_colorselector_palette_item_color_set(Elm_Object_Item *it, int r __UNUSED__, int g __UNUSED__, int b __UNUSED__, int a __UNUSED__)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Color_Item *item;
   item = (Elm_Color_Item *) it;
   item->color->r = r;
   item->color->g = g;
   item->color->b = b;
   item->color->a = a;
   evas_object_color_set(item->color_obj, item->color->r, item->color->g, item->color->b, item->color->a);
   _colors_save(WIDGET(it));
}

EAPI Elm_Object_Item *
elm_colorselector_palette_color_add(Evas_Object *obj, int r, int g, int b, int a)
{
   Elm_Color_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (wd->config_load)
     {
        _colors_remove(obj);
        wd->config_load = EINA_FALSE;
     }
   item = _item_new(obj);
   if (!item) return NULL;
   item->color = ELM_NEW(Elm_Color_RGBA);
   if (!item->color) return NULL;
   item->color->r = r;
   item->color->g = g;
   item->color->b = b;
   item->color->a = a;
   _elm_config_color_set(wd->palette_name, item->color->r, item->color->g,
                         item->color->b, item->color->a);
   elm_box_pack_end(wd->box, VIEW(item));
   evas_object_color_set(item->color_obj, item->color->r, item->color->g,
                         item->color->b, item->color->a);
   wd->items = eina_list_append(wd->items, item);
   _sizing_eval(obj);
   return (Elm_Object_Item *) item;
}

EAPI void
elm_colorselector_palette_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _colors_remove(obj);
}

EAPI void
elm_colorselector_palette_name_set(Evas_Object *obj, const char *palette_name)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!strcmp(wd->palette_name, palette_name)) return;
   if (palette_name)
     {
        _colors_remove(obj);
        eina_stringshare_replace(&wd->palette_name, palette_name);
        _colors_load_apply(obj);
     }
}

EAPI const char*
elm_colorselector_palette_name_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->palette_name;
}
