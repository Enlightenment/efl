#include <Elementary.h>
#include "elm_priv.h"

/**
 * @addtogroup Colorselector Colorselector
 *
 * By using colorselector, you can select a color.
 * Colorselector made a color using HSV/HSB mode.
 *
 * Signals that you can add callbacks for are:
 *
 * "changed" - when the color value changes
 */

#define BASE_STEP 360.0
#define HUE_STEP 360.0
#define SAT_STEP 128.0
#define LIG_STEP 256.0
#define ALP_STEP 256.0

typedef enum _Button_State
{
   BUTTON_RELEASED,
   L_BUTTON_PRESSED,
   R_BUTTON_PRESSED
} Button_State;

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
   Button_State button_state;
};

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data
{
   Evas_Object *base;
   Colorselector_Data *cp[4];
   Evas_Coord _x, _y, _w, _h;
   int r, g, b, a;
   int er, eg, eb;
   int sr, sg, sb;
   int lr, lg, lb;
   double h, s, l;
   Ecore_Timer *lp_timer;
   Ecore_Timer *mv_timer;
};

static const char *widtype = NULL;

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _rgb_to_hsl(void *data);
static void _hsl_to_rgb(void *data);
static void _color_with_saturation(void *data);
static void _color_with_lightness(void *data);
static void _draw_rects(void *data, double x);
static void _arrow_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _colorbar_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool _mv_timer(void *data);
static Eina_Bool _long_press_timer(void *data);
static void _left_button_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _right_button_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _left_button_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _right_button_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _add_colorbar(Evas_Object *obj);
static void _set_color(Evas_Object *obj, int r, int g, int b, int a);

static const char SIG_CHANGED[] = "changed";

static const Evas_Smart_Cb_Description _signals[] =
{
     {SIG_CHANGED, ""},
     {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int i = 0;

   if (!wd) return;
   if (wd->lp_timer) ecore_timer_del(wd->lp_timer);
   if (wd->mv_timer) ecore_timer_del(wd->mv_timer);
   for (i = 0; i < 4; i++) free(wd->cp[i]);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int i;

   if ((!wd) || (!wd->base)) return;

   _elm_theme_object_set(obj, wd->base, "colorselector", "bg",
                         elm_widget_style_get(obj));

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
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
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
   edje_object_size_min_restricted_calc(wd->base, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
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
         evas_object_color_set(wd->cp[3]->arrow, wd->er, wd->eg, wd->eb, wd->a);
         break;
      default:
         break;
     }
   _hsl_to_rgb(wd);
}

static void
_arrow_cb(void *data, Evas_Object *obj, const char *emission __UNUSED__, const char *source __UNUSED__)
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

static Eina_Bool
_mv_timer(void *data)
{
   Colorselector_Data *cp = data;
   Widget_Data *wd = elm_widget_data_get(cp->parent);
   double x, y;

   if (!wd) return EINA_FALSE;

   edje_object_part_drag_value_get(cp->colorbar, "elm.arrow", &x, &y);
   if (cp->button_state == L_BUTTON_PRESSED)
     {
        x -= 1.0 / BASE_STEP;
        if (x < 0.0) x = 0.0;
        edje_object_part_drag_value_set(cp->colorbar, "elm.arrow", x, y);
        _draw_rects(data, x);
        evas_object_smart_callback_call(cp->parent, SIG_CHANGED, NULL);
        return EINA_TRUE;
     }
   else if (cp->button_state == R_BUTTON_PRESSED)
     {
        x += 1.0 / BASE_STEP;
        if (x > 1.0) x = 1.0;
        edje_object_part_drag_value_set(cp->colorbar, "elm.arrow", x, y);
        _draw_rects(data, x);
        evas_object_smart_callback_call(cp->parent, SIG_CHANGED, NULL);
        return EINA_TRUE;
     }
   wd->mv_timer = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_long_press_timer(void *data)
{
   Colorselector_Data *cp = data;
   Widget_Data *wd = elm_widget_data_get(cp->parent);

   if (wd->mv_timer) ecore_timer_del(wd->mv_timer);
   wd->mv_timer = ecore_timer_add(0.01, _mv_timer, cp);

   wd->lp_timer = NULL;
   return EINA_FALSE;
}

static void
_left_button_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Colorselector_Data *cp = data;
   Widget_Data *wd = elm_widget_data_get(cp->parent);
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
   cp->button_state = L_BUTTON_PRESSED;
   if (wd->lp_timer) ecore_timer_del(wd->lp_timer);
   wd->lp_timer = ecore_timer_add(_elm_config->longpress_timeout, _long_press_timer, cp);
}

static void
_right_button_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Colorselector_Data *cp = data;
   Widget_Data *wd = elm_widget_data_get(cp->parent);
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
   cp->button_state = R_BUTTON_PRESSED;
   wd->lp_timer = ecore_timer_add(_elm_config->longpress_timeout, _long_press_timer, cp);
}

static void
_left_button_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Colorselector_Data *cp = data;
   Widget_Data *wd = elm_widget_data_get(cp->parent);

   if (wd->lp_timer)
     {
        ecore_timer_del(wd->lp_timer);
        wd->lp_timer = NULL;
     }
   if (wd->mv_timer)
     {
        ecore_timer_del(wd->mv_timer);
        wd->mv_timer = NULL;
     }

   cp->button_state = BUTTON_RELEASED;
   edje_object_signal_emit(cp->lbt, "elm,state,left,button,up", "left_button");
}

