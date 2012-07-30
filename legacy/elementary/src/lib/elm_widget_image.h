#ifndef ELM_WIDGET_IMAGE_H
#define ELM_WIDGET_IMAGE_H

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
  * @def ELM_IMAGE_CLASS
  *
  * Use this macro to cast whichever subclass of
  * #Elm_Image_Smart_Class into it, so to access its fields.
  *
  * @ingroup Widget
  */
 #define ELM_IMAGE_CLASS(x) ((Elm_Image_Smart_Class *) x)

/**
 * @def ELM_IMAGE_DATA
 *
 * Use this macro to cast whichever subdata of
 * #Elm_Image_Smart_Data into it, so to access its fields.
 *
 * @ingroup Widget
 */
#define ELM_IMAGE_DATA(x) ((Elm_Image_Smart_Data *) x)

/**
 * @def ELM_IMAGE_SMART_CLASS_VERSION
 *
 * Current version for Elementary image @b base smart class, a value
 * which goes to _Elm_Image_Smart_Class::version.
 *
 * @ingroup Widget
 */
#define ELM_IMAGE_SMART_CLASS_VERSION 1

/**
 * @def ELM_IMAGE_SMART_CLASS_INIT
 *
 * Initializer for a whole #Elm_Image_Smart_Class structure, with
 * @c NULL values on its specific fields.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (#Evas_Smart_Class).
 *
 * @see EVAS_SMART_CLASS_INIT_NULL
 * @see EVAS_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_IMAGE_SMART_CLASS_INIT_NULL
 * @see ELM_IMAGE_SMART_CLASS_INIT_NAME_VERSION
 *
 * @ingroup Widget
 */
