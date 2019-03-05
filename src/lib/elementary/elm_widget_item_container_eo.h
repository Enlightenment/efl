#ifndef _ELM_WIDGET_ITEM_CONTAINER_EO_H_
#define _ELM_WIDGET_ITEM_CONTAINER_EO_H_

#ifndef _ELM_WIDGET_ITEM_CONTAINER_EO_CLASS_TYPE
#define _ELM_WIDGET_ITEM_CONTAINER_EO_CLASS_TYPE

typedef Eo Elm_Widget_Item_Container;

#endif

#ifndef _ELM_WIDGET_ITEM_CONTAINER_EO_TYPES
#define _ELM_WIDGET_ITEM_CONTAINER_EO_TYPES


#endif
#define ELM_WIDGET_ITEM_CONTAINER_INTERFACE elm_widget_item_container_interface_get()

EWAPI const Efl_Class *elm_widget_item_container_interface_get(void);

/**
 * @brief Get the focused widget item.
 *
 * @param[in] obj The object.
 *
 * @return Focused item
 *
 * @ingroup Elm_Widget_Item_Container
 */
EOAPI Elm_Widget_Item *elm_widget_item_container_focused_item_get(const Eo *obj);

#endif
