/*
 * Various ICCCM related functions.
 *
 * This is ALL the code involving anything ICCCM related, for both WM and
 * client.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <xcb/xcb_icccm.h>

#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"

/**
 * @defgroup Ecore_X_ICCCM_Group ICCCM related functions.
 *
 * Functions related to ICCCM.
 */

static int _ecore_x_icccm_size_hints_get (const void       *reply,
                                          Ecore_X_Atom      property,
                                          xcb_size_hints_t *hints)
{
   uint32_t s;

   if (!hints)
      return 0;

   if (!reply)
      return 0;

   if ((((xcb_get_property_reply_t *)reply)->type != ECORE_X_ATOM_WM_SIZE_HINTS) &&
       ((((xcb_get_property_reply_t *)reply)->format != 8) ||
        (((xcb_get_property_reply_t *)reply)->format != 16) ||
        (((xcb_get_property_reply_t *)reply)->format != 32)) &&
       (((xcb_get_property_reply_t *)reply)->value_len < 15)) /* OldNumPropSizeElements = 15 (pre-ICCCM) */
      return 0;

   memcpy(hints,
          xcb_get_property_value((xcb_get_property_reply_t *)reply),
          ((xcb_get_property_reply_t *)reply)->value_len);

   s = (XCB_SIZE_HINT_US_POSITION | XCB_SIZE_HINT_US_SIZE |
        XCB_SIZE_HINT_P_POSITION | XCB_SIZE_HINT_P_SIZE |
        XCB_SIZE_HINT_P_MIN_SIZE | XCB_SIZE_HINT_P_MAX_SIZE |
        XCB_SIZE_HINT_P_RESIZE_INC | XCB_SIZE_HINT_P_ASPECT);

   if (((xcb_get_property_reply_t *)reply)->value_len >= 18) /* NumPropSizeElements = 18 (ICCCM version 1) */
      s |= (XCB_SIZE_HINT_BASE_SIZE | XCB_SIZE_HINT_P_WIN_GRAVITY);
   else
     {
        xcb_size_hints_set_base_size(hints, 0, 0);
        xcb_size_hints_set_win_gravity(hints, 0);
     }

   /* FIXME: is it necessary ? */
   /* hints->flags &= s; */         /* get rid of unwanted bits */

   return 1;
} /* _ecore_x_icccm_size_hints_get */

/**
 * Sets the state of a window.
 * @param window The window.
 * @param state  The state.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_state_set(Ecore_X_Window            window,
                        Ecore_X_Window_State_Hint state)
{
   uint32_t c[2];

   if (state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
      c[0] = XCB_WM_STATE_WITHDRAWN;
   else if (state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
      c[0] = XCB_WM_STATE_NORMAL;
   else if (state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
      c[0] = XCB_WM_STATE_ICONIC;

   c[1] = 0;
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_WM_STATE, ECORE_X_ATOM_WM_STATE, 32,
                       2, c);
} /* ecore_x_icccm_state_set */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_state_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_WM_STATE,
                                       ECORE_X_ATOM_WM_STATE,
                                       0L, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_state_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_state_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_state_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_state_get_fetch */

/**
 * Gets the state of a window.
 * @param window The window.
 * @return       The state of the window
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_state_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_state_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI Ecore_X_Window_State_Hint
ecore_x_icccm_state_get(Ecore_X_Window window __UNUSED__)
{
   xcb_get_property_reply_t *reply;
   uint8_t *prop;
   Ecore_X_Window_State_Hint hint = ECORE_X_WINDOW_STATE_HINT_NONE;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return hint;

   if ((reply->type == 0) ||
       (reply->format != 8) ||
       (reply->value_len != 2))
      return hint;

   prop = (uint8_t *)xcb_get_property_value(reply);
   switch (prop[0]) {
      case XCB_WM_STATE_WITHDRAWN:
         hint = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
         break;

      case XCB_WM_STATE_NORMAL:
         hint = ECORE_X_WINDOW_STATE_HINT_NORMAL;
         break;

      case XCB_WM_STATE_ICONIC:
         hint = ECORE_X_WINDOW_STATE_HINT_ICONIC;
         break;

      default:
         hint = ECORE_X_WINDOW_STATE_HINT_NONE;
         break;
     } /* switch */

   return hint;
} /* ecore_x_icccm_state_get */

/**
 * Sends the ClientMessage event with the DeleteWindow property.
 * @param window The window.
 * @param time   The time.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_delete_window_send(Ecore_X_Window window,
                                 Ecore_X_Time   time)
{
   ecore_x_client_message32_send(window, ECORE_X_ATOM_WM_PROTOCOLS,
                                 ECORE_X_EVENT_MASK_NONE,
                                 ECORE_X_ATOM_WM_DELETE_WINDOW,
                                 time, 0, 0, 0);
} /* ecore_x_icccm_delete_window_send */

/**
 * Sends the ClientMessage event with the TakeFocus property.
 * @param window The window.
 * @param time   The time.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_take_focus_send(Ecore_X_Window window,
                              Ecore_X_Time   time)
{
   ecore_x_client_message32_send(window, ECORE_X_ATOM_WM_PROTOCOLS,
                                 ECORE_X_EVENT_MASK_NONE,
                                 ECORE_X_ATOM_WM_TAKE_FOCUS,
                                 time, 0, 0, 0);
} /* ecore_x_icccm_take_focus_send */

/**
 * Sends the ClientMessage event with the SaveYourself property.
 * @param window The window.
 * @param time   The time.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_save_yourself_send(Ecore_X_Window window,
                                 Ecore_X_Time   time)
{
   ecore_x_client_message32_send(window, ECORE_X_ATOM_WM_PROTOCOLS,
                                 ECORE_X_EVENT_MASK_NONE,
                                 ECORE_X_ATOM_WM_SAVE_YOURSELF,
                                 time, 0, 0, 0);
} /* ecore_x_icccm_save_yourself_send */

