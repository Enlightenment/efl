/*
 * Copyright © 2011 Kristian Høgsberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define ALL_ACTIONS (WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY | \
                     WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE | \
                     WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK)

static uint32_t
data_offer_choose_action(Comp_Data_Device_Offer *offer)
{
   uint32_t available_actions, preferred_action = 0;
   uint32_t source_actions, offer_actions;

   if (wl_resource_get_version(offer->res) >=
       WL_DATA_OFFER_ACTION_SINCE_VERSION)
     {
        offer_actions = offer->dnd_actions;
        preferred_action = offer->preferred_dnd_action;
     }
   else
     {
        offer_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
     }

   if (wl_resource_get_version(offer->source->res) >=
       WL_DATA_SOURCE_ACTION_SINCE_VERSION)
     source_actions = offer->source->dnd_actions;
   else
     source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;

   available_actions = offer_actions & source_actions;

   if (!available_actions)
     return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;

   if (offer->source->seat &&
       offer->source->compositor_action & available_actions)
     return offer->source->compositor_action;

   /* If the dest side has a preferred DnD action, use it */
   if ((preferred_action & available_actions) != 0)
     return preferred_action;

   /* Use the first found action, in bit order */
   return 1 << (ffs(available_actions) - 1);
}

static void
data_offer_update_action(Comp_Data_Device_Offer *offer)
{
   uint32_t action;

   if (!offer->source)
     return;

   action = data_offer_choose_action(offer);

   if (offer->source->current_dnd_action == action)
     return;

   offer->source->current_dnd_action = action;

   if (offer->in_ask)
     return;

   if (wl_resource_get_version(offer->source->res) >=
       WL_DATA_SOURCE_ACTION_SINCE_VERSION)
     wl_data_source_send_action(offer->source->res, action);

   if (wl_resource_get_version(offer->res) >=
       WL_DATA_OFFER_ACTION_SINCE_VERSION)
     wl_data_offer_send_action(offer->res, action);
}

static void
data_device_offer_set_actions(struct wl_client *client,
                              struct wl_resource *resource,
                              uint32_t dnd_actions, uint32_t preferred_action)
{
   Comp_Data_Device_Offer *offer = wl_resource_get_user_data(resource);

   if (dnd_actions & ~ALL_ACTIONS)
     {
        wl_resource_post_error(offer->res,
                               WL_DATA_OFFER_ERROR_INVALID_ACTION_MASK,
                               "invalid action mask %x", dnd_actions);
        return;
     }

   if (preferred_action &&
       (!(preferred_action & dnd_actions) ||
        __builtin_popcount(preferred_action) > 1))
     {
        wl_resource_post_error(offer->res,
                               WL_DATA_OFFER_ERROR_INVALID_ACTION,
                               "invalid action %x", preferred_action);
        return;
     }

   offer->dnd_actions = dnd_actions;
   offer->preferred_dnd_action = preferred_action;
   data_offer_update_action(offer);
}

#ifdef HAVE_ECORE_X
static Ecore_X_Atom
action_convert(uint32_t action)
{
   if (action == WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE)
     return ECORE_X_ATOM_XDND_ACTION_MOVE;
   if (action == WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK)
     return ECORE_X_ATOM_XDND_ACTION_ASK;
   return ECORE_X_ATOM_XDND_ACTION_COPY;
}
#endif
static void
data_device_offer_accept(struct wl_client *client, struct wl_resource *resource,
                         uint32_t serial, const char *mime_type)
{
   Comp_Data_Device_Offer *offer = wl_resource_get_user_data(resource);
   Comp_Surface *cs;

   /* Protect against untimely calls from older data offers */
   if (!offer->source || offer != offer->source->offer)
     return;

   switch (offer->type)
     {
      case COMP_DATA_DEVICE_OFFER_TYPE_DND:
        cs = offer->source->seat->drag.enter;
        if (!offer->source->seat->drag.res) return;
        if ((!offer->source->seat->drag.source) &&
          (wl_resource_get_client(cs->res) != wl_resource_get_client(offer->source->seat->drag.res)))
          return;
#ifdef HAVE_ECORE_X
        if (offer->source->x11_owner)
          {
             Ecore_Window win = ecore_evas_window_get(ecore_evas_ecore_evas_get(offer->source->seat->c->evas));
             offer->source->accepted = mime_type != NULL;
             ecore_x_client_message32_send(offer->source->x11_owner,
               ECORE_X_ATOM_XDND_STATUS, ECORE_X_EVENT_MASK_NONE,
               win, 2 | !!mime_type, 0, 0,
               (!!mime_type) * action_convert(offer->source->current_dnd_action));
             return;
          }
#endif
        break;

      case COMP_DATA_DEVICE_OFFER_TYPE_CLIPBOARD:
        break;
      default: return;
     }
   if (offer->source->seat->client_offer)
     ecore_wl2_offer_accept(offer->source->seat->client_offer, mime_type);
   else
     wl_data_source_send_target(offer->source->res, mime_type);
   offer->source->accepted = mime_type != NULL;
}

