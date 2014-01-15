#ifndef ELM_WIDGET_PHOTOCAM_H
#define ELM_WIDGET_PHOTOCAM_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-photocam-class The Elementary Photocam Class
 *
 * Elementary, besides having the @ref Photocam widget, exposes its
 * foundation -- the Elementary Photocam Class -- in order to create
 * other widgets which are a photocam with some more logic on top.
 */

/**
 * Base widget smart data extended with photocam instance data.
 */
typedef struct _Elm_Photocam_Smart_Data     Elm_Photocam_Smart_Data;
typedef struct _Elm_Photocam_Pan_Smart_Data Elm_Photocam_Pan_Smart_Data;
typedef struct _Elm_Phocam_Grid             Elm_Phocam_Grid;
typedef struct _Elm_Photocam_Grid_Item      Elm_Photocam_Grid_Item;

struct _Elm_Photocam_Grid_Item
{
   Evas_Object             *obj;
   Elm_Photocam_Smart_Data *sd;
   Evas_Object             *img;

   struct
   {
      int x, y, w, h;
   } src, out;

   Eina_Bool                want : 1;
   Eina_Bool                have : 1;
};

struct _Elm_Phocam_Grid
{
   int                     tsize; /* size of tile (tsize x tsize pixels) */
   int                     zoom; /* zoom level tiles want for optimal
                                  * display (1, 2, 4, 8) */
   int                     iw, ih; /* size of image in pixels */
   int                     w, h; /* size of grid image in pixels
                                  * (represented by grid) */
   int                     gw, gh; /* size of grid in tiles */
   Elm_Photocam_Grid_Item *grid;  /* the grid (gw * gh items) */
   Eina_Bool               dead : 1; /* old grid. will die as soon as anim is
                                      * over */
};

struct _Elm_Photocam_Smart_Data
{
   Evas_Object                          *hit_rect;
   Evas_Object                          *g_layer;

   Evas_Object                          *pan_obj;

   Evas_Coord                            pan_x, pan_y, minw, minh;

   double                                g_layer_start;
   double                                zoom;
   Elm_Photocam_Zoom_Mode                mode;
   Evas_Coord                            pvx, pvy, px, py, zoom_point_x,
                                         zoom_point_y;

   struct
   {
      int imx, imy;
      struct
      {
         int             x_start, y_start;
         int             x_end, y_end;
         double          t_start;
         double          t_end;
         Ecore_Animator *animator;
      } bounce;
   } g_layer_zoom;

   const char     *file;
   Eina_File      *f;

   Elm_Url        *remote;
   void           *remote_data;

   Ecore_Job      *calc_job;
   Ecore_Timer    *scr_timer;
   Ecore_Timer    *long_timer;
   Ecore_Animator *zoom_animator;
   double          t_start, t_end;

   struct
   {
      int imw, imh;
      int w, h;
      int ow, oh, nw, nh;
      struct
      {
         double x, y;
      } spos;
   } size;

   struct
   {
      Eina_Bool  show : 1;
      Evas_Coord x, y, w, h;
   } show;

   int          tsize;
   Evas_Object *img;  /* low res version of image (scale down == 8) */
   int          no_smooth;
   int          preload_num;

   Eina_List   *grids;

   Eina_Bool    main_load_pending : 1;
   Eina_Bool    longpressed : 1;
   Eina_Bool    do_gesture : 1;
   Eina_Bool    do_region : 1;
   Eina_Bool    zoom_g_layer : 1;
   Eina_Bool    resized : 1;
   Eina_Bool    on_hold : 1;
   Eina_Bool    paused : 1;
};

struct _Elm_Photocam_Pan_Smart_Data
{
   Evas_Object            *wobj;
   Elm_Photocam_Smart_Data *wsd;
};

/**
 * @}
 */

#define ELM_PHOTOCAM_DATA_GET(o, sd) \
  Elm_Photocam_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_PHOTOCAM_CLASS)

#define ELM_PHOTOCAM_PAN_DATA_GET(o, sd) \
  Elm_Photocam_Pan_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_PHOTOCAM_PAN_CLASS)

#define ELM_PHOTOCAM_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_PHOTOCAM_DATA_GET(o, ptr);                     \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_PHOTOCAM_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_PHOTOCAM_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                               \
    {                                                    \
       CRI("No widget data for object %p (%s)",          \
           o, evas_object_type_get(o));                  \
       return val;                                       \
    }

#define ELM_PHOTOCAM_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_PHOTOCAM_CLASS))) \
    return

#endif
