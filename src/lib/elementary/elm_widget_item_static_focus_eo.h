#ifndef _ELM_WIDGET_ITEM_STATIC_FOCUS_EO_H_
#define _ELM_WIDGET_ITEM_STATIC_FOCUS_EO_H_

#ifndef _ELM_WIDGET_ITEM_STATIC_FOCUS_EO_CLASS_TYPE
#define _ELM_WIDGET_ITEM_STATIC_FOCUS_EO_CLASS_TYPE

typedef Eo Elm_Widget_Item_Static_Focus;

#endif

#ifndef _ELM_WIDGET_ITEM_STATIC_FOCUS_EO_TYPES
#define _ELM_WIDGET_ITEM_STATIC_FOCUS_EO_TYPES


#endif
/** A class that ensures that at least ONE focusable target is behind this
 * widget item. If there is none registered to it after a prepare call it will
 * register the view of the item as item.
 *
 * @ingroup Elm_Widget_Item_Static_Focus
 */
#define ELM_WIDGET_ITEM_STATIC_FOCUS_CLASS elm_widget_item_static_focus_class_get()

EWAPI const Efl_Class *elm_widget_item_static_focus_class_get(void);

#endif