static void
data_device_offer_receive(struct wl_client *client, struct wl_resource *resource,
                          const char *mime_type, int32_t fd)
{
   Comp_Data_Device_Offer *offer = wl_resource_get_user_data(resource);

   if (offer->source && offer == offer->source->offer)
     {
        if (offer->proxy)
          {
             Ecore_Wl2_Offer *off;
#ifdef HAVE_ECORE_X
             if (offer->source->x11_owner)
               {
                  x11_send_send(offer->source, mime_type, fd, offer->type);
                  return;
               }
#endif
             if (offer->type == COMP_DATA_DEVICE_OFFER_TYPE_CLIPBOARD)
               off = ecore_wl2_dnd_selection_get(offer->source->seat->seat);
             else
               {
                  off = offer->source->seat->client_offer;
                  offer->source->seat->client_offer = NULL;
               }
             ecore_wl2_offer_proxy_receive(off, mime_type, fd);
             offer->proxy_offer = off;
          }
        else
          wl_data_source_send_send(offer->source->res, mime_type, fd);
     }
   close(fd);
}

static void
data_source_notify_finish(Comp_Data_Device_Source *source)
{
   if (!source->actions_set)
     return;

   if (source->proxy && (!source->x11_owner))
     ecore_wl2_offer_finish(source->offer->proxy_offer);

   if (source->offer && source->offer->in_ask &&
       wl_resource_get_version(source->res) >=
       WL_DATA_SOURCE_ACTION_SINCE_VERSION)
     {
        wl_data_source_send_action(source->res,
                                   source->current_dnd_action);
     }

   if (wl_resource_get_version(source->res) >=
       WL_DATA_SOURCE_DND_FINISHED_SINCE_VERSION)
     {
        wl_data_source_send_dnd_finished(source->res);
     }

   source->offer = NULL;
}

static void
data_device_offer_finish(struct wl_client *client, struct wl_resource *resource)
{
   Comp_Data_Device_Offer *offer = wl_resource_get_user_data(resource);

   if (!offer->source || offer->source->offer != offer)
     return;

   /* Disallow finish while we have a grab driving drag-and-drop, or
    * if the negotiation is not at the right stage
    */
   if (((!offer->proxy) && offer->source->seat) ||
       !offer->source->accepted)
     {
        wl_resource_post_error(offer->res,
                               WL_DATA_OFFER_ERROR_INVALID_FINISH,
                               "premature finish request");
        return;
     }

   switch (offer->source->current_dnd_action)
     {
      case WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE:
      case WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK:
        wl_resource_post_error(offer->res,
                               WL_DATA_OFFER_ERROR_INVALID_OFFER,
                               "offer finished with an invalid action");
        return;

      default:
        break;
     }

   data_source_notify_finish(offer->source);
}

static void
data_device_offer_impl_destroy(struct wl_resource *resource)
{
   Comp_Data_Device_Offer *offer = wl_resource_get_user_data(resource);

   if (!offer->source)
     goto out;

   if (offer->source->offer != offer)
     goto out;

   if (offer->type == COMP_DATA_DEVICE_OFFER_TYPE_DND)
     {
        /* If the drag destination has version < 3, wl_data_offer.finish
         * won't be called, so do this here as a safety net, because
         * we still want the version >=3 drag source to be happy.
         */
        if (wl_resource_get_version(offer->res) <
            WL_DATA_OFFER_ACTION_SINCE_VERSION)
          {
             data_source_notify_finish(offer->source);
          }
        else if (offer->source->res &&
                 wl_resource_get_version(offer->source->res) >=
                 WL_DATA_SOURCE_DND_FINISHED_SINCE_VERSION)
          {
             wl_data_source_send_cancelled(offer->source->res);
          }
     }

   offer->source->offer = NULL;
   if (offer->proxy_offer && offer->proxy)
     ecore_wl2_offer_proxy_receive_end(offer->proxy_offer);
out:
   free(offer);
}

