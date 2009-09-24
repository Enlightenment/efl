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
   Eina_Bool dead : 1; // old grid. will die as soon as anim is over
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
   Ecore_Animator *zoom_animator;
   double t_start, t_end;
   struct {
      int imw, imh;
      int w, h;
      int ow, oh, nw, nh;
      struct {
         double x, y;
      } spos;
   } size;
   int tsize;
   Evas_Object *img; // low res version of image (scale down == 8)
   int nosmooth;
   Eina_List *grids;
   Eina_Bool main_load_pending : 1;
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

static void grid_place(Evas_Object *obj, Grid *g, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh);
static void grid_clear(Grid *g);
static Grid *grid_create(Evas_Object *obj);
static void grid_load(Evas_Object *obj, Grid *g);

static void
img_place(Evas_Object *obj, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord ax, ay, gw, gh;
   int x, y;

   ax = 0;
   ay = 0;
   gw = wd->size.w;
   gh = wd->size.h;
   if (ow > gw) ax = (ow - gw) / 2;
   if (oh > gh) ay = (oh - gh) / 2;
   evas_object_move(wd->img, 
                    ox + 0 - px + ax,
                    oy + 0 - py + ay);
   evas_object_resize(wd->img, gw, gh);
}

static void
grid_place(Evas_Object *obj, Grid *g, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord ax, ay, gw, gh, tx, ty;
   int x, y;

   ax = 0;
   ay = 0;
   gw = wd->size.w;
   gh = wd->size.h;
   if (ow > gw) ax = (ow - gw) / 2;
   if (oh > gh) ay = (oh - gh) / 2;
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn, xx, yy, ww, hh;
             
             tn = (y * g->gw) + x;
             xx = g->grid[tn].out.x;
             yy = g->grid[tn].out.y;
             ww = g->grid[tn].out.w;
             hh = g->grid[tn].out.h;
             if (gw != g->w)
               {
                  tx = xx;
                  xx = (gw * xx) / g->w;
                  ww = ((gw * (tx + ww)) / g->w) - xx;
               }
             if (gh != g->h)
               {
                  ty = yy;
                  yy = (gh * yy) / g->h;
                  hh = ((gh * (ty + hh)) / g->h) - yy;
               }
             evas_object_move(g->grid[tn].img,
                              ox + xx - px + ax,
                              oy + yy - py + ay);
             evas_object_resize(g->grid[tn].img,
                                ww, hh);
          }
     }
}

static void
grid_clear(Grid *g)
{
   int x, y;
   
   if (!g->grid) return;
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn;
             
             tn = (y * g->gw) + x;
             evas_object_del(g->grid[tn].img);
          }
     }
   free(g->grid);
   g->grid = NULL;
   g->gw = 0;
   g->gh = 0;
}

static void
_tile_preloaded(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Grid_Item *git = data;

   evas_object_show(git->img);
   git->have = 1;
}

static Grid *
grid_create(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;
   Grid *g;
   
   g = calloc(1, sizeof(Grid));
   
   g->zoom = wd->zoom;
   g->tsize = wd->tsize;
   g->iw = wd->size.imw;
   g->ih = wd->size.imh;
   
   g->w = g->iw / g->zoom;
   g->h = g->ih / g->zoom;
   if (g->zoom >= 8) return NULL;
   g->gw = (g->w + g->tsize - 1) / g->tsize;
   g->gh = (g->h + g->tsize - 1) / g->tsize;
   g->grid = calloc(1, sizeof(Grid_Item) * g->gw * g->gh);
   if (!g->grid)
     {
        g->gw = 0;
        g->gh = 0;
        return g;
     }
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn, xx, yy, ww, hh;
             
             tn = (y * g->gw) + x;
             g->grid[tn].src.x = x * g->tsize;
             if (x == (g->gw - 1))
               g->grid[tn].src.w = g->w - ((g->gw - 1) * g->tsize);
             else
               g->grid[tn].src.w = g->tsize;
             g->grid[tn].src.y = y * g->tsize;
             if (y == (g->gh - 1))
               g->grid[tn].src.h = g->h - ((g->gh - 1) * g->tsize);
             else
               g->grid[tn].src.h = g->tsize;
             
             g->grid[tn].out.x = g->grid[tn].src.x;
             g->grid[tn].out.y = g->grid[tn].src.y;
             g->grid[tn].out.w = g->grid[tn].src.w;
             g->grid[tn].out.h = g->grid[tn].src.h;
             
             g->grid[tn].img = 
               evas_object_image_add(evas_object_evas_get(obj));
             evas_object_image_scale_hint_set(g->grid[tn].img, EVAS_IMAGE_SCALE_HINT_STATIC);
             evas_object_smart_member_add(g->grid[tn].img, 
                                          wd->pan_smart);
             elm_widget_sub_object_add(obj, g->grid[tn].img);
             evas_object_image_filled_set(g->grid[tn].img, 1);
             evas_object_event_callback_add(g->grid[tn].img, 
                                            EVAS_CALLBACK_IMAGE_PRELOADED,
                                            _tile_preloaded, 
                                            &(g->grid[tn]));
          }
     }
   return g;
}