/**
 * Sends the ConfigureNotify event with the StructureNotify property.
 * @param window The window.
 * @param x      The X coordinate.
 * @param y      The Y coordinate.
 * @param width  The width.
 * @param height The height.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_move_resize_send(Ecore_X_Window window,
                               int            x,
                               int            y,
                               int            width,
                               int            height)
{
   xcb_configure_notify_event_t ev;

   ev.response_type = XCB_CONFIGURE_NOTIFY | 0x80;
   ev.pad0 = 0;
   ev.sequence = 0;
   ev.event = window;
   ev.window = window;
   ev.above_sibling = 0;
   ev.x = x;
   ev.y = y;
   ev.width = width;
   ev.height = height;
   ev.border_width = 0;
   ev.override_redirect = 0;
   xcb_send_event(_ecore_xcb_conn, 0, window,
                  XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ev);
} /* ecore_x_icccm_move_resize_send */

/**
 * Sets the hints of a window.
 * @param window        The window.
 * @param accepts_focus AcceptFocus hint
 * @param initial_state Initial state flags.
 * @param icon_pixmap   Icon pixmap.
 * @param icon_mask     Icon mask.
 * @param icon_window   Icon window.
 * @param window_group  Group window.
 * @param is_urgent     IsUrgent flag.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_hints_set(Ecore_X_Window            window,
                        Eina_Bool                 accepts_focus,
                        Ecore_X_Window_State_Hint initial_state,
                        Ecore_X_Pixmap            icon_pixmap,
                        Ecore_X_Pixmap            icon_mask,
                        Ecore_X_Window            icon_window,
                        Ecore_X_Window            window_group,
                        Eina_Bool                 is_urgent)
{
   xcb_wm_hints_t hints;

   memset(&hints, 0, sizeof(hints));
   xcb_wm_hints_set_input(&hints, accepts_focus);
   if (initial_state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
      xcb_wm_hints_set_withdrawn(&hints);
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
      xcb_wm_hints_set_normal(&hints);
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
      xcb_wm_hints_set_iconic(&hints);

   if (icon_pixmap != 0)
      xcb_wm_hints_set_icon_pixmap(&hints, icon_pixmap);

   if (icon_mask != 0)
      xcb_wm_hints_set_icon_mask(&hints, icon_mask);

   if (icon_window != 0)
      xcb_wm_hints_set_icon_window(&hints, icon_window);

   if (window_group != 0)
      xcb_wm_hints_set_window_group(&hints, window_group);

   if (is_urgent)
      xcb_wm_hints_set_urgency(&hints);

   xcb_set_wm_hints(_ecore_xcb_conn, window, &hints);
} /* ecore_x_icccm_hints_set */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_hints_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_WM_HINTS,
                                       ECORE_X_ATOM_WM_HINTS,
                                       0L, XCB_NUM_WM_HINTS_ELEMENTS);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_hints_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_hints_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_hints_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_hints_get_fetch */

/**
 * Gets the hints of a window.
 * @param window        The window.
 * @param accepts_focus AcceptFocus hint
 * @param initial_state Initial state flags.
 * @param icon_pixmap   Icon pixmap.
 * @param icon_mask     Icon mask.
 * @param icon_window   Icon window.
 * @param window_group  Group window.
 * @param is_urgent     IsUrgent flag.
 * @return              1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_hints_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_hints_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI Eina_Bool
ecore_x_icccm_hints_get(Ecore_X_Window window      __UNUSED__,
                        Eina_Bool                 *accepts_focus,
                        Ecore_X_Window_State_Hint *initial_state,
                        Ecore_X_Pixmap            *icon_pixmap,
                        Ecore_X_Pixmap            *icon_mask,
                        Ecore_X_Window            *icon_window,
                        Ecore_X_Window            *window_group,
                        Eina_Bool                 *is_urgent)
{
   xcb_wm_hints_t hints;
   xcb_get_property_reply_t *reply;
   int32_t hints_flags;
   uint32_t hints_input;
   int32_t hints_initial_state;
   xcb_pixmap_t hints_icon_pixmap;
   xcb_pixmap_t hints_icon_mask;
   xcb_window_t hints_icon_window;
   xcb_window_t hints_window_group;

   if (accepts_focus)
      *accepts_focus = 1;

   if (initial_state)
      *initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;

   if (icon_pixmap)
      *icon_pixmap = 0;

   if (icon_mask)
      *icon_mask = 0;

   if (icon_window)
      *icon_window = 0;

   if (window_group)
      *window_group = 0;

   if (is_urgent)
      *is_urgent = 0;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return 0;

   if ((reply->type != ECORE_X_ATOM_WM_HINTS) ||
       (reply->value_len < (XCB_NUM_WM_HINTS_ELEMENTS - 1)) ||
       (reply->format != 32))
      return 0;

   memcpy(&hints, xcb_get_property_value(reply), reply->value_len);
   hints_flags = hints.flags;
   hints_input = hints.input;
   hints_initial_state = hints.initial_state;
   hints_icon_pixmap = hints.icon_pixmap;
   hints_icon_mask = hints.icon_mask;
   hints_icon_window = hints.icon_window;
   hints_window_group = hints.window_group;

   if ((hints_flags & XCB_WM_HINT_INPUT) && (accepts_focus))
     {
        if(hints_input)
           *accepts_focus = 1;
        else
           *accepts_focus = 0;
     }

   if ((hints_flags & XCB_WM_HINT_STATE) && (initial_state))
     {
        if (hints_initial_state == XCB_WM_STATE_WITHDRAWN)
           *initial_state = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
        else if (hints_initial_state == XCB_WM_STATE_NORMAL)
           *initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
        else if (hints_initial_state == XCB_WM_STATE_ICONIC)
           *initial_state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
     }

   if ((hints_flags & XCB_WM_HINT_ICON_PIXMAP) && (icon_pixmap))
     {
        *icon_pixmap = hints_icon_pixmap;
     }

   if ((hints_flags & XCB_WM_HINT_ICON_MASK) && (icon_mask))
     {
        *icon_mask = hints_icon_mask;
     }

   if ((hints_flags & XCB_WM_HINT_ICON_WINDOW) && (icon_window))
     {
        *icon_window = hints_icon_window;
     }

   if ((hints_flags & XCB_WM_HINT_WINDOW_GROUP) && (window_group))
     {
        if (reply->value_len < XCB_NUM_WM_HINTS_ELEMENTS)
           *window_group = 0;
        else
           *window_group = hints_window_group;
     }

   if ((hints_flags & XCB_WM_HINT_X_URGENCY) && (is_urgent))
     {
        *is_urgent = 1;
     }

   return 1;
} /* ecore_x_icccm_hints_get */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_size_pos_hints_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_WM_NORMAL_HINTS,
                                       ECORE_X_ATOM_WM_SIZE_HINTS,
                                       0L, 18);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_size_pos_hints_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_size_pos_hints_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_size_pos_hints_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_size_pos_hints_get_fetch */

