#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_image_zoomable_private.h"

#define MY_PAN_CLASS EFL_UI_IMAGE_ZOOMABLE_PAN_CLASS

#define MY_PAN_CLASS_NAME "Efl.Ui.Image_Zoomable_Pan"
#define MY_PAN_CLASS_NAME_LEGACY "elm_photocam_pan"

#define MY_CLASS EFL_UI_IMAGE_ZOOMABLE_CLASS

#define MY_CLASS_NAME "Efl.Ui.Image_Zoomable"

/*
 * TODO (maybe - optional future stuff):
 *
 * 1. wrap photo in theme edje so u can have styling around photo (like white
 *    photo bordering).
 * 2. exif handling
 * 3. rotation flags in exif handling (nasty! should have rot in evas)
 */

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
static const char SIG_DOWNLOAD_START[] = "download,start";
static const char SIG_DOWNLOAD_PROGRESS[] = "download,progress";
static const char SIG_DOWNLOAD_DONE[] = "download,done";
static const char SIG_DOWNLOAD_ERROR[] = "download,error";
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
   {SIG_DOWNLOAD_START, ""},
   {SIG_DOWNLOAD_PROGRESS, ""},
   {SIG_DOWNLOAD_DONE, ""},
   {SIG_DOWNLOAD_ERROR, ""},
   {SIG_WIDGET_FOCUSED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_UNFOCUSED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static Eina_Error PHOTO_FILE_LOAD_ERROR_GENERIC;
static Eina_Error PHOTO_FILE_LOAD_ERROR_DOES_NOT_EXIST;
static Eina_Error PHOTO_FILE_LOAD_ERROR_PERMISSION_DENIED;
static Eina_Error PHOTO_FILE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
static Eina_Error PHOTO_FILE_LOAD_ERROR_CORRUPT_FILE;
static Eina_Error PHOTO_FILE_LOAD_ERROR_UNKNOWN_FORMAT;

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_zoom(Evas_Object *obj, const char *params);
static void _efl_ui_image_zoomable_remote_copier_cancel(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd);
static Eina_Bool _internal_efl_ui_image_zoomable_icon_set(Evas_Object *obj, const char *name, Eina_Bool *fdo, Eina_Bool resize);
static void _min_obj_size_get(Evas_Object *o, int *w, int *h);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"zoom", _key_action_zoom},
   {NULL, NULL}
};

static const char *icon_theme = NULL;
#define NON_EXISTING (void *)-1

static inline void
_photocam_image_file_set(Evas_Object *obj, Efl_Ui_Image_Zoomable_Data *sd)
{
   if (sd->f)
     evas_object_image_mmap_set(obj, sd->f, NULL);
   else
     evas_object_image_file_set(obj, sd->file, NULL);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = 0, minh = 0, maxw = -1, maxh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_size_hint_max_get
     (wd->resize_obj, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_calc_job_cb(void *data)
{
   Evas_Object *obj = data;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
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
             elm_photocam_zoom_set(obj, tz);
          }
     }
   if ((minw != sd->minw) || (minh != sd->minh))
     {
        sd->minw = minw;
        sd->minh = minh;

        efl_event_callback_call(sd->pan_obj, EFL_UI_PAN_EVENT_PAN_CONTENT_CHANGED, NULL);
        _sizing_eval(obj);
     }
   sd->calc_job = NULL;
   evas_object_smart_changed(sd->pan_obj);
}

EOLIAN static void
_efl_ui_image_zoomable_pan_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Image_Zoomable_Pan_Data *psd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_PAN_CLASS), pos);

   ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job_cb, psd->wobj);
}

EOLIAN static void
_efl_ui_image_zoomable_pan_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Image_Zoomable_Pan_Data *psd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_PAN_CLASS), sz);

   psd->wsd->resized = EINA_TRUE;
   ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job_cb, psd->wobj);
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

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   ax = 0;
   ay = 0;
   gw = sd->size.w;
   gh = sd->size.h;
   if (!sd->zoom_g_layer)
     {
        if (ow > gw) ax = (ow - gw) / 2;
        if (oh > gh) ay = (oh - gh) / 2;
     }
   evas_object_geometry_set(sd->img, ox + 0 - px + ax, oy + 0 - py + ay, gw, gh);

   if (sd->show_item)
     {
        sd->show_item = EINA_FALSE;
        efl_ui_scrollable_scroll
              (sd->smanager, sd->show, EINA_FALSE);
     }
}

static void
_grid_load(Evas_Object *obj,
           Efl_Ui_Image_Zoomable_Grid *g)
{
   int x, y;
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh, gw, gh, tx, ty;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

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
                  _photocam_image_file_set(g->grid[tn].img, sd);
                  evas_object_image_preload(g->grid[tn].img, 0);
                  sd->preload_num++;
                  if (sd->preload_num == 1)
                    {
                       if (elm_widget_is_legacy(obj))
                         edje_object_signal_emit
                            (wd->resize_obj,
                             "elm,state,busy,start", "elm");
                       else
                         edje_object_signal_emit
                            (wd->resize_obj,
                             "efl,state,busy,start", "efl");

                       efl_event_callback_legacy_call
                        (obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOAD_DETAIL, NULL);
                    }
               }
             else if ((g->grid[tn].want) && (!visible))
               {
                  sd->preload_num--;
                  if (!sd->preload_num)
                    {
                       if (elm_widget_is_legacy(obj))
                         edje_object_signal_emit
                            (wd->resize_obj,
                             "elm,state,busy,stop", "elm");
                       else
                         edje_object_signal_emit
                            (wd->resize_obj,
                             "efl,state,busy,stop", "efl");

                       efl_event_callback_legacy_call
                        (obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOADED_DETAIL, NULL);
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
            Efl_Ui_Image_Zoomable_Grid *g,
            Evas_Coord px,
            Evas_Coord py,
            Evas_Coord ox,
            Evas_Coord oy,
            Evas_Coord ow,
            Evas_Coord oh)
{
   Evas_Coord ax, ay, gw, gh, tx, ty;
   int x, y;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

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
             evas_object_geometry_set(g->grid[tn].img,
                                      ox + xx - px + ax,
                                      oy + yy - py + ay, ww, hh);
          }
     }
}

EOLIAN static void
_efl_ui_image_zoomable_pan_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Image_Zoomable_Pan_Data *psd)
{
   Efl_Ui_Image_Zoomable_Grid *g;
   Eina_List *l;
   Evas_Coord ox, oy, ow, oh;

   ELM_WIDGET_DATA_GET_OR_RETURN(psd->wobj, wd);

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   _image_place(
       wd->obj, psd->wsd->pan_x, psd->wsd->pan_y,
       ox - psd->wsd->g_layer_zoom.imx, oy - psd->wsd->g_layer_zoom.imy, ow,
       oh);

   EINA_LIST_FOREACH(psd->wsd->grids, l, g)
     {
        _grid_load(wd->obj, g);
        _grid_place(
              wd->obj, g, psd->wsd->pan_x,
             psd->wsd->pan_y, ox - psd->wsd->g_layer_zoom.imx,
             oy - psd->wsd->g_layer_zoom.imy, ow, oh);
     }
}

EOLIAN static void
_efl_ui_image_zoomable_pan_efl_ui_pan_pan_position_set(Eo *obj, Efl_Ui_Image_Zoomable_Pan_Data *psd, Eina_Position2D pos)
{
   if ((pos.x == psd->wsd->pan_x) && (pos.y == psd->wsd->pan_y)) return;
   psd->wsd->pan_x = pos.x;
   psd->wsd->pan_y = pos.y;
   evas_object_smart_changed(obj);

   efl_event_callback_call(obj, EFL_UI_PAN_EVENT_PAN_POSITION_CHANGED, NULL);
}

EOLIAN static Eina_Position2D
_efl_ui_image_zoomable_pan_efl_ui_pan_pan_position_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Pan_Data *psd)
{
   return EINA_POSITION2D(psd->wsd->pan_x, psd->wsd->pan_y);
}

EOLIAN static Eina_Position2D
_efl_ui_image_zoomable_pan_efl_ui_pan_pan_position_max_get(const Eo *obj, Efl_Ui_Image_Zoomable_Pan_Data *psd)
{
   Evas_Coord ow, oh;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = psd->wsd->minw - ow;
   if (ow < 0) ow = 0;
   oh = psd->wsd->minh - oh;
   if (oh < 0) oh = 0;

   return EINA_POSITION2D(ow, oh);
}

EOLIAN static Eina_Position2D
_efl_ui_image_zoomable_pan_efl_ui_pan_pan_position_min_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Pan_Data *_pd EINA_UNUSED)
{
   return EINA_POSITION2D(0, 0);
}

EOLIAN static Eina_Size2D
_efl_ui_image_zoomable_pan_efl_ui_pan_content_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Pan_Data *psd)
{
   return EINA_SIZE2D(psd->wsd->minw, psd->wsd->minh);
}

EOLIAN static void
_efl_ui_image_zoomable_pan_efl_object_destructor(Eo *obj, Efl_Ui_Image_Zoomable_Pan_Data *psd)
{
   efl_data_unref(psd->wobj, psd->wsd);

   efl_destructor(efl_super(obj, MY_PAN_CLASS));
}

EOLIAN static void
_efl_ui_image_zoomable_pan_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_PAN_CLASS_NAME_LEGACY, klass);
}

#include "efl_ui_image_zoomable_pan.eo.c"

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
            Efl_Ui_Image_Zoomable_Grid *g)
{
   int x, y;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

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
                       if (elm_widget_is_legacy(obj))
                         edje_object_signal_emit
                            (wd->resize_obj,
                             "elm,state,busy,stop", "elm");
                       else
                         edje_object_signal_emit
                            (wd->resize_obj,
                             "efl,state,busy,stop", "efl");

                       efl_event_callback_legacy_call
                         (obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOAD_DETAIL, NULL);
                    }
               }
          }
     }

   ELM_SAFE_FREE(g->grid, free);
   g->gw = 0;
   g->gh = 0;
}