static void
grid_load(Evas_Object *obj, Grid *g)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;
   Evas_Coord ow, oh;

   elm_smart_scroller_child_viewport_size_get(wd->scr, &ow, &oh);
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn;
             Eina_Bool visible = 0;
             
             tn = (y * g->gw) + x;
             if (ELM_RECTS_INTERSECT(wd->pan_x, wd->pan_y, 
                                     ow, oh,
                                     g->grid[tn].out.x,
                                     g->grid[tn].out.y,
                                     g->grid[tn].out.w,
                                     g->grid[tn].out.h))
               visible = 1;
             if ((visible) && (!g->grid[tn].want))
               {
                  g->grid[tn].want = 1;
                  evas_object_hide(g->grid[tn].img);
                  evas_object_image_file_set(g->grid[tn].img, NULL, NULL);
                  evas_object_image_load_scale_down_set(g->grid[tn].img, g->zoom);
                  evas_object_image_load_region_set(g->grid[tn].img,
                                                    g->grid[tn].src.x,
                                                    g->grid[tn].src.y,
                                                    g->grid[tn].src.w,
                                                    g->grid[tn].src.h);
                  evas_object_image_file_set(g->grid[tn].img, wd->file, NULL); 
                  evas_object_image_preload(g->grid[tn].img, 0);
               }
             else if ((g->grid[tn].want) && (!visible))
               {
                  g->grid[tn].want = 0;
                  g->grid[tn].have = 0;
                  evas_object_hide(g->grid[tn].img);
                  evas_object_image_preload(g->grid[tn].img, 1);
                  evas_object_image_file_set(g->grid[tn].img, NULL, NULL);
               }
          }
     }
}

static void
grid_clearall(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Grid *g;
   
   EINA_LIST_FREE(wd->grids, g)
     {
        grid_clear(g);
        free(g);
     }
}

static void
_smooth_update(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;
   Eina_List *l;
   Grid *g;

   EINA_LIST_FOREACH(wd->grids, l, g)
     {
        for (y = 0; y < g->gh; y++)
          {
             for (x = 0; x < g->gw; x++)
               {
                  int tn;
                  
                  tn = (y * g->gw) + x;
                  evas_object_image_smooth_scale_set(g->grid[tn].img, (wd->nosmooth == 0));
               }
          }
     }
   evas_object_image_smooth_scale_set(wd->img, (wd->nosmooth == 0));
}

static void
_grid_raise(Grid *g)
{
   int x, y;
   
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn;
             
             tn = (y * g->gw) + x;
             evas_object_raise(g->grid[tn].img);
          }
     }
}

static int
_scr_timeout(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->nosmooth--;
   if (wd->nosmooth == 0) _smooth_update(data);
   wd->scr_timer = NULL;
   return 0;
}

static void
_scr(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd->scr_timer)
     {
        wd->nosmooth++;
        if (wd->nosmooth == 1) _smooth_update(data);
     }
   if (wd->scr_timer) ecore_timer_del(wd->scr_timer);
   wd->scr_timer = ecore_timer_add(0.25, _scr_timeout, data);
}

static void
_main_preloaded(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);
   Grid *g;
   
   evas_object_show(wd->img);
   wd->main_load_pending = 0;
   g = grid_create(obj);
   if (g)
     {
        wd->grids = eina_list_prepend(wd->grids, g);
     }
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

static int
_zoom_anim(void *data)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);
   double t;
   Evas_Coord ow, oh;
   
   t = ecore_loop_time_get();
   if (t >= wd->t_end)
     {
        t = 1.0;
     }
   else t = (t - wd->t_start) / (wd->t_end - wd->t_start);
   t = 1.0 - t;
   t = 1.0 - (t * t);
   wd->size.w = (wd->size.ow * (1.0 - t)) + (wd->size.nw * t);
   wd->size.h = (wd->size.oh * (1.0 - t)) + (wd->size.nh * t);
   printf("%3.3f %3.3f\n", wd->size.spos.x, wd->size.spos.y);
