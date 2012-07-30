#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_photocam.h"

/*
 * TODO (maybe - optional future stuff):
 *
 * 1. wrap photo in theme edje so u can have styling around photo (like white
 *    photo bordering).
 * 2. exif handling
 * 3. rotation flags in exif handling (nasty! should have rot in evas)
 */

EAPI const char ELM_PHOTOCAM_SMART_NAME[] = "elm_photocam";
EAPI const char ELM_PHOTOCAM_PAN_SMART_NAME[] = "elm_photocam_pan";

static const char SIG_CLICKED[] = "clicked";
static const char SIG_PRESS[] = "press";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_LOAD[] = "load";
static const char SIG_LOADED[] = "loaded";
static const char SIG_LOAD_DETAIL[] = "load,detail";
static const char SIG_LOADED_DETAIL[] = "loaded,detail";
static const char SIG_ZOOM_START[] = "zoom,start";
static const char SIG_ZOOM_STOP[] = "zoom,stop";
static const char SIG_ZOOM_CHANGE[] = "zoom,change";
static const char SIG_SCROLL[] = "scroll";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_PRESS, ""},
   {SIG_LONGPRESSED, ""},
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_LOAD, ""},
   {SIG_LOADED, ""},
   {SIG_LOAD_DETAIL, ""},
   {SIG_LOADED_DETAIL, ""},
   {SIG_ZOOM_START, ""},
   {SIG_ZOOM_STOP, ""},
   {SIG_ZOOM_CHANGE, ""},
   {SIG_SCROLL, ""},
   {SIG_SCROLL_ANIM_START, ""},
   {SIG_SCROLL_ANIM_STOP, ""},
   {SIG_SCROLL_DRAG_START, ""},
   {SIG_SCROLL_DRAG_STOP, ""},
   {NULL, NULL}
};

static const Evas_Smart_Interface *_smart_interfaces[] =
{
   (Evas_Smart_Interface *)&ELM_SCROLLABLE_IFACE, NULL
};

EVAS_SMART_SUBCLASS_IFACE_NEW
  (ELM_PHOTOCAM_SMART_NAME, _elm_photocam, Elm_Photocam_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, _smart_callbacks,
  _smart_interfaces);

ELM_INTERNAL_SMART_SUBCLASS_NEW
  (ELM_PHOTOCAM_PAN_SMART_NAME, _elm_photocam_pan,
  Elm_Photocam_Pan_Smart_Class, Elm_Pan_Smart_Class, elm_pan_smart_class_get,
  NULL);

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   evas_object_size_hint_max_get
     (ELM_WIDGET_DATA(sd)->resize_obj, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_calc_job_cb(void *data)
{
   Elm_Photocam_Smart_Data *sd = data;
   Evas_Coord minw, minh;

   minw = sd->size.w;
   minh = sd->size.h;
   if (sd->resized)
     {
        sd->resized = EINA_FALSE;
        if (sd->mode != ELM_PHOTOCAM_ZOOM_MODE_MANUAL)
          {
             double tz = sd->zoom;
             sd->zoom = 0.0;
             elm_photocam_zoom_set(ELM_WIDGET_DATA(sd)->obj, tz);
          }
     }
   if ((minw != sd->minw) || (minh != sd->minh))
     {
        sd->minw = minw;
        sd->minh = minh;

        evas_object_smart_callback_call(sd->pan_obj, "changed", NULL);
        _sizing_eval(ELM_WIDGET_DATA(sd)->obj);
     }
   sd->calc_job = NULL;
   evas_object_smart_changed(sd->pan_obj);
}

static void
_elm_photocam_pan_smart_add(Evas_Object *obj)
{
   /* here just to allocate our extended data */
   EVAS_SMART_DATA_ALLOC(obj, Elm_Photocam_Pan_Smart_Data);

   ELM_PAN_CLASS(_elm_photocam_pan_parent_sc)->base.add(obj);
}

static void
_elm_photocam_pan_smart_move(Evas_Object *obj,
                             Evas_Coord x __UNUSED__,
                             Evas_Coord y __UNUSED__)
{
   ELM_PHOTOCAM_PAN_DATA_GET(obj, psd);

   if (psd->wsd->calc_job) ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job_cb, psd->wsd);
}

static void
_elm_photocam_pan_smart_resize(Evas_Object *obj,
                               Evas_Coord w,
                               Evas_Coord h)
{
   Evas_Coord ow, oh;

   ELM_PHOTOCAM_PAN_DATA_GET(obj, psd);

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;

   psd->wsd->resized = EINA_TRUE;
   if (psd->wsd->calc_job) ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job_cb, psd->wsd);
}

static void
_image_place(Evas_Object *obj,
             Evas_Coord px,
             Evas_Coord py,
             Evas_Coord ox,
             Evas_Coord oy,
             Evas_Coord ow,
             Evas_Coord oh)
{
   Evas_Coord ax, ay, gw, gh;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   ax = 0;
   ay = 0;
   gw = sd->size.w;
   gh = sd->size.h;
   if (!sd->zoom_g_layer)
     {
        if (ow > gw) ax = (ow - gw) / 2;
        if (oh > gh) ay = (oh - gh) / 2;
     }
   evas_object_move(sd->img, ox + 0 - px + ax, oy + 0 - py + ay);
   evas_object_resize(sd->img, gw, gh);

   if (sd->show.show)
     {
        sd->show.show = EINA_FALSE;
        sd->s_iface->content_region_show
          (obj, sd->show.x, sd->show.y, sd->show.w, sd->show.h);
     }
}