static void
_tile_preloaded_cb(void *data,
                   Evas *e EINA_UNUSED,
                   Evas_Object *o EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Efl_Ui_Image_Zoomable_Grid_Item *git = data;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(git->obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(git->obj, wd);

   if (git->want)
     {
        git->want = 0;
        evas_object_show(git->img);
        git->have = 1;
        sd->preload_num--;
        if (!sd->preload_num)
          {
             if (elm_widget_is_legacy(git->obj))
               edje_object_signal_emit
                  (wd->resize_obj, "elm,state,busy,stop",
                   "elm");
             else
               edje_object_signal_emit
                  (wd->resize_obj, "efl,state,busy,stop",
                   "efl");

             efl_event_callback_legacy_call
               (wd->obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOADED_DETAIL, NULL);
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

static Efl_Ui_Image_Zoomable_Grid *
_grid_create(Evas_Object *obj)
{
   int x, y;
   Efl_Ui_Image_Zoomable_Grid *g;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   g = calloc(1, sizeof(Efl_Ui_Image_Zoomable_Grid));
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

   g->grid = calloc(1, sizeof(Efl_Ui_Image_Zoomable_Grid_Item) * g->gw * g->gh);
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
             g->grid[tn].out.x = x * g->tsize;
             if (x == (g->gw - 1))
               g->grid[tn].out.w = g->w - ((g->gw - 1) * g->tsize);
             else
               g->grid[tn].out.w = g->tsize;
             g->grid[tn].out.y = y * g->tsize;
             if (y == (g->gh - 1))
               g->grid[tn].out.h = g->h - ((g->gh - 1) * g->tsize);
             else
               g->grid[tn].out.h = g->tsize;

             if (g->zoom <= 0) g->zoom = 1;
             g->grid[tn].src.x = g->grid[tn].out.x * g->zoom;
             g->grid[tn].src.y = g->grid[tn].out.y * g->zoom;
             g->grid[tn].src.w = g->grid[tn].out.w * g->zoom;
             g->grid[tn].src.h = g->grid[tn].out.h * g->zoom;

             g->grid[tn].obj = obj;
             g->grid[tn].img =
               evas_object_image_add(evas_object_evas_get(obj));
             evas_object_image_load_orientation_set(g->grid[tn].img, EINA_TRUE);
             efl_orientation_set(g->grid[tn].img, sd->orient);
             efl_orientation_flip_set(g->grid[tn].img, sd->flip);
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
   Efl_Ui_Image_Zoomable_Grid *g;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   EINA_LIST_FREE(sd->grids, g)
     {
        _grid_clear(obj, g);
        free(g);
     }
}

static void
_smooth_update(Evas_Object *obj)
{
   Efl_Ui_Image_Zoomable_Grid *g;
   int x, y;
   Eina_List *l;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->grids, l, g)
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
_grid_raise(Efl_Ui_Image_Zoomable_Grid *g)
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
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);

   sd->no_smooth--;
   if (!sd->no_smooth) _smooth_update(data);

   sd->scr_timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_main_img_preloaded_cb(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *o EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   Evas_Object *obj = data;
   Efl_Ui_Image_Zoomable_Grid *g;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   evas_object_show(sd->img);
   sd->main_load_pending = 0;
   g = _grid_create(obj);
   if (g)
     {
        sd->grids = eina_list_prepend(sd->grids, g);
        _grid_load(obj, g);
     }
   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, data);
   efl_event_callback_legacy_call(data, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOADED, NULL);
   sd->preload_num--;
   if (!sd->preload_num)
     {
        if (elm_widget_is_legacy(data))
          edje_object_signal_emit
             (wd->resize_obj, "elm,state,busy,stop", "elm");
        else
          edje_object_signal_emit
             (wd->resize_obj, "efl,state,busy,stop", "efl");

        efl_event_callback_legacy_call
          (obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOADED_DETAIL, NULL);
     }
}

static Eina_Bool
_zoom_do(Evas_Object *obj,
         double t)
{
   Evas_Coord xx, yy;
   Eina_Rect view = {};

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   sd->size.w = (sd->size.ow * (1.0 - t)) + (sd->size.nw * t);
   sd->size.h = (sd->size.oh * (1.0 - t)) + (sd->size.nh * t);
   view = efl_ui_scrollable_viewport_geometry_get(sd->smanager);
   xx = (sd->size.spos.x * sd->size.w) - (view.w / 2);
   yy = (sd->size.spos.y * sd->size.h) - (view.h / 2);
   if (xx < 0) xx = 0;
   else if (xx > (sd->size.w - view.w))
     xx = sd->size.w - view.w;
   if (yy < 0) yy = 0;
   else if (yy > (sd->size.h - view.h))
     yy = sd->size.h - view.h;

   sd->show_item = EINA_TRUE;
   sd->show.x = xx;
   sd->show.y = yy;
   sd->show.w = view.w;
   sd->show.h = view.h;

   if (sd->orientation_changed)
     {
        evas_object_smart_member_del(sd->img);
        elm_widget_sub_object_del(obj, sd->img);
        evas_object_smart_member_add(sd->img, sd->pan_obj);
        elm_widget_sub_object_add(obj, sd->img);
     }
   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, obj);
   if (t >= 1.0)
     {
        Eina_List *l, *l_next;
        Efl_Ui_Image_Zoomable_Grid *g;

        EINA_LIST_FOREACH_SAFE(sd->grids, l, l_next, g)
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

static void
_zoom_anim_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   double t;
   Evas_Object *obj = data;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   t = ecore_loop_time_get();
   if (t >= sd->t_end)
     t = 1.0;
   else if (sd->t_end > sd->t_start)
     t = (t - sd->t_start) / (sd->t_end - sd->t_start);
   else
     t = 1.0;
   t = 1.0 - t;
   t = 1.0 - (t * t);
   if (!_zoom_do(obj, t))
     {
        sd->no_smooth--;
        if (!sd->no_smooth) _smooth_update(data);
        efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _zoom_anim_cb, obj);
        efl_event_callback_legacy_call(obj, EFL_UI_EVENT_ZOOM_STOP, NULL);
     }
}

static Eina_Bool
_long_press_cb(void *data)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);

   sd->long_timer = NULL;
   sd->longpressed = EINA_TRUE;
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_LONGPRESSED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(void *data,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     efl_event_callback_legacy_call
       (data, EFL_UI_EVENT_CLICKED_DOUBLE, NULL);
   else
     efl_event_callback_legacy_call(data, EFL_UI_IMAGE_ZOOMABLE_EVENT_PRESS, NULL);
   sd->longpressed = EINA_FALSE;
   ecore_timer_del(sd->long_timer);
   sd->long_timer = ecore_timer_add
       (_elm_config->longpress_timeout, _long_press_cb, data);
}