static void
_right_button_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Colorselector_Data *cp = data;
   Widget_Data *wd = elm_widget_data_get(cp->parent);

   if (wd->lp_timer)
     {
        ecore_timer_del(wd->lp_timer);
        wd->lp_timer = NULL;
     }
   if (wd->mv_timer)
     {
        ecore_timer_del(wd->mv_timer);
        wd->mv_timer = NULL;
     }

   cp->button_state = BUTTON_RELEASED;
   edje_object_signal_emit(cp->rbt, "elm,state,right,button,up",
                           "right_button");
}

static void
_add_colorbar(Evas_Object *obj)
{
   char colorbar_name[128];
   char colorbar_s[128];
   Widget_Data *wd;
   Evas *e;
   int i = 0;

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
                              "default");
        snprintf(colorbar_name, sizeof(colorbar_name), "colorbar_%d", i);
        snprintf(colorbar_s, sizeof(colorbar_s), "elm.colorbar_%d", i);
        edje_object_signal_callback_add(wd->cp[i]->colorbar, "drag", "*",
                                        _arrow_cb, wd->cp[i]);
        edje_object_part_swallow(wd->base, colorbar_s, wd->cp[i]->colorbar);
        elm_widget_sub_object_add(obj, wd->cp[i]->colorbar);

        /* load colorbar image */
        wd->cp[i]->bar = edje_object_add(e);
        _elm_theme_object_set(obj, wd->cp[i]->bar, "colorselector", "image",
                              colorbar_name);
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
             _elm_theme_object_set(obj, wd->cp[i]->bg_rect, "colorselector",
                                   "bg_image", colorbar_name);
             edje_object_part_swallow(wd->cp[i]->colorbar, "elm.bar_bg",
                                      wd->cp[i]->bg_rect);
             elm_widget_sub_object_add(obj, wd->cp[i]->bg_rect);
             evas_object_color_set(wd->cp[i]->bar, wd->er, wd->eg, wd->eb, 255);
          }
        /* load arrow image, pointing the colorbar */
        wd->cp[i]->arrow = edje_object_add(e);
        _elm_theme_object_set(obj, wd->cp[i]->arrow, "colorselector", "image",
                              "updown");
        edje_object_part_swallow(wd->cp[i]->colorbar, "elm.arrow_icon",
                                 wd->cp[i]->arrow);
        elm_widget_sub_object_add(obj, wd->cp[i]->arrow);
        if (i == 2)
          evas_object_color_set(wd->cp[i]->arrow, 0, 0, 0, 255);
        else
          evas_object_color_set(wd->cp[i]->arrow, wd->er, wd->eg, wd->eb, 255);

        /* load left button */
        wd->cp[i]->lbt = edje_object_add(e);
        _elm_theme_object_set(obj, wd->cp[i]->lbt, "colorselector", "button",
                              "left");
        evas_object_event_callback_add(wd->cp[i]->lbt, EVAS_CALLBACK_MOUSE_DOWN,
                                       _left_button_down_cb, wd->cp[i]);
        evas_object_event_callback_add(wd->cp[i]->lbt, EVAS_CALLBACK_MOUSE_UP,
                                       _left_button_up_cb, wd->cp[i]);
        edje_object_part_swallow(wd->cp[i]->colorbar, "elm.l_button",
                                 wd->cp[i]->lbt);
        elm_widget_sub_object_add(obj, wd->cp[i]->lbt);

        /* load right button */
        wd->cp[i]->rbt = edje_object_add(e);
        _elm_theme_object_set(obj, wd->cp[i]->rbt, "colorselector", "button",
                              "right");
        evas_object_event_callback_add(wd->cp[i]->rbt, EVAS_CALLBACK_MOUSE_DOWN,
                                       _right_button_down_cb, wd->cp[i]);
        evas_object_event_callback_add(wd->cp[i]->rbt, EVAS_CALLBACK_MOUSE_UP,
                                       _right_button_up_cb, wd->cp[i]);
        edje_object_part_swallow(wd->cp[i]->colorbar, "elm.r_button",
                                 wd->cp[i]->rbt);
        elm_widget_sub_object_add(obj, wd->cp[i]->rbt);
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

/**
 * Add a new colorselector to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Colorselector
 */
EAPI Evas_Object *
elm_colorselector_add(Evas_Object *parent)
{
   Evas_Object *obj = NULL;
   Widget_Data *wd = NULL;
   Evas *e;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "colorselector");
   elm_widget_type_set(obj, "colorselector");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   /* load background edj */
   wd->base = edje_object_add(e);
   _elm_theme_object_set(obj, wd->base, "colorselector", "bg", "default");
   elm_widget_resize_object_set(obj, wd->base);

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

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * Set a color for the colorselector
 *
 * @param obj   Colorselector object
 * @param r     r-value of color
 * @param g     g-value of color
 * @param b     b-value of color
 * @param a     a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void
elm_colorselector_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   _set_color(obj, r, g, b, a);
}

/**
 * Get a color from the colorselector
 *
 * @param obj   Colorselector object
 * @param r     integer pointer for r-value of color
 * @param g     integer pointer for g-value of color
 * @param b     integer pointer for b-value of color
 * @param a     integer pointer for a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void
elm_colorselector_color_get(const Evas_Object *obj, int *r, int *g, int *b, int*a)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   ELM_CHECK_WIDTYPE(obj, widtype);

   if (r) *r = wd->r;
   if (g) *g = wd->g;
   if (b) *b = wd->b;
   if (a) *a = wd->a;
}