static void
_grid_load(Evas_Object *obj,
           Elm_Phocam_Grid *g)
{
   int x, y;
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh, gw, gh, tx, ty;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &ow, &oh);
   evas_output_viewport_get(evas_object_evas_get(obj), &cvx, &cvy, &cvw, &cvh);

   gw = sd->size.w;
   gh = sd->size.h;
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn, xx, yy, ww, hh;
             Eina_Bool visible = EINA_FALSE;

             tn = (y * g->gw) + x;
             xx = g->grid[tn].out.x;
             yy = g->grid[tn].out.y;
             ww = g->grid[tn].out.w;
             hh = g->grid[tn].out.h;
             if ((gw != g->w) && (g->w > 0))
               {
                  tx = xx;
                  xx = (gw * xx) / g->w;
                  ww = ((gw * (tx + ww)) / g->w) - xx;
               }
             if ((gh != g->h) && (g->h > 0))
               {
                  ty = yy;
                  yy = (gh * yy) / g->h;
                  hh = ((gh * (ty + hh)) / g->h) - yy;
               }
             if (ELM_RECTS_INTERSECT(xx - sd->pan_x + ox,
                                     yy - sd->pan_y + oy,
                                     ww, hh, cvx, cvy, cvw, cvh))
               visible = EINA_TRUE;
             if ((visible) && (!g->grid[tn].have) && (!g->grid[tn].want))
               {
                  g->grid[tn].want = 1;
                  evas_object_hide(g->grid[tn].img);
                  evas_object_image_file_set(g->grid[tn].img, NULL, NULL);
                  evas_object_image_load_scale_down_set
                    (g->grid[tn].img, g->zoom);
                  evas_object_image_load_region_set
                    (g->grid[tn].img, g->grid[tn].src.x, g->grid[tn].src.y,
                    g->grid[tn].src.w, g->grid[tn].src.h);
                  evas_object_image_file_set(g->grid[tn].img, sd->file, NULL);
                  evas_object_image_preload(g->grid[tn].img, 0);
                  sd->preload_num++;
                  if (sd->preload_num == 1)
                    {
                       edje_object_signal_emit
                         (ELM_WIDGET_DATA(sd)->resize_obj,
                         "elm,state,busy,start", "elm");
                       evas_object_smart_callback_call
                         (obj, SIG_LOAD_DETAIL, NULL);
                    }
               }
             else if ((g->grid[tn].want) && (!visible))
               {
                  sd->preload_num--;
                  if (!sd->preload_num)
                    {
                       edje_object_signal_emit
                         (ELM_WIDGET_DATA(sd)->resize_obj,
                         "elm,state,busy,stop", "elm");
                       evas_object_smart_callback_call
                         (obj, SIG_LOADED_DETAIL, NULL);
                    }
                  g->grid[tn].want = 0;
                  evas_object_hide(g->grid[tn].img);
                  evas_object_image_preload(g->grid[tn].img, 1);
                  evas_object_image_file_set(g->grid[tn].img, NULL, NULL);
               }
             else if ((g->grid[tn].have) && (!visible))
               {
                  g->grid[tn].have = 0;
                  evas_object_hide(g->grid[tn].img);
                  evas_object_image_preload(g->grid[tn].img, 1);
                  evas_object_image_file_set(g->grid[tn].img, NULL, NULL);
               }
          }
     }
}

static void
_grid_place(Evas_Object *obj,
            Elm_Phocam_Grid *g,
            Evas_Coord px,
            Evas_Coord py,
            Evas_Coord ox,
            Evas_Coord oy,
            Evas_Coord ow,
            Evas_Coord oh)
{
   Evas_Coord ax, ay, gw, gh, tx, ty;
   int x, y;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   ax = 0;
   ay = 0;
   gw = sd->size.w;
   gh = sd->size.h;
   if (!sd->zoom_g_layer)
     {
        if (ow > gw) ax = (ow - gw) / 2;
        if (oh > gh) ay = (oh - gh) / 2;
     }
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
             if ((gw != g->w) && (g->w > 0))
               {
                  tx = xx;
                  xx = (gw * xx) / g->w;
                  ww = ((gw * (tx + ww)) / g->w) - xx;
               }
             if ((gh != g->h) && (g->h > 0))
               {
                  ty = yy;
                  yy = (gh * yy) / g->h;
                  hh = ((gh * (ty + hh)) / g->h) - yy;
               }
             evas_object_move(g->grid[tn].img,
                              ox + xx - px + ax,
                              oy + yy - py + ay);
             evas_object_resize(g->grid[tn].img, ww, hh);
          }
     }
}

static void
_elm_photocam_pan_smart_calculate(Evas_Object *obj)
{
   Elm_Phocam_Grid *g;
   Eina_List *l;
   Evas_Coord ox, oy, ow, oh;

   ELM_PHOTOCAM_PAN_DATA_GET(obj, psd);

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   _image_place(
       ELM_WIDGET_DATA(psd->wsd)->obj, psd->wsd->pan_x, psd->wsd->pan_y,
       ox - psd->wsd->g_layer_zoom.imx, oy - psd->wsd->g_layer_zoom.imy, ow,
       oh);

   EINA_LIST_FOREACH (psd->wsd->grids, l, g)
     {
        _grid_load(ELM_WIDGET_DATA(psd->wsd)->obj, g);
        _grid_place(
            ELM_WIDGET_DATA(psd->wsd)->obj, g, psd->wsd->pan_x,
             psd->wsd->pan_y, ox - psd->wsd->g_layer_zoom.imx,
             oy - psd->wsd->g_layer_zoom.imy, ow, oh);
     }
}

static void
_elm_photocam_pan_smart_pos_set(Evas_Object *obj,
                                Evas_Coord x,
                                Evas_Coord y)
{
   ELM_PHOTOCAM_PAN_DATA_GET(obj, psd);

   if ((x == psd->wsd->pan_x) && (y == psd->wsd->pan_y)) return;
   psd->wsd->pan_x = x;
   psd->wsd->pan_y = y;
   evas_object_smart_changed(obj);
}

static void
_elm_photocam_pan_smart_pos_get(const Evas_Object *obj,
                                Evas_Coord *x,
                                Evas_Coord *y)
{
   ELM_PHOTOCAM_PAN_DATA_GET(obj, psd);

   if (x) *x = psd->wsd->pan_x;
   if (y) *y = psd->wsd->pan_y;
}

