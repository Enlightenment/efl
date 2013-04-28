/**
 * @addtogroup Icon
 * @{
 */

typedef enum
{
   ELM_ICON_NONE,
   ELM_ICON_FILE,
   ELM_ICON_STANDARD
} Elm_Icon_Type;

/**
 * @enum Elm_Icon_Lookup_Order
 * @typedef Elm_Icon_Lookup_Order
 *
 * Lookup order used by elm_icon_standard_set(). Should look for icons in the
 * theme, FDO paths, or both?
 *
 * @ingroup Icon
 */
typedef enum
{
   ELM_ICON_LOOKUP_FDO_THEME, /**< icon look up order: freedesktop, theme */
   ELM_ICON_LOOKUP_THEME_FDO, /**< icon look up order: theme, freedesktop */
   ELM_ICON_LOOKUP_FDO, /**< icon look up order: freedesktop */
   ELM_ICON_LOOKUP_THEME /**< icon look up order: theme */
} Elm_Icon_Lookup_Order;

