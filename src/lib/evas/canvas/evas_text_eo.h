#ifndef _EVAS_TEXT_EO_H_
#define _EVAS_TEXT_EO_H_

#ifndef _EVAS_TEXT_EO_CLASS_TYPE
#define _EVAS_TEXT_EO_CLASS_TYPE

typedef Eo Evas_Text;

#endif

#ifndef _EVAS_TEXT_EO_TYPES
#define _EVAS_TEXT_EO_TYPES


#endif
/** Evas text class
 *
 * @ingroup Evas_Text
 */
#define EVAS_TEXT_CLASS evas_text_class_get()

EWAPI const Efl_Class *evas_text_class_get(void);

/**
 * @brief Controls the shadow color for the given text object.
 *
 * Shadow effects are fading colors decorating the text underneath it.  They
 * will be shown if the object is set to one of the following styles:
 *
 * - @c EVAS_TEXT_STYLE_TYPE_SHADOW - @c EVAS_TEXT_STYLE_TYPE_OUTLINE_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_FAR_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_OUTLINE_SOFT_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_SOFT_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_FAR_SOFT_SHADOW
 *
 * One can also change the direction where the shadow grows to, with
 * @ref evas_obj_text_style_get
 *
 * See @ref evas_obj_text_shadow_color_get
 *
 * @param[in] obj The object.
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_shadow_color_set(Eo *obj, int r, int g, int b, int a);

/**
 * @brief Controls the shadow color for the given text object.
 *
 * Shadow effects are fading colors decorating the text underneath it.  They
 * will be shown if the object is set to one of the following styles:
 *
 * - @c EVAS_TEXT_STYLE_TYPE_SHADOW - @c EVAS_TEXT_STYLE_TYPE_OUTLINE_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_FAR_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_OUTLINE_SOFT_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_SOFT_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_FAR_SOFT_SHADOW
 *
 * One can also change the direction where the shadow grows to, with
 * @ref evas_obj_text_style_get
 *
 * See @ref evas_obj_text_shadow_color_get
 *
 * @note Use @c NULL pointers on the color components you're not interested in:
 * they'll be ignored by the function.
 *
 * @param[in] obj The object.
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_shadow_color_get(const Eo *obj, int *r, int *g, int *b, int *a);

/**
 * @brief Controls the ellipsis that should be used for the text object.
 *
 * This is a value between 0.0 and 1.0 indicating the position of the text to
 * be shown. 0.0 means the start will be shown and the end trimmed, 1.0 means
 * the beginning will be trimmed and the end will be shown, and any value in
 * between will cause ellipsis to be added in both end of the text and the
 * requested part to be shown. -1.0 means ellipsis is turned off.
 *
 * @param[in] obj The object.
 * @param[in] ellipsis The ellipsis. Allowed values: -1.0 or 0.0-1.0
 *
 * @since 1.8
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_ellipsis_set(Eo *obj, double ellipsis);

/**
 * @brief Controls the ellipsis that should be used for the text object.
 *
 * This is a value between 0.0 and 1.0 indicating the position of the text to
 * be shown. 0.0 means the start will be shown and the end trimmed, 1.0 means
 * the beginning will be trimmed and the end will be shown, and any value in
 * between will cause ellipsis to be added in both end of the text and the
 * requested part to be shown. -1.0 means ellipsis is turned off.
 *
 * @param[in] obj The object.
 *
 * @return The ellipsis. Allowed values: -1.0 or 0.0-1.0
 *
 * @since 1.8
 *
 * @ingroup Evas_Text
 */
EOAPI double evas_obj_text_ellipsis_get(const Eo *obj);

/**
 * @brief Sets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing RTL and LTR.
 *
 * @param[in] obj The object.
 * @param[in] delim A null terminated string of delimiters, e.g ",|".
 *
 * @since 1.1
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_bidi_delimiters_set(Eo *obj, const char *delim);

/**
 * @brief Sets the BiDi delimiters used in the textblock.
 *
 * BiDi delimiters are use for in-paragraph separation of bidi segments. This
 * is useful for example in recipients fields of e-mail clients where bidi
 * oddities can occur when mixing RTL and LTR.
 *
 * @param[in] obj The object.
 *
 * @return A null terminated string of delimiters, e.g ",|".
 *
 * @since 1.1
 *
 * @ingroup Evas_Text
 */
EOAPI const char *evas_obj_text_bidi_delimiters_get(const Eo *obj);

