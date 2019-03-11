#ifndef _ELM_SLIDESHOW_ITEM_EO_LEGACY_H_
#define _ELM_SLIDESHOW_ITEM_EO_LEGACY_H_

#ifndef _ELM_SLIDESHOW_ITEM_EO_CLASS_TYPE
#define _ELM_SLIDESHOW_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Slideshow_Item;

#endif

#ifndef _ELM_SLIDESHOW_ITEM_EO_TYPES
#define _ELM_SLIDESHOW_ITEM_EO_TYPES


#endif

/**
 * @brief Display a given slideshow widget's item, programmatically.
 *
 * The change between the current item and @c item will use the transition
 * @c obj is set to use.
 *
 * See: Elm.Slidshow.transition.get
 * @param[in] obj The object.
 *
 * @ingroup Elm_Slideshow_Item_Group
 */
EAPI void elm_slideshow_item_show(Elm_Slideshow_Item *obj);

/**
 * @brief Get the real Evas object created to implement the view of a given
 * slideshow item.
 *
 * This returns the actual Evas object used to implement the specified
 * slideshow item's view. This may be @c null, as it may not have been created
 * or may have been deleted, at any time, by the slideshow. **Do not modify
 * this object** (move, resize, show, hide, etc.), as the slideshow is
 * controlling it. This function is for querying, emitting custom signals or
 * hooking lower level callbacks for events on that object. Do not delete this
 * object under any circumstances.
 *
 * @param[in] obj The object.
 *
 * @return Real evas object
 *
 * @ingroup Elm_Slideshow_Item_Group
 */
EAPI Efl_Canvas_Object *elm_slideshow_item_object_get(const Elm_Slideshow_Item *obj);

#endif
