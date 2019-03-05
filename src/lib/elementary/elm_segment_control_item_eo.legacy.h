#ifndef _ELM_SEGMENT_CONTROL_ITEM_EO_LEGACY_H_
#define _ELM_SEGMENT_CONTROL_ITEM_EO_LEGACY_H_

#ifndef _ELM_SEGMENT_CONTROL_ITEM_EO_CLASS_TYPE
#define _ELM_SEGMENT_CONTROL_ITEM_EO_CLASS_TYPE

typedef Eo Elm_Segment_Control_Item;

#endif

#ifndef _ELM_SEGMENT_CONTROL_ITEM_EO_TYPES
#define _ELM_SEGMENT_CONTROL_ITEM_EO_TYPES


#endif

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
 * @ingroup Elm_Segment_Control_Item_Group
 */
EAPI int elm_segment_control_item_index_get(const Elm_Segment_Control_Item *obj);

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
 * @ingroup Elm_Segment_Control_Item_Group
 */
EAPI Efl_Canvas_Object *elm_segment_control_item_object_get(const Elm_Segment_Control_Item *obj);

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
 * @ingroup Elm_Segment_Control_Item_Group
 */
EAPI void elm_segment_control_item_selected_set(Elm_Segment_Control_Item *obj, Eina_Bool selected);

#endif