/**
 * Sets the hints of a window.
 * @param window      The window.
 * @param request_pos Request position flag.
 * @param gravity     Gravity.
 * @param min_w       Minimum width.
 * @param min_h       Minimum height.
 * @param max_w       Maximum width.
 * @param max_h       Maximum height.
 * @param base_w      Base width
 * @param base_h      Base height
 * @param step_x      X step coordinate.
 * @param step_y      Y step coordinate.
 * @param min_aspect  Minimum aspect ratio.
 * @param max_aspect  Maximum aspect ratio.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_size_pos_hints_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_size_pos_hints_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_size_pos_hints_set(Ecore_X_Window  window,
                                 Eina_Bool       request_pos,
                                 Ecore_X_Gravity gravity,
                                 int             min_w,
                                 int             min_h,
                                 int             max_w,
                                 int             max_h,
                                 int             base_w,
                                 int             base_h,
                                 int             step_x,
                                 int             step_y,
                                 double          min_aspect,
                                 double          max_aspect)
{
   xcb_size_hints_t hint;
   xcb_get_property_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply ||
       (reply->type != ECORE_X_ATOM_WM_SIZE_HINTS) ||
       ((reply->format != 8) &&
        (reply->format != 16) &&
        (reply->format != 32)) ||
       (reply->value_len < 15))
      return;

   hint.flags = 0;
   if (request_pos)
     {
        hint.flags = XCB_SIZE_HINT_US_POSITION;
     }

   if (gravity != ECORE_X_GRAVITY_NW)
     {
        hint.win_gravity = (uint8_t)gravity;
     }

   if ((min_w > 0) || (min_h > 0))
     {
        hint.min_width = min_w;
        hint.min_height = min_h;
     }

   if ((max_w > 0) || (max_h > 0))
     {
        hint.max_width = max_w;
        hint.max_height = max_h;
     }

   if ((base_w > 0) || (base_h > 0))
     {
        hint.base_width = base_w;
        hint.base_height = base_h;
     }

   if ((step_x > 1) || (step_y > 1))
     {
        hint.width_inc = step_x;
        hint.height_inc = step_y;
     }

   if ((min_aspect > 0.0) || (max_aspect > 0.0))
     {
        xcb_size_hints_set_aspect(&hint,
                                  (int32_t)(min_aspect * 10000),
                                  10000,
                                  (int32_t)(max_aspect * 10000),
                                  10000);
     }

   xcb_set_wm_normal_hints(_ecore_xcb_conn, window, &hint);
} /* ecore_x_icccm_size_pos_hints_set */

