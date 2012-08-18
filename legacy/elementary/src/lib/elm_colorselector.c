#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_colorselector.h"

EAPI const char ELM_COLORSELECTOR_SMART_NAME[] = "elm_colorselector";

#define BASE_STEP       360.0
#define HUE_STEP        360.0
#define SAT_STEP        128.0
#define LIG_STEP        256.0
#define ALP_STEP        256.0
#define DEFAULT_HOR_PAD 10
#define DEFAULT_VER_PAD 10

static const char SIG_CHANGED[] = "changed";
static const char SIG_COLOR_ITEM_SELECTED[] = "color,item,selected";
static const char SIG_COLOR_ITEM_LONGPRESSED[] = "color,item,longpressed";
static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_COLOR_ITEM_SELECTED, ""},
   {SIG_COLOR_ITEM_LONGPRESSED, ""},
   {SIG_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_COLORSELECTOR_SMART_NAME, _elm_colorselector,
  Elm_Colorselector_Smart_Class, Elm_Layout_Smart_Class,
  elm_layout_smart_class_get, _smart_callbacks);

static void
_items_del(Elm_Colorselector_Smart_Data *sd)
{
   Elm_Color_Item *item;

   if (!sd->items) return;

   EINA_LIST_FREE (sd->items, item)
     {
        free(item->color);
        elm_widget_item_free(item);
     }

   sd->items = NULL;
   sd->selected = NULL;
}

static void
_color_with_saturation(Elm_Colorselector_Smart_Data *sd)
{
   if (sd->er > 127)
     sd->sr = 127 + (int)((double)(sd->er - 127) * sd->s);
   else
     sd->sr = 127 - (int)((double)(127 - sd->er) * sd->s);

   if (sd->eg > 127)
     sd->sg = 127 + (int)((double)(sd->eg - 127) * sd->s);
   else
     sd->sg = 127 - (int)((double)(127 - sd->eg) * sd->s);

   if (sd->eb > 127)
     sd->sb = 127 + (int)((double)(sd->eb - 127) * sd->s);
   else
     sd->sb = 127 - (int)((double)(127 - sd->eb) * sd->s);
}

static void
_color_with_lightness(Elm_Colorselector_Smart_Data *sd)
{
   if (sd->l > 0.5)
     {
        sd->lr = sd->er + (int)((double)(255 - sd->er) * (sd->l - 0.5) * 2.0);
        sd->lg = sd->eg + (int)((double)(255 - sd->eg) * (sd->l - 0.5) * 2.0);
        sd->lb = sd->eb + (int)((double)(255 - sd->eb) * (sd->l - 0.5) * 2.0);
     }
   else if (sd->l < 0.5)
     {
        sd->lr = (double)sd->er * sd->l * 2.0;
        sd->lg = (double)sd->eg * sd->l * 2.0;
        sd->lb = (double)sd->eb * sd->l * 2.0;
     }
   else
     {
        sd->lr = sd->er;
        sd->lg = sd->eg;
        sd->lb = sd->eb;
     }
}

static void
_hsl_to_rgb(Elm_Colorselector_Smart_Data *sd)
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
   sd->r = (f <= 0.5) ? i : (i + 1);

   i = (int)(g * 255.0);
   f = (g * 255.0) - i;
   sd->g = (f <= 0.5) ? i : (i + 1);

   i = (int)(b * 255.0);
   f = (b * 255.0) - i;
   sd->b = (f <= 0.5) ? i : (i + 1);
}

static void
_rectangles_redraw(Color_Bar_Data *cb_data, double x)
{
   double one_six = 1.0 / 6.0;

   ELM_COLORSELECTOR_DATA_GET(cb_data->parent, sd);

   switch (cb_data->color_type)
     {
      case HUE:
        sd->h = 360.0 * x;

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

        evas_object_color_set
          (sd->cb_data[0]->arrow, sd->er, sd->eg, sd->eb, 255);
        evas_object_color_set
          (sd->cb_data[1]->bg_rect, sd->er, sd->eg, sd->eb, 255);
        evas_object_color_set
          (sd->cb_data[2]->bg_rect, sd->er, sd->eg, sd->eb, 255);
        evas_object_color_set
          (sd->cb_data[3]->bar, sd->er, sd->eg, sd->eb, 255);

        _color_with_saturation(sd);
        evas_object_color_set
          (sd->cb_data[1]->arrow, sd->sr, sd->sg, sd->sb, 255);

        _color_with_lightness(sd);
        evas_object_color_set
          (sd->cb_data[2]->arrow, sd->lr, sd->lg, sd->lb, 255);

        evas_object_color_set(sd->cb_data[3]->arrow,
                              (sd->er * sd->a) / 255,
                              (sd->eg * sd->a) / 255,
                              (sd->eb * sd->a) / 255,
                              sd->a);
        break;

      case SATURATION:
        sd->s = 1.0 - x;
        _color_with_saturation(sd);
        evas_object_color_set
          (sd->cb_data[1]->arrow, sd->sr, sd->sg, sd->sb, 255);
        break;

      case LIGHTNESS:
        sd->l = x;
        _color_with_lightness(sd);
        evas_object_color_set
          (sd->cb_data[2]->arrow, sd->lr, sd->lg, sd->lb, 255);
        break;

      case ALPHA:
        sd->a = 255.0 * x;
        evas_object_color_set(sd->cb_data[3]->arrow,
                              (sd->er * sd->a) / 255,
                              (sd->eg * sd->a) / 255,
                              (sd->eb * sd->a) / 255,
                              sd->a);
        break;

      default:
        break;
     }

   _hsl_to_rgb(sd);
}

