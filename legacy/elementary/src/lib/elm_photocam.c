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

typedef struct _Grid Grid;
typedef struct _Grid_Item Grid_Item;

struct _Grid_Item
{
   Evas_Object *img;
   struct {
      int x, y, w, h;
   } src, out;
   Eina_Bool want : 1;
   Eina_Bool have : 1;
   Eina_Bool want_new : 1;
   Eina_Bool have_new : 1;
};

struct _Grid
{
   int tsize; // size of tile (tsize x tsize pixels)
   int zoom; // zoom level tiles want for optimal display (1, 2, 4, 8)
   int iw, ih; // size of image in pixels
   int w, h; // size of grid image in pixels (represented by grid)
   int gw, gh; // size of grid in tiles
   Grid_Item *grid; // the grid (gw * gh items)
};

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *scr;
   Evas_Object *pan_smart;
   Pan *pan;
   Evas_Coord pan_x, pan_y, minw, minh;

   int zoom;
   Elm_Photocam_Zoom_Mode mode;
   const char *file;
   
   Ecore_Job *calc_job;
   Ecore_Timer *scr_timer;
   Evas_Object *img; // low res version of image (scale down == 8)
   Grid grid;
   Eina_Bool smooth : 1;

/*   
   Evas_Object *img[3];
   int ph_w, ph_h;
   double preload_time, fade_time;
   Ecore_Animator *fade_animator;
   int scale;
   int imuse, imfade;
   Eina_Bool do_fade : 1;
 */
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

static void grid_place(Evas_Object *obj, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy);
static void grid_clear(Evas_Object *obj);
static void grid_create(Evas_Object *obj);
static void grid_load(Evas_Object *obj);

static void
grid_place(Evas_Object *obj, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;
    
   evas_object_move(wd->img, 
                    ox + 0 - px,
                    oy + 0 - py);
   evas_object_resize(wd->img, 
                      wd->grid.w,
                      wd->grid.h);
   for (y = 0; y < wd->grid.gh; y++)
     {
        for (x = 0; x < wd->grid.gw; x++)
          {
             int tn, xx, yy, ww, hh;
             
             tn = (y * wd->grid.gw) + x;
             xx = wd->grid.grid[tn].out.x;
             yy = wd->grid.grid[tn].out.y;
             ww = wd->grid.grid[tn].out.w;
             hh = wd->grid.grid[tn].out.h;
             evas_object_move(wd->grid.grid[tn].img,
                              ox + xx - px,
                              oy + yy - py);
             evas_object_resize(wd->grid.grid[tn].img,
                                ww, hh);
          }
     }
}

static void
grid_clear(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;
   
   if (!wd->grid.grid) return;
   for (y = 0; y < wd->grid.gh; y++)
     {
        for (x = 0; x < wd->grid.gw; x++)
          {
             int tn;
             
             tn = (y * wd->grid.gw) + x;
             evas_object_del(wd->grid.grid[tn].img);
          }
     }
   free(wd->grid.grid);
   wd->grid.grid = NULL;
   wd->grid.gw = 0;
   wd->grid.gh = 0;
}

static void
_tile_preloaded(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Grid_Item *git = data;

   evas_object_show(git->img);
   git->have = 1;
}