static void
drag_grab_button(Comp_Seat *s,
                 uint32_t time, uint32_t button, uint32_t state_w)
{
   Comp_Data_Device_Source *data_source = s->drag.source;
   enum wl_pointer_button_state state = state_w;
   struct wl_resource *res;

   if (data_source &&
       s->drag.id == button &&
       state == WL_POINTER_BUTTON_STATE_RELEASED)
     {
        if ((s->drag.enter || (s->drag.x11_owner == ecore_evas_window_get(ecore_evas_ecore_evas_get(s->c->evas)))) &&
            data_source->accepted &&
            data_source->current_dnd_action)
          {
             if (s->drag.enter)
               {
                  res = data_device_find(s, s->drag.enter->res);
                  if (!res) return;

                  wl_data_device_send_drop(res);
               }
             if (wl_resource_get_version(data_source->res) >=
                 WL_DATA_SOURCE_DND_DROP_PERFORMED_SINCE_VERSION)
               wl_data_source_send_dnd_drop_performed(data_source->res);

             if (data_source->offer)
               data_source->offer->in_ask =
                 data_source->current_dnd_action ==
                 WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;

             if (!data_source->proxy)
               data_source->seat = NULL;
          }
        else if (wl_resource_get_version(data_source->res) >=
                 WL_DATA_SOURCE_DND_FINISHED_SINCE_VERSION)
          {
             wl_data_source_send_cancelled(data_source->res);
          }
        seat_drag_end(s);
        if (!data_source->x11_owner)
          s->drag.source = NULL;
#ifdef HAVE_ECORE_X
        if (ecore_x_display_get())
          ecore_x_pointer_ungrab();
#endif
     }
}

#ifdef HAVE_ECORE_X
static xkb_mod_index_t x11_kbd_shift_mod;
static xkb_mod_index_t x11_kbd_caps_mod;
static xkb_mod_index_t x11_kbd_ctrl_mod;
static xkb_mod_index_t x11_kbd_alt_mod;
static xkb_mod_index_t x11_kbd_mod2_mod;
static xkb_mod_index_t x11_kbd_mod3_mod;
static xkb_mod_index_t x11_kbd_super_mod;
static xkb_mod_index_t x11_kbd_mod5_mod;

static void
keymap_mods_init(struct xkb_keymap *keymap)
{
   x11_kbd_shift_mod = xkb_keymap_mod_get_index(keymap,  XKB_MOD_NAME_SHIFT);
   x11_kbd_caps_mod = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CAPS);
   x11_kbd_ctrl_mod = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CTRL);
   x11_kbd_alt_mod = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_ALT);
   x11_kbd_mod2_mod = xkb_keymap_mod_get_index(keymap, "Mod2");
   x11_kbd_mod3_mod = xkb_keymap_mod_get_index(keymap, "Mod3");
   x11_kbd_super_mod = xkb_keymap_mod_get_index(keymap,  XKB_MOD_NAME_LOGO);
   x11_kbd_mod5_mod = xkb_keymap_mod_get_index(keymap, "Mod5");
}

static uint32_t
get_xkb_mod_mask(uint32_t in)
{
	uint32_t ret = 0;

	if ((in & ECORE_X_MODIFIER_SHIFT) && x11_kbd_shift_mod != XKB_MOD_INVALID)
		ret |= (1 << x11_kbd_shift_mod);
	if ((in & ECORE_X_LOCK_CAPS) && x11_kbd_caps_mod != XKB_MOD_INVALID)
		ret |= (1 << x11_kbd_caps_mod);
	if ((in & ECORE_X_MODIFIER_CTRL) && x11_kbd_ctrl_mod != XKB_MOD_INVALID)
		ret |= (1 << x11_kbd_ctrl_mod);
	if ((in & ECORE_X_MODIFIER_ALT) && x11_kbd_alt_mod != XKB_MOD_INVALID)
		ret |= (1 << x11_kbd_alt_mod);
	if ((in & ECORE_X_LOCK_NUM) && x11_kbd_mod2_mod != XKB_MOD_INVALID)
		ret |= (1 << x11_kbd_mod2_mod);
	if ((in & ECORE_X_LOCK_SCROLL) && x11_kbd_mod3_mod != XKB_MOD_INVALID)
		ret |= (1 << x11_kbd_mod3_mod);
	if ((in & ECORE_X_MODIFIER_WIN) && x11_kbd_super_mod != XKB_MOD_INVALID)
		ret |= (1 << x11_kbd_super_mod);
	if ((in & ECORE_X_MODIFIER_ALTGR) && x11_kbd_mod5_mod != XKB_MOD_INVALID)
		ret |= (1 << x11_kbd_mod5_mod);

	return ret;
}

#endif