static void
_arrow_cb(void *data,
          Evas_Object *obj,
          const char *emission __UNUSED__,
          const char *source __UNUSED__)
{
   Color_Bar_Data *cb_data = data;
   double x, y;

   edje_object_part_drag_value_get(obj, "elm.arrow", &x, &y);

   _rectangles_redraw(data, x);
   evas_object_smart_callback_call(cb_data->parent, SIG_CHANGED, NULL);
}

static void
_colorbar_cb(void *data,
             Evas *e,
             Evas_Object *obj __UNUSED__,
             void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Color_Bar_Data *cb_data = data;
   double arrow_x = 0, arrow_y;
   Evas_Coord x, y, w, h;
   ELM_COLORSELECTOR_DATA_GET(cb_data->parent, sd);

   evas_object_geometry_get(cb_data->bar, &x, &y, &w, &h);
   edje_object_part_drag_value_get
     (cb_data->colorbar, "elm.arrow", &arrow_x, &arrow_y);

   if (w > 0) arrow_x = (double)(ev->canvas.x - x) / (double)w;
   if (arrow_x > 1) arrow_x = 1;
   if (arrow_x < 0) arrow_x = 0;
   edje_object_part_drag_value_set
     (cb_data->colorbar, "elm.arrow", arrow_x, arrow_y);

   _rectangles_redraw(data, arrow_x);
   evas_object_smart_callback_call(cb_data->parent, SIG_CHANGED, NULL);
   evas_event_feed_mouse_cancel(e, 0, NULL);
   evas_event_feed_mouse_down(e, 1, EVAS_BUTTON_NONE, 0, NULL);
   sd->sel_color_type = cb_data->color_type;
   sd->focused = ELM_COLORSELECTOR_COMPONENTS;
}

static void
_button_clicked_cb(void *data,
                   Evas_Object *obj,
                   void *event_info __UNUSED__)
{
   Eina_Bool is_right = EINA_FALSE;
   Color_Bar_Data *cb_data = data;
   double x, y, step;
   ELM_COLORSELECTOR_DATA_GET(cb_data->parent, sd);

   if (obj == cb_data->rbt)
     {
        is_right = EINA_TRUE;
        step = 1.0;
     }
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

   if (is_right)
     {
        if (x > 1.0) x = 1.0;
     }
   else
     {
        if (x < 0.0) x = 0.0;
     }

   edje_object_part_drag_value_set(cb_data->colorbar, "elm.arrow", x, y);
   _rectangles_redraw(data, x);
   evas_object_smart_callback_call(cb_data->parent, SIG_CHANGED, NULL);
   sd->sel_color_type = cb_data->color_type;
   sd->focused = ELM_COLORSELECTOR_COMPONENTS;
}

