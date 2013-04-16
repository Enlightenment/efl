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
 * @li "theme,changed": The theme was changed.
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

#define ELM_OBJ_LAYOUT_CLASS elm_obj_layout_class_get()

 const Eo_Class *elm_obj_layout_class_get(void) EINA_CONST;

 extern EAPI Eo_Op ELM_OBJ_LAYOUT_BASE_ID;

 enum
{
   ELM_OBJ_LAYOUT_SUB_ID_FILE_SET,
   ELM_OBJ_LAYOUT_SUB_ID_THEME_SET,
   ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_EMIT,
   ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_ADD,
   ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_DEL,
   ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET,
   ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_APPEND,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_PREPEND,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_BEFORE,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_AT,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE_ALL,
   ELM_OBJ_LAYOUT_SUB_ID_TABLE_PACK,
   ELM_OBJ_LAYOUT_SUB_ID_TABLE_UNPACK,
   ELM_OBJ_LAYOUT_SUB_ID_TABLE_CLEAR,
   ELM_OBJ_LAYOUT_SUB_ID_EDJE_GET,
   ELM_OBJ_LAYOUT_SUB_ID_DATA_GET,
   ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_SET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_GET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_UNSET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_SET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_GET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_SET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_GET,
   ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_SET,
   ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_GET,
   ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET,
   ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET,
   ELM_OBJ_LAYOUT_SUB_ID_SUB_OBJECT_ADD_ENABLE,
   ELM_OBJ_LAYOUT_SUB_ID_THEME_ENABLE,
   ELM_OBJ_LAYOUT_SUB_ID_FREEZE,
   ELM_OBJ_LAYOUT_SUB_ID_THAW,
   ELM_OBJ_LAYOUT_SUB_ID_LAST
};

#define ELM_OBJ_LAYOUT_ID(sub_id) (ELM_OBJ_LAYOUT_BASE_ID + sub_id)


/**
 * @def elm_obj_layout_file_set
 * @since 1.8
 *
 * Set the file that will be used as layout
 *
 * @param[in] file
 * @param[in] group
 * @param[out] ret
 *
 * @see elm_layout_file_set
 */
#define elm_obj_layout_file_set(file, group, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char *, group), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_theme_set
 * @since 1.8
 *
 * Set the edje group from the elementary theme that will be used as layout
 *
 * @param[in] klass
 * @param[in] group
 * @param[in] style
 * @param[out] ret
 *
 * @see elm_layout_theme_set
 */
#define elm_obj_layout_theme_set(klass, group, style, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_THEME_SET), EO_TYPECHECK(const char *, klass), EO_TYPECHECK(const char *, group), EO_TYPECHECK(const char *, style), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_signal_emit
 * @since 1.8
 *
 * Send a (Edje) signal to a given layout widget's underlying Edje
 *
 * @param[in] emission
 * @param[in] source
 *
 * @see elm_layout_signal_emit
 */
#define elm_obj_layout_signal_emit(emission, source) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_EMIT), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source)

/**
 * @def elm_obj_layout_signal_callback_add
 * @since 1.8
 *
 * Add a callback for a (Edje) signal emitted by a layout widget's
 *
 * @param[in] emission
 * @param[in] source
 * @param[in] func
 * @param[in] data
 *
 * @see elm_layout_signal_callback_add
 */
#define elm_obj_layout_signal_callback_add(emission, source, func, data) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_ADD), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Edje_Signal_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_layout_signal_callback_del
 * @since 1.8
 *
 * Remove a signal-triggered callback from a given layout widget.
 *
 * @param[in] emission
 * @param[in] source
 * @param[in] func
 * @param[out] ret
 *
 * @see elm_layout_signal_callback_del
 */
#define elm_obj_layout_signal_callback_del(emission, source, func, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_DEL), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Edje_Signal_Cb, func), EO_TYPECHECK(void **, ret)

/**
 * @def elm_obj_layout_text_set
 * @since 1.8
 *
 * Set the text of the given part
 *
 * @param[in] part
 * @param[in] text
 * @param[out] ret
 *
 * @see elm_layout_text_set
 */
