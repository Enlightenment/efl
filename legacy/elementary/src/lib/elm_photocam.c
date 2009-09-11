#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Photocam Photocam
 *
 * XXX
 *
 * Signals that you can add callbacks for are:
 *
 * clicked - This is called when a user has double-clicked an item. The
 * event_info parameter is the genlist item that was double-clicked.
 *
 */
typedef struct _Widget_Data Widget_Data;
typedef struct _Pan Pan;

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *scr;
   Evas_Object *img[2];
   Evas_Object *pan_smart;
   Pan *pan;
   Ecore_Job *calc_job;
   int ph_w, ph_h;
   Evas_Coord pan_x, pan_y, minw, minh;
   int scale;
   int imuse;
   const char *file;
};

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data *wd;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _show_region_hook(void *data, Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

static void _calc_job(void *data);

static Evas_Smart_Class _pan_sc = {NULL};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
   if (wd->file) eina_stringshare_del(wd->file);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_smart_scroller_theme_set(wd->scr, "scroller", "base", elm_widget_style_get(obj));
   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_show_region_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   evas_object_size_hint_min_get(wd->scr, &minw, &minh);
   evas_object_size_hint_max_get(wd->scr, &maxw, &maxh);
   minw = -1;
   minh = -1;
//   if (wd->mode != ELM_LIST_LIMIT) minw = -1;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Evas_Coord minw, minh;

   minw = wd->ph_w;
   minh = wd->ph_h;
   
   if ((minw != wd->minw) || (minh != wd->minh))
     {
        wd->minw = minw;
        wd->minh = minh;
        evas_object_size_hint_min_set(wd->pan_smart, wd->minw, wd->minh);
        evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
        _sizing_eval(wd->obj);
     }
   wd->calc_job = NULL;
   evas_object_smart_changed(wd->pan_smart);
}

static void
_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;
   evas_object_smart_changed(obj);
}

static void
_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

static void
_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = sd->wd->minw - ow;
   if (ow < 0) ow = 0;
   oh = sd->wd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

static void
_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   Evas_Object_Smart_Clipped_Data *cd;

   _pan_sc.add(obj);
   cd = evas_object_smart_data_get(obj);
   sd = calloc(1, sizeof(Pan));
   if (!sd) return;
   sd->__clipped_data = *cd;
   free(cd);
   evas_object_smart_data_set(obj, sd);
}

static void
_pan_del(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   _pan_sc.del(obj);
}

static void
_pan_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
}

static void
_pan_calculate(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ox, oy, ow, oh;

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_object_move(sd->wd->img[0], 
                    ox + 0 - sd->wd->pan_x,
                    oy + 0 - sd->wd->pan_y);
   evas_object_resize(sd->wd->img[0], sd->wd->minw, sd->wd->minh);
   evas_object_move(sd->wd->img[1], 
                    ox + 0 - sd->wd->pan_x,
                    oy + 0 - sd->wd->pan_y);
   evas_object_resize(sd->wd->img[1], sd->wd->minw, sd->wd->minh);
}

static void
_hold_on(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 1);
}

static void
_hold_off(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 0);
}

static void
_freeze_on(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 1);
}

static void
_freeze_off(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 0);
}

static void
_preloaded(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);
   int w, h;
   
   printf("scale: %i use %i\n", wd->scale, wd->imuse);
   evas_object_show(wd->img[wd->imuse]);
   if (wd->scale == 1) return;
   
   wd->imuse = 1 - wd->imuse;
   wd->scale /= 2;
   evas_object_hide(wd->img[wd->imuse]);
   evas_object_image_load_scale_down_set(wd->img[wd->imuse], wd->scale);
   evas_object_image_pixels_dirty_set(wd->img[wd->imuse], 1);
   evas_object_image_file_set(wd->img[wd->imuse], wd->file, NULL);
   evas_object_image_preload(wd->img[wd->imuse], 0);
   evas_object_image_size_get(wd->img[wd->imuse], &w, &h);
   wd->ph_w = w * wd->scale;
   wd->ph_h = h * wd->scale;
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}
   
/**
 * Add a new Photocam object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Photocam
 */
EAPI Evas_Object *
elm_photocam_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;
   static Evas_Smart *smart = NULL;
   int i;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "genlist");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->scr = elm_smart_scroller_add(e);
   elm_widget_resize_object_set(obj, wd->scr);

   elm_smart_scroller_bounce_allow_set(wd->scr, 1, 1);

   wd->obj = obj;

   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);
   
   if (!smart)
     {
	static Evas_Smart_Class sc;

	evas_object_smart_clipped_smart_set(&_pan_sc);
	sc = _pan_sc;
	sc.name = "elm_photocam_pan";
	sc.version = EVAS_SMART_CLASS_VERSION;
	sc.add = _pan_add;
	sc.del = _pan_del;
	sc.resize = _pan_resize;
	sc.calculate = _pan_calculate;
	smart = evas_smart_class_new(&sc);
     }
   if (smart)
     {
	wd->pan_smart = evas_object_smart_add(e, smart);
	wd->pan = evas_object_smart_data_get(wd->pan_smart);
	wd->pan->wd = wd;
     }

   elm_smart_scroller_extern_pan_set(wd->scr, wd->pan_smart,
				     _pan_set, _pan_get,
				     _pan_max_get, _pan_child_size_get);
   for (i = 0; i < 2; i++)
     {
        wd->img[i] = evas_object_image_add(e);
        evas_object_smart_member_add(wd->img[i], wd->pan_smart);
        elm_widget_sub_object_add(obj, wd->img[i]);
        evas_object_image_filled_set(wd->img[i], 1);
        evas_object_event_callback_add(wd->img[i], EVAS_CALLBACK_IMAGE_PRELOADED,
                                       _preloaded, obj);
        evas_object_show(wd->img[i]);
     }
   
   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), 
                             &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   _sizing_eval(obj);
   return obj;
}

/**
 * XXX
 *
 * xxx
 *
 * @param obj The photocam object
 * @param file The photo file
 *
 * @ingroup Photocam
 */
EAPI Elm_Genlist_Item *
elm_photocam_file_set(Evas_Object *obj, const char *file)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int w, h;

   if (wd->file) eina_stringshare_del(wd->file);
//   evas_object_image_preload(wd->img[0], 1);
//   evas_object_image_preload(wd->img[1], 1);
   evas_object_hide(wd->img[0]);
   evas_object_hide(wd->img[1]);
   
   wd->file = eina_stringshare_add(file);
   wd->scale = 8;
   wd->imuse = 0;
   evas_object_image_load_scale_down_set(wd->img[wd->imuse], wd->scale);
   evas_object_image_file_set(wd->img[wd->imuse], wd->file, NULL);
   evas_object_image_preload(wd->img[wd->imuse], 0);
   evas_object_image_size_get(wd->img[wd->imuse], &w, &h);
   wd->ph_w = w * wd->scale;
   wd->ph_h = h * wd->scale;
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}