/**
 * Gets the hints of a window.
 * @param window      The window.
 * @param request_pos Request position flag.
 * @param gravity     Gravity.
 * @param min_w       Minimum width.
 * @param min_h       Minimum height.
 * @param max_w       Maximum width.
 * @param max_h       Maximum height.
 * @param base_w      Base width
 * @param base_h      Base height
 * @param step_x      X step coordinate.
 * @param step_y      Y step coordinate.
 * @param min_aspect  Minimum aspect ratio.
 * @param max_aspect  M
 * @return            1 on success, 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_size_pos_hints_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_size_pos_hints_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI Eina_Bool
ecore_x_icccm_size_pos_hints_get(Ecore_X_Window window __UNUSED__,
                                 Eina_Bool            *request_pos,
                                 Ecore_X_Gravity      *gravity,
                                 int                  *min_w,
                                 int                  *min_h,
                                 int                  *max_w,
                                 int                  *max_h,
                                 int                  *base_w,
                                 int                  *base_h,
                                 int                  *step_x,
                                 int                  *step_y,
                                 double               *min_aspect,
                                 double               *max_aspect)
{
   xcb_size_hints_t hint;
   xcb_get_property_reply_t *reply;
   uint32_t flags;
   int32_t minw = 0;
   int32_t minh = 0;
   int32_t maxw = 32767;
   int32_t maxh = 32767;
   int32_t basew = -1;
   int32_t baseh = -1;
   int32_t stepx = -1;
   int32_t stepy = -1;
   double mina = 0.0;
   double maxa = 0.0;

   if (request_pos)
      *request_pos = 0;

   if (gravity)
      *gravity = ECORE_X_GRAVITY_NW;

   if (min_w)
      *min_w = minw;

   if (min_h)
      *min_h = minh;

   if (max_w)
      *max_w = maxw;

   if (max_h)
      *max_h = maxh;

   if (base_w)
      *base_w = basew;

   if (base_h)
      *base_h = baseh;

   if (step_x)
      *step_x = stepx;

   if (step_y)
      *step_y = stepy;

   if (min_aspect)
      *min_aspect = mina;

   if (max_aspect)
      *max_aspect = maxa;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return 0;

   if (!_ecore_x_icccm_size_hints_get(reply, ECORE_X_ATOM_WM_NORMAL_HINTS, &hint))
      return 0;

   flags = hint.flags;
   if ((flags & XCB_SIZE_HINT_US_POSITION) || (flags & XCB_SIZE_HINT_P_POSITION))
     {
        if (request_pos)
           *request_pos = 1;
     }

   if (flags & XCB_SIZE_HINT_P_WIN_GRAVITY)
     {
        if (gravity)
           *gravity = hint.win_gravity;
     }

   if (flags & XCB_SIZE_HINT_P_MIN_SIZE)
     {
        minw = hint.min_width;
        minh = hint.min_height;
     }

   if (flags & XCB_SIZE_HINT_P_MAX_SIZE)
     {
        maxw = hint.max_width;
        maxh = hint.max_height;
        if (maxw < minw)
           maxw = minw;

        if (maxh < minh)
           maxh = minh;
     }

   if (flags & XCB_SIZE_HINT_BASE_SIZE)
     {
        basew = hint.base_width;
        baseh = hint.base_height;
        if (basew > minw)
           minw = basew;

        if (baseh > minh)
           minh = baseh;
     }

   if (flags & XCB_SIZE_HINT_P_RESIZE_INC)
     {
        stepx = hint.width_inc;
        stepy = hint.height_inc;
        if (stepx < 1)
           stepx = 1;

        if (stepy < 1)
           stepy = 1;
     }

   if (flags & XCB_SIZE_HINT_P_ASPECT)
     {
        if (hint.min_aspect_den > 0)
           mina = ((double)hint.min_aspect_num) / ((double)hint.min_aspect_den);

        if (hint.max_aspect_den > 0)
           maxa = ((double)hint.max_aspect_num) / ((double)hint.max_aspect_den);
     }

   if (min_w)
      *min_w = minw;

   if (min_h)
      *min_h = minh;

   if (max_w)
      *max_w = maxw;

   if (max_h)
      *max_h = maxh;

   if (base_w)
      *base_w = basew;

   if (base_h)
      *base_h = baseh;

   if (step_x)
      *step_x = stepx;

   if (step_y)
      *step_y = stepy;

   if (min_aspect)
      *min_aspect = mina;

   if (max_aspect)
      *max_aspect = maxa;

   return 1;
} /* ecore_x_icccm_size_pos_hints_get */

/**
 * Set the title of a window
 * @param window The window.
 * @param title  The title.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_title_set(Ecore_X_Window window,
                        const char    *title)
{
   if (!title)
      return;

   /* FIXME: to do:  utf8 */

/*    xprop.value = NULL; */
/* #ifdef X_HAVE_UTF8_STRING */
/*    list[0] = strdup(t); */
/*    ret = */
/*       Xutf8TextListToTextProperty(_ecore_xcb_conn, list, 1, XUTF8StringStyle, */
/*				  &xprop); */
/* #else */
/*    list[0] = strdup(t); */
/*    ret = */
/*       XmbTextListToTextProperty(_ecore_xcb_conn, list, 1, XStdICCTextStyle, */
/*				&xprop); */
/* #endif */

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_WM_NAME, ECORE_X_ATOM_STRING, 8,
                       strlen(title), title);
} /* ecore_x_icccm_title_set */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_title_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_WM_NAME,
                                       XCB_GET_PROPERTY_TYPE_ANY,
                                       0L, 128);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_title_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_title_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_title_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_title_get_fetch */

/**
 * Gets the title of a window.
 * @param window The window (Unused).
 * @return       The title of the window
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_title_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_title_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI char *
ecore_x_icccm_title_get(Ecore_X_Window window __UNUSED__)
{
   char *title = NULL;
   xcb_get_property_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return NULL;

   if (reply->type == ECORE_X_ATOM_UTF8_STRING)
     {
        int length;

        /* FIXME: verify the value of length */
        length = (reply->value_len * reply->format) / 8;
        title = (char *)malloc((length + 1) * sizeof(char));
        memcpy(title, xcb_get_property_value(reply), length);
        title[length] = '\0';
     }
   /* not in UTF8, so we convert */
   else
     {
        /* convert to utf8 */

        /* FIXME: to do... */

/* #ifdef X_HAVE_UTF8_STRING */
/*              ret = Xutf8TextPropertyToTextList(_ecore_xcb_conn, &xprop, */
/*                                                &list, &num); */
/* #else */
/*              ret = XmbTextPropertyToTextList(_ecore_xcb_conn, &xprop, */
/*                                              &list, &num); */
/* #endif */

/*              if ((ret == XLocaleNotSupported) || */
/*                  (ret == XNoMemory) || (ret == XConverterNotFound)) */
/*                { */
/*                   t = strdup((char *)xprop.value); */
/*                } */
/*              else if ((ret >= Success) && (num > 0)) */
/*                { */
/*                   t = strdup(list[0]); */
/*                } */
/*              if (list) */
/*                XFreeStringList(list); */
/*           } */

/*         if (xprop.value) XFree(xprop.value); */
     }

   return title;
} /* ecore_x_icccm_title_get */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_protocol_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_WM_PROTOCOLS,
                                       ECORE_X_ATOM_ATOM, 0, 1000000L);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_protocol_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_protocol_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_protocol_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_protocol_get_fetch */