#define elm_obj_layout_text_set(part, text, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_text_get
 * @since 1.8
 *
 * Get the text set in the given part
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see elm_layout_text_get
 */
#define elm_obj_layout_text_get(part, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_layout_box_append
 * @since 1.8
 *
 * Append child to layout box part.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_layout_box_append
 */
#define elm_obj_layout_box_append(part, child, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_APPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_box_prepend
 * @since 1.8
 *
 * Prepend child to layout box part.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_layout_box_prepend
 */
#define elm_obj_layout_box_prepend(part, child, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_PREPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_box_insert_before
 * @since 1.8
 *
 * Insert child to layout box part before a reference object.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] reference
 * @param[out] ret
 *
 * @see elm_layout_box_insert_before
 */
#define elm_obj_layout_box_insert_before(part, child, reference, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_BEFORE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_box_insert_at
 * @since 1.8
 *
 * Insert child to layout box part at a given position.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] pos
 * @param[out] ret
 *
 * @see elm_layout_box_insert_at
 */
#define elm_obj_layout_box_insert_at(part, child, pos, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_AT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_box_remove
 * @since 1.8
 *
 * Remove a child of the given part box.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_layout_box_remove
 */
#define elm_obj_layout_box_remove(part, child, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_layout_box_remove_all
 * @since 1.8
 *
 * Remove all children of the given part box.
 *
 * @param[in] part
 * @param[in] clear
 * @param[out] ret
 *
 * @see elm_layout_box_remove_all
 */
#define elm_obj_layout_box_remove_all(part, clear, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE_ALL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_table_pack
 * @since 1.8
 *
 * Insert child to layout table part.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] col
 * @param[in] row
 * @param[in] colspan
 * @param[in] rowspan
 * @param[out] ret
 *
 * @see elm_layout_table_pack
 */
#define elm_obj_layout_table_pack(part, child, col, row, colspan, rowspan, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TABLE_PACK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned short, col), EO_TYPECHECK(unsigned short, row), EO_TYPECHECK(unsigned short, colspan), EO_TYPECHECK(unsigned short, rowspan), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_table_unpack
 * @since 1.8
 *
 * Unpack (remove) a child of the given part table.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_layout_table_unpack
 */
#define elm_obj_layout_table_unpack(part, child, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TABLE_UNPACK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_layout_table_clear
 * @since 1.8
 *
 * Remove all the child objects of the given part table.
 *
 * @param[in] part
 * @param[in] clear
 * @param[out] ret
 *
 * @see elm_layout_table_clear
 */
#define elm_obj_layout_table_clear(part, clear, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TABLE_CLEAR), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_edje_get
 * @since 1.8
 *
 * Get the edje layout
 *
 * @param[out] ret
 *
 * @see elm_layout_edje_get
 */
#define elm_obj_layout_edje_get(ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_EDJE_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_layout_data_get
 * @since 1.8
 *
 * Get the edje data from the given layout
 *
 * @param[in] key
 * @param[out] ret
 *
 * @see elm_layout_data_get
 */
#define elm_obj_layout_data_get(key, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_DATA_GET), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_layout_sizing_eval
 * @since 1.8
 *
 * Eval sizing
 *
 * @see elm_layout_sizing_eval
 */
#define elm_obj_layout_sizing_eval() ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL)

/**
 * @def elm_obj_layout_part_cursor_set
 * @since 1.8
 *
 * Sets a specific cursor for an edje part.
 *
 * @param[in] part_name
 * @param[in] cursor
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_set
 */
#define elm_obj_layout_part_cursor_set(part_name, cursor, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_SET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(const char *, cursor), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_part_cursor_get
 * @since 1.8
 *
 * Get the cursor to be shown when mouse is over an edje part
 *
 * @param[in] part_name
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_get
 */
#define elm_obj_layout_part_cursor_get(part_name, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_GET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_layout_part_cursor_unset
 * @since 1.8
 *
 * Unsets a cursor previously set with elm_layout_part_cursor_set().
 *
 * @param[in] part_name
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_unset
 */