static void
_mouse_up_cb(void *data,
             Evas *evas EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;
   ELM_SAFE_FREE(sd->long_timer, ecore_timer_del);
   if (!sd->on_hold)
     efl_event_callback_legacy_call
       (data, EFL_UI_EVENT_CLICKED, NULL);
   sd->on_hold = EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_image_zoomable_efl_ui_focus_object_on_focus_update(Eo *obj, Efl_Ui_Image_Zoomable_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   if (efl_ui_focus_object_focus_get(obj))
     {
        if (elm_widget_is_legacy(obj))
          edje_object_signal_emit
             (wd->resize_obj, "elm,action,focus", "elm");
        else
          edje_object_signal_emit
             (wd->resize_obj, "efl,action,focus", "efl");
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
     }
   else
     {
        if (elm_widget_is_legacy(obj))
          edje_object_signal_emit
             (wd->resize_obj, "elm,action,unfocus", "elm");
        else
          edje_object_signal_emit
             (wd->resize_obj, "efl,action,unfocus", "efl");
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Error
_efl_ui_image_zoomable_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   Eina_Bool fdo = EINA_FALSE;

   if (sd->stdicon)
     _internal_efl_ui_image_zoomable_icon_set(obj, sd->stdicon, &fdo, EINA_TRUE);

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   efl_ui_mirrored_set(sd->smanager, efl_ui_mirrored_get(obj));

   _sizing_eval(obj);

   return int_ret;
}

static void
_scroll_cb(void * data,
           const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *obj = data;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   if (!sd->scr_timer)
     {
        sd->no_smooth++;
        if (sd->no_smooth == 1) _smooth_update(obj);
     }

   ecore_timer_del(sd->scr_timer);
   sd->scr_timer = ecore_timer_add(0.5, _scroll_timeout_cb, obj);
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   Eina_Position2D pos = {};
   const char *dir = params;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   Evas_Coord step = 100;
   pos = efl_ui_scrollable_content_pos_get(sd->smanager);

   if (!strcmp(dir, "left"))
     {
        pos.x -= step;
     }
   else if (!strcmp(dir, "right"))
     {
        pos.x += step;
     }
   else if (!strcmp(dir, "up"))
     {
        pos.y -= step;
     }
   else if (!strcmp(dir, "down"))
     {
        pos.y += step;
     }
   else return EINA_FALSE;

   efl_ui_scrollable_content_pos_set(sd->smanager, pos);
   return EINA_TRUE;
}

static Eina_Bool
_key_action_zoom(Evas_Object *obj, const char *params)
{
   const char *dir = params;
   double zoom;

   if (!strcmp(dir, "in"))
     {
        zoom = elm_photocam_zoom_get(obj);
        zoom /= 1.5;
        elm_photocam_zoom_mode_set(obj, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
        elm_photocam_zoom_set(obj, zoom);
     }
   else if (!strcmp(dir, "out"))
     {
        zoom = elm_photocam_zoom_get(obj);
        zoom *= 1.5;
        elm_photocam_zoom_mode_set(obj, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
        elm_photocam_zoom_set(obj, zoom);
     }
   else return EINA_FALSE;

   return EINA_TRUE;
}

static void
_bounce_eval(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *obj = data;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   double t, tt;

   if ((sd->g_layer_zoom.imx == sd->g_layer_zoom.bounce.x_end) &&
       (sd->g_layer_zoom.imy == sd->g_layer_zoom.bounce.y_end))
     goto on_end;

   t = ecore_loop_time_get();
   tt = (t - sd->g_layer_zoom.bounce.t_start) /
     (sd->g_layer_zoom.bounce.t_end -
      sd->g_layer_zoom.bounce.t_start);
   tt = 1.0 - tt;
   tt = 1.0 - (tt * tt);

   if (t > sd->g_layer_zoom.bounce.t_end)
     {
        _zoom_do(obj, 1.0);
        goto on_end;
     }

   if (sd->g_layer_zoom.imx != sd->g_layer_zoom.bounce.x_end)
     sd->g_layer_zoom.imx =
       sd->g_layer_zoom.bounce.x_start * (1.0 - tt) +
       sd->g_layer_zoom.bounce.x_end * tt;

   if (sd->g_layer_zoom.imy != sd->g_layer_zoom.bounce.y_end)
     sd->g_layer_zoom.imy =
       sd->g_layer_zoom.bounce.y_start * (1.0 - tt) +
       sd->g_layer_zoom.bounce.y_end * tt;

   _zoom_do(obj, 1.0 - (1.0 - tt));

   return;

 on_end:
   sd->g_layer_zoom.imx = 0;
   sd->g_layer_zoom.imy = 0;
   sd->zoom_g_layer = EINA_FALSE;

   efl_ui_scrollable_scroll_freeze_set(sd->smanager, EINA_FALSE);

   efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _bounce_eval, obj);
}

static void
_efl_ui_image_zoomable_bounce_reset(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd EINA_UNUSED)
{
   Eina_Bool r;

   r = efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _bounce_eval, obj);
   if (r) _zoom_do(obj, 1.0);
}

static void
_efl_ui_image_zoomable_zoom_reset(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd)
{
   Eina_Bool r;

   r = efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _zoom_anim_cb, obj);
   if (r)
     {
        sd->no_smooth--;
        if (!sd->no_smooth) _smooth_update(obj);
        _zoom_do(obj, 1.0);
        efl_event_callback_legacy_call(obj, EFL_UI_EVENT_ZOOM_STOP, NULL);
     }
}

static void
_g_layer_zoom_do(Evas_Object *obj,
                 Evas_Coord px,
                 Evas_Coord py,
                 Elm_Gesture_Zoom_Info *g_layer)
{
   int regx, regy, regw, regh, ix, iy, iw, ih;
   int xx, yy;
   Eina_Rect view = {};

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   sd->mode = ELM_PHOTOCAM_ZOOM_MODE_MANUAL;
   sd->zoom = sd->g_layer_start / g_layer->zoom;
   sd->size.ow = sd->size.w;
   sd->size.oh = sd->size.h;
   view = efl_ui_scrollable_viewport_geometry_get(sd->smanager);
   if ((view.w <= 0) || (view.h <= 0)) return;

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

   if ((xx < 0) || (view.w > sd->size.nw))
     {
        sd->g_layer_zoom.imx = xx;
        xx = 0;
     }
   else if ((xx + view.w) > sd->size.nw)
     {
        sd->g_layer_zoom.imx = xx + view.w - sd->size.nw;
        xx = sd->size.nw - view.w;
     }

   if ((yy < 0) || (view.h > sd->size.nh))
     {
        sd->g_layer_zoom.imy = yy;
        yy = 0;
     }
   else if ((yy + view.h) > sd->size.nh)
     {
        sd->g_layer_zoom.imy = yy + view.h - sd->size.nh;
        yy = sd->size.nh - view.h;
     }

   sd->size.spos.x = (double)(xx + (view.w / 2)) / (double)(sd->size.nw);
   sd->size.spos.y = (double)(yy + (view.h / 2)) / (double)(sd->size.nh);

   _zoom_do(obj, 1.0);
}

static Evas_Event_Flags
_g_layer_zoom_start_cb(void *data,
                       void *event_info)
{
   Evas_Object *obj = data;
   Elm_Gesture_Zoom_Info *p = event_info;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   double marginx = 0, marginy = 0;
   int x, y, w, h;
   Eina_Rect view = {};

   _efl_ui_image_zoomable_bounce_reset(obj, sd);
   sd->zoom_g_layer = EINA_TRUE;

   efl_ui_scrollable_scroll_freeze_set(sd->smanager, EINA_TRUE);

   elm_photocam_image_region_get(obj, &x, &y, &w, &h);
   view = efl_ui_scrollable_viewport_geometry_get(sd->smanager);

   if (view.w > sd->size.nw)
     marginx = (view.w - sd->size.nw) / 2;
   if (view.h > sd->size.nh)
     marginy = (view.h - sd->size.nh) / 2;

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
   Efl_Ui_Image_Zoomable_Data *sd = efl_data_scope_get(data, MY_CLASS);
   Elm_Gesture_Zoom_Info *p = event_info;

   _g_layer_zoom_do(data, sd->zoom_point_x, sd->zoom_point_y, p);

   return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags
_g_layer_zoom_end_cb(void *data,
                     void *event_info EINA_UNUSED)
{
   Evas_Object *obj = data;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   Eina_Rect view = efl_ui_scrollable_viewport_geometry_get(sd->smanager);
   sd->g_layer_start = 1.0;

   if (sd->g_layer_zoom.imx || sd->g_layer_zoom.imy)
     {
        double t;

        t = ecore_loop_time_get();
        sd->g_layer_zoom.bounce.x_start = sd->g_layer_zoom.imx;
        sd->g_layer_zoom.bounce.y_start = sd->g_layer_zoom.imy;
        sd->g_layer_zoom.bounce.x_end = 0;
        sd->g_layer_zoom.bounce.y_end = 0;

        if (view.w > sd->size.nw &&
            view.h > sd->size.nh)
          {
             Evas_Coord pw, ph;
             double z;

             if ((sd->size.imw < view.w) && (sd->size.imh < view.h))
               {
                  sd->zoom = 1;
                  sd->size.nw = sd->size.imw;
                  sd->size.nh = sd->size.imh;
               }
             else
               {
                  ph = (sd->size.imh * view.w) / sd->size.imw;
                  if (ph > view.h)
                    {
                       pw = (sd->size.imw * view.h) / sd->size.imh;
                       ph = view.h;
                    }
                  else
                    {
                       pw = view.w;
                    }
                  if (sd->size.imw > sd->size.imh)
                    z = (double)sd->size.imw / pw;
                  else
                    z = (double)sd->size.imh / ph;

                  sd->zoom = z;
                  sd->size.nw = pw;
                  sd->size.nh = ph;
               }
             sd->g_layer_zoom.bounce.x_end = (sd->size.nw - view.w) / 2;
             sd->g_layer_zoom.bounce.y_end = (sd->size.nh - view.h) / 2;
          }
        else
          {
             int xx, yy;

             xx = (sd->zoom_point_x / sd->zoom) - sd->pvx;
             yy = (sd->zoom_point_y / sd->zoom) - sd->pvy;

             if (xx < 0) xx = 0;
             if (yy < 0) yy = 0;

             if (view.w > sd->size.nw)
               sd->g_layer_zoom.bounce.x_end = (sd->size.nw - view.w) / 2;
             if ((xx + view.w) > sd->size.nw)
               xx = sd->size.nw - view.w;

             if (view.h > sd->size.nh)
               sd->g_layer_zoom.bounce.y_end = (sd->size.nh - view.h) / 2;
             if ((yy + view.h) > sd->size.nh)
               yy = sd->size.nh - view.h;

             sd->size.spos.x = (double)(xx + (view.w / 2)) / (double)(sd->size.nw);
             sd->size.spos.y = (double)(yy + (view.h / 2)) / (double)(sd->size.nh);
          }

        sd->g_layer_zoom.bounce.t_start = t;
        sd->g_layer_zoom.bounce.t_end = t +
          _elm_config->page_scroll_friction;

        efl_event_callback_add(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _bounce_eval, obj);
     }
   else
     {
        efl_ui_scrollable_scroll_freeze_set(obj, EINA_FALSE);
        sd->zoom_g_layer = EINA_FALSE;
     }

   return EVAS_EVENT_FLAG_NONE;
}

static void
_orient_do(Evas_Object *obj, Efl_Ui_Image_Zoomable_Data *sd)
{
   evas_object_smart_member_del(sd->img);
   elm_widget_sub_object_del(obj, sd->img);
   evas_object_smart_member_add(sd->img, sd->pan_obj);
   elm_widget_sub_object_add(obj, sd->img);
   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, obj);
}

static void
_orient_apply(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd)
{
   int iw, ih;
   Eina_List *l;
   Efl_Ui_Image_Zoomable_Grid *g, *g_orient = NULL;

   // Note: This is based on legacy code. Separating flip & orient in eo api
   // means we need to do the apply twice. This could be delayed as a job.

   sd->orientation_changed = EINA_TRUE;
   g = _grid_create(obj);
   if (g)
     {
        if (eina_list_count(sd->grids) > 1)
          {
             g_orient = eina_list_last(sd->grids)->data;
             sd->grids = eina_list_remove(sd->grids, g_orient);
             _grid_clear(obj, g_orient);
             free(g_orient);
             EINA_LIST_FOREACH(sd->grids, l, g_orient)
               {
                  g_orient->dead = 1;
               }
          }
        sd->grids = eina_list_prepend(sd->grids, g);
     }
   else
     {
        EINA_LIST_FREE(sd->grids, g)
          {
             _grid_clear(obj, g);
             free(g);
          }
     }

   efl_orientation_set(sd->img, sd->orient);
   efl_orientation_flip_set(sd->img, sd->flip);
   evas_object_image_size_get(sd->img, &iw, &ih);
   sd->size.imw = iw;
   sd->size.imh = ih;
   sd->size.w = iw / sd->zoom;
   sd->size.h = ih / sd->zoom;
   _orient_do(obj, sd);
}

EOLIAN static void
_efl_ui_image_zoomable_efl_orientation_orientation_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd,
                                              Efl_Orient orient)
{
   if (sd->orient == orient) return;

   sd->orient = orient;
   _orient_apply(obj, sd);
}

EOLIAN static Efl_Orient
_efl_ui_image_zoomable_efl_orientation_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   return sd->orient;
}

EOLIAN static void
_efl_ui_image_zoomable_efl_orientation_flip_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Efl_Flip flip)
{
   if (sd->flip == flip) return;

   sd->flip = flip;
   _orient_apply(obj, sd);
}

EOLIAN static Efl_Flip
_efl_ui_image_zoomable_efl_orientation_flip_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   return sd->flip;
}

static void
_efl_ui_image_zoomable_bar_read_and_update(Eo *obj)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   double vx, vy;

   if (elm_widget_is_legacy(obj))
     {
        edje_object_part_drag_value_get
           (wd->resize_obj, "elm.dragable.vbar", NULL, &vy);
        edje_object_part_drag_value_get
           (wd->resize_obj, "elm.dragable.hbar", &vx, NULL);
     }
   else
     {
        edje_object_part_drag_value_get
           (wd->resize_obj, "efl.dragable.vbar", NULL, &vy);
        edje_object_part_drag_value_get
           (wd->resize_obj, "efl.dragable.hbar", &vx, NULL);
     }
   efl_ui_scrollbar_bar_position_set(sd->smanager, vx, vy);
}

static void
_efl_ui_image_zoomable_reload_cb(void *data,
                                 Evas_Object *obj EINA_UNUSED,
                                 const char *emission EINA_UNUSED,
                                 const char *source EINA_UNUSED)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);

   efl_ui_scrollbar_bar_visibility_update(sd->smanager);
}

static void
_efl_ui_image_zoomable_vbar_drag_cb(void *data,
                                    Evas_Object *obj EINA_UNUSED,
                                    const char *emission EINA_UNUSED,
                                    const char *source EINA_UNUSED)
{
   _efl_ui_image_zoomable_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_efl_ui_image_zoomable_vbar_press_cb(void *data,
                                     Evas_Object *obj EINA_UNUSED,
                                     const char *emission EINA_UNUSED,
                                     const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_efl_ui_image_zoomable_vbar_unpress_cb(void *data,
                                       Evas_Object *obj EINA_UNUSED,
                                       const char *emission EINA_UNUSED,
                                       const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_efl_ui_image_zoomable_edje_drag_start_cb(void *data,
                                          Evas_Object *obj EINA_UNUSED,
                                          const char *emission EINA_UNUSED,
                                          const char *source EINA_UNUSED)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);

   _efl_ui_image_zoomable_bar_read_and_update(data);

   sd->freeze_want = efl_ui_scrollable_scroll_freeze_get(sd->smanager);
   efl_ui_scrollable_scroll_freeze_set(sd->smanager, EINA_TRUE);
   efl_event_callback_call(data, EFL_UI_EVENT_SCROLL_DRAG_START, NULL);
}

static void
_efl_ui_image_zoomable_edje_drag_stop_cb(void *data,
                                         Evas_Object *obj EINA_UNUSED,
                                         const char *emission EINA_UNUSED,
                                         const char *source EINA_UNUSED)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);

   _efl_ui_image_zoomable_bar_read_and_update(data);

   efl_ui_scrollable_scroll_freeze_set(sd->smanager, sd->freeze_want);
   efl_event_callback_call(data, EFL_UI_EVENT_SCROLL_DRAG_STOP, NULL);
}

static void
_efl_ui_image_zoomable_edje_drag_cb(void *data,
                                    Evas_Object *obj EINA_UNUSED,
                                    const char *emission EINA_UNUSED,
                                    const char *source EINA_UNUSED)
{
   _efl_ui_image_zoomable_bar_read_and_update(data);
}