static void
_elm_photocam_pan_smart_pos_max_get(const Evas_Object *obj,
                                    Evas_Coord *x,
                                    Evas_Coord *y)
{
   Evas_Coord ow, oh;

   ELM_PHOTOCAM_PAN_DATA_GET(obj, psd);

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = psd->wsd->minw - ow;
   if (ow < 0) ow = 0;
   oh = psd->wsd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

static void
_elm_photocam_pan_smart_pos_min_get(const Evas_Object *obj __UNUSED__,
                                    Evas_Coord *x,
                                    Evas_Coord *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
}

static void
_elm_photocam_pan_smart_content_size_get(const Evas_Object *obj,
                                         Evas_Coord *w,
                                         Evas_Coord *h)
{
   ELM_PHOTOCAM_PAN_DATA_GET(obj, psd);

   if (w) *w = psd->wsd->minw;
   if (h) *h = psd->wsd->minh;
}

static void
_elm_photocam_pan_smart_set_user(Elm_Photocam_Pan_Smart_Class *sc)
{
   ELM_PAN_CLASS(sc)->base.add = _elm_photocam_pan_smart_add;
   ELM_PAN_CLASS(sc)->base.move = _elm_photocam_pan_smart_move;
   ELM_PAN_CLASS(sc)->base.resize = _elm_photocam_pan_smart_resize;
   ELM_PAN_CLASS(sc)->base.calculate = _elm_photocam_pan_smart_calculate;

   ELM_PAN_CLASS(sc)->pos_set = _elm_photocam_pan_smart_pos_set;
   ELM_PAN_CLASS(sc)->pos_get = _elm_photocam_pan_smart_pos_get;
   ELM_PAN_CLASS(sc)->pos_max_get = _elm_photocam_pan_smart_pos_max_get;
   ELM_PAN_CLASS(sc)->pos_min_get = _elm_photocam_pan_smart_pos_min_get;
   ELM_PAN_CLASS(sc)->content_size_get =
     _elm_photocam_pan_smart_content_size_get;
}

static int
_nearest_pow2_get(int num)
{
   unsigned int n = num - 1;

   n |= n >> 1;
   n |= n >> 2;
   n |= n >> 4;
   n |= n >> 8;
   n |= n >> 16;

   return n + 1;
}

static void
_grid_clear(Evas_Object *obj,
            Elm_Phocam_Grid *g)
{
   int x, y;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if (!g->grid) return;
   for (y = 0; y < g->gh; y++)
     {
        for (x = 0; x < g->gw; x++)
          {
             int tn;

             tn = (y * g->gw) + x;
             evas_object_del(g->grid[tn].img);
             if (g->grid[tn].want)
               {
                  sd->preload_num--;
                  if (!sd->preload_num)
                    {
                       edje_object_signal_emit
                         (ELM_WIDGET_DATA(sd)->resize_obj,
                         "elm,state,busy,stop", "elm");
                       evas_object_smart_callback_call
                         (obj, SIG_LOAD_DETAIL, NULL);
                    }
               }
          }
     }

   free(g->grid);
   g->grid = NULL;
   g->gw = 0;
   g->gh = 0;
}

static void
_tile_preloaded_cb(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *o __UNUSED__,
                   void *event_info __UNUSED__)
{
   Elm_Photocam_Grid_Item *git = data;

   if (git->want)
     {
        git->want = 0;
        evas_object_show(git->img);
        git->have = 1;
        git->sd->preload_num--;
        if (!git->sd->preload_num)
          {
             edje_object_signal_emit
               (ELM_WIDGET_DATA(git->sd)->resize_obj, "elm,state,busy,stop",
               "elm");
             evas_object_smart_callback_call
               (ELM_WIDGET_DATA(git->sd)->obj, SIG_LOADED_DETAIL, NULL);
          }
     }
}

static int
_grid_zoom_calc(double zoom)
{
   int z = zoom;

   if (z < 1) z = 1;
   return _nearest_pow2_get(z);
}

static Elm_Phocam_Grid *
_grid_create(Evas_Object *obj)
{
   int x, y;
   Elm_Phocam_Grid *g;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   g = calloc(1, sizeof(Elm_Phocam_Grid));
   if (!g) return NULL;

   g->zoom = _grid_zoom_calc(sd->zoom);
   g->tsize = sd->tsize;
   g->iw = sd->size.imw;
   g->ih = sd->size.imh;

   g->w = g->iw / g->zoom;
   g->h = g->ih / g->zoom;
   if (g->zoom >= 8)
     {
        free(g);

        return NULL;
     }
   if (sd->do_region)
     {
        g->gw = (g->w + g->tsize - 1) / g->tsize;
        g->gh = (g->h + g->tsize - 1) / g->tsize;
     }
   else
     {
        g->gw = 1;
        g->gh = 1;
     }

   g->grid = calloc(1, sizeof(Elm_Photocam_Grid_Item) * g->gw * g->gh);
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
             int tn;

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

             g->grid[tn].sd = sd;
             g->grid[tn].img =
               evas_object_image_add(evas_object_evas_get(obj));
             evas_object_image_load_orientation_set(g->grid[tn].img, EINA_TRUE);
             evas_object_image_scale_hint_set
               (g->grid[tn].img, EVAS_IMAGE_SCALE_HINT_DYNAMIC);
             evas_object_pass_events_set(g->grid[tn].img, EINA_TRUE);

             /* XXX: check this */
             evas_object_smart_member_add(g->grid[tn].img, sd->pan_obj);
             elm_widget_sub_object_add(obj, g->grid[tn].img);
             evas_object_image_filled_set(g->grid[tn].img, 1);
             evas_object_event_callback_add
               (g->grid[tn].img, EVAS_CALLBACK_IMAGE_PRELOADED,
               _tile_preloaded_cb, &(g->grid[tn]));
          }
     }

   return g;
}

static void
_grid_clear_all(Evas_Object *obj)
{
   Elm_Phocam_Grid *g;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   EINA_LIST_FREE (sd->grids, g)
     {
        _grid_clear(obj, g);
        free(g);
     }
}

static void
_smooth_update(Evas_Object *obj)
{
   Elm_Phocam_Grid *g;
   int x, y;
   Eina_List *l;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->grids, l, g)
     {
        for (y = 0; y < g->gh; y++)
          {
             for (x = 0; x < g->gw; x++)
               {
                  int tn;

                  tn = (y * g->gw) + x;
                  evas_object_image_smooth_scale_set
                    (g->grid[tn].img, (!sd->no_smooth));
               }
          }
     }

   evas_object_image_smooth_scale_set(sd->img, (!sd->no_smooth));
}

static void
_grid_raise(Elm_Phocam_Grid *g)
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

static Eina_Bool
_scroll_timeout_cb(void *data)
{
   ELM_PHOTOCAM_DATA_GET(data, sd);

   sd->no_smooth--;
   if (!sd->no_smooth) _smooth_update(data);

   sd->scr_timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_main_img_preloaded_cb(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *o __UNUSED__,
                       void *event_info __UNUSED__)
{
   Evas_Object *obj = data;
   Elm_Phocam_Grid *g;

   ELM_PHOTOCAM_DATA_GET(data, sd);

   evas_object_show(sd->img);
   sd->main_load_pending = 0;
   g = _grid_create(obj);
   if (g)
     {
        sd->grids = eina_list_prepend(sd->grids, g);
        _grid_load(obj, g);
     }
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, sd);
   evas_object_smart_callback_call(data, SIG_LOADED, NULL);
   sd->preload_num--;
   if (!sd->preload_num)
     {
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,state,busy,stop", "elm");
        evas_object_smart_callback_call(obj, SIG_LOADED_DETAIL, NULL);
     }
}