/**
 * @brief Controls the outline color for the given text object.
 *
 * Outline effects (colored lines around text glyphs) will be shown if the
 * object is set to one of the following styles: -
 * @c EVAS_TEXT_STYLE_TYPE_OUTLINE - @c EVAS_TEXT_STYLE_TYPE_SOFT_OUTLINE -
 * @c EVAS_TEXT_STYLE_TYPE_OUTLINE_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_OUTLINE_SOFT_SHADOW
 *
 * @param[in] obj The object.
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_outline_color_set(Eo *obj, int r, int g, int b, int a);

/**
 * @brief Controls the outline color for the given text object.
 *
 * Outline effects (colored lines around text glyphs) will be shown if the
 * object is set to one of the following styles: -
 * @c EVAS_TEXT_STYLE_TYPE_OUTLINE - @c EVAS_TEXT_STYLE_TYPE_SOFT_OUTLINE -
 * @c EVAS_TEXT_STYLE_TYPE_OUTLINE_SHADOW -
 * @c EVAS_TEXT_STYLE_TYPE_OUTLINE_SOFT_SHADOW
 *
 * @param[in] obj The object.
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_outline_color_get(const Eo *obj, int *r, int *g, int *b, int *a);

/**
 * @brief Sets the 'glow 2' color for the given text object.
 *
 * 'Glow 2' effects are glowing colors decorating the text's (immediate)
 * surroundings.  They will be shown if the object is set to the
 * @c EVAS_TEXT_STYLE_TYPE_GLOW style. See also
 * @ref evas_obj_text_glow_color_get.
 *
 * @param[in] obj The object.
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_glow2_color_set(Eo *obj, int r, int g, int b, int a);

/**
 * @brief Sets the 'glow 2' color for the given text object.
 *
 * 'Glow 2' effects are glowing colors decorating the text's (immediate)
 * surroundings.  They will be shown if the object is set to the
 * @c EVAS_TEXT_STYLE_TYPE_GLOW style. See also
 * @ref evas_obj_text_glow_color_get.
 *
 * @param[in] obj The object.
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_glow2_color_get(const Eo *obj, int *r, int *g, int *b, int *a);

/**
 * @brief Controls the style to apply on the given text object.
 *
 * Text object styles are one of the values in @ref Evas_Text_Style_Type Some
 * of those values are combinations of more than one style, and some account
 * for the direction of the rendering of shadow effects.
 *
 * @note One may use the helper macros #EVAS_TEXT_STYLE_BASIC_SET and
 * #EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET to assemble a style value.
 *
 * @param[in] obj The object.
 * @param[in] style Style type
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_style_set(Eo *obj, Evas_Text_Style_Type style);

/**
 * @brief Controls the style to apply on the given text object.
 *
 * Text object styles are one of the values in @ref Evas_Text_Style_Type Some
 * of those values are combinations of more than one style, and some account
 * for the direction of the rendering of shadow effects.
 *
 * @note One may use the helper macros #EVAS_TEXT_STYLE_BASIC_SET and
 * #EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET to assemble a style value.
 *
 * @param[in] obj The object.
 *
 * @return Style type
 *
 * @ingroup Evas_Text
 */
EOAPI Evas_Text_Style_Type evas_obj_text_style_get(const Eo *obj);

/**
 * @brief Sets the glow color for the given text object.
 *
 * Glow effects are glowing colors decorating the text's surroundings.  They
 * will be shown if the object is set to the @c EVAS_TEXT_STYLE_TYPE_GLOW
 * style.
 *
 * @note Glow effects are placed from a short distance of the text itself, but
 * not touching it. For glowing effects right on the borders of the glyphs, see
 * 'glow 2' effects.
 *
 * @param[in] obj The object.
 * @param[in] r The red component of the given color.
 * @param[in] g The green component of the given color.
 * @param[in] b The blue component of the given color.
 * @param[in] a The alpha component of the given color.
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_glow_color_set(Eo *obj, int r, int g, int b, int a);

/**
 * @brief Sets the glow color for the given text object.
 *
 * Glow effects are glowing colors decorating the text's surroundings.  They
 * will be shown if the object is set to the @c EVAS_TEXT_STYLE_TYPE_GLOW
 * style.
 *
 * @note Glow effects are placed from a short distance of the text itself, but
 * not touching it. For glowing effects right on the borders of the glyphs, see
 * 'glow 2' effects.
 *
 * @param[in] obj The object.
 * @param[out] r The red component of the given color.
 * @param[out] g The green component of the given color.
 * @param[out] b The blue component of the given color.
 * @param[out] a The alpha component of the given color.
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_glow_color_get(const Eo *obj, int *r, int *g, int *b, int *a);

/**
 * @brief Maximal descent property
 *
 * @param[in] obj The object.
 *
 * @return Evas coordinate
 *
 * @ingroup Evas_Text
 */