#define elm_obj_layout_part_cursor_unset(part_name, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_UNSET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_part_cursor_style_set
 * @since 1.8
 *
 * Sets a specific cursor style for an edje part.
 *
 * @param[in] part_name
 * @param[in] style
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_style_set
 */
#define elm_obj_layout_part_cursor_style_set(part_name, style, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_SET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(const char *, style), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_part_cursor_style_get
 * @since 1.8
 *
 * Gets a specific cursor style for an edje part.
 *
 * @param[in] part_name
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_style_get
 */
#define elm_obj_layout_part_cursor_style_get(part_name, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_GET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_layout_part_cursor_engine_only_set
 * @since 1.8
 *
 * Sets if the cursor set should be searched on the theme or should use
 *
 * @param[in] part_name
 * @param[in] engine_only
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_engine_only_set
 */
#define elm_obj_layout_part_cursor_engine_only_set(part_name, engine_only, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_SET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(Eina_Bool, engine_only), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_part_cursor_engine_only_get
 * @since 1.8
 *
 * Gets a specific cursor engine_only for an edje part.
 *
 * @param[in] part_name
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_engine_only_get
 */
#define elm_obj_layout_part_cursor_engine_only_get(part_name, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_GET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_edje_object_can_access_set
 * @since 1.8
 *
 * Set accessibility to all texblock(text) parts in the layout object
 *
 * @param[in] can_access
 * @param[out] ret
 *
 * @see elm_layout_edje_object_can_access_set
 */
#define elm_obj_layout_edje_object_can_access_set(can_access, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_SET), EO_TYPECHECK(Eina_Bool, can_access), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_edje_object_can_access_get
 * @since 1.8
 *
 * Get accessibility state of texblock(text) parts in the layout object
 *
 * @param[out] ret
 *
 * @see elm_layout_edje_object_can_access_get
 */
#define elm_obj_layout_edje_object_can_access_get(ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_smart_text_aliases_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] aliases
 */
#define elm_obj_layout_smart_text_aliases_get(aliases) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), EO_TYPECHECK(const Elm_Layout_Part_Alias_Description **, aliases)

/**
 * @def elm_obj_layout_smart_content_aliases_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] aliases
 */
#define elm_obj_layout_smart_content_aliases_get(aliases) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), EO_TYPECHECK(const Elm_Layout_Part_Alias_Description **, aliases)

/**
 * @def elm_obj_layout_sub_object_add_enable
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] enable
 *
 * @see elm_layout_sub_object_add_enable
 */
#define elm_obj_layout_sub_object_add_enable(enable) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SUB_OBJECT_ADD_ENABLE), EO_TYPECHECK(Eina_Bool *, enable)

/**
 * @def elm_obj_layout_theme_enable
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] enable
 */
#define elm_obj_layout_theme_enable(enable) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_THEME_ENABLE), EO_TYPECHECK(Eina_Bool *, enable)

/**
 * @def elm_obj_layout_freeze
 * @since 1.8
 *
 * @brief Freezes the Elementary layout object.
 *
 * @param[out] ret
 *
 * @see elm_layout_freeze
 */
#define elm_obj_layout_freeze(ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_FREEZE), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_layout_thaw
 * @since 1.8
 *
 * @brief Thaws the Elementary object.
 *
 * @param[out] ret
 *
 * @see elm_layout_thaw
 */
#define elm_obj_layout_thaw(ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_THAW), EO_TYPECHECK(int *, ret)

/**
 * Add a new layout to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_layout_file_set()
 * @see elm_layout_theme_set()
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_add(Evas_Object *parent);

/**
 * Set the file that will be used as layout
 *
 * @param obj The layout object
 * @param file The path to file (edj) that will be used as layout
 * @param group The group that the layout belongs in edje file
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 * @brief Freezes the Elementary layout object.
 *
 * @param obj A handle to an Elementary layout object.
 * @return The frozen state or 0 on Error
 *
 * This function puts all changes on hold. Successive freezes will
 * nest, requiring an equal number of thaws.
 *
 * @see elm_layout_thaw()
 */
