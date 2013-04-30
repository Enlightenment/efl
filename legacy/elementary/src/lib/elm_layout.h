/**
 * @defgroup Layout Layout
 * @ingroup Elementary
 *
 * @image html layout_inheritance_tree.png
 * @image latex layout_inheritance_tree.eps
 *
 * @image html img/widget/layout/preview-00.png
 * @image latex img/widget/layout/preview-00.eps width=\textwidth
 *
 * @image html img/layout-predefined.png
 * @image latex img/layout-predefined.eps width=\textwidth
 *
 * A Layout is a direct realization of @ref elm-layout-class.
 *
 * This is a container widget that takes a standard Edje design file
 * and wraps it very thinly in a widget.
 *
 * An Edje design (theme) file has a very wide range of possibilities
 * to describe the behavior of elements forming a layout. Check out
 * the Edje documentation and the EDC reference to get more
 * information about what can be done with Edje.
 *
 * Just like @ref List, @ref Box, and other container widgets, any
 * object added to the Layout will become its child, meaning that it
 * will be deleted if the Layout is deleted, moved if the Layout is
 * moved, and so on.
 *
 * The layout widget may contain as many parts/children as described
 * in its theme file. Some of these children can have special types,
 * such as content holder ones (swallow spots), boxes or tables. These
 * are parts meant to contain others. For instance, objects can be
 * added to different table parts by specifying the respective table
 * part names. The same is valid for swallows and boxes.
 *
 * The objects added as children of a layout will behave as described
 * in the part description where they were added. There are 3 possible
 * types of parts where a child can be added:
 *
 * @section secContent Content (@c SWALLOW part)
 *
 * Only one object can be added to the @c SWALLOW part at a time (but
 * you still can have many @c SWALLOW parts and one object on each of
 * them). Use the @c elm_layout_content_set()/get/unset functions to
 * set, retrieve and unset objects as content of the @c SWALLOW. After
 * being set to this part, the object's size, position, visibility,
 * clipping and other description properties will be totally
 * controlled by the description of the given part (inside the Edje
 * theme file).
 *
 * One can use @c evas_object_size_hint_* functions on the child to
 * have some kind of control over its behavior, but the resulting
 * behavior will still depend heavily on the @c SWALLOW part's
 * description.
 *
 * The Edje theme also can change the part description, based on
 * signals or scripts running inside the theme. This change can also
 * be animated. All of this will affect the child object set as
 * content accordingly. The object's size will be changed if the part
 * size is changed, it will animate moving accordingly if the part is
 * moving, and so on.
 *
 * The following picture demonstrates a layout widget with a child
 * object added to its @c SWALLOW:
 *
 * @image html layout_swallow.png
 * @image latex layout_swallow.eps width=\textwidth
 *
 * @section secBox Box (@c BOX part)
 *
 * An Edje @c BOX part is very similar to the Elementary @ref Box
 * widget. It allows one to add objects to the box and have them
 * distributed along its area, accordingly to the specified @c layout
 * property (now by @c layout we mean the chosen layouting design of
 * the Box, not the layout widget itself).
 *
 * A similar effect for having a box with its position, size and other
 * things controlled by the layout theme would be to create an
 * Elementary @ref Box widget and add it as content in a @c SWALLOW part.
 *
 * The main difference to that, by using the layout box instead, is
 * that its behavior, like layouting format, padding, align, etc.,
 * will <b>all be controlled by the theme</b>. This means, for
 * example, that a signal could be sent to the layout's theme (with
 * elm_layout_signal_emit()) and the signal be handled by changing the
 * box's padding, or alignment, or both. Using the Elementary @ref Box
 * widget is not necessarily harder or easier, it just depends on the
 * circumstances and requirements.
 *
 * The layout box can be used through the @c elm_layout_box_* set of
 * functions.
 *
 * The following picture demonstrates a Layout widget with many child
 * objects added to its @c BOX part:
 *
 * @image html layout_box.png
 * @image latex layout_box.eps width=\textwidth
 *
 * @section secTable Table (@c TABLE part)
 *
 * Just like the @ref secBox, the layout table is very similar to the
 * Elementary @ref Table widget. It allows one to add objects to the
 * table by specifying the row and column where the object should be
 * added, and any column or row span, if necessary.
 *
 * Again, we could have this design by adding a @ref table widget to a
 * @c SWALLOW part, using elm_layout_content_set(). The same
 * difference happens here when choosing to use the layout table (a
 * @c TABLE part) instead of the @ref table in a @c SWALLOW part. It's
 * just a matter of convenience.
 *
 * The layout table can be used through the @c elm_layout_table_* set of
 * functions.
 *
 * The following picture demonstrates a layout widget with many child
 * objects added to its @c TABLE part:
 *
 * @image html layout_table.png
 * @image latex layout_table.eps width=\textwidth
 *
 * @section secPredef Predefined Layouts
 *
 * Another interesting thing about the layout widget is that it offers
 * some predefined themes that come with the default Elementary
 * theme. These themes can be set by the call elm_layout_theme_set(),
 * and provide some basic functionality depending on the theme used.
 *
 * Most of them already send some signals, some already provide a
 * toolbar or back and next buttons.
 *
 * These are the available predefined theme layouts. All of them have
 * class = @c layout, group = @c application, and style = one of the
 * following options:
 *
 * @li @c toolbar-content - for applications with a toolbar and main
 *                          content area
 * @li @c toolbar-content-back - for applications with a toolbar and
 *                               main content (with a back button)
 *                               and title areas
 * @li @c toolbar-content-back-next - for applications with a toolbar
 *                                    and main content (with back and
 *                                    next buttons) and title areas
 * @li @c content-back - for application with main content (with a
 *                       back button) and title areas
 * @li @c content-back-next - for applications with main content (with
 *                            back and next buttons) and title areas
 * @li @c toolbar-vbox - for applications with a toolbar and main
 *                       content area as a vertical box
 * @li @c toolbar-table - for applications with a toolbar and main
 *                        content area as a table
 *
 * @section layout-signals Emitted signals
 *
 * This widget emits the following signals:
 *
 * @li "theme,changed" - The theme was changed.
 * @li "language,changed" - the program's language changed
 *
 * @section secExamples Examples
 *
 * Some examples of the Layout widget can be found here:
 * @li @ref layout_example_01
 * @li @ref layout_example_02
 * @li @ref layout_example_03
 * @li @ref layout_example_edc
 *
 */

#include <elm_layout_common.h>
#ifdef EFL_EO_API_SUPPORT
#include <elm_layout_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_layout_legacy.h>
#endif

/**
 * @}
 */
