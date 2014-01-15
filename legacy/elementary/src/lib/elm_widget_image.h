#ifndef ELM_WIDGET_IMAGE_H
#define ELM_WIDGET_IMAGE_H

#include "Elementary.h"

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-image-class The Elementary Image Class
 *
 * This class defines a common interface for @b image objects having
 * an image as their basic graphics. This interface is so that one can
 * tune various properties of the image, like:
 * - smooth scaling,
 * - orientation,
 * - aspect ratio during resizes, etc.
 *
 * Image files may be set via memory buffers, image files, EET files
 * with image data or Edje files. On the last case (which is
 * exceptional), most of the properties cited above will @b not be
 * changeable anymore.
 */

/**
 * Base widget smart data extended with image instance data.
 */
typedef struct _Elm_Image_Smart_Data Elm_Image_Smart_Data;
struct _Elm_Image_Smart_Data
{
   Evas_Object          *hit_rect;
   Evas_Object          *img;
   Evas_Object          *prev_img;
   Ecore_Timer          *anim_timer;

   Elm_Url              *remote;
   const char           *key;
   void                 *remote_data;

   double                scale;
   double                frame_duration;

   Evas_Coord            img_x, img_y, img_w, img_h;

   int                   load_size;
   int                   frame_count;
   int                   cur_frame;

   Elm_Image_Orient      orient;

   Eina_Bool             aspect_fixed : 1;
   Eina_Bool             fill_inside : 1;
   Eina_Bool             resize_down : 1;
   Eina_Bool             preloading : 1;
   Eina_Bool             resize_up : 1;
   Eina_Bool             no_scale : 1;
   Eina_Bool             smooth : 1;
   Eina_Bool             show : 1;
   Eina_Bool             edit : 1;
   Eina_Bool             edje : 1;
   Eina_Bool             anim : 1;
   Eina_Bool             play : 1;
};

/**
 * @}
 */

#define ELM_IMAGE_DATA_GET(o, sd) \
  Elm_Image_Smart_Data * sd = eo_data_scope_get(o, ELM_OBJ_IMAGE_CLASS)

#define ELM_IMAGE_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_IMAGE_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define ELM_IMAGE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_IMAGE_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define ELM_IMAGE_CHECK(obj)                              \
  if (EINA_UNLIKELY(!eo_isa((obj), ELM_OBJ_IMAGE_CLASS))) \
    return

#endif
