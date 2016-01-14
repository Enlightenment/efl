#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

static void
_seat_cb_unbind(struct wl_resource *resource)
{
   Ecore_Wl2_Seat *seat;

   DBG("Seat Unbind");

   seat = wl_resource_get_user_data(resource);
   if (!seat) return;

   seat->resources = eina_list_remove(seat->resources, resource);

   if (seat->unbind_cb) seat->unbind_cb(resource);
}

static void
_seat_cb_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   Ecore_Wl2_Seat *seat;
   struct wl_resource *res;

   seat = data;
   seat->id = id;

   DBG("Seat Bind");

   res = wl_resource_create(client, &wl_seat_interface, version, id);
   if (!res)
     {
        ERR("Failed to create seat resource");
        return;
     }

   seat->resources = eina_list_append(seat->resources, res);

   wl_resource_set_implementation(res, seat->implementation, seat,
                                  _seat_cb_unbind);

   if (version >= WL_SEAT_NAME_SINCE_VERSION)
     wl_seat_send_name(res, seat->name);

   if (seat->bind_cb) seat->bind_cb(client, seat, version, id);
}

static void
_pointer_cb_unbind(struct wl_resource *resource)
{
   Ecore_Wl2_Pointer *ptr;

   DBG("Pointer Unbind");

   ptr = wl_resource_get_user_data(resource);
   if (!ptr) return;

   ptr->resources = eina_list_remove(ptr->resources, resource);

   /* wl_pointer_release(); */
}

static Ecore_Wl2_Pointer *
_ecore_wl2_seat_pointer_create(Ecore_Wl2_Seat *seat)
{
   Ecore_Wl2_Pointer *ptr;

   ptr = calloc(1, sizeof(Ecore_Wl2_Pointer));
   if (!ptr) return NULL;

   /* FIXME: Init pointer fields */

   ptr->seat = seat;

   return ptr;
}

static void
_ecore_wl2_seat_pointer_destroy(Ecore_Wl2_Pointer *ptr)
{
   /* FIXME: Free pointer fields */
   free(ptr);
}

static void
_keyboard_cb_unbind(struct wl_resource *resource)
{
   Ecore_Wl2_Keyboard *kbd;

   DBG("Keyboard Unbind");

   kbd = wl_resource_get_user_data(resource);
   if (!kbd) return;

   kbd->resources = eina_list_remove(kbd->resources, resource);

   /* wl_keyboard_release(); */
}

static Ecore_Wl2_Keyboard *
_ecore_wl2_seat_keyboard_create(Ecore_Wl2_Seat *seat)
{
   Ecore_Wl2_Keyboard *kbd;

   kbd = calloc(1, sizeof(Ecore_Wl2_Keyboard));
   if (!kbd) return NULL;

   /* FIXME: Init keyboard fields */

   kbd->seat = seat;

   return kbd;
}

static void
_ecore_wl2_seat_keyboard_destroy(Ecore_Wl2_Keyboard *kbd)
{
   /* FIXME: Free keyboard fields */
   free(kbd);
}

EAPI Ecore_Wl2_Seat *
ecore_wl2_seat_create(Ecore_Wl2_Display *display, const char *name, const struct wl_seat_interface *implementation, int version, Ecore_Wl2_Bind_Cb bind_cb, Ecore_Wl2_Unbind_Cb unbind_cb)
{
   Ecore_Wl2_Seat *seat;

   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);

   if (!name) name = "default";

   EINA_INLIST_FOREACH(display->seats, seat)
     {
        if (!strcmp(seat->name, name))
          return seat;
     }

   seat = calloc(1, sizeof(Ecore_Wl2_Seat));
   if (!seat) return NULL;

   seat->version = version;
   seat->implementation = implementation;
   seat->bind_cb = bind_cb;
   seat->unbind_cb = unbind_cb;

   eina_stringshare_replace(&seat->name, name);

   seat->global =
     wl_global_create(display->wl.display, &wl_seat_interface, seat->version,
                      seat, _seat_cb_bind);
   if (!seat->global)
     {
        ERR("Could not create seat global");
        free(seat);
        return NULL;
     }

   return seat;
}

EAPI void
ecore_wl2_seat_destroy(Ecore_Wl2_Seat *seat)
{
   EINA_SAFETY_ON_NULL_RETURN(seat);

   eina_stringshare_del(seat->name);

   if (seat->pointer) _ecore_wl2_seat_pointer_destroy(seat->pointer);
   if (seat->keyboard) _ecore_wl2_seat_keyboard_destroy(seat->keyboard);

   /* NB: Hmmm, should we iterate and free resources here ?? */

   wl_global_destroy(seat->global);

   free(seat);
}

EAPI void
ecore_wl2_seat_capabilities_send(Ecore_Wl2_Seat *seat, enum wl_seat_capability caps)
{
   Eina_List *l;
   struct wl_resource *res;

   EINA_SAFETY_ON_NULL_RETURN(seat);

   EINA_LIST_FOREACH(seat->resources, l, res)
     wl_seat_send_capabilities(res, caps);
}

