/**
 * @addtogroup Elm_Slideshow
 *
 * @{
 */

typedef Evas_Object                        *(*SlideshowItemGetFunc)(void *data, Evas_Object *obj); /**< Image fetching class function for slideshow item classes. */
typedef void                                (*SlideshowItemDelFunc)(void *data, Evas_Object *obj); /**< Deletion class function for slideshow item classes. */

/**
 * Slideshow item class definition struct
 */
typedef struct _Elm_Slideshow_Item_Class      Elm_Slideshow_Item_Class;

/**
 * Class functions for slideshow item classes.
 */
typedef struct _Elm_Slideshow_Item_Class_Func Elm_Slideshow_Item_Class_Func;
/**
 * @struct _Elm_Slideshow_Item_Class
 *
 * Slideshow item class definition. See @ref Slideshow_Items for
 * field details.
 */
struct _Elm_Slideshow_Item_Class
{
   struct _Elm_Slideshow_Item_Class_Func
     {
        SlideshowItemGetFunc get;
        SlideshowItemDelFunc del;
     } func;
};   /**< member definitions of #Elm_Slideshow_Item_Class */

/**
 * @}
 */