/**
 * Set or unset a wm protocol property.
 * @param window   The Window
 * @param protocol The protocol to enable/disable
 * @param on       On/Off
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_protocol_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_protocol_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_protocol_set(Ecore_X_Window      window,
                           Ecore_X_WM_Protocol protocol,
                           Eina_Bool           on)
{
   xcb_get_property_reply_t *reply;
   Ecore_X_Atom *protos = NULL;
   Ecore_X_Atom proto;
   uint32_t protos_count = 0;
   uint8_t already_set = 0;

   /* Check for invalid values */
   if (protocol >= ECORE_X_WM_PROTOCOL_NUM)
      return;

   proto = _ecore_xcb_atoms_wm_protocols[protocol];

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return;

   if ((reply->type == ECORE_X_ATOM_ATOM) && (reply->format == 32))
     {
        uint32_t i;

        protos_count = reply->value_len / sizeof(Ecore_X_Atom);
        protos = (Ecore_X_Atom *)xcb_get_property_value(reply);
        for (i = 0; i < protos_count; i++)
          {
             if (protos[i] == proto)
               {
                  already_set = 1;
                  break;
               }
          }
     }

   if (on)
     {
        Ecore_X_Atom *new_protos = NULL;

        if (already_set)
           return;

        new_protos = (Ecore_X_Atom *)malloc((protos_count + 1) * sizeof(Ecore_X_Atom));
        if (!new_protos)
           return;

        memcpy(new_protos, protos, reply->value_len);
        new_protos[protos_count] = proto;
        xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                            ECORE_X_ATOM_WM_PROTOCOLS,
                            ECORE_X_ATOM_ATOM, 32,
                            protos_count + 1, new_protos);
        free(new_protos);
     }
   else
     {
        uint32_t i;

        if (!already_set)
           return;

        for (i = 0; i < protos_count; i++)
          {
             if (protos[i] == proto)
               {
                  uint32_t j;

                  for (j = i + 1; j < protos_count; j++)
                     protos[j - 1] = protos[j];
                  if (protos_count > 1)
                     xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                                         ECORE_X_ATOM_WM_PROTOCOLS,
                                         ECORE_X_ATOM_ATOM, 32,
                                         protos_count - 1, protos);
                  else
                     xcb_delete_property(_ecore_xcb_conn, window,
                                         ECORE_X_ATOM_WM_PROTOCOLS);

                  return;
               }
          }
     }
} /* ecore_x_icccm_protocol_set */

/**
 * Determines whether a protocol is set for a window.
 * @param window   The Window (Unused).
 * @param protocol The protocol to query.
 * @return         1 if the protocol is set, else 0.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_protocol_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_protocol_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI Eina_Bool
ecore_x_icccm_protocol_isset(Ecore_X_Window window __UNUSED__,
                             Ecore_X_WM_Protocol   protocol)
{
   xcb_get_property_reply_t *reply;
   Ecore_X_Atom *protos = NULL;
   Ecore_X_Atom proto;
   uint32_t i;
   uint8_t ret = 0;

   /* check for invalid values */
   if (protocol >= ECORE_X_WM_PROTOCOL_NUM)
      return 0;

   proto = _ecore_xcb_atoms_wm_protocols[protocol];

   reply = _ecore_xcb_reply_get();
   if (!reply || (reply->type != ECORE_X_ATOM_ATOM) || (reply->format != 32))
     {
        return 0;
     }

   protos = (Ecore_X_Atom *)xcb_get_property_value(reply);
   for (i = 0; i < reply->value_len; i++)
      if (protos[i] == proto)
        {
           ret = 1;
           break;
        }

   return ret;
} /* ecore_x_icccm_protocol_isset */

/**
 * Set a window name & class.
 * @param window The window
 * @param name   The name string
 * @param class  The class string
 *
 * Set the name and class of @p window to respectively @p name and @p class.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_name_class_set(Ecore_X_Window window,
                             const char    *name,
                             const char    *class)
{
   char *class_string;
   char *s;
   int length_name;
   int length_class;

   length_name = strlen(name);
   length_class = strlen(class);
   class_string = (char *)malloc(sizeof(char) * (length_name + length_class + 2));
   if (!class_string)
      return;

   s = class_string;
   if (length_name)
     {
        strcpy(s, name);
        s += length_name + 1;
     }
   else
      *s++ = '\0';

   if(length_class)
      strcpy(s, class);
   else
      *s = '\0';

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_WM_CLASS, ECORE_X_ATOM_STRING, 8,
                       length_name + length_class + 2, (void *)class_string);
   free(class_string);
} /* ecore_x_icccm_name_class_set */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_name_class_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_WM_CLASS,
                                       ECORE_X_ATOM_STRING,
                                       0, 2048L);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_name_class_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_name_class_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_name_class_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_name_class_get_fetch */