EAPI void
ecore_wl2_seat_pointer_release(Ecore_Wl2_Seat *seat)
{
   enum wl_seat_capability caps = 0;

   EINA_SAFETY_ON_NULL_RETURN(seat);

   seat->pointer_count--;
   if (seat->pointer_count == 0)
     {
        if (seat->pointer_count > 0)
          caps |= WL_SEAT_CAPABILITY_POINTER;
        if (seat->keyboard_count > 0)
          caps |= WL_SEAT_CAPABILITY_KEYBOARD;
        if (seat->touch_count > 0)
          caps |= WL_SEAT_CAPABILITY_TOUCH;

        ecore_wl2_seat_capabilities_send(seat, caps);
     }
}

EAPI Ecore_Wl2_Pointer *
ecore_wl2_pointer_get(Ecore_Wl2_Seat *seat)
{
   enum wl_seat_capability caps = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);

   if (seat->pointer_count > 0)
     caps |= WL_SEAT_CAPABILITY_POINTER;
   if (seat->keyboard_count > 0)
     caps |= WL_SEAT_CAPABILITY_KEYBOARD;
   if (seat->touch_count > 0)
     caps |= WL_SEAT_CAPABILITY_TOUCH;

   if (seat->pointer)
     {
        seat->pointer_count += 1;
        if (seat->pointer_count == 1)
          {
             caps |= WL_SEAT_CAPABILITY_POINTER;
             ecore_wl2_seat_capabilities_send(seat, caps);
          }

        return seat->pointer;
     }

   seat->pointer = _ecore_wl2_seat_pointer_create(seat);
   seat->pointer_count = 1;

   caps |= WL_SEAT_CAPABILITY_POINTER;
   ecore_wl2_seat_capabilities_send(seat, caps);

   return seat->pointer;
}

EAPI Eina_Bool
ecore_wl2_pointer_resource_create(Ecore_Wl2_Pointer *ptr, struct wl_client *client, const struct wl_pointer_interface *implementation, int version, uint32_t id)
{
   struct wl_resource *res;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ptr, EINA_FALSE);

   res = wl_resource_create(client, &wl_pointer_interface, version, id);
   if (!res)
     {
        ERR("Could not create pointer resource");
        wl_client_post_no_memory(client);
        return EINA_FALSE;
     }

   wl_resource_set_implementation(res, implementation, ptr, _pointer_cb_unbind);

   ptr->resources = eina_list_append(ptr->resources, res);

   /* FIXME: Hmmm, should we sent a pointer_enter to ptr->focus'd surface
    * here like weston does ? */

   return EINA_TRUE;
}

EAPI Ecore_Wl2_Keyboard *
ecore_wl2_keyboard_get(Ecore_Wl2_Seat *seat)
{
   enum wl_seat_capability caps = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);

   if (seat->pointer_count > 0)
     caps |= WL_SEAT_CAPABILITY_POINTER;
   if (seat->keyboard_count > 0)
     caps |= WL_SEAT_CAPABILITY_KEYBOARD;
   if (seat->touch_count > 0)
     caps |= WL_SEAT_CAPABILITY_TOUCH;

   if (seat->keyboard)
     {
        seat->keyboard_count += 1;
        if (seat->keyboard_count == 1)
          {
             caps |= WL_SEAT_CAPABILITY_KEYBOARD;
             ecore_wl2_seat_capabilities_send(seat, caps);
          }

        return seat->keyboard;
     }

   seat->keyboard = _ecore_wl2_seat_keyboard_create(seat);
   seat->keyboard_count = 1;

   caps |= WL_SEAT_CAPABILITY_KEYBOARD;
   ecore_wl2_seat_capabilities_send(seat, caps);

   return seat->keyboard;
}

EAPI Eina_Bool
ecore_wl2_keyboard_resource_create(Ecore_Wl2_Keyboard *kbd, struct wl_client *client, const struct wl_keyboard_interface *implementation, int version, uint32_t id)
{
   struct wl_resource *res;

   EINA_SAFETY_ON_NULL_RETURN_VAL(kbd, EINA_FALSE);

   res = wl_resource_create(client, &wl_keyboard_interface, version, id);
   if (!res)
     {
        ERR("Could not create keyboard resource");
        wl_client_post_no_memory(client);
        return EINA_FALSE;
     }

   wl_resource_set_implementation(res, implementation, kbd, _keyboard_cb_unbind);

   kbd->resources = eina_list_append(kbd->resources, res);

   /* FIXME: Hmmm, should we sent a keyboard_enter to kbd->focus'd surface
    * here like weston does ? */

   return EINA_TRUE;
}

EAPI void
ecore_wl2_keyboard_repeat_info_set(Ecore_Wl2_Keyboard *kbd, double rate, double delay)
{
   struct wl_resource *res;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(kbd);

   kbd->repeat.rate = rate;
   kbd->repeat.delay = delay;

   EINA_LIST_FOREACH(kbd->resources, l, res)
     {
        if (wl_resource_get_version(res) >=
            WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
          wl_keyboard_send_repeat_info(res, rate, delay);
     }
}