static void
grid_create(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;
   
   grid_clear(obj);
   wd->grid.zoom = wd->zoom;
   wd->grid.w = wd->grid.iw / wd->grid.zoom;
   wd->grid.h = wd->grid.ih / wd->grid.zoom;
   if (wd->grid.zoom >= 8) return;
   wd->grid.gw = (wd->grid.w + wd->grid.tsize - 1) / wd->grid.tsize;
   wd->grid.gh = (wd->grid.h + wd->grid.tsize - 1) / wd->grid.tsize;
   wd->grid.grid = calloc(1, sizeof(Grid_Item) * wd->grid.gw * wd->grid.gh);
   if (!wd->grid.grid)
     {
        wd->grid.gw = 0;
        wd->grid.gh = 0;
        return;
     }
   for (y = 0; y < wd->grid.gh; y++)
     {
        for (x = 0; x < wd->grid.gw; x++)
          {
             int tn, xx, yy, ww, hh;
             
             tn = (y * wd->grid.gw) + x;
             wd->grid.grid[tn].src.x = x * wd->grid.tsize;
             if (x == (wd->grid.gw - 1))
               wd->grid.grid[tn].src.w = wd->grid.w - ((wd->grid.gw - 1) * wd->grid.tsize);
             else
               wd->grid.grid[tn].src.w = wd->grid.tsize;
             wd->grid.grid[tn].src.y = y * wd->grid.tsize;
             if (y == (wd->grid.gh - 1))
               wd->grid.grid[tn].src.h = wd->grid.h - ((wd->grid.gh - 1) * wd->grid.tsize);
             else
               wd->grid.grid[tn].src.h = wd->grid.tsize;
             
             wd->grid.grid[tn].out.x = wd->grid.grid[tn].src.x;
             wd->grid.grid[tn].out.y = wd->grid.grid[tn].src.y;
             wd->grid.grid[tn].out.w = wd->grid.grid[tn].src.w;
             wd->grid.grid[tn].out.h = wd->grid.grid[tn].src.h;
             
             wd->grid.grid[tn].img = 
               evas_object_image_add(evas_object_evas_get(obj));
             evas_object_image_scale_hint_set(wd->grid.grid[tn].img, EVAS_IMAGE_SCALE_HINT_STATIC);
             evas_object_smart_member_add(wd->grid.grid[tn].img, 
                                          wd->pan_smart);
             elm_widget_sub_object_add(obj, wd->grid.grid[tn].img);
             evas_object_image_filled_set(wd->grid.grid[tn].img, 1);
             evas_object_event_callback_add(wd->grid.grid[tn].img, 
                                            EVAS_CALLBACK_IMAGE_PRELOADED,
                                            _tile_preloaded, 
                                            &(wd->grid.grid[tn]));
          }
     }
}

static void
grid_load(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;
   Evas_Coord ow, oh;

   elm_smart_scroller_child_viewport_size_get(wd->scr, &ow, &oh);
   for (y = 0; y < wd->grid.gh; y++)
     {
        for (x = 0; x < wd->grid.gw; x++)
          {
             int tn;
             Eina_Bool visible = 0;
             
             tn = (y * wd->grid.gw) + x;
             if (ELM_RECTS_INTERSECT(wd->pan_x, wd->pan_y, 
                                     ow, oh,
                                     wd->grid.grid[tn].out.x,
                                     wd->grid.grid[tn].out.y,
                                     wd->grid.grid[tn].out.w,
                                     wd->grid.grid[tn].out.h))
               visible = 1;
             if ((visible) && (!wd->grid.grid[tn].want))
               {
                  wd->grid.grid[tn].want = 1;
                  evas_object_hide(wd->grid.grid[tn].img);
                  evas_object_image_file_set(wd->grid.grid[tn].img, NULL, NULL);
                  evas_object_image_load_scale_down_set(wd->grid.grid[tn].img, wd->grid.zoom);
                  evas_object_image_load_region_set(wd->grid.grid[tn].img,
                                                    wd->grid.grid[tn].src.x,
                                                    wd->grid.grid[tn].src.y,
                                                    wd->grid.grid[tn].src.w,
                                                    wd->grid.grid[tn].src.h);
//                  evas_object_image_pixels_dirty_set(wd->grid.grid[tn].img, 1);
                  evas_object_image_file_set(wd->grid.grid[tn].img, wd->file, NULL); 
                  evas_object_image_preload(wd->grid.grid[tn].img, 0);
               }
             else if ((wd->grid.grid[tn].want) && (!visible))
               {
                  wd->grid.grid[tn].want = 0;
                  wd->grid.grid[tn].have = 0;
                  evas_object_hide(wd->grid.grid[tn].img);
                  evas_object_image_preload(wd->grid.grid[tn].img, 1);
                  evas_object_image_file_set(wd->grid.grid[tn].img, NULL, NULL);
               }
          }
     }
}

static void
_smooth_update(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;

   for (y = 0; y < wd->grid.gh; y++)
     {
        for (x = 0; x < wd->grid.gw; x++)
          {
             int tn;
             
             tn = (y * wd->grid.gw) + x;
             evas_object_image_smooth_scale_set(wd->grid.grid[tn].img, wd->smooth);
          }
     }
   evas_object_image_smooth_scale_set(wd->img, wd->smooth);
}

static int
_scr_timeout(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd->smooth)
     {
        wd->smooth = 1;
        _smooth_update(data);
     }
   wd->scr_timer = NULL;
   return 0;
}

static void
_scr(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (wd->scr_timer) ecore_timer_del(wd->scr_timer);
   wd->scr_timer = ecore_timer_add(0.25, _scr_timeout, data);
   if (wd->smooth)
     {
        wd->smooth = 0;
        _smooth_update(data);
     }
}