/**
 * Get a window name and class.
 * @param window The window (Unused).
 * @param name   The name string.
 * @param class  The class string.
 *
 * Store the name and class of @p window into respectively @p name and
 * @p class.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_name_class_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_name_class_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_name_class_get(Ecore_X_Window window __UNUSED__,
                             char                **name,
                             char                **class)
{
   xcb_get_property_reply_t *reply;
   void *data;
   char *n = NULL;
   char *c = NULL;
   int length;
   int length_name;
   int length_class;

   if (name)
      *name = NULL;

   if (class)
      *class = NULL;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return;

   if ((reply->type != ECORE_X_ATOM_STRING) ||
       (reply->format != 8))
      return;

   length = reply->value_len;
   data = xcb_get_property_value(reply);
   /* data contains the name string and the class string */
   /* separated by the NULL character. data is not NULL-terminated */
   length_name = strlen(data);
   n = (char *)malloc(sizeof(char) * (length_name + 1));
   if (!n)
      return;

   length_class = length - length_name - 1;
   c = (char *)malloc(sizeof(char) * (length_class + 1));
   if (!c)
     {
        free(n);
        return;
     }

   memcpy(n, data, length_name);
   n[length_name] = '\0';
   length_class = length - length_name - 1;
   data += length_name + 1;
   memcpy(c, data, length_class);
   c[length_class] = '\0';

   if (name)
      *name = n;

   if (class)
      *class = c;
} /* ecore_x_icccm_name_class_get */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_client_machine_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window ? window : ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                       ECORE_X_ATOM_WM_CLIENT_MACHINE,
                                       XCB_GET_PROPERTY_TYPE_ANY,
                                       0L, 1000000L);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_client_machine_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_client_machine_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_client_machine_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_client_machine_get_fetch */

/**
 * Get a window client machine string.
 * @param window The window
 * @return The windows client machine string
 *
 * Return the client machine of a window. String must be free'd when done with.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_client_machine_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_client_machine_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI char *
ecore_x_icccm_client_machine_get(Ecore_X_Window window)
{
   char *name;

   name = ecore_x_window_prop_string_get(window, ECORE_X_ATOM_WM_CLIENT_MACHINE);
   return name;
} /* ecore_x_icccm_client_machine_get */

/**
 * Sets the WM_COMMAND property for @a win.
 *
 * @param window The window.
 * @param argc   Number of arguments.
 * @param argv   Arguments.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_command_set(Ecore_X_Window window,
                          int            argc,
                          char         **argv)
{
   void *buf;
   char *b;
   int nbytes;
   int i;

   for (i = 0, nbytes = 0; i < argc; i++)
     {
        nbytes += strlen(argv[i]) + 1;
     }
   buf = malloc(sizeof(char) * nbytes);
   if (!buf)
      return;

   b = (char *)buf;
   for (i = 0; i < argc; i++)
     {
        if (argv[i])
          {
             strcpy(b, argv[i]);
             b += strlen(argv[i]) + 1;
          }
        else
           *b++ = '\0';
     }
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_WM_COMMAND, ECORE_X_ATOM_STRING, 8,
                       nbytes, buf);
   free(buf);
} /* ecore_x_icccm_command_set */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_command_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_WM_COMMAND,
                                       XCB_GET_PROPERTY_TYPE_ANY,
                                       0L, 1000000L);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_command_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_command_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_command_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_command_get_fetch */

/**
 * Get the WM_COMMAND property for a window.
 *
 * @param win  The window.
 * @param argc Number of arguments.
 * @param argv Arguments.
 *
 * Return the command of @p window and store it in @p argv. @p argc
 * contains the number of arguments. String must be free'd when done with.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_command_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_command_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_command_get(Ecore_X_Window window __UNUSED__,
                          int                  *argc,
                          char               ***argv)
{
   xcb_get_property_reply_t *reply;
   char **v;
   char *data;
   char *cp;
   char *start;
   uint32_t value_len;
   int c;
   int i;
   int j;

   if (argc)
      *argc = 0;

   if (argv)
      *argv = NULL;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return;

   if ((reply->type != ECORE_X_ATOM_STRING) ||
       (reply->format != 8))
      return;

   value_len = reply->value_len;
   data = (char *)xcb_get_property_value(reply);
   if (value_len && (data[value_len - 1] == '\0'))
      value_len--;

   c = 1;
   for (cp = (char *)data, i = value_len; i > 0; cp++, i--)
     {
        if (*cp == '\0')
           c++;
     }
   v = (char **)malloc((c + 1) * sizeof(char *));
   if (!v)
      return;

   start = (char *)malloc((value_len + 1) * sizeof(char));
   if (!start)
     {
        free(v);
        return;
     }

   memcpy (start, (char *)data, value_len);
   start[value_len] = '\0';
   for (cp = start, i = value_len + 1, j = 0; i > 0; cp++, i--) {
        if (*cp == '\0')
          {
             v[j] = start;
             start = (cp + 1);
             j++;
          }
     }

   if (c < 1)
     {
        free(v);
        return;
     }

   if (argc)
      *argc = c;

   if (argv)
     {
        (*argv) = malloc(c * sizeof(char *));
        if (!*argv)
          {
             free(v);
             if (argc)
                *argc = 0;

             return;
          }

        for (i = 0; i < c; i++)
          {
             if (v[i])
                (*argv)[i] = strdup(v[i]);
             else
                (*argv)[i] = strdup("");
          }
     }

   free(v);
} /* ecore_x_icccm_command_get */

/**
 * Set a window icon name.
 * @param window The window.
 * @param title  The icon name string.
 *
 * Set @p window icon name.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_icon_name_set(Ecore_X_Window window,
                            const char    *title)
{
   /* FIXME: do the UTF8 conversion... */

/* #ifdef X_HAVE_UTF8_STRING */
/*    list[0] = strdup(t); */
/*    ret = Xutf8TextListToTextProperty(_ecore_xcb_conn, list, 1, */
/*				     XUTF8StringStyle, &xprop); */
/* #else */
/*    list[0] = strdup(t); */
/*    ret = XmbTextListToTextProperty(_ecore_xcb_conn, list, 1, */
/*				   XStdICCTextStyle, &xprop); */
/* #endif */

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_WM_ICON_NAME, ECORE_X_ATOM_WM_ICON_NAME,
                       8, strlen(title), title);
} /* ecore_x_icccm_icon_name_set */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_icon_name_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_WM_ICON_NAME,
                                       XCB_GET_PROPERTY_TYPE_ANY,
                                       0L, 128L);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_icon_name_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_icon_name_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_icon_name_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_icon_name_get_fetch */

