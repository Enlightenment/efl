/**
 * @defgroup Box Box
 * @ingroup Elementary
 *
 * @image html box_inheritance_tree.png
 * @image latex box_inheritance_tree.eps
 *
 * @image html img/widget/box/preview-00.png
 * @image latex img/widget/box/preview-00.eps width=\textwidth
 *
 * @image html img/box.png
 * @image latex img/box.eps width=\textwidth
 *
 * A box arranges objects in a linear fashion, governed by a layout function
 * that defines the details of this arrangement.
 *
 * By default, the box will use an internal function to set the layout to
 * a single row, either vertical or horizontal. This layout is affected
 * by a number of parameters, such as the homogeneous flag set by
 * elm_box_homogeneous_set(), the values given by elm_box_padding_set() and
 * elm_box_align_set() and the hints set to each object in the box.
 *
 * For this default layout, it's possible to change the orientation with
 * elm_box_horizontal_set(). The box will start in the vertical orientation,
 * placing its elements ordered from top to bottom. When horizontal is set,
 * the order will go from left to right. If the box is set to be
 * homogeneous, every object in it will be assigned the same space, that
 * of the largest object. Padding can be used to set some spacing between
 * the cell given to each object. The alignment of the box, set with
 * elm_box_align_set(), determines how the bounding box of all the elements
 * will be placed within the space given to the box widget itself.
 *
 * The size hints of each object also affect how they are placed and sized
 * within the box. evas_object_size_hint_min_set() will give the minimum
 * size the object can have, and the box will use it as the basis for all
 * latter calculations. Elementary widgets set their own minimum size as
 * needed, so there's rarely any need to use it manually.
 *
 * evas_object_size_hint_weight_set(), when not in homogeneous mode, is
 * used to tell whether the object will be allocated the minimum size it
 * needs or if the space given to it should be expanded. It's important
 * to realize that expanding the size given to the object is not the same
 * thing as resizing the object. It could very well end being a small
 * widget floating in a much larger empty space. If not set, the weight
 * for objects will normally be 0.0 for both axis, meaning the widget will
 * not be expanded. To take as much space possible, set the weight to
 * EVAS_HINT_EXPAND (defined to 1.0) for the desired axis to expand.
 *
 * Besides how much space each object is allocated, it's possible to control
 * how the widget will be placed within that space using
 * evas_object_size_hint_align_set(). By default, this value will be 0.5
 * for both axis, meaning the object will be centered, but any value from
 * 0.0 (left or top, for the @c x and @c y axis, respectively) to 1.0
 * (right or bottom) can be used. The special value EVAS_HINT_FILL, which
 * is -1.0, means the object will be resized to fill the entire space it
 * was allocated.
 *
 * In addition, customized functions to define the layout can be set, which
 * allow the application developer to organize the objects within the box
 * in any number of ways.
 *
 * The special elm_box_layout_transition() function can be used
 * to switch from one layout to another, animating the motion of the
 * children of the box.
 *
 * @note Objects should not be added to box objects using _add() calls.
 *
 * Some examples on how to use boxes follow:
 * @li @ref box_example_01
 * @li @ref box_example_02
 *
 * @{
 */

#include "elm_box_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_box_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_box_legacy.h"
#endif
/**
 * @}
 */