static void
_efl_ui_image_zoomable_hbar_drag_cb(void *data,
                                    Evas_Object *obj EINA_UNUSED,
                                    const char *emission EINA_UNUSED,
                                    const char *source EINA_UNUSED)
{
   _efl_ui_image_zoomable_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_efl_ui_image_zoomable_hbar_press_cb(void *data,
                                     Evas_Object *obj EINA_UNUSED,
                                     const char *emission EINA_UNUSED,
                                     const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_efl_ui_image_zoomable_hbar_unpress_cb(void *data,
                                       Evas_Object *obj EINA_UNUSED,
                                       const char *emission EINA_UNUSED,
                                       const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_efl_ui_image_zoomable_bar_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double width = 0.0, height = 0.0;

   efl_ui_scrollbar_bar_size_get(sd->smanager, &width, &height);

   if (elm_widget_is_legacy(obj))
     {
        edje_object_part_drag_size_set(wd->resize_obj, "elm.dragable.hbar", width, 1.0);
        edje_object_part_drag_size_set(wd->resize_obj, "elm.dragable.vbar", 1.0, height);
     }
   else
     {
        edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.hbar", width, 1.0);
        edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.vbar", 1.0, height);
     }
}

static void
_efl_ui_image_zoomable_bar_pos_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double posx = 0.0, posy = 0.0;

   efl_ui_scrollbar_bar_position_get(sd->smanager, &posx, &posy);

   if (elm_widget_is_legacy(obj))
     {
        edje_object_part_drag_value_set(wd->resize_obj, "elm.dragable.hbar", posx, 0.0);
        edje_object_part_drag_value_set(wd->resize_obj, "elm.dragable.vbar", 0.0, posy);
     }
   else
     {
        edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.hbar", posx, 0.0);
        edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.vbar", 0.0, posy);
     }
}

static void
_efl_ui_image_zoomable_bar_show_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (elm_widget_is_legacy(obj))
     {
        if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
          edje_object_signal_emit(wd->resize_obj, "elm,action,show,hbar", "elm");
        else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
          edje_object_signal_emit(wd->resize_obj, "elm,action,show,vbar", "elm");
     }
   else
     {
        if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
          edje_object_signal_emit(wd->resize_obj, "efl,action,show,hbar", "efl");
        else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
          edje_object_signal_emit(wd->resize_obj, "efl,action,show,vbar", "efl");
     }
}

static void
_efl_ui_image_zoomable_bar_hide_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (elm_widget_is_legacy(obj))
     {
        if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
          edje_object_signal_emit(wd->resize_obj, "elm,action,hide,hbar", "elm");
        else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
          edje_object_signal_emit(wd->resize_obj, "elm,action,hide,vbar", "elm");
     }
   else
     {
        if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
          edje_object_signal_emit(wd->resize_obj, "efl,action,hide,hbar", "efl");
        else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
          edje_object_signal_emit(wd->resize_obj, "efl,action,hide,vbar", "efl");
     }
}

static void
_efl_ui_image_zoomable_edje_object_attach(Eo *obj)
{
   if (elm_widget_is_legacy(obj))
     {
        efl_layout_signal_callback_add
          (obj, "reload", "elm", obj, _efl_ui_image_zoomable_reload_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_vbar_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,set", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,start", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_start_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,stop", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_stop_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,step", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,page", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "elm,vbar,press", "elm",
            obj, _efl_ui_image_zoomable_vbar_press_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "elm,vbar,unpress", "elm",
            obj, _efl_ui_image_zoomable_vbar_unpress_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_hbar_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,set", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,start", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_start_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,stop", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_stop_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,step", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,page", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "elm,hbar,press", "elm",
            obj, _efl_ui_image_zoomable_hbar_press_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "elm,hbar,unpress", "elm",
            obj, _efl_ui_image_zoomable_hbar_unpress_cb, NULL);
     }
   else
     {
        efl_layout_signal_callback_add
          (obj, "reload", "efl", obj, _efl_ui_image_zoomable_reload_cb, NULL);
        efl_layout_signal_callback_add
          (obj, "drag", "efl.dragable.vbar",
           obj, _efl_ui_image_zoomable_vbar_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,set", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,start", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_start_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,stop", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_stop_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,step", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,page", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "efl,vbar,press", "efl",
            obj, _efl_ui_image_zoomable_vbar_press_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "efl,vbar,unpress", "efl",
            obj, _efl_ui_image_zoomable_vbar_unpress_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_hbar_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,set", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,start", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_start_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,stop", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_stop_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,step", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "drag,page", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "efl,hbar,press", "efl",
            obj, _efl_ui_image_zoomable_hbar_press_cb, NULL);
        efl_layout_signal_callback_add
           (obj, "efl,hbar,unpress", "efl",
            obj, _efl_ui_image_zoomable_hbar_unpress_cb, NULL);
     }
}

static void
_efl_ui_image_zoomable_edje_object_detach(Evas_Object *obj)
{
   if (elm_widget_is_legacy(obj))
     {
        efl_layout_signal_callback_del
          (obj, "reload", "elm", obj, _efl_ui_image_zoomable_reload_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_vbar_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,set", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,start", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_start_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,stop", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_stop_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,step", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,page", "elm.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "elm,vbar,press", "elm",
            obj, _efl_ui_image_zoomable_vbar_press_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "elm,vbar,unpress", "elm",
            obj, _efl_ui_image_zoomable_vbar_unpress_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_hbar_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,set", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,start", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_start_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,stop", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_stop_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,step", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,page", "elm.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "elm,hbar,press", "elm",
            obj, _efl_ui_image_zoomable_hbar_press_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "elm,hbar,unpress", "elm",
            obj, _efl_ui_image_zoomable_hbar_unpress_cb, NULL);
     }
   else
     {
        efl_layout_signal_callback_del
          (obj, "reload", "efl", obj, _efl_ui_image_zoomable_reload_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_vbar_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,set", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,start", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_start_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,stop", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_stop_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,step", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,page", "efl.dragable.vbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "efl,vbar,press", "efl",
            obj, _efl_ui_image_zoomable_vbar_press_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "efl,vbar,unpress", "efl",
            obj, _efl_ui_image_zoomable_vbar_unpress_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_hbar_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,set", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,start", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_start_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,stop", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_stop_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,step", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "drag,page", "efl.dragable.hbar",
            obj, _efl_ui_image_zoomable_edje_drag_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "efl,hbar,press", "efl",
            obj, _efl_ui_image_zoomable_hbar_press_cb, NULL);
        efl_layout_signal_callback_del
           (obj, "efl,hbar,unpress", "efl",
            obj, _efl_ui_image_zoomable_hbar_unpress_cb, NULL);
     }
}

EOLIAN static void
_efl_ui_image_zoomable_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Image_Zoomable_Data *priv)
{
   Eina_Bool bounce = _elm_config->thumbscroll_bounce_enable;
   Efl_Ui_Image_Zoomable_Pan_Data *pan_data;
   Evas_Object *edje;
   Evas_Coord minw, minh;

   edje = edje_object_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, edje);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   elm_widget_theme_object_set
      (obj, edje, "photocam", "base", elm_widget_style_get(obj));

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
   efl_composite_attach(obj, priv->smanager);

   efl_ui_mirrored_set(priv->smanager, efl_ui_mirrored_get(obj));
   efl_ui_scrollable_bounce_enabled_set(priv->smanager, bounce, bounce);

   priv->pan_obj = efl_add(MY_PAN_CLASS, obj);

   efl_ui_scroll_manager_pan_set(priv->smanager, priv->pan_obj);
   if (elm_widget_is_legacy(obj))
     edje_object_part_swallow(edje, "elm.swallow.content", priv->pan_obj);
   else
     edje_object_part_swallow(edje, "efl.content", priv->pan_obj);

   pan_data = efl_data_scope_get(priv->pan_obj, MY_PAN_CLASS);
   efl_data_ref(obj, MY_CLASS);
   pan_data->wobj = obj;
   pan_data->wsd = priv;

   efl_event_callback_add(obj, EFL_UI_EVENT_SCROLL, _scroll_cb, obj);

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

   edje_object_size_min_calc(edje, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   _efl_ui_image_zoomable_edje_object_attach(obj);

   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                          _efl_ui_image_zoomable_bar_size_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                          _efl_ui_image_zoomable_bar_pos_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                          _efl_ui_image_zoomable_bar_show_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _efl_ui_image_zoomable_bar_hide_cb, obj);
   _sizing_eval(obj);
}

EOLIAN static void
_efl_ui_image_zoomable_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd)
{
   Efl_Ui_Image_Zoomable_Grid *g;

   ecore_timer_del(sd->anim_timer);
   ELM_SAFE_FREE(sd->edje, evas_object_del);

   EINA_LIST_FREE(sd->grids, g)
     {
        _grid_clear(obj, g);
        free(g->grid);
        free(g);
     }
   ELM_SAFE_FREE(sd->pan_obj, evas_object_del);

   if (sd->f) eina_file_close(sd->f);
   if (sd->remote.copier) _efl_ui_image_zoomable_remote_copier_cancel(obj, sd);
   if (sd->remote.binbuf) ELM_SAFE_FREE(sd->remote.binbuf, eina_binbuf_free);
   eina_stringshare_del(sd->file);
   ecore_job_del(sd->calc_job);
   ecore_timer_del(sd->scr_timer);
   ecore_timer_del(sd->long_timer);
   efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _zoom_anim_cb, obj);
   efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _bounce_eval, obj);
   efl_event_callback_del(obj, EFL_UI_EVENT_SCROLL, _scroll_cb, obj);

   _efl_ui_image_zoomable_edje_object_detach(obj);
   efl_del(sd->pan_obj);
   sd->pan_obj = NULL;
   efl_del(sd->smanager);
   sd->smanager = NULL;
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_image_zoomable_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sdi EINA_UNUSED, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
}

EOLIAN static void
_efl_ui_image_zoomable_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd EINA_UNUSED, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
}

EOLIAN static void
_efl_ui_image_zoomable_efl_canvas_group_group_member_add(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd EINA_UNUSED, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);
}

EOLIAN static Eo *
_efl_ui_image_zoomable_efl_object_constructor(Eo *obj, Efl_Ui_Image_Zoomable_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_IMAGE);
   legacy_object_focus_handle(obj);
   return obj;
}

EOLIAN static Eina_Size2D
_efl_ui_image_zoomable_efl_gfx_image_image_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *pd)
{
   return EINA_SIZE2D(pd->size.imw, pd->size.imh);
}

EOLIAN static Eina_Size2D
_efl_ui_image_zoomable_efl_layout_group_group_size_min_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   if (sd->edje)
     return efl_layout_group_size_min_get(sd->edje);
   else
     return EINA_SIZE2D(0, 0);
}

EOLIAN static Eina_Size2D
_efl_ui_image_zoomable_efl_layout_group_group_size_max_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   if (sd->edje)
     return efl_layout_group_size_max_get(sd->edje);
   else
     return EINA_SIZE2D(0, 0);
}