static Eina_Bool
_zoom_do(Evas_Object *obj,
         double t)
{
   Evas_Coord xx, yy, ow, oh;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   sd->size.w = (sd->size.ow * (1.0 - t)) + (sd->size.nw * t);
   sd->size.h = (sd->size.oh * (1.0 - t)) + (sd->size.nh * t);
   sd->s_iface->content_viewport_size_get(obj, &ow, &oh);
   xx = (sd->size.spos.x * sd->size.w) - (ow / 2);
   yy = (sd->size.spos.y * sd->size.h) - (oh / 2);
   if (xx < 0) xx = 0;
   else if (xx > (sd->size.w - ow))
     xx = sd->size.w - ow;
   if (yy < 0) yy = 0;
   else if (yy > (sd->size.h - oh))
     yy = sd->size.h - oh;

   sd->show.show = EINA_TRUE;
   sd->show.x = xx;
   sd->show.y = yy;
   sd->show.w = ow;
   sd->show.h = oh;

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, sd);
   if (t >= 1.0)
     {
        Eina_List *l, *l_next;
        Elm_Phocam_Grid *g;

        EINA_LIST_FOREACH_SAFE (sd->grids, l, l_next, g)
          {
             if (g->dead)
               {
                  sd->grids = eina_list_remove_list(sd->grids, l);
                  _grid_clear(obj, g);
                  free(g);
               }
          }
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_zoom_anim_cb(void *data)
{
   double t;
   Eina_Bool go;
   Evas_Object *obj = data;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   t = ecore_loop_time_get();
   if (t >= sd->t_end)
     t = 1.0;
   else if (sd->t_end > sd->t_start)
     t = (t - sd->t_start) / (sd->t_end - sd->t_start);
   else
     t = 1.0;
   t = 1.0 - t;
   t = 1.0 - (t * t);
   go = _zoom_do(obj, t);
   if (!go)
     {
        sd->no_smooth--;
        if (!sd->no_smooth) _smooth_update(data);
        sd->zoom_animator = NULL;
        evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }

   return go;
}

static Eina_Bool
_long_press_cb(void *data)
{
   ELM_PHOTOCAM_DATA_GET(data, sd);

   sd->long_timer = NULL;
   sd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(data, SIG_LONGPRESSED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(void *data,
               Evas *evas __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   ELM_PHOTOCAM_DATA_GET(data, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(data, SIG_CLICKED_DOUBLE, NULL);
   else
     evas_object_smart_callback_call(data, SIG_PRESS, NULL);
   sd->longpressed = EINA_FALSE;
   if (sd->long_timer) ecore_timer_del(sd->long_timer);
   sd->long_timer = ecore_timer_add
       (_elm_config->longpress_timeout, _long_press_cb, data);
}

static void
_mouse_up_cb(void *data,
             Evas *evas __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;

   ELM_PHOTOCAM_DATA_GET(data, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;
   if (sd->long_timer)
     {
        ecore_timer_del(sd->long_timer);
        sd->long_timer = NULL;
     }
   if (!sd->on_hold)
     evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
   sd->on_hold = EINA_FALSE;
}

static Eina_Bool
_elm_photocam_smart_on_focus(Evas_Object *obj)
{
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,action,focus", "elm");
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_FALSE);
     }

   return EINA_TRUE;
}

static Eina_Bool
_elm_photocam_smart_theme(Evas_Object *obj)
{
   if (!ELM_WIDGET_CLASS(_elm_photocam_parent_sc)->theme(obj))
     return EINA_FALSE;

   _sizing_eval(obj);

   return EINA_TRUE;
}

static void
_scroll_animate_start_cb(Evas_Object *obj,
                         void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scroll_animate_stop_cb(Evas_Object *obj,
                        void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_STOP, NULL);
}

static void
_scroll_cb(Evas_Object *obj,
           void *data __UNUSED__)
{
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if (!sd->scr_timer)
     {
        sd->no_smooth++;
        if (sd->no_smooth == 1) _smooth_update(obj);
     }

   if (sd->scr_timer) ecore_timer_del(sd->scr_timer);
   sd->scr_timer = ecore_timer_add(0.5, _scroll_timeout_cb, obj);

   evas_object_smart_callback_call(obj, SIG_SCROLL, NULL);
}

static Eina_Bool
_elm_photocam_smart_event(Evas_Object *obj,
                          Evas_Object *src __UNUSED__,
                          Evas_Callback_Type type,
                          void *event_info)
{
   double zoom;
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;
   Evas_Event_Key_Down *ev = event_info;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   sd->s_iface->content_pos_get(obj, &x, &y);
   sd->s_iface->step_size_get(obj, &step_x, &step_y);
   sd->s_iface->page_size_get(obj, &page_x, &page_y);
   sd->s_iface->content_viewport_size_get(obj, &v_w, &v_h);

   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        y += step_y;
     }
   else if ((!strcmp(ev->keyname, "Prior")) ||
            ((!strcmp(ev->keyname, "KP_Prior")) && (!ev->string)))
     {
        if (page_y < 0)
          y -= -(page_y * v_h) / 100;
        else
          y -= page_y;
     }
   else if ((!strcmp(ev->keyname, "Next")) ||
            ((!strcmp(ev->keyname, "KP_Next")) && (!ev->string)))
     {
        if (page_y < 0)
          y += -(page_y * v_h) / 100;
        else
          y += page_y;
     }
   else if ((!strcmp(ev->keyname, "KP_Add")))
     {
        zoom = elm_photocam_zoom_get(obj);
        zoom -= 0.5;
        elm_photocam_zoom_mode_set(obj, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
        elm_photocam_zoom_set(obj, zoom);
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "KP_Subtract")))
     {
        zoom = elm_photocam_zoom_get(obj);
        zoom += 0.5;
        elm_photocam_zoom_mode_set(obj, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
        elm_photocam_zoom_set(obj, zoom);
        return EINA_TRUE;
     }
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   sd->s_iface->content_pos_set(obj, x, y);

   return EINA_TRUE;
}

Eina_Bool
_bounce_eval(void *data)
{
   Elm_Photocam_Smart_Data *sd = data;
   double t, tt;

   if ((sd->g_layer_zoom.imx == sd->g_layer_zoom.bounce.x_end) &&
       (sd->g_layer_zoom.imy == sd->g_layer_zoom.bounce.y_end))
     {
        sd->g_layer_zoom.imx = 0;
        sd->g_layer_zoom.imy = 0;
        sd->zoom_g_layer = EINA_FALSE;
        sd->g_layer_zoom.bounce.animator = NULL;

        sd->s_iface->freeze_set(ELM_WIDGET_DATA(sd)->obj, EINA_FALSE);

        return ECORE_CALLBACK_CANCEL;
     }

   t = ecore_loop_time_get();
   tt = (t - sd->g_layer_zoom.bounce.t_start) /
     (sd->g_layer_zoom.bounce.t_end -
      sd->g_layer_zoom.bounce.t_start);
   tt = 1.0 - tt;
   tt = 1.0 - (tt * tt);

   if (t > sd->g_layer_zoom.bounce.t_end)
     {
        sd->g_layer_zoom.imx = 0;
        sd->g_layer_zoom.imy = 0;
        sd->zoom_g_layer = EINA_FALSE;

        sd->s_iface->freeze_set(ELM_WIDGET_DATA(sd)->obj, EINA_FALSE);

        _zoom_do(ELM_WIDGET_DATA(sd)->obj, 1.0);
        sd->g_layer_zoom.bounce.animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   if (sd->g_layer_zoom.imx != sd->g_layer_zoom.bounce.x_end)
     sd->g_layer_zoom.imx =
       sd->g_layer_zoom.bounce.x_start * (1.0 - tt) +
       sd->g_layer_zoom.bounce.x_end * tt;

   if (sd->g_layer_zoom.imy != sd->g_layer_zoom.bounce.y_end)
     sd->g_layer_zoom.imy =
       sd->g_layer_zoom.bounce.y_start * (1.0 - tt) +
       sd->g_layer_zoom.bounce.y_end * tt;

   _zoom_do(ELM_WIDGET_DATA(sd)->obj, 1.0 - (1.0 - tt));

   return ECORE_CALLBACK_RENEW;
}

static void
_g_layer_zoom_do(Elm_Photocam_Smart_Data *sd,
                 Evas_Coord px,
                 Evas_Coord py,
                 Elm_Gesture_Zoom_Info *g_layer)
{
   int regx, regy, regw, regh, ix, iy, iw, ih;
   Evas_Coord rx, ry, rw, rh;
   Evas_Object *obj;
   int xx, yy;

   obj = ELM_WIDGET_DATA(sd)->obj;
   sd->mode = ELM_PHOTOCAM_ZOOM_MODE_MANUAL;
   sd->zoom = sd->g_layer_start / g_layer->zoom;
   sd->size.ow = sd->size.w;
   sd->size.oh = sd->size.h;
   sd->s_iface->content_pos_get(obj, &rx, &ry);
   sd->s_iface->content_viewport_size_get(obj, &rw, &rh);
   if ((rw <= 0) || (rh <= 0)) return;

   sd->size.nw = (double)sd->size.imw / sd->zoom;
   sd->size.nh = (double)sd->size.imh / sd->zoom;

   elm_photocam_image_region_get(obj, &regx, &regy, &regw, &regh);
   evas_object_geometry_get(sd->img, &ix, &iy, &iw, &ih);

   sd->pvx = g_layer->x;
   sd->pvy = g_layer->y;

   xx = (px / sd->zoom) - sd->pvx;
   yy = (py / sd->zoom) - sd->pvy;
   sd->g_layer_zoom.imx = 0;
   sd->g_layer_zoom.imy = 0;

   if ((xx < 0) || (rw > sd->size.nw))
     {
        sd->g_layer_zoom.imx = xx;
        xx = 0;
     }
   else if ((xx + rw) > sd->size.nw)
     {
        sd->g_layer_zoom.imx = xx + rw - sd->size.nw;
        xx = sd->size.nw - rw;
     }

   if ((yy < 0) || (rh > sd->size.nh))
     {
        sd->g_layer_zoom.imy = yy;
        yy = 0;
     }
   else if ((yy + rh) > sd->size.nh)
     {
        sd->g_layer_zoom.imy = yy + rh - sd->size.nh;
        yy = sd->size.nh - rh;
     }

   sd->size.spos.x = (double)(xx + (rw / 2)) / (double)(sd->size.nw);
   sd->size.spos.y = (double)(yy + (rh / 2)) / (double)(sd->size.nh);

   _zoom_do(obj, 1.0);
}

static Evas_Event_Flags
_g_layer_zoom_start_cb(void *data,
                       void *event_info)
{
   Elm_Gesture_Zoom_Info *p = event_info;
   Elm_Photocam_Smart_Data *sd = data;
   double marginx = 0, marginy = 0;
   Evas_Coord rw, rh;
   int x, y, w, h;

   if (sd->g_layer_zoom.bounce.animator)
     {
        ecore_animator_del(sd->g_layer_zoom.bounce.animator);
        sd->g_layer_zoom.bounce.animator = NULL;
     }
   sd->zoom_g_layer = EINA_TRUE;

   sd->s_iface->freeze_set(ELM_WIDGET_DATA(sd)->obj, EINA_TRUE);

   elm_photocam_image_region_get(ELM_WIDGET_DATA(sd)->obj, &x, &y, &w, &h);
   sd->s_iface->content_viewport_size_get(ELM_WIDGET_DATA(sd)->obj, &rw, &rh);

   if (rw > sd->size.nw)
     marginx = (rw - sd->size.nw) / 2;
   if (rh > sd->size.nh)
     marginy = (rh - sd->size.nh) / 2;

   sd->g_layer_start = sd->zoom;

   sd->zoom_point_x = x + ((p->x - marginx) * sd->zoom) +
     sd->g_layer_zoom.imx;
   sd->zoom_point_y = y + ((p->y - marginy) * sd->zoom) +
     sd->g_layer_zoom.imy;

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_g_layer_zoom_move_cb(void *data,
                      void *event_info)
{
   Elm_Photocam_Smart_Data *sd = data;
   Elm_Gesture_Zoom_Info *p = event_info;

   _g_layer_zoom_do(sd, sd->zoom_point_x, sd->zoom_point_y, p);

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_g_layer_zoom_end_cb(void *data,
                     void *event_info __UNUSED__)
{
   Elm_Photocam_Smart_Data *sd = data;
   Evas_Coord rw, rh;

   sd->s_iface->content_viewport_size_get(ELM_WIDGET_DATA(sd)->obj, &rw, &rh);
   sd->g_layer_start = 1.0;

   if (sd->g_layer_zoom.imx || sd->g_layer_zoom.imy)
     {
        double t;

        t = ecore_loop_time_get();
        sd->g_layer_zoom.bounce.x_start = sd->g_layer_zoom.imx;
        sd->g_layer_zoom.bounce.y_start = sd->g_layer_zoom.imy;
        sd->g_layer_zoom.bounce.x_end = 0;
        sd->g_layer_zoom.bounce.y_end = 0;

        if (rw > sd->size.nw &&
            rh > sd->size.nh)
          {
             Evas_Coord pw, ph;
             double z;

             if ((sd->size.imw < rw) && (sd->size.imh < rh))
               {
                  sd->zoom = 1;
                  sd->size.nw = sd->size.imw;
                  sd->size.nh = sd->size.imh;
               }
             else
               {
                  ph = (sd->size.imh * rw) / sd->size.imw;
                  if (ph > rh)
                    {
                       pw = (sd->size.imw * rh) / sd->size.imh;
                       ph = rh;
                    }
                  else
                    {
                       pw = rw;
                    }
                  if (sd->size.imw > sd->size.imh)
                    z = (double)sd->size.imw / pw;
                  else
                    z = (double)sd->size.imh / ph;

                  sd->zoom = z;
                  sd->size.nw = pw;
                  sd->size.nh = ph;
               }
             sd->g_layer_zoom.bounce.x_end = (sd->size.nw - rw) / 2;
             sd->g_layer_zoom.bounce.y_end = (sd->size.nh - rh) / 2;
          }
        else
          {
             int xx, yy;

             xx = (sd->zoom_point_x / sd->zoom) - sd->pvx;
             yy = (sd->zoom_point_y / sd->zoom) - sd->pvy;

             if (xx < 0) xx = 0;
             if (yy < 0) yy = 0;

             if (rw > sd->size.nw)
               sd->g_layer_zoom.bounce.x_end = (sd->size.nw - rw) / 2;
             if ((xx + rw) > sd->size.nw)
               xx = sd->size.nw - rw;

             if (rh > sd->size.nh)
               sd->g_layer_zoom.bounce.y_end = (sd->size.nh - rh) / 2;
             if ((yy + rh) > sd->size.nh)
               yy = sd->size.nh - rh;

             sd->size.spos.x = (double)(xx + (rw / 2)) / (double)(sd->size.nw);
             sd->size.spos.y = (double)(yy + (rh / 2)) / (double)(sd->size.nh);
          }

        sd->g_layer_zoom.bounce.t_start = t;
        sd->g_layer_zoom.bounce.t_end = t +
          _elm_config->page_scroll_friction;

        sd->g_layer_zoom.bounce.animator =
          ecore_animator_add(_bounce_eval, sd);
     }
   else
     {
        sd->s_iface->freeze_set(ELM_WIDGET_DATA(sd)->obj, EINA_FALSE);
        sd->zoom_g_layer = EINA_FALSE;
     }

   return EVAS_EVENT_FLAG_NONE;
}

static void
_elm_photocam_smart_add(Evas_Object *obj)
{
   Evas_Coord minw, minh;
   Elm_Photocam_Pan_Smart_Data *pan_data;
   Eina_Bool bounce = _elm_config->thumbscroll_bounce_enable;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Photocam_Smart_Data);

   ELM_WIDGET_DATA(priv)->resize_obj =
     edje_object_add(evas_object_evas_get(obj));

   ELM_WIDGET_CLASS(_elm_photocam_parent_sc)->base.add(obj);

   elm_widget_theme_object_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, "photocam", "base",
     elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->s_iface = evas_object_smart_interface_get
       (obj, ELM_SCROLLABLE_IFACE_NAME);

   priv->s_iface->objects_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, priv->hit_rect);

   priv->s_iface->animate_start_cb_set(obj, _scroll_animate_start_cb);
   priv->s_iface->animate_stop_cb_set(obj, _scroll_animate_stop_cb);
   priv->s_iface->drag_start_cb_set(obj, _scroll_drag_start_cb);
   priv->s_iface->drag_stop_cb_set(obj, _scroll_drag_stop_cb);
   priv->s_iface->scroll_cb_set(obj, _scroll_cb);

   priv->s_iface->bounce_allow_set(obj, bounce, bounce);

   priv->pan_obj = evas_object_smart_add
       (evas_object_evas_get(obj), _elm_photocam_pan_smart_class_new());
   pan_data = evas_object_smart_data_get(priv->pan_obj);
   pan_data->wsd = priv;

   priv->s_iface->extern_pan_set(obj, priv->pan_obj);

   priv->zoom_g_layer = EINA_FALSE;
   priv->g_layer_start = 1.0;
   priv->zoom = 1;
   priv->mode = ELM_PHOTOCAM_ZOOM_MODE_MANUAL;
   priv->tsize = 512;

   priv->img = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_image_load_orientation_set(priv->img, EINA_TRUE);
   evas_object_image_scale_hint_set(priv->img, EVAS_IMAGE_SCALE_HINT_DYNAMIC);
   evas_object_event_callback_add
     (priv->img, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, obj);
   evas_object_event_callback_add
     (priv->img, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, obj);
   evas_object_image_scale_hint_set(priv->img, EVAS_IMAGE_SCALE_HINT_STATIC);

   /* XXX: mmm... */
   evas_object_smart_member_add(priv->img, priv->pan_obj);

   elm_widget_sub_object_add(obj, priv->img);
   evas_object_image_filled_set(priv->img, EINA_TRUE);
   evas_object_event_callback_add
     (priv->img, EVAS_CALLBACK_IMAGE_PRELOADED, _main_img_preloaded_cb, obj);

   edje_object_size_min_calc(ELM_WIDGET_DATA(priv)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   _sizing_eval(obj);

}

static void
_elm_photocam_smart_del(Evas_Object *obj)
{
   Elm_Phocam_Grid *g;

   ELM_PHOTOCAM_DATA_GET(obj, sd);

   EINA_LIST_FREE (sd->grids, g)
     {
        if (g->grid) free(g->grid);
        free(g);
     }
   evas_object_del(sd->pan_obj);
   sd->pan_obj = NULL;

   if (sd->file) eina_stringshare_del(sd->file);
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   if (sd->scr_timer) ecore_timer_del(sd->scr_timer);
   if (sd->zoom_animator) ecore_animator_del(sd->zoom_animator);
   if (sd->g_layer_zoom.bounce.animator)
     ecore_animator_del(sd->g_layer_zoom.bounce.animator);
   if (sd->long_timer) ecore_timer_del(sd->long_timer);

   ELM_WIDGET_CLASS(_elm_photocam_parent_sc)->base.del(obj);
}

static void
_elm_photocam_smart_move(Evas_Object *obj,
                         Evas_Coord x,
                         Evas_Coord y)
{
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_photocam_parent_sc)->base.move(obj, x, y);

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_photocam_smart_resize(Evas_Object *obj,
                           Evas_Coord w,
                           Evas_Coord h)
{
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_photocam_parent_sc)->base.resize(obj, w, h);

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_photocam_smart_member_add(Evas_Object *obj,
                               Evas_Object *member)
{
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_photocam_parent_sc)->base.member_add(obj, member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_elm_photocam_smart_set_user(Elm_Photocam_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_photocam_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_photocam_smart_del;
   ELM_WIDGET_CLASS(sc)->base.move = _elm_photocam_smart_move;
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_photocam_smart_resize;
   ELM_WIDGET_CLASS(sc)->base.member_add = _elm_photocam_smart_member_add;

   ELM_WIDGET_CLASS(sc)->on_focus = _elm_photocam_smart_on_focus;
   ELM_WIDGET_CLASS(sc)->theme = _elm_photocam_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_photocam_smart_event;
}

EAPI const Elm_Photocam_Smart_Class *
elm_photocam_smart_class_get(void)
{
   static Elm_Photocam_Smart_Class _sc =
     ELM_PHOTOCAM_SMART_CLASS_INIT_NAME_VERSION(ELM_PHOTOCAM_SMART_NAME);
   static const Elm_Photocam_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_photocam_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_photocam_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_photocam_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Evas_Load_Error
elm_photocam_file_set(Evas_Object *obj,
                      const char *file)
{
   int w, h;
   double tz;

   ELM_PHOTOCAM_CHECK(obj) EVAS_LOAD_ERROR_NONE;
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if (!eina_stringshare_replace(&sd->file, file)) return EVAS_LOAD_ERROR_NONE;
   _grid_clear_all(obj);

   evas_object_hide(sd->img);
   evas_object_image_smooth_scale_set(sd->img, (sd->no_smooth == 0));
   evas_object_image_file_set(sd->img, NULL, NULL);
   evas_object_image_load_scale_down_set(sd->img, 0);
   evas_object_image_file_set(sd->img, sd->file, NULL);
   evas_object_image_size_get(sd->img, &w, &h);

   sd->do_region = evas_object_image_region_support_get(sd->img);
   sd->size.imw = w;
   sd->size.imh = h;
   sd->size.w = sd->size.imw / sd->zoom;
   sd->size.h = sd->size.imh / sd->zoom;
   if (sd->g_layer_zoom.bounce.animator)
     {
        ecore_animator_del(sd->g_layer_zoom.bounce.animator);
        sd->g_layer_zoom.bounce.animator = NULL;
     }
   if (sd->zoom_animator)
     {
        sd->no_smooth--;
        if (sd->no_smooth == 0) _smooth_update(obj);
        ecore_animator_del(sd->zoom_animator);
        sd->zoom_animator = NULL;
     }
   evas_object_image_file_set(sd->img, NULL, NULL);
   evas_object_image_load_scale_down_set(sd->img, 8);
   evas_object_image_file_set(sd->img, sd->file, NULL);
   evas_object_image_preload(sd->img, 0);
   sd->main_load_pending = EINA_TRUE;

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, sd);
   evas_object_smart_callback_call(obj, SIG_LOAD, NULL);
   sd->preload_num++;
   if (sd->preload_num == 1)
     {
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,state,busy,start", "elm");
        evas_object_smart_callback_call(obj, SIG_LOAD_DETAIL, NULL);
     }

   tz = sd->zoom;
   sd->zoom = 0.0;
   elm_photocam_zoom_set(ELM_WIDGET_DATA(sd)->obj, tz);

   return evas_object_image_load_error_get(sd->img);
}

EAPI const char *
elm_photocam_file_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) NULL;
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   return sd->file;
}

EAPI void
elm_photocam_zoom_set(Evas_Object *obj,
                      double zoom)
{
   double z;
   Eina_List *l;
   Ecore_Animator *an;
   Elm_Phocam_Grid *g, *g_zoom = NULL;
   Evas_Coord pw, ph, rx, ry, rw, rh;
   int zoom_changed = 0, started = 0;

   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if (zoom <= (1.0 / 256.0)) zoom = (1.0 / 256.0);
   if (zoom == sd->zoom) return;

   sd->zoom = zoom;
   sd->size.ow = sd->size.w;
   sd->size.oh = sd->size.h;
   sd->s_iface->content_pos_get(obj, &rx, &ry);
   sd->s_iface->content_viewport_size_get(obj, &rw, &rh);
   if ((rw <= 0) || (rh <= 0)) return;

   if (sd->mode == ELM_PHOTOCAM_ZOOM_MODE_MANUAL)
     {
        sd->size.nw = (double)sd->size.imw / sd->zoom;
        sd->size.nh = (double)sd->size.imh / sd->zoom;
     }
   else if (sd->mode == ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT)
     {
        if ((sd->size.imw < 1) || (sd->size.imh < 1))
          {
             sd->size.nw = 0;
             sd->size.nh = 0;
          }
        else
          {
             ph = (sd->size.imh * rw) / sd->size.imw;
             if (ph > rh)
               {
                  pw = (sd->size.imw * rh) / sd->size.imh;
                  ph = rh;
               }
             else
               {
                  pw = rw;
               }
             if (sd->size.imw > sd->size.imh)
               z = (double)sd->size.imw / pw;
             else
               z = (double)sd->size.imh / ph;
             if (z != sd->zoom)
               zoom_changed = 1;
             sd->zoom = z;
             sd->size.nw = pw;
             sd->size.nh = ph;
          }
     }
   else if (sd->mode == ELM_PHOTOCAM_ZOOM_MODE_AUTO_FILL)
     {
        if ((sd->size.imw < 1) || (sd->size.imh < 1))
          {
             sd->size.nw = 0;
             sd->size.nw = 0;
          }
        else
          {
             ph = (sd->size.imh * rw) / sd->size.imw;
             if (ph < rh)
               {
                  pw = (sd->size.imw * rh) / sd->size.imh;
                  ph = rh;
               }
             else
               {
                  pw = rw;
               }
             if (sd->size.imw > sd->size.imh)
               z = (double)sd->size.imw / pw;
             else
               z = (double)sd->size.imh / ph;
             if (z != sd->zoom)
               zoom_changed = 1;
             sd->zoom = z;
             sd->size.nw = pw;
             sd->size.nh = ph;
          }
     }
   else if (sd->mode == ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT_IN)
     {
        if ((sd->size.imw < 1) || (sd->size.imh < 1))
          {
             sd->size.nw = 0;
             sd->size.nh = 0;
          }
        else if ((sd->size.imw < rw) && (sd->size.imh < rh))
          {
             if (1 != sd->zoom) zoom_changed = 1;
             sd->zoom = 1;
             sd->size.nw = sd->size.imw;
             sd->size.nh = sd->size.imh;
          }
        else
          {
             ph = (sd->size.imh * rw) / sd->size.imw;
             if (ph > rh)
               {
                  pw = (sd->size.imw * rh) / sd->size.imh;
                  ph = rh;
               }
             else
               pw = rw;
             if (sd->size.imw > sd->size.imh)
               z = (double)sd->size.imw / pw;
             else
               z = (double)sd->size.imh / ph;
             if (z != sd->zoom)
               zoom_changed = 1;
             sd->zoom = z;
             sd->size.nw = pw;
             sd->size.nh = ph;
          }
     }

   if (sd->main_load_pending)
     {
        sd->size.w = sd->size.nw;
        sd->size.h = sd->size.nh;

        goto done;
     }

   EINA_LIST_FOREACH (sd->grids, l, g)
     {
        if (g->zoom == _grid_zoom_calc(sd->zoom))
          {
             sd->grids = eina_list_remove(sd->grids, g);
             sd->grids = eina_list_prepend(sd->grids, g);
             _grid_raise(g);
             goto done;
          }
     }

   g = _grid_create(obj);
   if (g)
     {
        if (eina_list_count(sd->grids) > 1)
          {
             g_zoom = eina_list_last(sd->grids)->data;
             sd->grids = eina_list_remove(sd->grids, g_zoom);
             _grid_clear(obj, g_zoom);
             free(g_zoom);
             EINA_LIST_FOREACH (sd->grids, l, g_zoom)
               {
                  g_zoom->dead = 1;
               }
          }
        sd->grids = eina_list_prepend(sd->grids, g);
     }
   else
     {
        EINA_LIST_FREE (sd->grids, g)
          {
             _grid_clear(obj, g);
             free(g);
          }
     }

done:
   sd->t_start = ecore_loop_time_get();
   sd->t_end = sd->t_start + _elm_config->zoom_friction;
   if ((sd->size.w > 0) && (sd->size.h > 0))
     {
        sd->size.spos.x = (double)(rx + (rw / 2)) / (double)sd->size.w;
        sd->size.spos.y = (double)(ry + (rh / 2)) / (double)sd->size.h;
     }
   else
     {
        sd->size.spos.x = 0.5;
        sd->size.spos.y = 0.5;
     }
   if (rw > sd->size.w) sd->size.spos.x = 0.5;
   if (rh > sd->size.h) sd->size.spos.y = 0.5;
   if (sd->size.spos.x > 1.0) sd->size.spos.x = 1.0;
   if (sd->size.spos.y > 1.0) sd->size.spos.y = 1.0;

   if (sd->paused)
     {
        _zoom_do(obj, 1.0);
     }
   else
     {
        if (!sd->zoom_animator)
          {
             sd->zoom_animator = ecore_animator_add(_zoom_anim_cb, obj);
             sd->no_smooth++;
             if (sd->no_smooth == 1) _smooth_update(obj);
             started = 1;
          }
     }

   an = sd->zoom_animator;
   if (an)
     {
        if (!_zoom_anim_cb(obj))
          {
             ecore_animator_del(an);
             an = NULL;
          }
     }

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, sd);
   if (!sd->paused)
     {
        if (started)
          evas_object_smart_callback_call(obj, SIG_ZOOM_START, NULL);
        if (!an)
          evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }
   if (zoom_changed)
     evas_object_smart_callback_call(obj, SIG_ZOOM_CHANGE, NULL);
}

EAPI double
elm_photocam_zoom_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) 1.0;
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   return sd->zoom;
}

