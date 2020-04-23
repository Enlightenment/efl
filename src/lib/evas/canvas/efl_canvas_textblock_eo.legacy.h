#ifndef _EFL_CANVAS_TEXTBLOCK_EO_LEGACY_H_
#define _EFL_CANVAS_TEXTBLOCK_EO_LEGACY_H_

#ifndef _EFL_CANVAS_TEXTBLOCK_EO_CLASS_TYPE
#define _EFL_CANVAS_TEXTBLOCK_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Textblock;
typedef Efl_Canvas_Textblock Efl_Canvas_Text;

#endif

#ifndef _EFL_CANVAS_TEXTBLOCK_EO_TYPES
#define _EFL_CANVAS_TEXTBLOCK_EO_TYPES


#endif


/**
 * @brief Returns the currently visible range.
 *
 * The given @c start and @c end cursor act like out-variables here, as they
 * are set to the positions of the start and the end of the visible range in
 * the text, respectively.
 *
 * @param[in] obj The object.
 * @param[in] start Range start position
 * @param[in] end Range end position
 *
 * @return @c true on success, @c false otherwise
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI Eina_Bool evas_object_textblock_visible_range_get(Efl_Canvas_Textblock *obj, Efl_Text_Cursor_Handle *start, Efl_Text_Cursor_Handle *end);

/**
 * @brief Gets the left, right, top and bottom insets of the text.
 *
 * The inset is any applied padding on the text.
 *
 * @param[in] obj The object.
 * @param[out] left The left padding
 * @param[out] right The right padding
 * @param[out] top The top padding
 * @param[out] bottom The bottom padding
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI void evas_object_textblock_style_insets_get(const Efl_Canvas_Textblock *obj, int *left, int *right, int *top, int *bottom);

/**
 * @brief BiDi delimiters are used for in-paragraph separation of bidi
 * segments. This is useful, for example, in the recipient fields of e-mail
 * clients where bidi oddities can occur when mixing RTL and LTR.
 *
 * @param[in] obj The object.
 * @param[in] delim A null terminated string of delimiters, e.g ",|" or @c null
 * if empty
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI void evas_object_textblock_bidi_delimiters_set(Efl_Canvas_Textblock *obj, const char *delim);

/**
 * @brief BiDi delimiters are used for in-paragraph separation of bidi
 * segments. This is useful, for example, in the recipient fields of e-mail
 * clients where bidi oddities can occur when mixing RTL and LTR.
 *
 * @param[in] obj The object.
 *
 * @return A null terminated string of delimiters, e.g ",|" or @c null if empty
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI const char *evas_object_textblock_bidi_delimiters_get(const Efl_Canvas_Textblock *obj);

/**
 * @brief When @c true, newline character will behave as a paragraph separator.
 *
 * @param[in] obj The object.
 * @param[in] mode @c true for legacy mode, @c false otherwise
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI void evas_object_textblock_legacy_newline_set(Efl_Canvas_Textblock *obj, Eina_Bool mode);

/**
 * @brief When @c true, newline character will behave as a paragraph separator.
 *
 * @param[in] obj The object.
 *
 * @return @c true for legacy mode, @c false otherwise
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI Eina_Bool evas_object_textblock_legacy_newline_get(const Efl_Canvas_Textblock *obj);



/**
 * @brief The formatted width and height.
 *
 * This calculates the actual size after restricting the textblock to the
 * current size of the object.
 *
 * The main difference between this and
 * @ref evas_object_textblock_size_native_get is that the "native" function
 * does not wrapping into account it just calculates the real width of the
 * object if it was placed on an infinite canvas, while this function gives the
 * size after wrapping according to the size restrictions of the object.
 *
 * For example for a textblock containing the text: "You shall not pass!" with
 * no margins or padding and assuming a monospace font and a size of 7x10 char
 * widths (for simplicity) has a native size of 19x1 and a formatted size of
 * 5x4.
 *
 * @param[in] obj The object.
 * @param[out] width The width of the object.
 * @param[out] height The height of the object.
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI void evas_object_textblock_size_formatted_get(const Efl_Canvas_Textblock *obj, int *width, int *height);

/**
 * @brief The native width and height.
 *
 * This calculates the actual size without taking account the current size of
 * the object.
 *
 * The main difference between this and
 * @ref evas_object_textblock_size_formatted_get is that the "native" function
 * does not take wrapping into account it just calculates the real width of the
 * object if it was placed on an infinite canvas, while the "formatted"
 * function gives the size after  wrapping text according to the size
 * restrictions of the object.
 *
 * For example for a textblock containing the text: "You shall not pass!" with
 * no margins or padding and assuming a monospace font and a size of 7x10 char
 * widths (for simplicity) has a native size of 19x1 and a formatted size of
 * 5x4.
 *
 * @param[in] obj The object.
 * @param[out] width The width returned.
 * @param[out] height The height returned.
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI void evas_object_textblock_size_native_get(const Efl_Canvas_Textblock *obj, int *width, int *height);





/**
 * @brief Add obstacle evas object @c eo_obs to be observed during layout of
 * text.
 *
 * The textblock does the layout of the text according to the position of the
 * obstacle.
 *
 * @param[in] obj The object.
 * @param[in] eo_obs Obstacle object
 *
 * @return @c true on success, @c false otherwise.
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI Eina_Bool evas_object_textblock_obstacle_add(Efl_Canvas_Textblock *obj, Efl_Canvas_Object *eo_obs);

/**
 * @brief Removes @c eo_obs from observation during text layout.
 *
 * @param[in] obj The object.
 * @param[in] eo_obs Obstacle object
 *
 * @return @c true on success, @c false otherwise.
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI Eina_Bool evas_object_textblock_obstacle_del(Efl_Canvas_Textblock *obj, Efl_Canvas_Object *eo_obs);

/**
 * @brief Triggers for relayout due to obstacles' state change.
 *
 * The obstacles alone don't affect the layout, until this is called. Use this
 * after doing changes (moving, positioning etc.) in the obstacles that you
 *  would like to be considered in the layout.
 *
 * For example: if you have just repositioned the obstacles to differrent
 * coordinates relative to the textblock, you need to call this so it will
 * consider this new state and will relayout the text.
 *
 * @param[in] obj The object.
 *
 * @since 1.18
 *
 * @ingroup Evas_Object_Textblock_Group
 */
EAPI void evas_object_textblock_obstacles_update(Efl_Canvas_Textblock *obj);


#endif