EOLIAN static Eina_Bool
_efl_ui_image_zoomable_efl_layout_signal_signal_callback_add(Eo *obj, Efl_Ui_Image_Zoomable_Data *pd EINA_UNUSED,
                                                             const char *emission, const char *source,
                                                             void *func_data, EflLayoutSignalCb func, Eina_Free_Cb func_free_cb)
{
   Eina_Bool ok;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   ok = efl_layout_signal_callback_add(wd->resize_obj, emission, source, func_data, func, func_free_cb);

   return ok;
}

EOLIAN static Eina_Bool
_efl_ui_image_zoomable_efl_layout_signal_signal_callback_del(Eo *obj, Efl_Ui_Image_Zoomable_Data *pd EINA_UNUSED,
                                                             const char *emission, const char *source,
                                                             void *func_data, EflLayoutSignalCb func, Eina_Free_Cb func_free_cb)
{
   Eina_Bool ok;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   ok = efl_layout_signal_callback_del(wd->resize_obj, emission, source, func_data, func, func_free_cb);

   return ok;
}

static Eina_Error
_img_proxy_set(Evas_Object *obj, Efl_Ui_Image_Zoomable_Data *sd,
               Eina_Bool resize)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   double tz;
   int w, h;
   Eina_Error err = 0;

   sd->zoom = 1.0;
   evas_object_image_smooth_scale_set(sd->img, (sd->no_smooth == 0));
   evas_object_image_file_set(sd->img, NULL, NULL);
   evas_object_image_source_set(sd->img, NULL);
   evas_object_image_load_scale_down_set(sd->img, 0);

   if (!sd->edje)
     sd->edje = edje_object_add(evas_object_evas_get(obj));
   if (!resize)
     {
        efl_file_key_set(sd->edje, efl_file_key_get(obj));
        err = efl_file_mmap_set(sd->edje, efl_file_mmap_get(obj));
        if (err)
          {
             ERR("failed to set edje file '%s', group '%s': %s", efl_file_get(obj), efl_file_key_get(obj),
                 edje_load_error_str(edje_object_load_error_get(sd->edje)));
             return err;
          }
     }

   _min_obj_size_get(obj, &w, &h);
   evas_object_resize(sd->edje, w, h);

   evas_object_image_source_set(sd->img, sd->edje);
   evas_object_image_source_visible_set(sd->img, EINA_FALSE);
   evas_object_size_hint_min_set(sd->img, w, h);
   evas_object_show(sd->img);
   evas_object_show(sd->edje);

   sd->do_region = 0;
   sd->size.imw = w;
   sd->size.imh = h;
   sd->size.w = sd->size.imw / sd->zoom;
   sd->size.h = sd->size.imh / sd->zoom;
   evas_object_image_preload(sd->img, 0);
   sd->main_load_pending = EINA_TRUE;

   sd->calc_job = ecore_job_add(_calc_job_cb, obj);
   efl_event_callback_legacy_call(obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOAD, NULL);
   sd->preload_num++;
   if (sd->preload_num == 1)
     {
        if (elm_widget_is_legacy(obj))
          edje_object_signal_emit
             (wd->resize_obj, "elm,state,busy,start", "elm");
        else
          edje_object_signal_emit
             (wd->resize_obj, "efl,state,busy,start", "efl");
        efl_event_callback_legacy_call(obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOAD_DETAIL, NULL);
     }

   tz = sd->zoom;
   sd->zoom = 0.0;
   elm_photocam_zoom_set(obj, tz);
   sd->orient = EFL_ORIENT_NONE;
   sd->flip = EFL_FLIP_NONE;
   sd->orientation_changed = EINA_FALSE;

   return 0;
}

static Eina_Error
_image_zoomable_edje_file_set(Evas_Object *obj)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_GFX_IMAGE_LOAD_ERROR_GENERIC);

   return _img_proxy_set(obj, sd, EINA_FALSE);
}

static Eina_Error
_internal_file_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Evas_Load_Error *ret)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_GFX_IMAGE_LOAD_ERROR_GENERIC);
   int w, h;
   double tz;
   Eina_Error err = 0;
   const char *file;

   file = efl_file_get(obj);

   if (eina_str_has_extension(file, ".edj"))
     {
        return _image_zoomable_edje_file_set(obj);
     }

   // It is actually to late, we have lost the reference to the previous
   // file descriptor already, so we can't know if the file changed. To
   // be safe we do for now just force a full reload on file_set and hope
   // on evas to catch it, if there is no change.
   sd->f = eina_file_dup(efl_file_mmap_get(obj));

   evas_object_image_smooth_scale_set(sd->img, (sd->no_smooth == 0));
   evas_object_image_file_set(sd->img, NULL, NULL);
   evas_object_image_load_scale_down_set(sd->img, 0);
   _photocam_image_file_set(sd->img, sd);
   err = evas_object_image_load_error_get(sd->img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Things are going bad for '%s' (%p) : %i", file, sd->img, err);
        if (ret) *ret = err;
        return err;
     }
   evas_object_image_size_get(sd->img, &w, &h);

   sd->do_region = evas_object_image_region_support_get(sd->img);
   sd->size.imw = w;
   sd->size.imh = h;
   sd->size.w = sd->size.imw / sd->zoom;
   sd->size.h = sd->size.imh / sd->zoom;
   evas_object_image_file_set(sd->img, NULL, NULL);
   _photocam_image_file_set(sd->img, sd);
   err = evas_object_image_load_error_get(sd->img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Things are going bad for '%s' (%p)", file, sd->img);
        if (ret) *ret = err;
        return err;
     }

   evas_object_image_preload(sd->img, 0);
   sd->main_load_pending = EINA_TRUE;

   sd->calc_job = ecore_job_add(_calc_job_cb, obj);
   efl_event_callback_legacy_call(obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOAD, NULL);
   sd->preload_num++;
   if (sd->preload_num == 1)
     {
        if (elm_widget_is_legacy(obj))
          edje_object_signal_emit
             (wd->resize_obj, "elm,state,busy,start", "elm");
        else
          edje_object_signal_emit
             (wd->resize_obj, "efl,state,busy,start", "efl");
        efl_event_callback_legacy_call(obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_LOAD_DETAIL, NULL);
     }

   tz = sd->zoom;
   sd->zoom = 0.0;
   elm_photocam_zoom_set(obj, tz);
   sd->orient = EFL_ORIENT_NONE;
   sd->flip = EFL_FLIP_NONE;
   sd->orientation_changed = EINA_FALSE;

   if (ret) *ret = evas_object_image_load_error_get(sd->img);
   return efl_gfx_image_load_error_get(sd->img);
}

static void
_efl_ui_image_zoomable_remote_copier_del(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = efl_io_copier_source_get(event->object);
   efl_del(dialer);
}

static void
_efl_ui_image_zoomable_remote_copier_cancel(Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   Eo *copier = sd->remote.copier;

   if (!copier) return;
   /* copier is flagged as close_on_invalidate, thus:
    * efl_del()
    *  -> efl_io_closer_close()
    *      -> "done" event
    *          -> _efl_ui_image_zoomable_remote_copier_done()
    *
    * flag sd->remote.copier = NULL so _efl_ui_image_zoomable_remote_copier_done()
    * knows about it.
    */
   sd->remote.copier = NULL;
   efl_del(copier);
}

static void
_efl_ui_image_zoomable_remote_copier_done(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   Efl_Ui_Image_Zoomable_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Eina_File *f;
   Eo *dialer;
   const char *url;
   Evas_Load_Error ret = EVAS_LOAD_ERROR_NONE;

   /* we're called from _efl_ui_image_zoomable_remote_copier_cancel() */
   if (!sd->remote.copier) return;

   if (sd->remote.binbuf) eina_binbuf_free(sd->remote.binbuf);
   sd->remote.binbuf = efl_io_copier_binbuf_steal(sd->remote.copier);

   dialer = efl_io_copier_source_get(sd->remote.copier);
   url = efl_net_dialer_address_dial_get(dialer);
   f = eina_file_virtualize(url,
                            eina_binbuf_string_get(sd->remote.binbuf),
                            eina_binbuf_length_get(sd->remote.binbuf),
                            EINA_FALSE);
   efl_file_mmap_set(obj, f);
   eina_file_close(f);
   ret = _internal_file_set(obj, sd, &ret);

   if (ret)
     {
        Elm_Photocam_Error err = { 0, EINA_TRUE };

        ELM_SAFE_FREE(sd->remote.binbuf, eina_binbuf_free);
        efl_event_callback_legacy_call
          (obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_DOWNLOAD_ERROR, &err);
     }
   else
     {
        efl_event_callback_legacy_call
          (obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_DOWNLOAD_DONE, NULL);
     }

   ELM_SAFE_FREE(sd->remote.copier, efl_del);
}

static void
_efl_ui_image_zoomable_remote_copier_error(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_Image_Zoomable_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Eina_Error *perr = event->info;
   Elm_Photocam_Error err = { *perr, EINA_FALSE };

   efl_event_callback_legacy_call(obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_DOWNLOAD_ERROR, &err);
   _efl_ui_image_zoomable_remote_copier_cancel(obj, sd);
}

static void
_efl_ui_image_zoomable_remote_copier_progress(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Elm_Photocam_Progress progress;
   uint64_t now, total;

   efl_io_copier_progress_get(event->object, &now, NULL, &total);

   progress.now = now;
   progress.total = total;
   efl_event_callback_legacy_call
     (obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_DOWNLOAD_PROGRESS, &progress);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_ui_image_zoomable_remote_copier_cbs,
                           { EFL_EVENT_DEL, _efl_ui_image_zoomable_remote_copier_del },
                           { EFL_IO_COPIER_EVENT_DONE, _efl_ui_image_zoomable_remote_copier_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _efl_ui_image_zoomable_remote_copier_error },
                           { EFL_IO_COPIER_EVENT_PROGRESS, _efl_ui_image_zoomable_remote_copier_progress });

static Eina_Bool
_efl_ui_image_zoomable_download(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, const char *url)
{
   Eo *dialer;
   Elm_Photocam_Error img_err = { ENOSYS, EINA_FALSE };
   Eina_Error err;

   dialer = efl_add(EFL_NET_DIALER_HTTP_CLASS, obj,
                    efl_net_dialer_http_allow_redirects_set(efl_added, EINA_TRUE));
   EINA_SAFETY_ON_NULL_GOTO(dialer, error_dialer);

   sd->remote.copier = efl_add(EFL_IO_COPIER_CLASS, obj,
                               efl_io_copier_source_set(efl_added, dialer),
                               efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),
                               efl_event_callback_array_add(efl_added, _efl_ui_image_zoomable_remote_copier_cbs(), obj));
   EINA_SAFETY_ON_NULL_GOTO(sd->remote.copier, error_copier);

   err = efl_net_dialer_dial(dialer, url);
   if (err)
     {
        img_err.status = err;
        ERR("Could not download %s: %s", url, eina_error_msg_get(err));
        evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &img_err);
        goto error_dial;
     }
   return EINA_TRUE;

 error_dial:
   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &img_err);
   _efl_ui_image_zoomable_remote_copier_cancel(obj, sd);
   return EINA_FALSE;

 error_copier:
   efl_del(dialer);
 error_dialer:
   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &img_err);
   return EINA_FALSE;
}

static const Eina_Slice remote_uri[] = {
  EINA_SLICE_STR_LITERAL("http://"),
  EINA_SLICE_STR_LITERAL("https://"),
  EINA_SLICE_STR_LITERAL("ftp://"),
  { }
};

