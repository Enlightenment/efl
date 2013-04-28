typedef struct _Elm_Slideshow_Item_Class      Elm_Slideshow_Item_Class; /**< Slideshow item class definition struct */
typedef struct _Elm_Slideshow_Item_Class_Func Elm_Slideshow_Item_Class_Func;    /**< Class functions for slideshow item classes. */
typedef Evas_Object                        *(*SlideshowItemGetFunc)(void *data, Evas_Object *obj); /**< Image fetching class function for slideshow item classes. */
typedef void                                (*SlideshowItemDelFunc)(void *data, Evas_Object *obj); /**< Deletion class function for slideshow item classes. */

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
 * Display a given slideshow widget's item, programmatically.
 *
 * @param it The item to display on @p obj's viewport
 *
 * The change between the current item and @p item will use the
 * transition @p obj is set to use (@see
 * elm_slideshow_transition_set()).
 *
 * @ingroup Slideshow
 */
EAPI void                  elm_slideshow_item_show(Elm_Object_Item *it);

/**
 * Get the real Evas object created to implement the view of a
 * given slideshow item
 *
 * @param it The slideshow item.
 * @return the Evas object implementing this item's view.
 *
 * This returns the actual Evas object used to implement the
 * specified slideshow item's view. This may be @c NULL, as it may
 * not have been created or may have been deleted, at any time, by
 * the slideshow. <b>Do not modify this object</b> (move, resize,
 * show, hide, etc.), as the slideshow is controlling it. This
 * function is for querying, emitting custom signals or hooking
 * lower level callbacks for events on that object. Do not delete
 * this object under any circumstances.
 *
 * @see elm_object_item_data_get()
 *
 * @ingroup Slideshow
 */
EAPI Evas_Object          *elm_slideshow_item_object_get(const Elm_Object_Item *it);