EOAPI int evas_obj_text_max_descent_get(const Eo *obj);

/**
 * @brief Gets the text style pad of a text object.
 *
 * @param[in] obj The object.
 * @param[out] l The left pad (or @c NULL).
 * @param[out] r The right pad (or @c NULL).
 * @param[out] t The top pad (or @c NULL).
 * @param[out] b The bottom pad (or @c NULL).
 *
 * @ingroup Evas_Text
 */
EOAPI void evas_obj_text_style_pad_get(const Eo *obj, int *l, int *r, int *t, int *b);

/**
 * @brief Retrieves the direction of the text currently being displayed in the
 * text object.
 *
 * @param[in] obj The object.
 *
 * @return Bidirectional type
 *
 * @ingroup Evas_Text
 */
EOAPI Efl_Text_Bidirectional_Type evas_obj_text_direction_get(const Eo *obj);

/**
 * @brief Ascent property
 *
 * @param[in] obj The object.
 *
 * @return Evas coordinate
 *
 * @ingroup Evas_Text
 */
EOAPI int evas_obj_text_ascent_get(const Eo *obj);

/**
 * @brief Horizontal advance property
 *
 * @param[in] obj The object.
 *
 * @return Evas coordinate
 *
 * @ingroup Evas_Text
 */
EOAPI int evas_obj_text_horiz_advance_get(const Eo *obj);

/**
 * @brief Inset property
 *
 * @param[in] obj The object.
 *
 * @return Evas coordinate
 *
 * @ingroup Evas_Text
 */
EOAPI int evas_obj_text_inset_get(const Eo *obj);

/**
 * @brief Maximal ascent property
 *
 * @param[in] obj The object.
 *
 * @return Evas coordinate
 *
 * @ingroup Evas_Text
 */
EOAPI int evas_obj_text_max_ascent_get(const Eo *obj);

/**
 * @brief Vertical advance property
 *
 * @param[in] obj The object.
 *
 * @return Evas coordinate
 *
 * @ingroup Evas_Text
 */
EOAPI int evas_obj_text_vert_advance_get(const Eo *obj);

/**
 * @brief descent property
 *
 * @param[in] obj The object.
 *
 * @return Evas coordinate
 *
 * @ingroup Evas_Text
 */
EOAPI int evas_obj_text_descent_get(const Eo *obj);

/**
 * @brief Returns the logical position of the last char in the text up to the
 * pos given.
 *
 * This is NOT the position of the last char because of the possibility of RTL
 * in the text.
 *
 * @param[in] obj The object.
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 *
 * @return Logical position of the last char
 *
 * @ingroup Evas_Text
 */
EOAPI int evas_obj_text_last_up_to_pos(const Eo *obj, int x, int y);

/**
 * @brief Get character coordinates
 *
 * @param[in] obj The object.
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[out] cx X coordinate
 * @param[out] cy Y coordinate
 * @param[out] cw Width
 * @param[out] ch Height
 *
 * @return Logical position of char
 *
 * @ingroup Evas_Text
 */
EOAPI int evas_obj_text_char_coords_get(const Eo *obj, int x, int y, int *cx, int *cy, int *cw, int *ch);

/**
 * @brief Retrieve position and dimension information of a character within a
 * text @ref Efl_Canvas_Object
 *
 * This function is used to obtain the X, Y, width and height of the character
 * located at @c pos within the @ref Efl_Canvas_Object @c obj. @c obj must be a
 * text object Any of the @c Evas_Coord parameters ($cx, @c cy, @c cw, @c ch)
 * may be @c NULL in which case no value will be assigned to that parameter.
 *
 * @param[in] obj The object.
 * @param[in] pos The character position to request co-ordinates for.
 * @param[out] cx A pointer to an int to store the X value in (can be NULL).
 * @param[out] cy A pointer to an int to store the Y value in (can be NULL).
 * @param[out] cw A pointer to an int to store the Width value in (can be
 * NULL).
 * @param[out] ch A pointer to an int to store the Height value in (can be
 * NULL).
 *
 * @return @c false on success, @c true otherwise
 *
 * @ingroup Evas_Text
 */
EOAPI Eina_Bool evas_obj_text_char_pos_get(const Eo *obj, int pos, int *cx, int *cy, int *cw, int *ch);

#endif
