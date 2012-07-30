#ifndef ELM_WIDGET_PHOTOCAM_H
#define ELM_WIDGET_PHOTOCAM_H

#include "elm_interface_scrollable.h"

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
 * @def ELM_PHOTOCAM_CLASS
 *
 * Use this macro to cast whichever subclass of
 * #Elm_Photocam_Smart_Class into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_PHOTOCAM_CLASS(x) ((Elm_Photocam_Smart_Class *) x)

/**
 * @def ELM_PHOTOCAM_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Photocam_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_PHOTOCAM_DATA(x) ((Elm_Photocam_Smart_Data *) x)

/**
 * @def ELM_PHOTOCAM_SMART_CLASS_VERSION
 *
 * Current version for Elementary photocam @b base smart class, a value
 * which goes to _Elm_Photocam_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_PHOTOCAM_SMART_CLASS_VERSION 1

/**
 * @def ELM_PHOTOCAM_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Photocam_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_PHOTOCAM_SMART_CLASS_INIT_NULL
 * @see ELM_PHOTOCAM_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_PHOTOCAM_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_PHOTOCAM_SMART_CLASS_VERSION}

/**
 * @def ELM_PHOTOCAM_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Photocam_Smart_Class structure.
 *
 * @see ELM_PHOTOCAM_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_PHOTOCAM_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_PHOTOCAM_SMART_CLASS_INIT_NULL \
  ELM_PHOTOCAM_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_PHOTOCAM_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Photocam_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_PHOTOCAM_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Photocam_Smart_Class (base field)
 * to the latest #ELM_PHOTOCAM_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_PHOTOCAM_SMART_CLASS_INIT_NULL
 * @see ELM_PHOTOCAM_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_PHOTOCAM_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_PHOTOCAM_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary photocam base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets extending the
 * behavior of a photocam.
 *
 * All of the functions listed on @ref Photocam namespace will work for
 * objects deriving from #Elm_Photocam_Smart_Class.
 */
typedef struct _Elm_Photocam_Smart_Class
{
   Elm_Widget_Smart_Class base;

   int                    version; /**< Version of this smart class definition */
} Elm_Photocam_Smart_Class;

/**
 * Base widget smart data extended with photocam instance data.
 */
typedef struct _Elm_Photocam_Smart_Data     Elm_Photocam_Smart_Data;
typedef struct _Elm_Photocam_Pan_Smart_Data Elm_Photocam_Pan_Smart_Data;
typedef struct _Elm_Phocam_Grid             Elm_Phocam_Grid;
typedef struct _Elm_Photocam_Grid_Item      Elm_Photocam_Grid_Item;

struct _Elm_Photocam_Grid_Item
{
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
   Elm_Widget_Smart_Data                 base; /* base widget smart data as
                                                * first member obligatory, as
                                                * we're inheriting from it */

   Evas_Object                          *hit_rect;
   Evas_Object                          *g_layer;

   const Elm_Scrollable_Smart_Interface *s_iface;

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

typedef struct _Elm_Photocam_Pan_Smart_Class
{
   Elm_Pan_Smart_Class base;
} Elm_Photocam_Pan_Smart_Class;

struct _Elm_Photocam_Pan_Smart_Data
{
   Elm_Pan_Smart_Data       base;
   Elm_Photocam_Smart_Data *wsd;
};

/**
 * @}
 */

EAPI extern const char ELM_PHOTOCAM_SMART_NAME[];
EAPI extern const char ELM_PHOTOCAM_PAN_SMART_NAME[];
EAPI const Elm_Photocam_Smart_Class *elm_photocam_smart_class_get(void);

#define ELM_PHOTOCAM_DATA_GET(o, sd) \
  Elm_Photocam_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_PHOTOCAM_PAN_DATA_GET(o, sd) \
  Elm_Photocam_Pan_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_PHOTOCAM_DATA_GET_OR_RETURN(o, ptr)      \
  ELM_PHOTOCAM_DATA_GET(o, ptr);                     \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_PHOTOCAM_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_PHOTOCAM_DATA_GET(o, ptr);                         \
  if (!ptr)                                              \
    {                                                    \
       CRITICAL("No widget data for object %p (%s)",     \
                o, evas_object_type_get(o));             \
       return val;                                       \
    }

#define ELM_PHOTOCAM_CHECK(obj)                     \
  if (!obj || !elm_widget_type_check                \
        ((obj), ELM_PHOTOCAM_SMART_NAME, __func__)) \
    return

#endif