/**
 * Get a window icon name.
 * @param window The window.
 * @return       The windows icon name string.
 *
 * Return the icon name of @p window. String must be free'd when done with.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_icon_name_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_icon_name_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI char *
ecore_x_icccm_icon_name_get(Ecore_X_Window window __UNUSED__)
{
   xcb_get_property_reply_t *reply;
   char *title = NULL;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return NULL;

   ERR("reply->bytes_afer (should be 0): %d", ((xcb_get_property_reply_t *)reply)->bytes_after);

   if (reply->type == ECORE_X_ATOM_UTF8_STRING)
     {
        int length;

        /* FIXME: verify the value of length */
        length = reply->value_len * reply->format / 8;
        title = (char *)malloc((length + 1) * sizeof(char));
        memcpy(title, xcb_get_property_value(reply), length);
        title[length] = '\0';
     }
   /* not in UTF8, so we convert */
   else
     {
        /* FIXME: do the UTF8... */

        /* convert to utf8 */
/* #ifdef X_HAVE_UTF8_STRING */
/*		  ret = Xutf8TextPropertyToTextList(_ecore_xcb_conn, &xprop, */
/*						    &list, &num); */
/* #else */
/*		  ret = XmbTextPropertyToTextList(_ecore_xcb_conn, &xprop, */
/*						  &list, &num); */
/* #endif */

/*		  if ((ret == XLocaleNotSupported) || */
/*		      (ret == XNoMemory) || (ret == XConverterNotFound)) */
/*		    { */
/*		       t = strdup((char *)xprop.value); */
/*		    } */
/*		  else if (ret >= Success) */
/*		    { */
/*		       if ((num >= 1) && (list)) */
/*			 { */
/*			    t = strdup(list[0]); */
/*			 } */
/*		       if (list) */
/*			 XFreeStringList(list); */
/*		    } */
/*	       } */

/*	     if (xprop.value) XFree(xprop.value); */
     }

   return title;
} /* ecore_x_icccm_icon_name_get */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_colormap_window_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window ? ((xcb_screen_t *)_ecore_xcb_screen)->root : window,
                                       ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
                                       ECORE_X_ATOM_WINDOW,
                                       0L, LONG_MAX);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_colormap_window_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_colormap_window_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_colormap_window_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_colormap_window_get_fetch */

/**
 * Add a subwindow to the list of windows that need a different colormap installed.
 * @param window     The toplevel window
 * @param sub_window The subwindow to be added to the colormap windows list
 *
 * Add @p sub_window to the list of windows that need a different
 * colormap installed.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_colormap_window_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_colormap_window_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_colormap_window_set(Ecore_X_Window window,
                                  Ecore_X_Window sub_window)
{
   void *data = NULL;
   xcb_get_property_reply_t *reply;
   uint32_t num;

   if (window == 0)
      window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   reply = _ecore_xcb_reply_get();
   if (!reply || (reply->format != 32) || (reply->value_len == 0))
     {
        data = calloc(1, sizeof(Ecore_X_Window));
        if (!data)
          {
             if (reply)
                free(reply);

             return;
          }

        num = 1;
     }
   else
     {
        Ecore_X_Window *newset = NULL;
        Ecore_X_Window *oldset = NULL;
        uint32_t i;

        num = reply->value_len;
        data = calloc(num + 1, sizeof(Ecore_X_Window));
        if (!data)
           return;

        newset = (Ecore_X_Window *)data;
        oldset = (Ecore_X_Window *)xcb_get_property_value(reply);
        for (i = 0; i < num; ++i)
          {
             if (oldset[i] == sub_window)
               {
                  free(newset);
                  return;
               }

             newset[i] = oldset[i];
          }

        newset[num++] = sub_window;
     }

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
                       ECORE_X_ATOM_WINDOW,
                       32, num, data);
   free(data);
} /* ecore_x_icccm_colormap_window_set */

/**
 * Remove a window from the list of colormap windows.
 * @param window     The toplevel window
 * @param sub_window The window to be removed from the colormap window list.
 *
 * Remove @p sub_window from the list of colormap windows.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_colormap_window_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_colormap_window_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_colormap_window_unset(Ecore_X_Window window,
                                    Ecore_X_Window sub_window)
{
   void *data = NULL;
   Ecore_X_Window *oldset = NULL;
   Ecore_X_Window *newset = NULL;
   xcb_get_property_reply_t *reply;
   uint32_t num;
   uint32_t i;
   uint32_t j;
   uint32_t k = 0;

   if (window == 0)
      window = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   reply = _ecore_xcb_reply_get();
   if (!reply || (reply->format != 32) || (reply->value_len == 0))
      return;

   num = reply->value_len;
   oldset = (Ecore_X_Window *)xcb_get_property_value(reply);
   for (i = 0; i < num; i++)
     {
        if (oldset[i] == sub_window)
          {
             if (num == 1)
               {
                  xcb_delete_property(_ecore_xcb_conn, window,
                                      ECORE_X_ATOM_WM_COLORMAP_WINDOWS);
                  return;
               }
             else
               {
                  data = calloc(num - 1, sizeof(Ecore_X_Window));
                  newset = (Ecore_X_Window *)data;
                  for (j = 0; j < num; ++j)
                     if (oldset[j] != sub_window)
                        newset[k++] = oldset[j];

                  xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                                      ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
                                      ECORE_X_ATOM_WINDOW,
                                      32, k, data);
                  free(newset);
                  return;
               }
          }
     }
} /* ecore_x_icccm_colormap_window_unset */

