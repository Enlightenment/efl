/**
 * Identifiers on how a background widget is to display its image --
 * if it was set to use an image file.
 *
 * @see elm_bg_option_set()
 * @see elm_bg_option_get()
 *
 * @ingroup Bg
 */
typedef enum
{
   ELM_BG_OPTION_CENTER, /**< center the background image */
   ELM_BG_OPTION_SCALE, /**< scale the background image, retaining aspect ratio */
   ELM_BG_OPTION_STRETCH, /**< stretch the background image to fill the widget's area */
   ELM_BG_OPTION_TILE, /**< tile background image at its original size */
   ELM_BG_OPTION_LAST /**< sentinel value, also used to indicate errors */
} Elm_Bg_Option;