static inline Eina_Bool
_efl_ui_image_zoomable_is_remote(const char *file)
{
   const Eina_Slice *itr;
   if (!file) return EINA_FALSE;
   Eina_Slice s = EINA_SLICE_STR(file);;

   for (itr = remote_uri; itr->mem; itr++)
     if (eina_slice_startswith(s, *itr))
       return EINA_TRUE;

   return EINA_FALSE;
}

static Eina_Error
_efl_ui_image_zoomable_file_set_internal(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Evas_Load_Error *ret)
{
   const char *file = efl_file_get(obj);
   ELM_SAFE_FREE(sd->edje, evas_object_del);
   eina_stringshare_replace(&sd->stdicon, NULL);

   _grid_clear_all(obj);
   _efl_ui_image_zoomable_zoom_reset(obj, sd);
   _efl_ui_image_zoomable_bounce_reset(obj, sd);
   sd->no_smooth--;
   if (sd->no_smooth == 0) _smooth_update(obj);

   ecore_job_del(sd->calc_job);
   evas_object_hide(sd->img);
   if (sd->f) eina_file_close(sd->f);
   sd->f = NULL;

   if (sd->remote.copier) _efl_ui_image_zoomable_remote_copier_cancel(obj, sd);
   if (sd->remote.binbuf) ELM_SAFE_FREE(sd->remote.binbuf, eina_binbuf_free);

   sd->preload_num = 0;

   if (_efl_ui_image_zoomable_is_remote(file))
     {
        if (_efl_ui_image_zoomable_download(obj, sd, file))
          {
             efl_event_callback_legacy_call
               (obj, EFL_UI_IMAGE_ZOOMABLE_EVENT_DOWNLOAD_START, NULL);
             *ret = EVAS_LOAD_ERROR_NONE;
             return 0;
          }
     }

   return _internal_file_set(obj, sd, ret);
}

EOLIAN static Eina_Error
_efl_ui_image_zoomable_efl_file_load(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd)
{
   Evas_Load_Error ret;
   Eina_Error err;

   if (efl_file_loaded_get(obj)) return 0;
   err = _efl_ui_image_zoomable_file_set_internal(obj, sd, &ret);

   if ((!ret) && (!err)) return 0;

   eina_error_set(
     ret == EVAS_LOAD_ERROR_DOES_NOT_EXIST             ? PHOTO_FILE_LOAD_ERROR_DOES_NOT_EXIST :
     ret == EVAS_LOAD_ERROR_PERMISSION_DENIED          ? PHOTO_FILE_LOAD_ERROR_PERMISSION_DENIED :
     ret == EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED ? PHOTO_FILE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED :
     ret == EVAS_LOAD_ERROR_CORRUPT_FILE               ? PHOTO_FILE_LOAD_ERROR_CORRUPT_FILE :
     ret == EVAS_LOAD_ERROR_UNKNOWN_FORMAT             ? PHOTO_FILE_LOAD_ERROR_UNKNOWN_FORMAT :
     PHOTO_FILE_LOAD_ERROR_GENERIC
   );
   return err;
}

EOLIAN static Eina_Error
_efl_ui_image_zoomable_efl_file_file_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd, const char *file)
{
   eina_stringshare_replace(&sd->file, file);
   return 0;
}

EOLIAN static const char *
_efl_ui_image_zoomable_efl_file_file_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   return sd->file;
}

EOLIAN static void
_efl_ui_image_zoomable_efl_ui_zoom_zoom_level_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, double zoom)
{
   double z;
   Eina_List *l;
   Efl_Ui_Image_Zoomable_Grid *g, *g_zoom = NULL;
   Evas_Coord pw, ph;
   int zoom_changed = 0, started = 0;
   Eina_Bool an = EINA_FALSE;
   Eina_Rect view = {};
   Eina_Position2D pos = {};

   if (zoom <= (1.0 / 256.0)) zoom = (1.0 / 256.0);
   if (EINA_DBL_EQ(zoom, sd->zoom)) return;

   sd->zoom = zoom;
   sd->size.ow = sd->size.w;
   sd->size.oh = sd->size.h;
   pos = efl_ui_scrollable_content_pos_get(sd->smanager);
   view = efl_ui_scrollable_viewport_geometry_get(sd->smanager);
   if ((view.w <= 0) || (view.h <= 0)) return;

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
             ph = (sd->size.imh * view.w) / sd->size.imw;
             if (ph > view.h)
               {
                  pw = (sd->size.imw * view.h) / sd->size.imh;
                  ph = view.h;
               }
             else
               {
                  pw = view.w;
               }
             if (sd->size.imw > sd->size.imh)
               z = (double)sd->size.imw / pw;
             else
               z = (double)sd->size.imh / ph;
             if (!EINA_DBL_EQ(z, sd->zoom))
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
             sd->size.nh = 0;
          }
        else
          {
             ph = (sd->size.imh * view.w) / sd->size.imw;
             if (ph < view.h)
               {
                  pw = (sd->size.imw * view.h) / sd->size.imh;
                  ph = view.h;
               }
             else
               {
                  pw = view.w;
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
        else if ((sd->size.imw < view.w) && (sd->size.imh < view.h))
          {
             if (!EINA_DBL_EQ(sd->zoom, 1)) zoom_changed = 1;
             sd->zoom = 1;
             sd->size.nw = sd->size.imw;
             sd->size.nh = sd->size.imh;
          }
        else
          {
             ph = (sd->size.imh * view.w) / sd->size.imw;
             if (ph > view.h)
               {
                  pw = (sd->size.imw * view.h) / sd->size.imh;
                  ph = view.h;
               }
             else
               pw = view.w;
             if (sd->size.imw > sd->size.imh)
               z = (double)sd->size.imw / pw;
             else
               z = (double)sd->size.imh / ph;
             if (EINA_DBL_EQ(z, sd->zoom))
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

   EINA_LIST_FOREACH(sd->grids, l, g)
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
             EINA_LIST_FOREACH(sd->grids, l, g_zoom)
               {
                  g_zoom->dead = 1;
               }
          }
        sd->grids = eina_list_prepend(sd->grids, g);
     }
   else
     {
        EINA_LIST_FREE(sd->grids, g)
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
        sd->size.spos.x = (double)(pos.x + (view.w / 2)) / (double)sd->size.w;
        sd->size.spos.y = (double)(pos.y + (view.h / 2)) / (double)sd->size.h;
     }
   else
     {
        sd->size.spos.x = 0.5;
        sd->size.spos.y = 0.5;
     }
   if (view.w > sd->size.w) sd->size.spos.x = 0.5;
   if (view.h > sd->size.h) sd->size.spos.y = 0.5;
   if (sd->size.spos.x > 1.0) sd->size.spos.x = 1.0;
   if (sd->size.spos.y > 1.0) sd->size.spos.y = 1.0;

   if (sd->paused)
     {
        _zoom_do(obj, 1.0);
     }
   else
     {
        an = efl_event_callback_del(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _zoom_anim_cb, obj);
        efl_event_callback_add(obj, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, _zoom_anim_cb, obj);
        if (!an)
          {
             sd->no_smooth++;
             if (sd->no_smooth == 1) _smooth_update(obj);
             started = 1;
          }
     }

   if (an)
     {
        // FIXME: If one day we do support partial animator in photocam, this would require change
        Efl_Event event = {};
        event.object = evas_object_evas_get(obj);
        _zoom_anim_cb(obj, &event);
        // FIXME: Unhandled.
        if (0)
          {
             _efl_ui_image_zoomable_bounce_reset(obj, sd);
             an = 0;
          }
     }

   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job_cb, obj);
   if (!sd->paused)
     {
        if (started)
          efl_event_callback_legacy_call(obj, EFL_UI_EVENT_ZOOM_START, NULL);
        if (!an)
          efl_event_callback_legacy_call(obj, EFL_UI_EVENT_ZOOM_STOP, NULL);
     }
   if (zoom_changed)
     efl_event_callback_legacy_call(obj, EFL_UI_EVENT_ZOOM_CHANGE, NULL);
}

EOLIAN static double
_efl_ui_image_zoomable_efl_ui_zoom_zoom_level_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   return sd->zoom;
}

EOLIAN static void
_efl_ui_image_zoomable_efl_ui_zoom_zoom_mode_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Efl_Ui_Zoom_Mode mode)
{
   double tz;
   if (sd->mode == (Elm_Photocam_Zoom_Mode)mode) return;
   sd->mode = mode;

   tz = sd->zoom;
   sd->zoom = 0.0;
   elm_photocam_zoom_set(obj, tz);
}

EOLIAN static Efl_Ui_Zoom_Mode
_efl_ui_image_zoomable_efl_ui_zoom_zoom_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   return sd->mode;
}

EOLIAN static Eina_Size2D
_efl_ui_image_zoomable_efl_gfx_view_view_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *pd)
{
   return EINA_SIZE2D(pd->size.imw, pd->size.imh);
}

EOLIAN static Eina_Rect
_efl_ui_image_zoomable_image_region_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   Eina_Rect region = {};

   Eina_Position2D pos = efl_ui_scrollable_content_pos_get(sd->smanager);
   Eina_Rect view = efl_ui_scrollable_viewport_geometry_get(sd->smanager);

   if (sd->size.w > 0)
     {
        region.x = (sd->size.imw * pos.x) / sd->size.w;
        if (region.x > sd->size.imw) region.x = sd->size.imw;
        region.w = (sd->size.imw * view.w) / sd->size.w;
        if (region.w > sd->size.imw) region.w = sd->size.imw;
        else if (region.w < 0)
          region.w = 0;
     }

   if (sd->size.h > 0)
     {
        region.y = (sd->size.imh * pos.y) / sd->size.h;
        if (region.y > sd->size.imh) region.y = sd->size.imh;
        region.h = (sd->size.imh * view.h) / sd->size.h;
        if (region.h > sd->size.imh) region.h = sd->size.imh;
        else if (region.h < 0)
          region.h = 0;
     }

   return region;
}

EOLIAN static void
_efl_ui_image_zoomable_image_region_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Eina_Rect region)
{
   int rx, ry, rw, rh;

   if ((sd->size.imw < 1) || (sd->size.imh < 1)) return;
   rx = (region.x * sd->size.w) / sd->size.imw;
   ry = (region.y * sd->size.h) / sd->size.imh;
   rw = (region.w * sd->size.w) / sd->size.imw;
   rh = (region.h * sd->size.h) / sd->size.imh;
   if (rw < 1) rw = 1;
   if (rh < 1) rh = 1;
   if ((rx + rw) > sd->size.w) rx = sd->size.w - rw;
   if ((ry + rh) > sd->size.h) ry = sd->size.h - rh;

   _efl_ui_image_zoomable_bounce_reset(obj, sd);
   _efl_ui_image_zoomable_zoom_reset(obj, sd);

   efl_ui_scrollable_scroll(sd->smanager, EINA_RECT(rx, ry, rw, rh), EINA_FALSE);
}