EAPI int elm_layout_freeze(Evas_Object *obj);

/**
 * @brief Thaws the Elementary object.
 *
 * @param obj A handle to an Elementary layout object.
 * @return The frozen state or 0 if the object is not frozen or on error.
 *
 * This function thaws the given Edje object and the Elementary sizing calc.
 *
 * @note: If sucessives freezes were done, an equal number of
 *        thaws will be required.
 *
 * @see elm_layout_freeze()
 */
EAPI int elm_layout_thaw(Evas_Object *obj);

/**
 * Set the edje group from the elementary theme that will be used as layout
 *
 * @param obj The layout object
 * @param clas the class of the group
 * @param group the group
 * @param style the style to used
 *
 * @return (1 = success, 0 = error)
 *
 * Note that @a style will be the new style of @a obj too, as in an
 * elm_object_style_set() call.
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_theme_set(Evas_Object *obj, const char *clas, const char *group, const char *style);

/**
 * Send a (Edje) signal to a given layout widget's underlying Edje
 * object.
 *
 * @param obj The layout object handle
 * @param emission The signal's name string
 * @param source The signal's source string
 *
 * This function sends a signal to the underlying Edje object of @a
 * obj. An Edje program on that Edje object's definition can respond
 * to a signal by specifying matching 'signal' and 'source' fields.
 *
 * @ingroup Layout
 */
EAPI void elm_layout_signal_emit(Evas_Object *obj, const char *emission, const char *source);

/**
 * Add a callback for a (Edje) signal emitted by a layout widget's
 * underlying Edje object.
 *
 * @param obj The layout object handle
 * @param emission The signal's name string
 * @param source The signal's source string
 * @param func The callback function to be executed when the signal is
 * emitted.
 * @param data A pointer to data to pass in to the callback function.
 *
 * This function connects a callback function to a signal emitted by
 * the underlying Edje object of @a obj. Globs are accepted in either
 * the emission or source strings (see @c
 * edje_object_signal_callback_add()).
 *
 * @ingroup Layout
 */
EAPI void elm_layout_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

/**
 * Remove a signal-triggered callback from a given layout widget.
 *
 * @param obj The layout object handle
 * @param emission The signal's name string
 * @param source The signal's source string
 * @param func The callback function being executed when the signal
 * was emitted.
 * @return The data pointer of the signal callback (passed on
 * elm_layout_signal_callback_add()) or @c NULL, on errors.
 *
 * This function removes the @b last callback attached to a signal
 * emitted by the undelying Edje object of @a obj, with parameters @a
 * emission, @a source and @c func matching exactly those passed to a
 * previous call to elm_layout_signal_callback_add(). The data pointer
 * that was passed to this call will be returned.
 *
 * @ingroup Layout
 */
EAPI void *elm_layout_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);

