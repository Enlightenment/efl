#ifndef EFL_UI_WIDGET_COMMON_H
#define EFL_UI_WIDGET_COMMON_H

/**
 * @brief Get an iterator over all subelements located at obj.
 *
 * This iterator contains also the canvas objects which are part of the widgets,
 * be aware that the construction of this tree is internal and might change heavily
 * between versions.
 *
 * @param obj The widget which is the root of the subtree.
 *
 * @return A iterator that contains subelement widgets and canvas objects of the root widget. Every contained object is a Efl.Gfx.Entity.
 */
EAPI Eina_Iterator* efl_ui_widget_tree_iterator(Efl_Ui_Widget *obj);

/**
 * @brief Get an iterator over all subelements located at obj.
 *
 * @param obj The widget which is the root of the subtree.
 *
 * @return A iterator that contains subelement widgets of the root widget. Every contained object is a Efl.Ui.Widget.
 */
EAPI Eina_Iterator* efl_ui_widget_tree_widget_iterator(Efl_Ui_Widget *obj);

/**
 * @brief Get an iterator that contains all parents of the passed object.
 *
 * @param obj The object to fetch the parents from.
 *
 * @return A iterator that contains all parents of the object. Every contained object is a Efl.Ui.Widget.
 */
EAPI Eina_Iterator* efl_ui_widget_parent_iterator(Efl_Ui_Widget *obj);

#endif
