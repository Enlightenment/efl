/**
 * @defgroup Fonts Elementary Fonts
 * @ingroup Elementary
 *
 * These are functions dealing with font rendering, selection and the
 * like for Elementary applications. One might fetch which system
 * fonts are there to use and set custom fonts for individual classes
 * of UI items containing text (text classes).
 *
 * @{
 */

typedef struct _Elm_Font_Properties
{
   const char *name;
   Eina_List  *styles;
} Elm_Font_Properties;

/**
 * Translate a font (family) name string in fontconfig's font names
 * syntax into an @c Elm_Font_Properties struct.
 *
 * @param font The font name and styles string
 * @return the font properties struct
 *
 * @ingroup Fonts
 *
 * @note The reverse translation can be achieved with
 * elm_font_fontconfig_name_get(), for one style only (single font
 * instance, not family).
 */
EAPI Elm_Font_Properties *elm_font_properties_get(const char *font);

/**
 * Free font properties return by elm_font_properties_get().
 *
 * @param efp the font properties struct
 *
 * @ingroup Fonts
 */
EAPI void             elm_font_properties_free(Elm_Font_Properties *efp);

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
 * @note The reverse translation can be achieved with
 * elm_font_properties_get(), for one style only (single font
 * instance, not family).
 */
EAPI char      *elm_font_fontconfig_name_get(const char *name, const char *style);

/**
 * Free the font string return by elm_font_fontconfig_name_get().
 *
 * @param name the font properties struct
 *
 * @ingroup Fonts
 */
EAPI void             elm_font_fontconfig_name_free(char *name);

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
 * Free the hash returned by elm_font_available_hash_add().
 *
 * @param hash the hash to be freed.
 *
 * @ingroup Fonts
 */
EAPI void             elm_font_available_hash_del(Eina_Hash *hash);

/**
 * @}
 */