static void
_button_repeat_cb(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Eina_Bool is_right = EINA_FALSE;
   Color_Bar_Data *cb_data = data;
   double x, y, step;

   if (obj == cb_data->rbt)
     {
        is_right = EINA_TRUE;
        step = 1.0;
     }
   else step = -1.0;

   edje_object_part_drag_value_get(cb_data->colorbar, "elm.arrow", &x, &y);
   x += step / BASE_STEP;

   if (is_right)
     {
        if (x > 1.0) x = 1.0;
     }
   else
     {
        if (x < 0.0) x = 0.0;
     }

   edje_object_part_drag_value_set(cb_data->colorbar, "elm.arrow", x, y);
   _rectangles_redraw(data, x);
   evas_object_smart_callback_call(cb_data->parent, SIG_CHANGED, NULL);
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
   ai = _elm_access_object_get(ao);

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
        sd->cb_data[i] = ELM_NEW(Color_Bar_Data);
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

           default:
             break;
          }

        /* load colorbar area */
        sd->cb_data[i]->colorbar = edje_object_add(e);
        elm_widget_theme_object_set
          (obj, sd->cb_data[i]->colorbar, "colorselector", "base",
          elm_widget_style_get(obj));
        snprintf(colorbar_name, sizeof(colorbar_name), "colorbar_%d", i);
        snprintf(colorbar_s, sizeof(colorbar_s), "elm.colorbar_%d", i);
        edje_object_signal_callback_add
          (sd->cb_data[i]->colorbar, "drag", "*", _arrow_cb, sd->cb_data[i]);
        edje_object_part_swallow
          (sd->col_bars_area, colorbar_s, sd->cb_data[i]->colorbar);
        elm_widget_sub_object_add(obj, sd->cb_data[i]->colorbar);

        /* load colorbar image */
        sd->cb_data[i]->bar = edje_object_add(e);
        snprintf(buf, sizeof(buf), "%s/%s", colorbar_name,
                 elm_widget_style_get(obj));
        elm_widget_theme_object_set
          (obj, sd->cb_data[i]->bar, "colorselector", "image", buf);
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.bar", sd->cb_data[i]->bar);
        elm_widget_sub_object_add(obj, sd->cb_data[i]->bar);

        /* provide expanded touch area */
        sd->cb_data[i]->touch_area = evas_object_rectangle_add(e);
        evas_object_color_set(sd->cb_data[i]->touch_area, 0, 0, 0, 0);
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.arrow_bg",
          sd->cb_data[i]->touch_area);
        evas_object_event_callback_add
          (sd->cb_data[i]->touch_area, EVAS_CALLBACK_MOUSE_DOWN, _colorbar_cb,
          sd->cb_data[i]);
        elm_widget_sub_object_add(obj, sd->cb_data[i]->touch_area);

        // ACCESS
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
           _access_colorbar_register(obj, sd->cb_data[i], "elm.arrow_bg_access");

        /* load background rectangle of the colorbar. used for
           changing color of the opacity bar */
        if ((i == 1) || (i == 2))
          {
             sd->cb_data[i]->bg_rect = evas_object_rectangle_add(e);
             evas_object_color_set
               (sd->cb_data[i]->bg_rect, sd->er, sd->eg, sd->eb, 255);
             edje_object_part_swallow
               (sd->cb_data[i]->colorbar, "elm.bar_bg",
               sd->cb_data[i]->bg_rect);

             elm_widget_sub_object_add(obj, sd->cb_data[i]->bg_rect);
          }
        if (i == 3)
          {
             sd->cb_data[i]->bg_rect = edje_object_add(e);
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
        sd->cb_data[i]->arrow = edje_object_add(e);
        elm_widget_theme_object_set
          (obj, sd->cb_data[i]->arrow, "colorselector", "arrow",
          elm_widget_style_get(obj));
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.arrow_icon",
          sd->cb_data[i]->arrow);
        elm_widget_sub_object_add(obj, sd->cb_data[i]->arrow);

        if (i == 2)
          evas_object_color_set(sd->cb_data[i]->arrow, 0, 0, 0, 255);
        else
          evas_object_color_set
            (sd->cb_data[i]->arrow, sd->er, sd->eg, sd->eb, 255);

        /* load left button */
        sd->cb_data[i]->lbt = elm_button_add(obj);
        snprintf(buf, sizeof(buf), "colorselector/left/%s",
                 elm_widget_style_get(obj));
        elm_object_style_set(sd->cb_data[i]->lbt, buf);
        elm_widget_sub_object_add(obj, sd->cb_data[i]->lbt);
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.l_button", sd->cb_data[i]->lbt);
        evas_object_smart_callback_add
          (sd->cb_data[i]->lbt, "clicked", _button_clicked_cb,
          sd->cb_data[i]);
        elm_button_autorepeat_set(sd->cb_data[i]->lbt, EINA_TRUE);
        elm_button_autorepeat_initial_timeout_set
          (sd->cb_data[i]->lbt, _elm_config->longpress_timeout);
        elm_button_autorepeat_gap_timeout_set
          (sd->cb_data[i]->lbt, (1.0 / _elm_config->fps));
        evas_object_smart_callback_add
          (sd->cb_data[i]->lbt, "repeated", _button_repeat_cb,
          sd->cb_data[i]);

        /* load right button */
        sd->cb_data[i]->rbt = elm_button_add(obj);
        snprintf(buf, sizeof(buf), "colorselector/right/%s",
                 elm_widget_style_get(obj));
        elm_object_style_set(sd->cb_data[i]->rbt, buf);
        elm_widget_sub_object_add(obj, sd->cb_data[i]->rbt);
        edje_object_part_swallow
          (sd->cb_data[i]->colorbar, "elm.r_button", sd->cb_data[i]->rbt);
        evas_object_smart_callback_add
          (sd->cb_data[i]->rbt, "clicked", _button_clicked_cb,
          sd->cb_data[i]);
        elm_button_autorepeat_set(sd->cb_data[i]->rbt, EINA_TRUE);
        elm_button_autorepeat_initial_timeout_set
          (sd->cb_data[i]->rbt, _elm_config->longpress_timeout);
        elm_button_autorepeat_gap_timeout_set
          (sd->cb_data[i]->rbt, (1.0 / _elm_config->fps));
        evas_object_smart_callback_add
          (sd->cb_data[i]->rbt, "repeated", _button_repeat_cb,
          sd->cb_data[i]);
     }
}