EOLIAN static void
_efl_ui_image_zoomable_efl_ui_scrollable_interactive_scroll(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Eina_Rect rc, Eina_Bool animation)
{
   int rx, ry, rw, rh;

   if ((sd->size.imw < 1) || (sd->size.imh < 1)) return;
   rx = (rc.x * sd->size.w) / sd->size.imw;
   ry = (rc.y * sd->size.h) / sd->size.imh;
   rw = (rc.w * sd->size.w) / sd->size.imw;
   rh = (rc.h * sd->size.h) / sd->size.imh;
   if (rw < 1) rw = 1;
   if (rh < 1) rh = 1;
   if ((rx + rw) > sd->size.w) rx = sd->size.w - rw;
   if ((ry + rh) > sd->size.h) ry = sd->size.h - rh;

   _efl_ui_image_zoomable_bounce_reset(obj, sd);
   _efl_ui_image_zoomable_zoom_reset(obj, sd);

   efl_ui_scrollable_scroll(sd->smanager, EINA_RECT(rx, ry, rw, rh), animation);
}

EOLIAN static void
_efl_ui_image_zoomable_efl_ui_zoom_zoom_animation_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Eina_Bool paused)
{
   paused = !!paused;

   if (sd->paused == paused) return;
   sd->paused = paused;
   if (!sd->paused) return;

   _efl_ui_image_zoomable_bounce_reset(obj, sd);
   _efl_ui_image_zoomable_zoom_reset(obj, sd);
}

EOLIAN static Eina_Bool
_efl_ui_image_zoomable_efl_ui_zoom_zoom_animation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   return sd->paused;
}

EOLIAN static void
_efl_ui_image_zoomable_gesture_enabled_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Eina_Bool gesture)
{
   gesture = !!gesture;

   if (sd->do_gesture == gesture) return;

   sd->do_gesture = gesture;

   ELM_SAFE_FREE(sd->g_layer, evas_object_del);

   if (!gesture) return;

   sd->g_layer = elm_gesture_layer_add(obj);
   if (!sd->g_layer) return;

   elm_gesture_layer_attach(sd->g_layer, obj);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START,
     _g_layer_zoom_start_cb, obj);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE,
     _g_layer_zoom_move_cb, obj);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_END,
     _g_layer_zoom_end_cb, obj);
   elm_gesture_layer_cb_set
     (sd->g_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_ABORT,
     _g_layer_zoom_end_cb, obj);
}

static void
_min_obj_size_get(Evas_Object *o, int *w, int *h)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(o, sd);

   *w = sd->size.imw;
   *h = sd->size.imh;

   if (*w == 0 || *h == 0)
     {
        evas_object_geometry_get(o, NULL, NULL, w, h);

        if (*w == 0 || *h == 0)
          {
             edje_object_size_min_get(sd->edje, w, h);
          }
     }
}

static Eina_Error
_image_zoomable_object_icon_set(Evas_Object *o, const char *group, char *style, Eina_Bool resize)
{
   Elm_Theme *th = elm_widget_theme_get(o);
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(o, sd);

   char buf[1024];
   Eina_File *f;

   if (!th) th = elm_theme_default_get();

   snprintf(buf, sizeof(buf), "elm/icon/%s/%s", group, style);
   f = _elm_theme_group_file_find(th, buf);
   if (f)
     {
        if (sd->f) eina_file_close(sd->f);
        efl_file_key_set(o, buf);
        efl_file_mmap_set(o, f);

        return _img_proxy_set(o, sd, resize);
     }

   ELM_SAFE_FREE(sd->edje, evas_object_del);
   _sizing_eval(o);
   WRN("Failed to set icon '%s'. Icon theme '%s' not found", group, buf);
   ELM_SAFE_FREE(sd->f, eina_file_close);

   return EFL_GFX_IMAGE_LOAD_ERROR_UNKNOWN_COLLECTION;
}

static Eina_Bool
_icon_standard_set(Evas_Object *obj, const char *name, Eina_Bool resize)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   if (!_image_zoomable_object_icon_set(obj, name, "default", resize))
     {
        /* TODO: elm_unneed_efreet() */
        sd->freedesktop.use = EINA_FALSE;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_icon_freedesktop_set(Evas_Object *obj, const char *name, int size)
{
   const char *path;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   elm_need_efreet();

   if (icon_theme == NON_EXISTING) return EINA_FALSE;

   if (!icon_theme)
     {
        Efreet_Icon_Theme *theme;
        /* TODO: Listen for EFREET_EVENT_ICON_CACHE_UPDATE */
        theme = efreet_icon_theme_find(elm_config_icon_theme_get());
        if (!theme)
          {
             const char **itr;
             static const char *themes[] = {
                "gnome", "Human", "oxygen", "hicolor", NULL
             };
             for (itr = themes; *itr; itr++)
               {
                  theme = efreet_icon_theme_find(*itr);
                  if (theme) break;
               }
          }

        if (!theme)
          {
             icon_theme = NON_EXISTING;
             return EINA_FALSE;
          }
        else
          icon_theme = eina_stringshare_add(theme->name.internal);
     }
   path = efreet_icon_path_find(icon_theme, name, size);
   sd->freedesktop.use = !!path;
   if (sd->freedesktop.use)
     {
        sd->freedesktop.requested_size = size;
        efl_file_simple_load(obj, path, NULL);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static inline int
_icon_size_min_get(Evas_Object *image)
{
   int w, h;

   evas_object_geometry_get(image, NULL, NULL, &w, &h);

   return MAX(16, MIN(w, h));
}

/* FIXME: move this code to ecore */
#ifdef _WIN32
static Eina_Bool
_path_is_absolute(const char *path)
{
   //TODO: Check if this works with all absolute paths in windows
   return (isalpha(*path)) && (*(path + 1) == ':') &&
           ((*(path + 2) == '\\') || (*(path + 2) == '/'));
}

#else
static Eina_Bool
_path_is_absolute(const char *path)
{
   return *path == '/';
}

#endif

static Eina_Bool
_internal_efl_ui_image_zoomable_icon_set(Evas_Object *obj, const char *name, Eina_Bool *fdo, Eina_Bool resize)
{
   char *tmp;
   Eina_Bool ret = EINA_FALSE;

   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);

   /* try locating the icon using the specified theme */
   if (!strcmp(ELM_CONFIG_ICON_THEME_ELEMENTARY, elm_config_icon_theme_get()))
     {
        ret = _icon_standard_set(obj, name, resize);
        if (ret && fdo) *fdo = EINA_FALSE;
     }
   else
     {
        ret = _icon_freedesktop_set(obj, name, _icon_size_min_get(obj));
        if (ret && fdo) *fdo = EINA_TRUE;
     }

   if (ret)
     {
        eina_stringshare_replace(&sd->stdicon, name);
        _sizing_eval(obj);
        return EINA_TRUE;
     }
    else
      eina_stringshare_replace(&sd->stdicon, NULL);

   if (_path_is_absolute(name))
     {
        if (fdo)
          *fdo = EINA_FALSE;
        return efl_file_simple_load(obj, name, NULL);
     }

   /* if that fails, see if icon name is in the format size/name. if so,
		try locating a fallback without the size specification */
   if (!(tmp = strchr(name, '/'))) return EINA_FALSE;
   ++tmp;
   if (*tmp) return _internal_efl_ui_image_zoomable_icon_set(obj, tmp, fdo, resize);
   /* give up */
   return EINA_FALSE;
}

static void
_efl_ui_image_zoomable_icon_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);
   const char *refup = eina_stringshare_ref(sd->stdicon);
   Eina_Bool fdo = EINA_FALSE;

   if (!_internal_efl_ui_image_zoomable_icon_set(obj, sd->stdicon, &fdo, EINA_TRUE) || (!fdo))
     evas_object_event_callback_del_full
       (obj, EVAS_CALLBACK_RESIZE, _efl_ui_image_zoomable_icon_resize_cb, data);
   eina_stringshare_del(refup);
}

EOLIAN static Eina_Bool
_efl_ui_image_zoomable_efl_ui_image_icon_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *pd EINA_UNUSED, const char *name)
{
   Eina_Bool fdo = EINA_FALSE;

   if (!name) return EINA_FALSE;

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _efl_ui_image_zoomable_icon_resize_cb, obj);

   Eina_Bool int_ret = _internal_efl_ui_image_zoomable_icon_set(obj, name, &fdo, EINA_FALSE);

   if (fdo)
     evas_object_event_callback_add
       (obj, EVAS_CALLBACK_RESIZE, _efl_ui_image_zoomable_icon_resize_cb, obj);

   return int_ret;
}

EOLIAN static const char *
_efl_ui_image_zoomable_efl_ui_image_icon_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *pd)
{
   return pd->stdicon;
}

EOLIAN static Eina_Bool
_efl_ui_image_zoomable_gesture_enabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   return sd->do_gesture;
}

EOLIAN static Eina_Bool
_efl_ui_image_zoomable_efl_player_playable_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   if (sd->edje) return EINA_TRUE;
   return evas_object_image_animated_get(sd->img);
}

static Eina_Bool
_efl_ui_image_zoomable_animated_get_internal(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   if (sd->edje)
     return edje_object_animation_get(sd->edje);
   return sd->anim;
}

static void
_efl_ui_image_zoomable_animated_set_internal(Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd, Eina_Bool anim)
{
   anim = !!anim;
   if (sd->anim == anim) return;

   sd->anim = anim;

   if (sd->edje)
     {
        edje_object_animation_set(sd->edje, anim);
        return;
     }

   if (!evas_object_image_animated_get(sd->img)) return;

   if (anim)
     {
        sd->frame_count = evas_object_image_animated_frame_count_get(sd->img);
        sd->cur_frame = 1;
        sd->frame_duration =
          evas_object_image_animated_frame_duration_get
            (sd->img, sd->cur_frame, 0);
        evas_object_image_animated_frame_set(sd->img, sd->cur_frame);
     }
   else
     {
        sd->frame_count = -1;
        sd->cur_frame = -1;
        sd->frame_duration = -1;
     }
}

static Eina_Bool
_efl_ui_image_zoomable_animate_cb(void *data)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(data, sd);
   _grid_clear_all(data);

   if (!sd->anim) return ECORE_CALLBACK_CANCEL;

   sd->cur_frame++;
   if ((sd->frame_count > 0) && (sd->cur_frame > sd->frame_count))
     sd->cur_frame = sd->cur_frame % sd->frame_count;

   evas_object_image_animated_frame_set(sd->img, sd->cur_frame);
   sd->frame_duration = evas_object_image_animated_frame_duration_get
       (sd->img, sd->cur_frame, 0);

   if (sd->frame_duration > 0)
     ecore_timer_interval_set(sd->anim_timer, sd->frame_duration);

   return ECORE_CALLBACK_RENEW;
}

static void
_efl_ui_image_zoomable_animated_play_set_internal(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Eina_Bool play)
{
   if (!sd->anim) return;
   if (sd->play == play) return;
   sd->play = play;
   if (sd->edje)
     {
        edje_object_play_set(sd->edje, play);
        return;
     }
   if (play)
     {
        sd->anim_timer = ecore_timer_add
            (sd->frame_duration, _efl_ui_image_zoomable_animate_cb, obj);
     }
   else
     {
        ELM_SAFE_FREE(sd->anim_timer, ecore_timer_del);
     }
}