/**
 * Specify that a window is transient for another top-level window and should be handled accordingly.
 * @param window    The transient window
 * @param forwindow The toplevel window
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_transient_for_set(Ecore_X_Window window,
                                Ecore_X_Window forwindow)
{
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       ECORE_X_ATOM_WM_TRANSIENT_FOR, ECORE_X_ATOM_WINDOW, 32,
                       1, (void *)&forwindow);
} /* ecore_x_icccm_transient_for_set */

/**
 * Remove the transient_for setting from a window.
 * @param window The window.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_transient_for_unset(Ecore_X_Window window)
{
   xcb_delete_property(_ecore_xcb_conn, window, ECORE_X_ATOM_WM_TRANSIENT_FOR);
} /* ecore_x_icccm_transient_for_unset */

/*
 * Sends the GetProperty request.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_transient_for_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, window,
                                       ECORE_X_ATOM_WM_TRANSIENT_FOR,
                                       ECORE_X_ATOM_WINDOW,
                                       0L, 1L);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_transient_for_get_prefetch */

/*
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_transient_for_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_transient_for_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_transient_for_get_fetch */

/**
 * Get the window this window is transient for, if any.
 * @param window The window to check (Unused).
 * @return       The window ID of the top-level window, or 0 if the property does not exist.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_transient_for_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_transient_for_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI Ecore_X_Window
ecore_x_icccm_transient_for_get(Ecore_X_Window window __UNUSED__)
{
   xcb_get_property_reply_t *reply;
   Ecore_X_Window forwin = 0;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return forwin;

   if ((reply->format != 32) ||
       (reply->value_len == 0) ||
       (reply->type != ECORE_X_ATOM_WINDOW))
      return forwin;

   forwin = *(Ecore_X_Window *)xcb_get_property_value(reply);

   return forwin;
} /* ecore_x_icccm_transient_for_get */

/**
 * Set the window role hint.
 * @param window The window
 * @param role   The role string.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_window_role_set(Ecore_X_Window window,
                              const char    *role)
{
   ecore_x_window_prop_string_set(window, ECORE_X_ATOM_WM_WINDOW_ROLE,
                                  (char *)role);
} /* ecore_x_icccm_window_role_set */

/**
 * Sends the GetProperty request.
 * @param window Window whose properties are requested.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_window_role_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window ? window : ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                       ECORE_X_ATOM_WM_WINDOW_ROLE, XCB_GET_PROPERTY_TYPE_ANY,
                                       0L, 1000000L);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_window_role_get_prefetch */

/**
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_window_role_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_window_role_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_window_role_get_fetch */

/**
 * Get the window role.
 * @param window The window.
 * @return       The window's role string.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_window_role_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_window_role_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI char *
ecore_x_icccm_window_role_get(Ecore_X_Window window)
{
   return ecore_x_window_prop_string_get(window, ECORE_X_ATOM_WM_WINDOW_ROLE);
} /* ecore_x_icccm_window_role_get */

/**
 * Set the window's client leader.
 * @param window The window
 * @param leader The client leader window
 *
 * All non-transient top-level windows created by an app other than
 * the main window must have this property set to the app's main window.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_client_leader_set(Ecore_X_Window window,
                                Ecore_X_Window leader)
{
   ecore_x_window_prop_window_set(window, ECORE_X_ATOM_WM_CLIENT_LEADER,
                                  &leader, 1);
} /* ecore_x_icccm_client_leader_set */

/**
 * Sends the GetProperty request.
 * @param window Window whose properties are requested.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_client_leader_get_prefetch(Ecore_X_Window window)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window,
                                       ECORE_X_ATOM_WM_CLIENT_LEADER,
                                       ECORE_X_ATOM_WINDOW,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_icccm_client_leader_get_prefetch */

/**
 * Gets the reply of the GetProperty request sent by ecore_x_icccm_client_leader_get_prefetch().
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_client_leader_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_icccm_client_leader_get_fetch */

/**
 * Get the window's client leader.
 * @param window The window
 * @return       The window's client leader window, or 0 if unset.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_icccm_client_leader_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_icccm_client_leader_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI Ecore_X_Window
ecore_x_icccm_client_leader_get(Ecore_X_Window window)
{
   Ecore_X_Window leader;

   if (ecore_x_window_prop_window_get(window, ECORE_X_ATOM_WM_CLIENT_LEADER,
                                      &leader, 1) > 0)
      return leader;

   return 0;
} /* ecore_x_icccm_client_leader_get */

/**
 * Send the ClientMessage event with the ChangeState property.
 * @param window The window.
 * @param root   The root window.
 * @ingroup Ecore_X_ICCCM_Group
 */
EAPI void
ecore_x_icccm_iconic_request_send(Ecore_X_Window window,
                                  Ecore_X_Window root)
{
   xcb_client_message_event_t ev;

   if (!window)
      return;

   if (!root)
      root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   /* send_event is bit 7 (0x80) of response_type */
   ev.response_type = XCB_CLIENT_MESSAGE | 0x80;
   ev.format = 32;
   ev.sequence = 0;
   ev.window = window;
   ev.type = ECORE_X_ATOM_WM_CHANGE_STATE;
   ev.data.data32[0] = XCB_WM_STATE_ICONIC;

   xcb_send_event(_ecore_xcb_conn, 0, root,
                  XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                  (const char *)&ev);
} /* ecore_x_icccm_iconic_request_send */

/* FIXME: there are older E hints, gnome hints and mwm hints and new netwm */
/*        hints. each should go in their own file/section so we know which */
/*        is which. also older kde hints too. we should try support as much */
/*        as makese sense to support */
