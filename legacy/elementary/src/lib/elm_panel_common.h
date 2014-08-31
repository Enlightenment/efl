/**
 * Panel orientation mode
 *
 * @ingroup Panel
 */
typedef enum
{
   ELM_PANEL_ORIENT_TOP, /**< Panel (dis)appears from the top */
   ELM_PANEL_ORIENT_BOTTOM, /**< Panel (dis)appears from the bottom */
   ELM_PANEL_ORIENT_LEFT, /**< Panel (dis)appears from the left */
   ELM_PANEL_ORIENT_RIGHT, /**< Panel (dis)appears from the right */
} Elm_Panel_Orient;

/**
 * Panel scroll information
 *
 * @since 1.11
 * @ingroup Panel
 */
typedef struct _Elm_Panel_Scroll_Info
{
   double rel_x;  /**<content scrolled position (0.0 ~ 1.0) in the panel>*/
   double rel_y;  /**<content scrolled position (0.0 ~ 1.0) in the panel>*/

} Elm_Panel_Scroll_Info;