EOLIAN static void
_efl_ui_image_zoomable_efl_player_play_set(Eo *obj, Efl_Ui_Image_Zoomable_Data *sd, Eina_Bool play)
{
   evas_object_image_preload(sd->img, EINA_FALSE);
   if (play && !_efl_ui_image_zoomable_animated_get_internal(obj, sd))
     _efl_ui_image_zoomable_animated_set_internal(obj, sd, play);

   _efl_ui_image_zoomable_animated_play_set_internal(obj, sd, play);
}

static Eina_Bool
_efl_ui_image_zoomable_animated_play_get_internal(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *sd)
{
   if (sd->edje)
     return edje_object_play_get(sd->edje);
   return sd->play;
}

EOLIAN static Eina_Bool
_efl_ui_image_zoomable_efl_player_play_get(const Eo *obj, Efl_Ui_Image_Zoomable_Data *sd)
{
   return _efl_ui_image_zoomable_animated_play_get_internal(obj, sd);
}

EOLIAN static void
_efl_ui_image_zoomable_class_constructor(Efl_Class *klass EINA_UNUSED)
{
   PHOTO_FILE_LOAD_ERROR_GENERIC = eina_error_msg_static_register("Generic load error");
   PHOTO_FILE_LOAD_ERROR_DOES_NOT_EXIST = eina_error_msg_static_register("File does not exist");
   PHOTO_FILE_LOAD_ERROR_PERMISSION_DENIED = eina_error_msg_static_register("Permission denied to an existing file");
   PHOTO_FILE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = eina_error_msg_static_register("Allocation of resources failure prevented load");
   PHOTO_FILE_LOAD_ERROR_CORRUPT_FILE = eina_error_msg_static_register("File corrupt (but was detected as a known format)");
   PHOTO_FILE_LOAD_ERROR_UNKNOWN_FORMAT = eina_error_msg_static_register("File is not a known format");
}

EOLIAN const Efl_Access_Action_Data *
_efl_ui_image_zoomable_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Zoomable_Data *pd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "move,prior", "move", "prior", _key_action_move},
          { "move,next", "move", "next", _key_action_move},
          { "move,left", "move", "left", _key_action_move},
          { "move,right", "move", "right", _key_action_move},
          { "move,up", "move", "up", _key_action_move},
          { "move,down", "move", "down", _key_action_move},
          { "zoom,in", "zoom", "in", _key_action_zoom},
          { "zoom,out", "zoom", "out", _key_action_zoom},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_image_zoomable, Efl_Ui_Image_Zoomable_Data)

/* Internal EO APIs and hidden overrides */

#define EFL_UI_IMAGE_ZOOMABLE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_image_zoomable)

#include "efl_ui_image_zoomable.eo.c"
#include "efl_ui_image_zoomable_eo.legacy.c"

#include "efl_ui_image_zoomable_legacy_eo.h"
#define MY_CLASS_NAME_LEGACY "elm_photocam"

static void
_efl_ui_image_zoomable_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_image_zoomable_legacy_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_IMAGE_ZOOMABLE_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   return obj;
}

EAPI Evas_Object *
elm_photocam_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(EFL_UI_IMAGE_ZOOMABLE_LEGACY_CLASS, parent);
}

static inline void
_evas_orient_to_eo_orient_flip(const Evas_Image_Orient evas_orient,
                               Efl_Orient *orient, Efl_Flip *flip)
{
   switch (evas_orient) {
      case EVAS_IMAGE_ORIENT_NONE:
        *orient = EFL_ORIENT_NONE;
        *flip = EFL_FLIP_NONE;
        break;
      case EVAS_IMAGE_ORIENT_90:
        *orient = EFL_ORIENT_90;
        *flip = EFL_FLIP_NONE;
        break;
      case EVAS_IMAGE_ORIENT_180:
        *orient = EFL_ORIENT_180;
        *flip = EFL_FLIP_NONE;
        break;
      case EVAS_IMAGE_ORIENT_270:
        *orient = EFL_ORIENT_270;
        *flip = EFL_FLIP_NONE;
        break;
      case EVAS_IMAGE_FLIP_HORIZONTAL:
        *orient = EFL_ORIENT_NONE;
        *flip = EFL_FLIP_HORIZONTAL;
        break;
      case EVAS_IMAGE_FLIP_VERTICAL:
        *orient = EFL_ORIENT_NONE;
        *flip = EFL_FLIP_VERTICAL;
        break;
      case EVAS_IMAGE_FLIP_TRANSVERSE:
        *orient = EFL_ORIENT_270;
        *flip = EFL_FLIP_HORIZONTAL;
        break;
      case EVAS_IMAGE_FLIP_TRANSPOSE:
        *orient = EFL_ORIENT_270;
        *flip = EFL_FLIP_VERTICAL;
        break;
      default:
        *orient = EFL_ORIENT_NONE;
        *flip = EFL_FLIP_NONE;
        break;
     }
}

static inline Evas_Image_Orient
_eo_orient_flip_to_evas_orient(Efl_Orient orient, Efl_Flip flip)
{
   switch (flip)
     {
      default:
      case EFL_FLIP_NONE:
        switch (orient)
          {
           default:
           case EFL_ORIENT_0: return EVAS_IMAGE_ORIENT_0;
           case EFL_ORIENT_90: return EVAS_IMAGE_ORIENT_90;
           case EFL_ORIENT_180: return EVAS_IMAGE_ORIENT_180;
           case EFL_ORIENT_270: return EVAS_IMAGE_ORIENT_270;
          }
      case EFL_FLIP_HORIZONTAL:
        switch (orient)
          {
           default:
           case EFL_ORIENT_0: return EVAS_IMAGE_FLIP_HORIZONTAL;
           case EFL_ORIENT_90: return EVAS_IMAGE_FLIP_TRANSPOSE;
           case EFL_ORIENT_180: return EVAS_IMAGE_FLIP_VERTICAL;
           case EFL_ORIENT_270: return EVAS_IMAGE_FLIP_TRANSVERSE;
          }
      case EFL_FLIP_VERTICAL:
        switch (orient)
          {
           default:
           case EFL_ORIENT_0: return EVAS_IMAGE_FLIP_VERTICAL;
           case EFL_ORIENT_90: return EVAS_IMAGE_FLIP_TRANSVERSE;
           case EFL_ORIENT_180: return EVAS_IMAGE_FLIP_HORIZONTAL;
           case EFL_ORIENT_270: return EVAS_IMAGE_FLIP_TRANSPOSE;
          }
     }
}

EAPI void
elm_photocam_image_orient_set(Eo *obj, Evas_Image_Orient evas_orient)
{
   Efl_Orient orient;
   Efl_Flip flip;

   _evas_orient_to_eo_orient_flip(evas_orient, &orient, &flip);
   efl_orientation_set(obj, orient);
   efl_orientation_flip_set(obj, flip);
}

EAPI Evas_Image_Orient
elm_photocam_image_orient_get(const Eo *obj)
{
   ELM_PHOTOCAM_CHECK(obj) EVAS_IMAGE_ORIENT_NONE;
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   return _eo_orient_flip_to_evas_orient(sd->orient, sd->flip);
}

EAPI Evas_Object*
elm_photocam_internal_image_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return sd->img;
}

EAPI void
elm_photocam_image_size_get(const Evas_Object *obj, int *w, int *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_view_size_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

EAPI Eina_Bool
elm_photocam_paused_get(const Evas_Object *obj)
{
   return efl_ui_zoom_animation_get(obj);
}

EAPI void
elm_photocam_paused_set(Evas_Object *obj, Eina_Bool paused)
{
   efl_ui_zoom_animation_set(obj, paused);
}

EAPI void
elm_photocam_zoom_set(Evas_Object *obj, double zoom)
{
   efl_ui_zoom_level_set(obj, zoom);
}

EAPI double
elm_photocam_zoom_get(const Evas_Object *obj)
{
   return efl_ui_zoom_level_get(obj);
}

EAPI void
elm_photocam_zoom_mode_set(Evas_Object *obj, Elm_Photocam_Zoom_Mode mode)
{
   efl_ui_zoom_mode_set(obj, mode);
}

EAPI Elm_Photocam_Zoom_Mode
elm_photocam_zoom_mode_get(const Evas_Object *obj)
{
   return efl_ui_zoom_mode_get(obj);
}

EAPI Evas_Load_Error
elm_photocam_file_set(Evas_Object *obj, const char *file)
{
   ELM_PHOTOCAM_CHECK(obj) EVAS_LOAD_ERROR_GENERIC;
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EVAS_LOAD_ERROR_GENERIC);
   if (efl_file_simple_load(obj, file, NULL)) return EVAS_LOAD_ERROR_NONE;

   Eina_Error err = eina_error_get();
   return err == PHOTO_FILE_LOAD_ERROR_DOES_NOT_EXIST ?
            EVAS_LOAD_ERROR_DOES_NOT_EXIST :
          err == PHOTO_FILE_LOAD_ERROR_PERMISSION_DENIED ?
            EVAS_LOAD_ERROR_PERMISSION_DENIED :
          err == PHOTO_FILE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED ?
            EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED :
          err == PHOTO_FILE_LOAD_ERROR_CORRUPT_FILE ?
            EVAS_LOAD_ERROR_CORRUPT_FILE :
          err == PHOTO_FILE_LOAD_ERROR_UNKNOWN_FORMAT ?
            EVAS_LOAD_ERROR_UNKNOWN_FORMAT :
          EVAS_LOAD_ERROR_GENERIC;
}

EAPI const char*
elm_photocam_file_get(const Evas_Object *obj)
{
   return efl_file_get(obj);
}

EAPI void
elm_photocam_image_region_show(Evas_Object *obj, int x, int y, int w, int h)
{
   efl_ui_image_zoomable_image_region_set(obj, EINA_RECT(x, y, w, h));
}

EAPI void
elm_photocam_image_region_bring_in(Evas_Object *obj,
                                   int x,
                                   int y,
                                   int w,
                                   int h EINA_UNUSED)
{
   ELM_PHOTOCAM_CHECK(obj);
   EFL_UI_IMAGE_ZOOMABLE_DATA_GET(obj, sd);
   efl_ui_scrollable_scroll(sd->smanager, EINA_RECT(x, y, w, h), EINA_TRUE);
}

EAPI void
elm_photocam_bounce_set(Evas_Object *obj,
                        Eina_Bool h_bounce,
                        Eina_Bool v_bounce)
{
   ELM_PHOTOCAM_CHECK(obj);

   efl_ui_scrollable_bounce_enabled_set(obj, h_bounce, v_bounce);
}

EAPI void
elm_photocam_bounce_get(const Evas_Object *obj,
                        Eina_Bool *h_bounce,
                        Eina_Bool *v_bounce)
{
   ELM_PHOTOCAM_CHECK(obj);

   efl_ui_scrollable_bounce_enabled_get((Eo *)obj, h_bounce, v_bounce);
}

EAPI void
elm_photocam_image_region_get(const Efl_Ui_Image_Zoomable *obj, int *x, int *y, int *w, int *h)
{
   Eina_Rect r;

   r = efl_ui_image_zoomable_image_region_get(obj);
   if (x) *x = r.x;
   if (y) *y = r.y;
   if (w) *w = r.w;
   if (h) *h = r.h;
}

#include "efl_ui_image_zoomable_legacy_eo.c"