//   elm_smart_scroller_child_viewport_size_get(wd->scr, &ow, &oh);
//   elm_smart_scroller_child_region_show
//     (wd->scr, 
//      (wd->size.spos.x * wd->size.w) - (ow / 2),
//      (wd->size.spos.y * wd->size.h) - (oh / 2),
//      ow, oh);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
   if (t >= 1.0)
     {
        Eina_List *l, *l_next;
        Grid *g;
        
        EINA_LIST_FOREACH_SAFE(wd->grids, l, l_next, g)
          {
             if (g->dead)
               {
                  wd->grids = eina_list_remove_list(wd->grids, l);
                  grid_clear(g);
                  free(g);
               }
          }
        wd->nosmooth--;
        if (wd->nosmooth == 0) _smooth_update(data);
        wd->zoom_animator = NULL;
        return 0;
     }
   return 1;
}

static Evas_Smart_Class _pan_sc = {NULL};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   grid_clearall(obj);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
   if (wd->file) eina_stringshare_del(wd->file);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   if (wd->scr_timer) ecore_timer_del(wd->scr_timer);
   if (wd->zoom_animator) ecore_animator_del(wd->zoom_animator);
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

   minw = wd->size.w;
   minh = wd->size.h;
   
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
   Eina_List *l;
   Grid *g;
   
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   img_place(sd->wd->obj, sd->wd->pan_x, sd->wd->pan_y, ox, oy, ow, oh);
   EINA_LIST_FOREACH(sd->wd->grids, l, g)
     {
        grid_load(sd->wd->obj, g);
        grid_place(sd->wd->obj, g, sd->wd->pan_x, sd->wd->pan_y, ox, oy, ow, oh);
     }
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
   
   wd->tsize = 512;
   
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
   evas_object_image_smooth_scale_set(wd->img, (wd->nosmooth == 0));
   evas_object_image_file_set(wd->img, NULL, NULL);
   evas_object_image_load_scale_down_set(wd->img, 0);
   evas_object_image_file_set(wd->img, wd->file, NULL);
   evas_object_image_size_get(wd->img, &w, &h);
   wd->size.imw = w;
   wd->size.imh = h;
   wd->size.w = wd->size.imw / wd->zoom;
   wd->size.h = wd->size.imh / wd->zoom;
   if (wd->zoom_animator)
     {
        wd->nosmooth--;
        if (wd->nosmooth == 0) _smooth_update(obj);
        ecore_animator_del(wd->zoom_animator);
        wd->zoom_animator = NULL;
     }
   evas_object_image_file_set(wd->img, NULL, NULL);
   evas_object_image_load_scale_down_set(wd->img, 8);
   evas_object_image_file_set(wd->img, wd->file, NULL);
   evas_object_image_preload(wd->img, 0);
   wd->main_load_pending = 1;
   grid_clearall(obj);
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
   Eina_List *l;
   Grid *g, *g_zoom = NULL;
   Evas_Coord pw, ph, rx, ry, rw, rh;
   Ecore_Animator *an;
   
   if (zoom < 1) zoom = 1;
   if (zoom == wd->zoom) return;
   wd->zoom = zoom;
   wd->size.ow = wd->size.w;
   wd->size.oh = wd->size.h;
   wd->size.nw = wd->size.imw / wd->zoom;
   wd->size.nh = wd->size.imh / wd->zoom;
   wd->size.w = wd->size.nw;
   wd->size.h = wd->size.nh;
   if (wd->main_load_pending) goto done;
   EINA_LIST_FOREACH(wd->grids, l, g)
     {
        if (g->zoom == wd->zoom)
          {
             g_zoom = g;
             g->dead = 0;
          }
        else
          g->dead = 1;
     }
   if (g_zoom)
     {
        g = g_zoom;
        wd->grids = eina_list_remove(wd->grids, g);
        wd->grids = eina_list_prepend(wd->grids, g);
        _grid_raise(g);
        goto done;
     }
   g = grid_create(obj);
   if (g)
     {
        wd->grids = eina_list_prepend(wd->grids, g);
     }
   done:
   if (!wd->zoom_animator)
     {
        wd->zoom_animator = ecore_animator_add(_zoom_anim, obj);
        wd->nosmooth++;
        if (wd->nosmooth == 1) _smooth_update(obj);
     }
   wd->t_start = ecore_loop_time_get();
   elm_smart_scroller_child_pos_get(wd->scr, &rx, &ry);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &rw, &rh);
   wd->size.spos.x = (double)(rx + (rw / 2)) / (double)wd->size.w;
   wd->size.spos.y = (double)(ry + (rh / 2)) / (double)wd->size.h;
   wd->t_end = wd->t_start + _elm_config->zoom_friction;
   an = wd->zoom_animator;
   if (!_zoom_anim(obj)) ecore_animator_del(an);
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