#define ELM_IMAGE_SMART_CLASS_INIT(smart_class_init)                        \
  {smart_class_init, ELM_IMAGE_SMART_CLASS_VERSION, NULL, NULL, NULL, NULL, \
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  \
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

/**
 * @def ELM_IMAGE_SMART_CLASS_INIT_NULL
 *
 * Initializer to zero out a whole #Elm_Image_Smart_Class structure.
 *
 * @see ELM_IMAGE_SMART_CLASS_INIT_NAME_VERSION
 * @see ELM_IMAGE_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_IMAGE_SMART_CLASS_INIT_NULL \
  ELM_IMAGE_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * @def ELM_IMAGE_SMART_CLASS_INIT_NAME_VERSION
 *
 * Initializer to zero out a whole #Elm_Image_Smart_Class structure and
 * set its name and version.
 *
 * This is similar to #ELM_IMAGE_SMART_CLASS_INIT_NULL, but it will
 * also set the version field of #Elm_Image_Smart_Class (base field)
 * to the latest #ELM_IMAGE_SMART_CLASS_VERSION and name it to the
 * specific value.
 *
 * It will keep a reference to the name field as a <c>"const char *"</c>,
 * i.e., the name must be available while the structure is
 * used (hint: static or global variable!) and must not be modified.
 *
 * @see ELM_IMAGE_SMART_CLASS_INIT_NULL
 * @see ELM_IMAGE_SMART_CLASS_INIT
 *
 * @ingroup Widget
 */
#define ELM_IMAGE_SMART_CLASS_INIT_NAME_VERSION(name) \
  ELM_IMAGE_SMART_CLASS_INIT(ELM_WIDGET_SMART_CLASS_INIT_NAME_VERSION(name))

/**
 * Elementary image base smart class. This inherits directly from
 * #Elm_Widget_Smart_Class and is meant to build widgets relying on an
 * image as the building block of its visuals.
 */

typedef struct _Elm_Image_Smart_Class
{
   Elm_Widget_Smart_Class base; /**< Base Elementary widget class struct, since we're inheriting from it */

   int                    version; /**< Version of this smart class definition */

   void                 (*sizing_eval)(Evas_Object *obj); /* 'Virtual' function on evalutating the object's final geometry */
   Eina_Bool            (*memfile_set)(Evas_Object *obj,
                                       const void *img,
                                       size_t size,
                                       const char *format,
                                       const char *key); /* 'Virtual' function on setting the image content on the object via a memory buffer */
   Eina_Bool            (*file_set)(Evas_Object *obj,
                                    const char *file,
                                    const char *key); /* 'Virtual' function on setting the image content on the object via a file. It may be a direct image, an EET-encoded image or an Edje file. They @a key argument will be used for the last two cases mentioned (@c NULL must be used on the first). Beware that the Edje case will only be detected by an @c "edj" extension on the file name. */
   void                 (*file_get)(const Evas_Object *obj,
                                    const char **file,
                                    const char **key); /* 'Virtual' function on retrieving back the image object's file path and key */

   Evas_Object        *(*image_object_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving the object's internal image file, which may me an Evas image or an Edje file */

   void                 (*size_get)(const Evas_Object *obj,
                                    int *w,
                                    int *h); /* 'Virtual' function on retrieving the size of the object's internal image. */

   void                 (*preload_set)(Evas_Object *obj,
                                       Eina_Bool disable); /* 'Virtual' function on enabling/disabling pre-loading for the object's image data. */
   void                 (*fill_inside_set)(Evas_Object *obj,
                                           Eina_Bool fill_inside); /* 'Virtual' function on how to resize the object's internal image, when maintaining a given aspect ratio -- leave blank spaces or scale to fill all space, with pixels out of bounds. */
   Eina_Bool            (*fill_inside_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving how the object's internal image is to be resized, when maintaining a given aspect ratio. */

   void                 (*scale_set)(Evas_Object *obj,
                                     double scale); /* 'Virtual' function on setting the scale for the object's image size (@c 1.0 meaning original size). */
   double               (*scale_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving the scale for the object's image size. */

   void                 (*smooth_scale_set)(Evas_Object *obj,
                                            Eina_Bool smooth); /* 'Virtual' function on setting whether the object's image should be scaled smoothly or not. */
   Eina_Bool            (*smooth_scale_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving whether the object's image is to scaled smoothly or not. */
   void                 (*resize_up_set)(Evas_Object *obj,
                                        Eina_Bool resize_up); /* 'Virtual' function on setting whether the object's image can be resized to a size greater than the original one. */
   Eina_Bool            (*resize_up_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving whether the object's image can be resized to a size greater than the original one. */
   void                 (*resize_down_set)(Evas_Object *obj,
                                        Eina_Bool resize_down); /* 'Virtual' function on setting whether the object's image can be resized to a size smaller than the original one. */
   Eina_Bool            (*resize_down_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving whether the object's image can be resized to a size smaller than the original one. */
   void                 (*load_size_set)(Evas_Object *obj,
                                         int size); /* 'Virtual' function on setting the object's image loading size (in pixels, applied to both axis). */
   int                  (*load_size_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving the object's image loading size. */

   void                 (*orient_set)(Evas_Object *obj,
                                      Elm_Image_Orient orient); /* 'Virtual' function on setting the object's image orientation. */
   Elm_Image_Orient     (*orient_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving the object's image orientation. */

   void                 (*aspect_fixed_set)(Evas_Object *obj,
                                            Eina_Bool fixed); /* 'Virtual' function on setting whether the original aspect ratio of the object's image should be kept if it's resized. */
   Eina_Bool            (*aspect_fixed_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving whether the original aspect ratio of the object's image is to be kept if it's resized. */

   void                 (*edit_set)(Evas_Object *obj,
                                    Eina_Bool edit,
                                    Evas_Object *parent); /* 'Virtual' function on setting whether the object is a valid target/source for drag and drop actions. */
   Eina_Bool            (*edit_get)(const Evas_Object *obj); /* 'Virtual' function on retrieving whether the object is a valid target/source for drag and drop actions. */
} Elm_Image_Smart_Class;

/**
 * Base widget smart data extended with image instance data.
 */
typedef struct _Elm_Image_Smart_Data Elm_Image_Smart_Data;
struct _Elm_Image_Smart_Data
{
   Elm_Widget_Smart_Data base;

   Evas_Object          *img;
   Evas_Object          *prev_img;

   Evas_Coord            img_x, img_y, img_w, img_h;

   int                   load_size;
   double                scale;
   Elm_Image_Orient      orient;

   int                   frame_count;
   int                   cur_frame;
   double                frame_duration;

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

   Ecore_Timer          *anim_timer;
};

/**
 * @}
 */

EAPI extern const char ELM_IMAGE_SMART_NAME[];
EAPI const Elm_Image_Smart_Class *elm_image_smart_class_get(void);

#define ELM_IMAGE_DATA_GET(o, sd) \
  Elm_Image_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_IMAGE_DATA_GET_OR_RETURN(o, ptr)         \
  ELM_IMAGE_DATA_GET(o, ptr);                        \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_IMAGE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_IMAGE_DATA_GET(o, ptr);                         \
  if (!ptr)                                           \
    {                                                 \
       CRITICAL("No widget data for object %p (%s)",  \
                o, evas_object_type_get(o));          \
       return val;                                    \
    }

#define ELM_IMAGE_CHECK(obj)                                                 \
  if (!obj || !elm_widget_type_check((obj), ELM_IMAGE_SMART_NAME, __func__)) \
    return

#endif