/**
 * Append child to layout box part.
 *
 * @param obj the layout object
 * @param part the box part to which the object will be appended.
 * @param child the child object to append to box.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is appended, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use elm_layout_box_remove() to
 * make this layout forget about the object.
 *
 * @see elm_layout_box_prepend()
 * @see elm_layout_box_insert_before()
 * @see elm_layout_box_insert_at()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_append(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * Prepend child to layout box part.
 *
 * @param obj the layout object
 * @param part the box part to prepend.
 * @param child the child object to prepend to box.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is prepended, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use elm_layout_box_remove() to
 * make this layout forget about the object.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_insert_before()
 * @see elm_layout_box_insert_at()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_prepend(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * Insert child to layout box part before a reference object.
 *
 * @param obj the layout object
 * @param part the box part to insert.
 * @param child the child object to insert into box.
 * @param reference another reference object to insert before in box.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is inserted, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use elm_layout_box_remove() to
 * make this layout forget about the object.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_prepend()
 * @see elm_layout_box_insert_before()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_insert_before(Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);

/**
 * Insert child to layout box part at a given position.
 *
 * @param obj the layout object
 * @param part the box part to insert.
 * @param child the child object to insert into box.
 * @param pos the numeric position >=0 to insert the child.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is inserted, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use elm_layout_box_remove() to
 * make this layout forget about the object.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_prepend()
 * @see elm_layout_box_insert_before()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_insert_at(Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);

/**
 * Remove a child of the given part box.
 *
 * @param obj The layout object
 * @param part The box part name to remove child.
 * @param child The object to remove from box.
 * @return The object that was being used, or NULL if not found.
 *
 * The object will be removed from the box part and its lifetime will
 * not be handled by the layout anymore. This is equivalent to
 * elm_layout_content_unset() for box.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_remove_all()
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_box_remove(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * Remove all children of the given part box.
 *
 * @param obj The layout object
 * @param part The box part name to remove child.
 * @param clear If EINA_TRUE, then all objects will be deleted as
 *        well, otherwise they will just be removed and will be
 *        dangling on the canvas.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * The objects will be removed from the box part and their lifetime will
 * not be handled by the layout anymore. This is equivalent to
 * elm_layout_box_remove() for all box children.
 *
 * @see elm_layout_box_append()
 * @see elm_layout_box_remove()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_box_remove_all(Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * Insert child to layout table part.
 *
 * @param obj the layout object
 * @param part the box part to pack child.
 * @param child_obj the child object to pack into table.
 * @param col the column to which the child should be added. (>= 0)
 * @param row the row to which the child should be added. (>= 0)
 * @param colspan how many columns should be used to store this object. (>=
 *        1)
 * @param rowspan how many rows should be used to store this object. (>= 1)
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the object is inserted, it will become child of the table. Its
 * lifetime will be bound to the layout, and whenever the layout dies the
 * child will be deleted automatically. One should use
 * elm_layout_table_remove() to make this layout forget about the object.
 *
 * If @p colspan or @p rowspan are bigger than 1, that object will occupy
 * more space than a single cell. For instance, the following code:
 * @code
 * elm_layout_table_pack(layout, "table_part", child, 0, 1, 3, 1);
 * @endcode
 *
 * Would result in an object being added like the following picture:
 *
 * @image html layout_colspan.png
 * @image latex layout_colspan.eps width=\textwidth
 *
 * @see elm_layout_table_unpack()
 * @see elm_layout_table_clear()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_table_pack(Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);

/**
 * Unpack (remove) a child of the given part table.
 *
 * @param obj The layout object
 * @param part The table part name to remove child.
 * @param child_obj The object to remove from table.
 * @return The object that was being used, or NULL if not found.
 *
 * The object will be unpacked from the table part and its lifetime
 * will not be handled by the layout anymore. This is equivalent to
 * elm_layout_content_unset() for table.
 *
 * @see elm_layout_table_pack()
 * @see elm_layout_table_clear()
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_table_unpack(Evas_Object *obj, const char *part, Evas_Object *child_obj);

/**
 * Remove all the child objects of the given part table.
 *
 * @param obj The layout object
 * @param part The table part name to remove child.
 * @param clear If EINA_TRUE, then all objects will be deleted as
 *        well, otherwise they will just be removed and will be
 *        dangling on the canvas.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * The objects will be removed from the table part and their lifetime will
 * not be handled by the layout anymore. This is equivalent to
 * elm_layout_table_unpack() for all table children.
 *
 * @see elm_layout_table_pack()
 * @see elm_layout_table_unpack()
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_table_clear(Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * Get the edje layout
 *
 * @param obj The layout object
 *
 * @return A Evas_Object with the edje layout settings loaded
 * with function elm_layout_file_set
 *
 * This returns the edje object. It is not expected to be used to then
 * swallow objects via edje_object_part_swallow() for example. Use
 * elm_layout_content_set() instead so child object handling and sizing is
 * done properly.
 *
 * @note This function should only be used if you really need to call some
 * low level Edje function on this edje object. All the common stuff (setting
 * text, emitting signals, hooking callbacks to signals, etc.) can be done
 * with proper elementary functions.
 *
 * @see elm_layout_signal_callback_add()
 * @see elm_layout_signal_emit()
 * @see elm_layout_text_set()
 * @see elm_layout_content_set()
 * @see elm_layout_box_append()
 * @see elm_layout_table_pack()
 * @see elm_layout_data_get()
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_edje_get(const Evas_Object *obj);

/**
 * Get the edje data from the given layout
 *
 * @param obj The layout object
 * @param key The data key
 *
 * @return The edje data string
 *
 * This function fetches data specified inside the edje theme of this layout.
 * This function return NULL if data is not found.
 *
 * In EDC this comes from a data block within the group block that @p
 * obj was loaded from. E.g.
 *
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     data {
 *       item: "key1" "value1";
 *       item: "key2" "value2";
 *     }
 *   }
 * }
 * @endcode
 *
 * @ingroup Layout
 */