static Eina_Bool
_elm_colorselector_smart_theme(Evas_Object *obj)
{
   int i;
   Eina_List *elist;
   Elm_Color_Item *item;
   const char *hpadstr, *vpadstr;
   unsigned int h_pad = DEFAULT_HOR_PAD;
   unsigned int v_pad = DEFAULT_VER_PAD;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_colorselector_parent_sc)->theme(obj))
     return EINA_FALSE;

   if (!sd->col_bars_area) return EINA_FALSE;

   elm_widget_theme_object_set
     (obj, sd->col_bars_area, "colorselector", "bg",
     elm_widget_style_get(obj));

   hpadstr = edje_object_data_get
       (ELM_WIDGET_DATA(sd)->resize_obj, "horizontal_pad");
   if (hpadstr) h_pad = atoi(hpadstr);
   vpadstr = edje_object_data_get
       (ELM_WIDGET_DATA(sd)->resize_obj, "vertical_pad");
   if (vpadstr) v_pad = atoi(vpadstr);

   elm_box_padding_set
     (sd->palette_box,
     (h_pad * elm_widget_scale_get(obj) * elm_config_scale_get()),
     (v_pad * elm_widget_scale_get(obj) * elm_config_scale_get()));

   EINA_LIST_FOREACH (sd->items, elist, item)
     {
        elm_layout_theme_set
          (VIEW(item), "colorselector", "item", elm_widget_style_get(obj));
        elm_widget_theme_object_set
          (obj, item->color_obj, "colorselector", "item/color",
          elm_widget_style_get(obj));
     }
   for (i = 0; i < 4; i++)
     {
        if (sd->cb_data[i])
          {
             evas_object_del(sd->cb_data[i]->colorbar);
             sd->cb_data[i]->colorbar = NULL;
             evas_object_del(sd->cb_data[i]->bar);
             sd->cb_data[i]->bar = NULL;
             evas_object_del(sd->cb_data[i]->lbt);
             sd->cb_data[i]->lbt = NULL;
             evas_object_del(sd->cb_data[i]->rbt);
             sd->cb_data[i]->rbt = NULL;
             if (i != 0)
               {
                  evas_object_del(sd->cb_data[i]->bg_rect);
                  sd->cb_data[i]->bg_rect = NULL;
               }
             evas_object_del(sd->cb_data[i]->arrow);
             sd->cb_data[i]->arrow = NULL;
             evas_object_del(sd->cb_data[i]->touch_area);
             sd->cb_data[i]->touch_area = NULL;
          }
     }

   _color_bars_add(obj);
   elm_colorselector_color_set(obj, sd->r, sd->g, sd->b, sd->a);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_sub_obj_size_hints_set(Evas_Object *sobj,
                        int timesw,
                        int timesh)
{
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(timesw, &minw, timesh, &minh);
   edje_object_size_min_restricted_calc(sobj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(sobj, minw, minh);
   evas_object_size_hint_max_set(sobj, -1, -1);
}

static void
_item_sizing_eval(Elm_Color_Item *item)
{
   Evas_Coord minw = -1, minh = -1;

   if (!item) return;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(VIEW(item), &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(VIEW(item), minw, minh);
}

/* fix size hints of color palette items, so that the box gets it */
static void
_palette_sizing_eval(Evas_Object *obj)
{
   Eina_List *elist;
   Elm_Color_Item *item;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->items, elist, item)
     _item_sizing_eval(item);
}

static void
_component_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;
   int i;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

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
     (sd->col_bars_area, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(sd->col_bars_area, minw, minh);
}

static void
_full_sizing_eval(Evas_Object *obj)
{
   _palette_sizing_eval(obj);
   _component_sizing_eval(obj);
}

static void
_elm_colorselector_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);

   switch (sd->mode)
     {
      case ELM_COLORSELECTOR_PALETTE:
        _palette_sizing_eval(obj);
        break;

      case ELM_COLORSELECTOR_COMPONENTS:
        _component_sizing_eval(obj);
        break;

      case ELM_COLORSELECTOR_BOTH:
        _full_sizing_eval(obj);
        break;

      default:
        return;
     }

   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static Eina_Bool