/*
static void _next_stage(Evas_Object *obj);

static int
_fade_animator(void *data)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);
   double t;
   int v;
   
   t = ecore_loop_time_get() - wd->preload_time;
   v = (255.0 * t) / wd->fade_time;
   if (v > 255) v = 255;
   evas_object_color_set(wd->img[wd->imfade], v, v, v, v);
   if (v < 255) return 1;
   
   wd->fade_animator = NULL;
   if (wd->do_fade)
     {
        wd->do_fade = 0;
        _next_stage(obj);
     }
   return 0;
}

static void
_next_stage(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int w, h;
   double t, td;
   
   wd->imfade = wd->imuse;
   evas_object_raise(wd->img[wd->imfade]);
   evas_object_color_set(wd->img[wd->imfade], 0, 0, 0, 0);
   evas_object_show(wd->img[wd->imfade]);
   
   t = ecore_loop_time_get();
   td = t - wd->preload_time;
   printf("scale: %i use %i. done in %3.3f\n", wd->scale, wd->imuse, td);
   if (td > 1.0) td = 1.0;
   wd->fade_time = td;
   wd->preload_time = t;
   wd->fade_animator = ecore_animator_add(_fade_animator, obj);
   
   if (wd->scale == 1) return;
   
   wd->imuse++;
   if (wd->imuse >= 3) wd->imuse = 0;
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
*/

static Evas_Smart_Class _pan_sc = {NULL};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
   if (wd->file) eina_stringshare_del(wd->file);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   if (wd->scr_timer) ecore_timer_del(wd->scr_timer);
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

   minw = wd->grid.w;
   minh = wd->grid.h;
   
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
   grid_load(sd->wd->obj);
   grid_place(sd->wd->obj, sd->wd->pan_x, sd->wd->pan_y, ox, oy);
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
_main_preloaded(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);

   evas_object_show(wd->img);
   grid_load(obj);
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
   evas_object_smart_callback_add(wd->scr, "scroll", _scr, obj);
   evas_object_smart_callback_add(wd->scr, "drag", _scr, obj);
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

   wd->zoom = 1;
   wd->mode = ELM_PHOTOCAM_ZOOM_MODE_MANUAL;
   wd->smooth = 1;
   
   wd->grid.tsize = 512;
   
   wd->img = evas_object_image_add(e);
   evas_object_image_scale_hint_set(wd->img, EVAS_IMAGE_SCALE_HINT_STATIC);
   evas_object_smart_member_add(wd->img, wd->pan_smart);
   elm_widget_sub_object_add(obj, wd->img);
   evas_object_image_filled_set(wd->img, 1);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_IMAGE_PRELOADED,
                                  _main_preloaded, obj);
   
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
EAPI void
elm_photocam_file_set(Evas_Object *obj, const char *file)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int w, h;

   if (wd->file) eina_stringshare_del(wd->file);
   
   wd->file = eina_stringshare_add(file);
   evas_object_hide(wd->img);
   evas_object_image_smooth_scale_set(wd->img, wd->smooth);
   evas_object_image_file_set(wd->img, NULL, NULL);
   evas_object_image_load_scale_down_set(wd->img, 0);
   evas_object_image_file_set(wd->img, wd->file, NULL);
   evas_object_image_size_get(wd->img, &w, &h);
   wd->grid.iw = w;
   wd->grid.ih = h;
   evas_object_image_file_set(wd->img, NULL, NULL);
   evas_object_image_load_scale_down_set(wd->img, 8);
   evas_object_image_file_set(wd->img, wd->file, NULL);
   evas_object_image_preload(wd->img, 0);
   grid_create(obj);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

/**
 * XXX
 *
 * xxx
 *
 * @param obj The photocam object
 *
 * @ingroup Photocam
 */
EAPI void
elm_photocam_zoom_set(Evas_Object *obj, int zoom)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (zoom < 1) zoom = 1;
   if (zoom == wd->zoom) return;
   wd->zoom = zoom;
   grid_create(obj);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

/**
 * XXX
 *
 * xxx
 *
 * @param obj The photocam object
 *
 * @ingroup Photocam
 */
EAPI int
elm_photocam_zoom_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->zoom;
}

/**
 * XXX
 *
 * xxx
 *
 * @param obj The photocam object
 *
 * @ingroup Photocam
 */
EAPI void
elm_photocam_zoom_mode_set(Evas_Object *obj, Elm_Photocam_Zoom_Mode mode)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->mode == mode) return;
}

/**
 * XXX
 *
 * xxx
 *
 * @param obj The photocam object
 *
 * @ingroup Photocam
 */
EAPI Elm_Photocam_Zoom_Mode
elm_photocam_zoom_mode_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->mode;
}