EAPI const char                  *elm_layout_data_get(const Evas_Object *obj, const char *key);

/**
 * Eval sizing
 *
 * @param obj The layout object
 *
 * Manually forces a sizing re-evaluation. This is useful when the minimum
 * size required by the edje theme of this layout has changed. The change on
 * the minimum size required by the edje theme is not immediately reported to
 * the elementary layout, so one needs to call this function in order to tell
 * the widget (layout) that it needs to reevaluate its own size.
 *
 * The minimum size of the theme is calculated based on minimum size of
 * parts, the size of elements inside containers like box and table, etc. All
 * of this can change due to state changes, and that's when this function
 * should be called.
 *
 * Also note that a standard signal of "size,eval" "elm" emitted from the
 * edje object will cause this to happen too.
 *
 * @ingroup Layout
 */
EAPI void                         elm_layout_sizing_eval(Evas_Object *obj);

/**
 * Sets a specific cursor for an edje part.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 * @param cursor cursor name to use, see Elementary_Cursor.h
 *
 * @return EINA_TRUE on success or EINA_FALSE on failure, that may be
 *         part not exists or it has "mouse_events: 0".
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_set(Evas_Object *obj, const char *part_name, const char *cursor);

/**
 * Get the cursor to be shown when mouse is over an edje part
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 * @return the cursor name.
 *
 * @ingroup Layout
 */
EAPI const char                  *elm_layout_part_cursor_get(const Evas_Object *obj, const char *part_name);

/**
 * Unsets a cursor previously set with elm_layout_part_cursor_set().
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group, that had a cursor set
 *        with elm_layout_part_cursor_set().
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_unset(Evas_Object *obj, const char *part_name);

/**
 * Sets a specific cursor style for an edje part.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @return EINA_TRUE on success or EINA_FALSE on failure, that may be
 *         part not exists or it did not had a cursor set.
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_style_set(Evas_Object *obj, const char *part_name, const char *style);

/**
 * Gets a specific cursor style for an edje part.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 *
 * @return the theme style in use, defaults to "default". If the
 *         object does not have a cursor set, then NULL is returned.
 *
 * @ingroup Layout
 */
EAPI const char                  *elm_layout_part_cursor_style_get(const Evas_Object *obj, const char *part_name);

/**
 * Sets if the cursor set should be searched on the theme or should use
 * the provided by the engine, only.
 *
 * @note before you set if should look on theme you should define a
 * cursor with elm_layout_part_cursor_set(). By default it will only
 * look for cursors provided by the engine.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 * @param engine_only if cursors should be just provided by the engine (EINA_TRUE)
 *        or should also search on widget's theme as well (EINA_FALSE)
 *
 * @return EINA_TRUE on success or EINA_FALSE on failure, that may be
 *         part not exists or it did not had a cursor set.
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_engine_only_set(Evas_Object *obj, const char *part_name, Eina_Bool engine_only);

/*
* Set accessibility to all texblock(text) parts in the layout object
*
* @param obj The layout object.
* @param can_access makes all textblock(text) parts in the layout @p obj possible
* to have accessibility. @c EINA_TRUE means textblock(text) parts can be accessible
*
* @return @c EINA_TRUE on success or @c EINA_FALSE on failure. If @p obj is not
* a proper layout object, @c EINA_FALSE is returned.
*
* @since 1.7
*
* @ingroup Layout
*/
EAPI Eina_Bool                    elm_layout_edje_object_can_access_set(Evas_Object *obj, Eina_Bool can_access);