EAPI void
elm_photocam_zoom_mode_set(Evas_Object *obj,
                           Elm_Photocam_Zoom_Mode mode)
{
   double tz;

   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if (sd->mode == mode) return;
   sd->mode = mode;

   tz = sd->zoom;
   sd->zoom = 0.0;
   elm_photocam_zoom_set(ELM_WIDGET_DATA(sd)->obj, tz);
}

EAPI Elm_Photocam_Zoom_Mode
elm_photocam_zoom_mode_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) ELM_PHOTOCAM_ZOOM_MODE_LAST;
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   return sd->mode;
}

EAPI void
elm_photocam_image_size_get(const Evas_Object *obj,
                            int *w,
                            int *h)
{
   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if (w) *w = sd->size.imw;
   if (h) *h = sd->size.imh;
}

EAPI void
elm_photocam_image_region_get(const Evas_Object *obj,
                              int *x,
                              int *y,
                              int *w,
                              int *h)
{
   Evas_Coord sx, sy, sw, sh;

   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   sd->s_iface->content_pos_get(obj, &sx, &sy);
   sd->s_iface->content_viewport_size_get(obj, &sw, &sh);
   if (sd->size.w > 0)
     {
        if (x)
          {
             *x = (sd->size.imw * sx) / sd->size.w;
             if (*x > sd->size.imw) *x = sd->size.imw;
          }
        if (w)
          {
             *w = (sd->size.imw * sw) / sd->size.w;
             if (*w > sd->size.imw) *w = sd->size.imw;
             else if (*w < 0)
               *w = 0;
          }
     }
   else
     {
        if (x) *x = 0;
        if (w) *w = 0;
     }

   if (sd->size.h > 0)
     {
        if (y)
          {
             *y = (sd->size.imh * sy) / sd->size.h;
             if (*y > sd->size.imh) *y = sd->size.imh;
          }
        if (h)
          {
             *h = (sd->size.imh * sh) / sd->size.h;
             if (*h > sd->size.imh) *h = sd->size.imh;
             else if (*h < 0)
               *h = 0;
          }
     }
   else
     {
        if (y) *y = 0;
        if (h) *h = 0;
     }
}

