#ifndef _ELM_SEGMENT_CONTROL_ITEM_EO_H_
#define _ELM_SEGMENT_CONTROL_ITEM_EO_H_

#ifndef _ELM_SEGMENT_CONTROL_ITEM_EO_CLASS_TYPE
#define _ELM_SEGMENT_CONTROL_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Segment_Control_Item;

#endif

#ifndef _ELM_SEGMENT_CONTROL_ITEM_EO_TYPES
#define _ELM_SEGMENT_CONTROL_ITEM_EO_TYPES


#endif
/** Elementary segment control item class
 *
 * @ingroup Elm_Segment_Control_Item
 */
#define ELM_SEGMENT_CONTROL_ITEM_CLASS elm_segment_control_item_class_get()

EWAPI const Efl_Class *elm_segment_control_item_class_get(void);

/**
 * @brief Get the index of an item.
 *
 * Index is the position of an item in segment control widget. Its range is
 * from 0 to (count - 1). Count is the number of items, that can be get with
 * elm_segment_control_item_count_get()
 *
 * @param[in] obj The object.
 *
 * @return The position of item in segment control widget.
 *
 * @ingroup Elm_Segment_Control_Item
 */
EOAPI int elm_obj_segment_control_item_index_get(const Eo *obj);

/**
 * @brief Get the real Evas(Edje) object created to implement the view of a
 * given segment_control item.
 *
 * Base object is the @ref Efl_Canvas_Object that represents that item.
 *
 * @param[in] obj The object.
 *
 * @return The base Edje object associated with @c it
 *
 * @ingroup Elm_Segment_Control_Item
 */
EOAPI Efl_Canvas_Object *elm_obj_segment_control_item_object_get(const Eo *obj);

/**
 * @brief Set the selected state of an item.
 *
 * This sets the selected state of the given item @c it. @c true for selected,
 * @c false for not selected.
 *
 * If a new item is selected the previously selected will be unselected.
 * Selected item can be got with function
 * elm_segment_control_item_selected_get().
 *
 * The selected item always will be highlighted on segment control.
 *
 * @param[in] obj The object.
 * @param[in] selected The selected state.
 *
 * @ingroup Elm_Segment_Control_Item
 */
EOAPI void elm_obj_segment_control_item_selected_set(Eo *obj, Eina_Bool selected);

#endif
