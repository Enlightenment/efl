#ifndef EFL_UI_IMAGE_ZOOMABLE_PRIVATE_H
#define EFL_UI_IMAGE_ZOOMABLE_PRIVATE_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

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
typedef struct _Efl_Ui_Image_Zoomable_Data           Efl_Ui_Image_Zoomable_Data;
typedef struct _Efl_Ui_Image_Zoomable_Pan_Data       Efl_Ui_Image_Zoomable_Pan_Data;
typedef struct _Efl_Ui_Image_Zoomable_Grid           Efl_Ui_Image_Zoomable_Grid;
typedef struct _Efl_Ui_Image_Zoomable_Grid_Item      Efl_Ui_Image_Zoomable_Grid_Item;

struct _Efl_Ui_Image_Zoomable_Grid_Item
{
   Evas_Object             *obj;
   Efl_Ui_Image_Zoomable_Data       *sd;
   Evas_Object             *img;

   struct
   {
      int x, y, w, h;
   } src, out;

   Eina_Bool                want : 1;
   Eina_Bool                have : 1;
};

struct _Efl_Ui_Image_Zoomable_Grid
{
   int                     tsize; /* size of tile (tsize x tsize pixels) */
   int                     zoom; /* zoom level tiles want for optimal
                                  * display (1, 2, 4, 8) */
   int                     iw, ih; /* size of image in pixels */
   int                     w, h; /* size of grid image in pixels
                                  * (represented by grid) */
   int                     gw, gh; /* size of grid in tiles */
   Efl_Ui_Image_Zoomable_Grid_Item *grid;  /* the grid (gw * gh items) */
   Eina_Bool               dead : 1; /* old grid. will die as soon as anim is
                                      * over */
};

struct _Efl_Ui_Image_Zoomable_Data
{
   Eo                                   *smanager;
   Eo                                   *pan_obj;
   Evas_Object                          *g_layer;


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
      } bounce;
   } g_layer_zoom;

   const char     *file;
   Eina_File      *f;

   struct
   {
      Eo *copier;
      Eina_Binbuf *binbuf;
   } remote;

   Ecore_Job      *calc_job;
   Ecore_Timer    *scr_timer;
   Ecore_Timer    *long_timer;
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

   Eina_Rect show;

   int          tsize;
   Evas_Object *img;  /* low res version of image (scale down == 8) */
   int          no_smooth;
   int          preload_num;

   struct {
      int       requested_size;
      Eina_Bool use : 1;
   } freedesktop;
   Evas_Object          *edje;
   const char           *stdicon;

   Ecore_Timer          *anim_timer;
   double                frame_duration;
   int                   cur_frame;
   int                   frame_count;

   Eina_List   *grids;
   Efl_Orient   orient;

   Eina_Bool    main_load_pending : 1;
   Eina_Bool    longpressed : 1;
   Eina_Bool    do_gesture : 1;
   Eina_Bool    do_region : 1;
   Eina_Bool    zoom_g_layer : 1;
   Eina_Bool    resized : 1;
   Eina_Bool    on_hold : 1;
   Eina_Bool    paused : 1;
   Eina_Bool    orientation_changed : 1;
   Eina_Bool    play : 1;
   Eina_Bool    anim : 1;
   Eina_Bool    freeze_want : 1;
   Eina_Bool    show_item: 1;
};

struct _Efl_Ui_Image_Zoomable_Pan_Data
{
   Eo                              *wobj;
   Efl_Ui_Image_Zoomable_Data      *wsd;
};

/**
 * @}
 */

#define EFL_UI_IMAGE_ZOOMABLE_DATA_GET(o, sd) \
  Efl_Ui_Image_Zoomable_Data * sd = efl_data_scope_get(o, EFL_UI_IMAGE_ZOOMABLE_CLASS)

#define EFL_UI_IMAGE_ZOOMABLE_PAN_DATA_GET(o, sd) \
  Efl_Ui_Image_Zoomable_Pan_Data * sd = efl_data_scope_get(o, EFL_UI_IMAGE_ZOOMABLE_PAN_CLASS)

#define EFL_UI_IMAGE_ZOOMABLE_DATA_GET_OR_RETURN(o, ptr)      \
  EFL_UI_IMAGE_ZOOMABLE_DATA_GET(o, ptr);                     \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_IMAGE_ZOOMABLE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_IMAGE_ZOOMABLE_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                               \
    {                                                    \
       ERR("No widget data for object %p (%s)",          \
           o, evas_object_type_get(o));                  \
       return val;                                       \
    }

#define ELM_PHOTOCAM_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_IMAGE_ZOOMABLE_CLASS))) \
    return

#endif