EAPI void
elm_photocam_image_region_show(Evas_Object *obj,
                               int x,
                               int y,
                               int w,
                               int h __UNUSED__)
{
   int rx, ry, rw, rh;

   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if ((sd->size.imw < 1) || (sd->size.imh < 1)) return;
   rx = (x * sd->size.w) / sd->size.imw;
   ry = (y * sd->size.h) / sd->size.imh;
   rw = (w * sd->size.w) / sd->size.imw;
   rh = (h * sd->size.h) / sd->size.imh;
   if (rw < 1) rw = 1;
   if (rh < 1) rh = 1;
   if ((rx + rw) > sd->size.w) rx = sd->size.w - rw;
   if ((ry + rh) > sd->size.h) ry = sd->size.h - rh;
   if (sd->g_layer_zoom.bounce.animator)
     {
        ecore_animator_del(sd->g_layer_zoom.bounce.animator);
        sd->g_layer_zoom.bounce.animator = NULL;
        _zoom_do(obj, 1.0);
     }
   if (sd->zoom_animator)
     {
        sd->no_smooth--;
        ecore_animator_del(sd->zoom_animator);
        sd->zoom_animator = NULL;
        _zoom_do(obj, 1.0);
        evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }
   sd->s_iface->content_region_show(obj, rx, ry, rw, rh);
}