/*
* Get accessibility state of texblock(text) parts in the layout object
*
* @param obj The layout object.
*
* @return @c EINA_TRUE, if all textblock(text) parts in the layout can be accessible
* @c EINA_FALSET if those cannot be accessible. If @p obj is not a proper layout
* object, @c EINA_FALSE is returned.
*
* @see elm_layout_edje_object_access_set()
*
* @since 1.7
*
* @ingroup Layout
*/
EAPI Eina_Bool                    elm_layout_edje_object_can_access_get(Evas_Object *obj);

/**
 * Gets a specific cursor engine_only for an edje part.
 *
 * @param obj The layout object.
 * @param part_name a part from loaded edje group.
 *
 * @return whenever the cursor is just provided by engine or also from theme.
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_part_cursor_engine_only_get(const Evas_Object *obj, const char *part_name);

/**
 * Set the layout content.
 *
 * @param obj The layout object
 * @param swallow The swallow part name in the edje file
 * @param content The child that will be added in this layout object
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_layout_content_unset() function.
 *
 * @note In an Edje theme, the part used as a content container is called
 * @c SWALLOW. This is why the parameter name is called @p swallow, but it is
 * expected to be a part name just like the second parameter of
 * elm_layout_box_append().
 *
 * @see elm_layout_box_append()
 * @see elm_layout_content_get()
 * @see elm_layout_content_unset()
 *
 * @see @ref secBox
 *
 * @ingroup Layout
 */
EAPI Eina_Bool                    elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);

/**
 * Get the child object in the given content part.
 *
 * @param obj The layout object
 * @param swallow The SWALLOW part to get its content
 *
 * @return The swallowed object or NULL if none or an error occurred
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_content_get(const Evas_Object *obj, const char *swallow);

/**
 * Unset the layout content.
 *
 * @param obj The layout object
 * @param swallow The swallow part name in the edje file
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this part.
 *
 * @ingroup Layout
 */
EAPI Evas_Object                 *elm_layout_content_unset(Evas_Object *obj, const char *swallow);

/**
 * Set the text of the given part
 *
 * @param obj The layout object
 * @param part The TEXT part where to set the text
 * @param text The text to set
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 */
EAPI Eina_Bool                    elm_layout_text_set(Evas_Object *obj, const char *part, const char *text);

/**
 * Get the text set in the given part
 *
 * @param obj The layout object
 * @param part The TEXT part to retrieve the text off
 *
 * @return The text set in @p part
 */
EAPI const char                  *elm_layout_text_get(const Evas_Object *obj, const char *part);

/**
 * @def elm_layout_icon_set
 * Convenience macro to set the icon object in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 */
#define elm_layout_icon_set(_ly, _obj)                                 \
  do {                                                                 \
       const char *sig;                                                \
       elm_layout_content_set((_ly), "elm.swallow.icon", (_obj));      \
       if ((_obj)) sig = "elm,state,icon,visible";                     \
       else sig = "elm,state,icon,hidden";                             \
       elm_layout_signal_emit((_ly), sig, "elm");                      \
    } while (0)

/**
 * @def elm_layout_icon_get
 * Convenience macro to get the icon object from a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 */
#define elm_layout_icon_get(_ly) \
  elm_layout_content_get((_ly), "elm.swallow.icon")

/**
 * @def elm_layout_end_set
 * Convenience macro to set the end object in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 */
#define elm_layout_end_set(_ly, _obj)                                 \
  do {                                                                \
       const char *sig;                                               \
       elm_layout_content_set((_ly), "elm.swallow.end", (_obj));      \
       if ((_obj)) sig = "elm,state,end,visible";                     \
       else sig = "elm,state,end,hidden";                             \
       elm_layout_signal_emit((_ly), sig, "elm");                     \
    } while (0)

/**
 * @def elm_layout_end_get
 * Convenience macro to get the end object in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 */
#define elm_layout_end_get(_ly) \
  elm_layout_content_get((_ly), "elm.swallow.end")

/**
 * @}
 */
