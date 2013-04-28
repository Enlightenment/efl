/**
 * @brief Possible orient values for popup.
 *
 * These values should be used in conjunction to elm_popup_orient_set() to
 * set the position in which the popup should appear(relative to its parent)
 * and in conjunction with elm_popup_orient_get() to know where the popup
 * is appearing.
 *
 * @ingroup Popup
 */
typedef enum
{
   ELM_POPUP_ORIENT_TOP = 0, /**< Popup should appear in the top of parent, default */
   ELM_POPUP_ORIENT_CENTER, /**< Popup should appear in the center of parent */
   ELM_POPUP_ORIENT_BOTTOM, /**< Popup should appear in the bottom of parent */
   ELM_POPUP_ORIENT_LEFT, /**< Popup should appear in the left of parent */
   ELM_POPUP_ORIENT_RIGHT, /**< Popup should appear in the right of parent */
   ELM_POPUP_ORIENT_TOP_LEFT, /**< Popup should appear in the top left of parent */
   ELM_POPUP_ORIENT_TOP_RIGHT, /**< Popup should appear in the top right of parent */
   ELM_POPUP_ORIENT_BOTTOM_LEFT, /**< Popup should appear in the bottom left of parent */
   ELM_POPUP_ORIENT_BOTTOM_RIGHT, /**< Notify should appear in the bottom right of parent */
   ELM_POPUP_ORIENT_LAST /**< Sentinel value, @b don't use */
 } Elm_Popup_Orient;