EAPI void
elm_photocam_image_region_bring_in(Evas_Object *obj,
                                   int x,
                                   int y,
                                   int w,
                                   int h __UNUSED__)
{
   int rx, ry, rw, rh;

   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   if ((sd->size.imw < 1) || (sd->size.imh < 1)) return;
   rx = (x * sd->size.w) / sd->size.imw;
   ry = (y * sd->size.h) / sd->size.imh;
   rw = (w * sd->size.w) / sd->size.imw;
   rh = (h * sd->size.h) / sd->size.imh;
   if (rw < 1) rw = 1;
   if (rh < 1) rh = 1;
   if ((rx + rw) > sd->size.w) rx = sd->size.w - rw;
   if ((ry + rh) > sd->size.h) ry = sd->size.h - rh;
   if (sd->g_layer_zoom.bounce.animator)
     {
        ecore_animator_del(sd->g_layer_zoom.bounce.animator);
        sd->g_layer_zoom.bounce.animator = NULL;
        _zoom_do(obj, 1.0);
     }
   if (sd->zoom_animator)
     {
        sd->no_smooth--;
        if (!sd->no_smooth) _smooth_update(obj);
        ecore_animator_del(sd->zoom_animator);
        sd->zoom_animator = NULL;
        _zoom_do(obj, 1.0);
        evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }
   sd->s_iface->region_bring_in(obj, rx, ry, rw, rh);
}