_on_color_long_press(void *data)
{
   Elm_Color_Item *item = (Elm_Color_Item *)data;

   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   sd->longpress_timer = NULL;
   sd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call
     (WIDGET(item), SIG_COLOR_ITEM_LONGPRESSED, item);

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_color_pressed(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Elm_Color_Item *item = (Elm_Color_Item *)data;
   Evas_Event_Mouse_Down *ev = event_info;

   if (!item) return;

   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   if (ev->button != 1) return;
   elm_object_signal_emit(VIEW(item), "elm,state,selected", "elm");
   sd->longpressed = EINA_FALSE;

   if (sd->longpress_timer) ecore_timer_del(sd->longpress_timer);
   sd->longpress_timer = ecore_timer_add
       (_elm_config->longpress_timeout, _on_color_long_press, data);
}

static void
_on_color_released(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   Elm_Color_Item *item = (Elm_Color_Item *)data;
   Eina_List *l;
   Elm_Color_Item *temp_item;
   Evas_Event_Mouse_Down *ev = event_info;
   if (!item) return;

   ELM_COLORSELECTOR_DATA_GET(WIDGET(item), sd);

   if (ev->button != 1) return;
   if (sd->longpress_timer)
     {
        ecore_timer_del(sd->longpress_timer);
        sd->longpress_timer = NULL;
     }
   elm_object_signal_emit(VIEW(item), "elm,state,unselected", "elm");
   if (!sd->longpressed)
     {
        evas_object_smart_callback_call
          (WIDGET(item), SIG_COLOR_ITEM_SELECTED, item);
        elm_colorselector_color_set
          (WIDGET(item), item->color->r, item->color->g, item->color->b,
          item->color->a);
     }
   EINA_LIST_FOREACH (sd->items, l, temp_item)
     if (item == temp_item) sd->selected = l;
   sd->focused = ELM_COLORSELECTOR_PALETTE;
}

static char *
_access_info_cb(void *data,
                Evas_Object *obj __UNUSED__,
                Elm_Widget_Item *item __UNUSED__)
{
   char *ret;
   Eina_Strbuf *buf;
   buf = eina_strbuf_new();
   int r = 0, g = 0, b = 0 ,a = 0;

   Elm_Color_Item *it = data;
   ELM_COLORSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);

   elm_colorselector_palette_item_color_get((Elm_Object_Item *)it, &r, &g, &b, &a);

   eina_strbuf_append_printf(buf, "red %d, green %d, blue %d, alpha %d", r, g, b, a);
   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_access_widget_item_register(Elm_Color_Item *it)
{
   Elm_Access_Info *ai;

   _elm_access_widget_item_register((Elm_Widget_Item *)it);

   ai = _elm_access_object_get(it->base.access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("color selector palette item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
}

static Elm_Color_Item *
_item_new(Evas_Object *obj)
{
   Elm_Color_Item *item;

   item = elm_widget_item_new(obj, Elm_Color_Item);
   if (!item) return NULL;

   VIEW(item) = elm_layout_add(obj);
   elm_layout_theme_set
     (VIEW(item), "colorselector", "item", elm_widget_style_get(obj));
   evas_object_size_hint_weight_set
     (VIEW(item), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);

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
     (item->color_obj, EVAS_CALLBACK_MOUSE_UP, _on_color_released, item);
   elm_object_part_content_set(VIEW(item), "color_obj", item->color_obj);

   _item_sizing_eval(item);
   evas_object_show(VIEW(item));

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_widget_item_register(item);

   return item;
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
   Elm_Color_Item *item;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   _elm_config_colors_free(sd->palette_name);
   EINA_LIST_FOREACH (sd->items, elist, item)
     _elm_config_color_set(sd->palette_name, item->color->r, item->color->g,
                           item->color->b, item->color->a);
}

static void
_palette_colors_load(Evas_Object *obj)
{
   Eina_List *elist;
   Elm_Color_Item *item;
   Eina_List *color_list;
   Elm_Color_RGBA *color;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   color_list = _elm_config_color_list_get(sd->palette_name);
   if (!color_list) return;

   EINA_LIST_FOREACH (color_list, elist, color)
     {
        item = _item_new(obj);
        if (!item) return;

        item->color = ELM_NEW(Elm_Color_RGBA);
        if (!item->color) return;
        item->color->r = color->r;
        item->color->g = color->g;
        item->color->b = color->b;
        item->color->a = color->a;

        elm_box_pack_end(sd->palette_box, VIEW(item));
        evas_object_color_set
          (item->color_obj, item->color->r, item->color->g, item->color->b,
          item->color->a);

        sd->items = eina_list_append(sd->items, item);
     }

   sd->config_load = EINA_TRUE;
}

static void
_rgb_to_hsl(Elm_Colorselector_Smart_Data *sd)
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

static void
_colors_set(Evas_Object *obj,
            int r,
            int g,
            int b,
            int a)
{
   double x, y;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   sd->r = r;
   sd->g = g;
   sd->b = b;
   sd->a = a;

   _rgb_to_hsl(sd);

   edje_object_part_drag_value_get
     (sd->cb_data[0]->colorbar, "elm.arrow", &x, &y);
   x = sd->h / 360.0;
   edje_object_part_drag_value_set
     (sd->cb_data[0]->colorbar, "elm.arrow", x, y);
   _rectangles_redraw(sd->cb_data[0], x);

   edje_object_part_drag_value_get
     (sd->cb_data[1]->colorbar, "elm.arrow", &x, &y);
   x = 1.0 - sd->s;
   edje_object_part_drag_value_set
     (sd->cb_data[1]->colorbar, "elm.arrow", x, y);
   _rectangles_redraw(sd->cb_data[1], x);

   edje_object_part_drag_value_get
     (sd->cb_data[2]->colorbar, "elm.arrow", &x, &y);
   x = sd->l;
   edje_object_part_drag_value_set(sd->cb_data[2]->colorbar, "elm.arrow", x, y);
   _rectangles_redraw(sd->cb_data[2], x);

   edje_object_part_drag_value_get
     (sd->cb_data[3]->colorbar, "elm.arrow", &x, &y);
   x = sd->a / 255.0;
   edje_object_part_drag_value_set
     (sd->cb_data[3]->colorbar, "elm.arrow", x, y);

   _rectangles_redraw(sd->cb_data[3], x);
}

static void
_elm_colorselector_smart_add(Evas_Object *obj)
{
   const char *hpadstr, *vpadstr;
   unsigned int h_pad = DEFAULT_HOR_PAD;
   unsigned int v_pad = DEFAULT_VER_PAD;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Colorselector_Smart_Data);

   ELM_WIDGET_CLASS(_elm_colorselector_parent_sc)->base.add(obj);

   elm_layout_theme_set
     (obj, "colorselector", "palette", elm_object_style_get(obj));

   priv->palette_box = elm_box_add(obj);
   elm_box_layout_set
     (priv->palette_box, evas_object_box_layout_flow_horizontal, NULL, NULL);
   elm_box_horizontal_set(priv->palette_box, EINA_TRUE);
   evas_object_size_hint_weight_set
     (priv->palette_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
     (priv->palette_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_homogeneous_set(priv->palette_box, EINA_TRUE);

   hpadstr =
     edje_object_data_get(ELM_WIDGET_DATA(priv)->resize_obj, "horizontal_pad");
   if (hpadstr) h_pad = atoi(hpadstr);
   vpadstr = edje_object_data_get
       (ELM_WIDGET_DATA(priv)->resize_obj, "vertical_pad");
   if (vpadstr) v_pad = atoi(vpadstr);

   elm_box_padding_set
     (priv->palette_box,
     (h_pad * elm_widget_scale_get(obj) * elm_config_scale_get()),
     (v_pad * elm_widget_scale_get(obj) * elm_config_scale_get()));

   elm_box_align_set(priv->palette_box, 0.5, 0.5);
   elm_layout_content_set(obj, "palette", priv->palette_box);
   priv->palette_name = eina_stringshare_add("default");
   _palette_colors_load(obj);

   /* load background edj */
   priv->col_bars_area = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set
     (obj, priv->col_bars_area, "colorselector", "bg",
     elm_widget_style_get(obj));
   elm_layout_content_set(obj, "selector", priv->col_bars_area);

   priv->mode = ELM_COLORSELECTOR_BOTH;
   priv->focused = ELM_COLORSELECTOR_PALETTE;
   priv->sel_color_type = HUE;
   priv->selected = priv->items;
   priv->er = 255;
   priv->eg = 0;
   priv->eb = 0;
   priv->h = 0.0;
   priv->s = 1.0;
   priv->l = 0.0;
   priv->a = 255;

   _hsl_to_rgb(priv);
   _color_bars_add(obj);

   elm_layout_sizing_eval(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_colorselector_smart_del(Evas_Object *obj)
{
   int i = 0;
   void *tmp[4];

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (sd->longpress_timer) ecore_timer_del(sd->longpress_timer);
   if (sd->palette_name) eina_stringshare_del(sd->palette_name);

   _items_del(sd);
   /* This cb_data are used during the destruction process of base.del */
   for (i = 0; i < 4; i++)
     tmp[i] = sd->cb_data[i];
   ELM_WIDGET_CLASS(_elm_colorselector_parent_sc)->base.del(obj);
   for (i = 0; i < 4; i++)
     free(tmp[i]);
}

static Eina_Bool
_elm_colorselector_smart_event(Evas_Object *obj,
                               Evas_Object *src __UNUSED__,
                               Evas_Callback_Type type,
                               void *event_info)
{
   Eina_List *cl = NULL;
   Elm_Color_Item *item = NULL;
   char colorbar_s[128];
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (!sd) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (!sd->selected) sd->selected = sd->items;

   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        if (sd->focused == ELM_COLORSELECTOR_PALETTE && sd->selected)
          cl = eina_list_prev(sd->selected);
        else if (sd->focused == ELM_COLORSELECTOR_COMPONENTS)
          _button_clicked_cb(sd->cb_data[sd->sel_color_type], sd->cb_data[sd->sel_color_type]->lbt, NULL);
        else return EINA_FALSE;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        if (sd->focused == ELM_COLORSELECTOR_PALETTE && sd->selected)
          cl = eina_list_next(sd->selected);
        else if (sd->focused == ELM_COLORSELECTOR_COMPONENTS)
          _button_clicked_cb(sd->cb_data[sd->sel_color_type], sd->cb_data[sd->sel_color_type]->rbt, NULL);
        else return EINA_FALSE;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
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
          return EINA_FALSE;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        if (sd->focused == ELM_COLORSELECTOR_PALETTE)
          {
             if (sd->mode == ELM_COLORSELECTOR_BOTH)
               {
                  sd->focused = ELM_COLORSELECTOR_COMPONENTS;
                  /*when focus is shifted to component start from
                   * first color type*/
                  sd->sel_color_type = HUE;
               }
             else return EINA_FALSE;
          }
        else if (sd->focused == ELM_COLORSELECTOR_COMPONENTS)
          {
             snprintf(colorbar_s, sizeof(colorbar_s), "elm.colorbar_%d",
                      (sd->sel_color_type + 1));
             /*Append color type only if next color bar is available*/
             if (edje_object_part_swallow_get(sd->col_bars_area, colorbar_s))
               sd->sel_color_type = sd->sel_color_type + 1;
             else return EINA_FALSE;
          }
     }
   else return EINA_FALSE;
   if (cl)
     {
        item = eina_list_data_get(cl);
        elm_object_signal_emit(VIEW(item), "elm,anim,activate", "elm");
        evas_object_smart_callback_call
          (WIDGET(item), SIG_COLOR_ITEM_SELECTED, item);
        elm_colorselector_color_set
          (WIDGET(item), item->color->r, item->color->g, item->color->b,
          item->color->a);
        sd->selected = cl;
     }
   else if (!cl && sd->focused == ELM_COLORSELECTOR_PALETTE)
     return EINA_FALSE;
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static Eina_Bool
_elm_colorselector_smart_focus_next(const Evas_Object *obj,
                                    Elm_Focus_Direction dir,
                                    Evas_Object **next)
{
   Eina_List *items = NULL;
   Eina_List *l;
   Elm_Widget_Item *item;
   int i = 0;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);
   if (!sd) return EINA_FALSE;

   if (!sd->items) return EINA_FALSE;

   EINA_LIST_FOREACH(sd->items, l, item)
     items = eina_list_append(items, item->access_obj);

   for (i = 0; i < 4; i++)
     {
        items = eina_list_append(items, sd->cb_data[i]->lbt);
        items = eina_list_append(items, sd->cb_data[i]->access_obj);
        items = eina_list_append(items, sd->cb_data[i]->rbt);
     }

   return elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   Eina_List *l;
   Elm_Color_Item *it;
   int i = 0;

   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (is_access)
     {
        EINA_LIST_FOREACH(sd->items, l, it)
          _access_widget_item_register(it);

        for (i = 0; i < 4; i++)
          _access_colorbar_register(obj, sd->cb_data[i],
                                    "elm.arrow_bg_access");
     }
   else
     {
        EINA_LIST_FOREACH(sd->items, l, it)
          _elm_access_widget_item_unregister((Elm_Widget_Item *)it);

        //TODO: _elm_access_edje_object_part_object_unregister() ?
     }
}

static void
_access_hook(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_COLORSELECTOR_CHECK(obj);
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (is_access)
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next =
       _elm_colorselector_smart_focus_next;
   else
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next = NULL;

   _access_obj_process(obj, is_access);
}

static void
_elm_colorselector_smart_set_user(Elm_Colorselector_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_colorselector_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_colorselector_smart_del;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_WIDGET_CLASS(sc)->theme = _elm_colorselector_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_colorselector_smart_event;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_colorselector_smart_sizing_eval;

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
      ELM_WIDGET_CLASS(sc)->focus_next = _elm_colorselector_smart_focus_next;

   ELM_WIDGET_CLASS(sc)->access = _access_hook;
}

EAPI const Elm_Colorselector_Smart_Class *
elm_colorselector_smart_class_get(void)
{
   static Elm_Colorselector_Smart_Class _sc =
     ELM_COLORSELECTOR_SMART_CLASS_INIT_NAME_VERSION
       (ELM_COLORSELECTOR_SMART_NAME);
   static const Elm_Colorselector_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_colorselector_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_colorselector_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_colorselector_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_colorselector_color_set(Evas_Object *obj,
                            int r,
                            int g,
                            int b,
                            int a)
{
   ELM_COLORSELECTOR_CHECK(obj);

   _colors_set(obj, r, g, b, a);
}

EAPI void
elm_colorselector_color_get(const Evas_Object *obj,
                            int *r,
                            int *g,
                            int *b,
                            int *a)
{
   ELM_COLORSELECTOR_CHECK(obj);
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (r) *r = sd->r;
   if (g) *g = sd->g;
   if (b) *b = sd->b;
   if (a) *a = sd->a;
}

EAPI void
elm_colorselector_mode_set(Evas_Object *obj,
                           Elm_Colorselector_Mode mode)
{
   ELM_COLORSELECTOR_CHECK(obj);
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (sd->mode == mode) return;
   sd->mode = mode;

   evas_object_hide(elm_layout_content_unset(obj, "selector"));
   evas_object_hide(elm_layout_content_unset(obj, "palette"));

   switch (sd->mode)
     {
      case ELM_COLORSELECTOR_PALETTE:
        elm_layout_content_set(obj, "palette", sd->palette_box);
        elm_layout_signal_emit(obj, "elm,state,palette", "elm");
        sd->focused = ELM_COLORSELECTOR_PALETTE;
        sd->selected = sd->items;
        break;

      case ELM_COLORSELECTOR_COMPONENTS:
        elm_layout_content_set(obj, "selector", sd->col_bars_area);
        elm_layout_signal_emit(obj, "elm,state,components", "elm");
        sd->focused = ELM_COLORSELECTOR_COMPONENTS;
        sd->sel_color_type = HUE;
        break;

      case ELM_COLORSELECTOR_BOTH:
        elm_layout_content_set(obj, "palette", sd->palette_box);
        elm_layout_content_set(obj, "selector", sd->col_bars_area);
        elm_layout_signal_emit(obj, "elm,state,both", "elm");
        sd->focused = ELM_COLORSELECTOR_PALETTE;
        sd->selected = sd->items;
        break;

      default:
        return;
     }

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);

   elm_layout_sizing_eval(obj);
}

EAPI Elm_Colorselector_Mode
elm_colorselector_mode_get(const Evas_Object *obj)
{
   ELM_COLORSELECTOR_CHECK(obj) ELM_COLORSELECTOR_BOTH;
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   return sd->mode;
}

EAPI void
elm_colorselector_palette_item_color_get(const Elm_Object_Item *it,
                                         int *r,
                                         int *g,
                                         int *b,
                                         int *a)
{
   Elm_Color_Item *item;

   ELM_COLORSELECTOR_ITEM_CHECK_OR_RETURN(it);

   item = (Elm_Color_Item *)it;
   if (item)
     {
        if (r) *r = item->color->r;
        if (g) *g = item->color->g;
        if (b) *b = item->color->b;
        if (a) *a = item->color->a;
     }
}

EAPI void
elm_colorselector_palette_item_color_set(Elm_Object_Item *it,
                                         int r,
                                         int g,
                                         int b,
                                         int a)
{
   Elm_Color_Item *item;

   ELM_COLORSELECTOR_ITEM_CHECK_OR_RETURN(it);

   item = (Elm_Color_Item *)it;
   item->color->r = r;
   item->color->g = g;
   item->color->b = b;
   item->color->a = a;
   evas_object_color_set
     (item->color_obj, item->color->r, item->color->g, item->color->b,
     item->color->a);

   _colors_save(WIDGET(it));
}

EAPI Elm_Object_Item *
elm_colorselector_palette_color_add(Evas_Object *obj,
                                    int r,
                                    int g,
                                    int b,
                                    int a)
{
   Elm_Color_Item *item;

   ELM_COLORSELECTOR_CHECK(obj) NULL;
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (sd->config_load)
     {
        _colors_remove(obj);
        sd->config_load = EINA_FALSE;
     }
   item = _item_new(obj);
   if (!item) return NULL;

   item->color = ELM_NEW(Elm_Color_RGBA);
   if (!item->color) return NULL;

   item->color->r = r;
   item->color->g = g;
   item->color->b = b;
   item->color->a = a;
   _elm_config_color_set
     (sd->palette_name, item->color->r, item->color->g, item->color->b,
     item->color->a);

   elm_box_pack_end(sd->palette_box, VIEW(item));
   evas_object_color_set
     (item->color_obj, item->color->r, item->color->g, item->color->b,
     item->color->a);

   sd->items = eina_list_append(sd->items, item);

   elm_layout_sizing_eval(obj);

   return (Elm_Object_Item *)item;
}

EAPI void
elm_colorselector_palette_clear(Evas_Object *obj)
{
   ELM_COLORSELECTOR_CHECK(obj);
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   _colors_remove(obj);
   if (sd->mode == ELM_COLORSELECTOR_BOTH)
     sd->focused = ELM_COLORSELECTOR_COMPONENTS;
}

EAPI void
elm_colorselector_palette_name_set(Evas_Object *obj,
                                   const char *palette_name)
{
   ELM_COLORSELECTOR_CHECK(obj);
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   if (!strcmp(sd->palette_name, palette_name)) return;

   if (palette_name)
     {
        _colors_remove(obj);
        eina_stringshare_replace(&sd->palette_name, palette_name);
        _palette_colors_load(obj);
     }
}

EAPI const char *
elm_colorselector_palette_name_get(const Evas_Object *obj)
{
   ELM_COLORSELECTOR_CHECK(obj) NULL;
   ELM_COLORSELECTOR_DATA_GET(obj, sd);

   return sd->palette_name;
}
