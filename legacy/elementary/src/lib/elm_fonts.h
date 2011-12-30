/**
 * @defgroup Fonts Elementary Fonts
 *
 * These are functions dealing with font rendering, selection and the
 * like for Elementary applications. One might fetch which system
 * fonts are there to use and set custom fonts for individual classes
 * of UI items containing text (text classes).
 *
 * @{
 */

typedef struct _Elm_Text_Class
{
   const char *name;
   const char *desc;
} Elm_Text_Class;

typedef struct _Elm_Font_Overlay
{
   const char    *text_class;
   const char    *font;
   Evas_Font_Size size;
} Elm_Font_Overlay;

typedef struct _Elm_Font_Properties
{
   const char *name;
   Eina_List  *styles;
} Elm_Font_Properties;

/**
 * Get Elementary's list of supported text classes.
 *
 * @return The text classes list, with @c Elm_Text_Class blobs as data.
 * @ingroup Fonts
 *
 * Release the list with elm_text_classes_list_free().
 */
EAPI const Eina_List *elm_text_classes_list_get(void);

/**
 * Free Elementary's list of supported text classes.
 *
 * @ingroup Fonts
 *
 * @see elm_text_classes_list_get().
 */
EAPI void             elm_text_classes_list_free(const Eina_List *list);

/**
 * Get Elementary's list of font overlays, set with
 * elm_font_overlay_set().
 *
 * @return The font overlays list, with @c Elm_Font_Overlay blobs as
 * data.
 *
 * @ingroup Fonts
 *
 * For each text class, one can set a <b>font overlay</b> for it,
 * overriding the default font properties for that class coming from
 * the theme in use. There is no need to free this list.
 *
 * @see elm_font_overlay_set() and elm_font_overlay_unset().
 */
EAPI const Eina_List *elm_font_overlay_list_get(void);

/**
 * Set a font overlay for a given Elementary text class.
 *
 * @param text_class Text class name
 * @param font Font name and style string
 * @param size Font size
 *
 * @ingroup Fonts
 *
 * @p font has to be in the format returned by
 * elm_font_fontconfig_name_get(). @see elm_font_overlay_list_get()
 * and elm_font_overlay_unset().
 */
EAPI void             elm_font_overlay_set(const char *text_class, const char *font, Evas_Font_Size size);

/**
 * Unset a font overlay for a given Elementary text class.
 *
 * @param text_class Text class name
 *
 * @ingroup Fonts
 *
 * This will bring back text elements belonging to text class
 * @p text_class back to their default font settings.
 */
EAPI void             elm_font_overlay_unset(const char *text_class);

/**
 * Apply the changes made with elm_font_overlay_set() and
 * elm_font_overlay_unset() on the current Elementary window.
 *
 * @ingroup Fonts
 *
 * This applies all font overlays set to all objects in the UI.
 */
EAPI void             elm_font_overlay_apply(void);

/**
 * Apply the changes made with elm_font_overlay_set() and
 * elm_font_overlay_unset() on all Elementary application windows.
 *
 * @ingroup Fonts
 *
 * This applies all font overlays set to all objects in the UI.
 */
// XXX: deprecate and replace with elm_config_all_flush()
EAPI void             elm_font_overlay_all_apply(void);

/**
 * Translate a font (family) name string in fontconfig's font names
 * syntax into an @c Elm_Font_Properties struct.
 *
 * @param font The font name and styles string
 * @return the font properties struct
 *
 * @ingroup Fonts
 *
 * @note The reverse translation can be achived with
 * elm_font_fontconfig_name_get(), for one style only (single font
 * instance, not family).
 */
EAPI Elm_Font_Properties *
                      elm_font_properties_get(const char *font)
EINA_ARG_NONNULL(1);

/**
 * Free font properties return by elm_font_properties_get().
 *
 * @param efp the font properties struct
 *
 * @ingroup Fonts
 */
EAPI void             elm_font_properties_free(Elm_Font_Properties *efp) EINA_ARG_NONNULL(1);

/**
 * Translate a font name, bound to a style, into fontconfig's font names
 * syntax.
 *
 * @param name The font (family) name
 * @param style The given style (may be @c NULL)
 *
 * @return the font name and style string
 *
 * @ingroup Fonts
 *
 * @note The reverse translation can be achived with
 * elm_font_properties_get(), for one style only (single font
 * instance, not family).
 */
EAPI const char      *elm_font_fontconfig_name_get(const char *name, const char *style) EINA_ARG_NONNULL(1);

/**
 * Free the font string return by elm_font_fontconfig_name_get().
 *
 * @param name the font properties struct
 *
 * @ingroup Fonts
 */
EAPI void             elm_font_fontconfig_name_free(const char *name) EINA_ARG_NONNULL(1);

/**
 * Create a font hash table of available system fonts.
 *
 * One must call it with @p list being the return value of
 * evas_font_available_list(). The hash will be indexed by font
 * (family) names, being its values @c Elm_Font_Properties blobs.
 *
 * @param list The list of available system fonts, as returned by
 * evas_font_available_list().
 * @return the font hash.
 *
 * @ingroup Fonts
 *
 * @note The user is supposed to get it populated at least with 3
 * default font families (Sans, Serif, Monospace), which should be
 * present on most systems.
 */
EAPI Eina_Hash       *elm_font_available_hash_add(Eina_List *list);

/**
 * Free the hash return by elm_font_available_hash_add().
 *
 * @param hash the hash to be freed.
 *
 * @ingroup Fonts
 */
EAPI void             elm_font_available_hash_del(Eina_Hash *hash);

/**
 * @}
 */