EAPI void
elm_photocam_paused_set(Evas_Object *obj,
                        Eina_Bool paused)
{
   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   paused = !!paused;

   if (sd->paused == paused) return;
   sd->paused = paused;
   if (!sd->paused) return;

   if (sd->g_layer_zoom.bounce.animator)
     {
        ecore_animator_del(sd->g_layer_zoom.bounce.animator);
        sd->g_layer_zoom.bounce.animator = NULL;
        _zoom_do(obj, 1.0);
     }
   if (sd->zoom_animator)
     {
        ecore_animator_del(sd->zoom_animator);
        sd->zoom_animator = NULL;
        _zoom_do(obj, 1.0);
        evas_object_smart_callback_call(obj, SIG_ZOOM_STOP, NULL);
     }
}

EAPI Eina_Bool
elm_photocam_paused_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) EINA_FALSE;
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   return sd->paused;
}

EAPI Evas_Object *
elm_photocam_internal_image_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) NULL;
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   return sd->img;
}

EAPI void
elm_photocam_bounce_set(Evas_Object *obj,
                        Eina_Bool h_bounce,
                        Eina_Bool v_bounce)
{
   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   sd->s_iface->bounce_allow_set(obj, h_bounce, v_bounce);
}

EAPI void
elm_photocam_bounce_get(const Evas_Object *obj,
                        Eina_Bool *h_bounce,
                        Eina_Bool *v_bounce)
{
   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   sd->s_iface->bounce_allow_get(obj, h_bounce, v_bounce);
}

EAPI void
elm_photocam_gesture_enabled_set(Evas_Object *obj,
                                 Eina_Bool gesture)
{
   ELM_PHOTOCAM_CHECK(obj);
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   gesture = !!gesture;

   if (sd->do_gesture == gesture) return;

   sd->do_gesture = gesture;

   if (sd->g_layer)
     {
        evas_object_del(sd->g_layer);
        sd->g_layer = NULL;
     }

   if (!gesture) return;

   sd->g_layer = elm_gesture_layer_add(obj);
   if (!sd->g_layer) return;

   elm_gesture_layer_attach(sd->g_layer, obj);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START,
     _g_layer_zoom_start_cb, sd);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE,
     _g_layer_zoom_move_cb, sd);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_END,
     _g_layer_zoom_end_cb, sd);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_ABORT,
     _g_layer_zoom_end_cb, sd);
}

EAPI Eina_Bool
elm_photocam_gesture_enabled_get(const Evas_Object *obj)
{
   ELM_PHOTOCAM_CHECK(obj) EINA_FALSE;
   ELM_PHOTOCAM_DATA_GET(obj, sd);

   return sd->do_gesture;
}
