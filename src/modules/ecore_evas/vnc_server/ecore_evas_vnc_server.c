#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <rfb/rfb.h>
#include <rfb/rfbregion.h>
#include <rfb/keysym.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Input.h>
#include <Evas.h>
#include <Ecore_Evas.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "ecore_private.h"
#include "ecore_evas_private.h"
#include "ecore_evas_vnc_server_fb_keymap.h"

static int _ecore_evas_vnc_server_log_dom;
static unsigned int _available_seat = 1;

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif /* ! DLL_EXPORT */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_evas_vnc_server_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_evas_vnc_server_log_dom, __VA_ARGS__)

typedef Eina_Bool (*Ecore_Evas_Vnc_Key_Info_Get)(rfbKeySym key,
                                                 const char **key_name,
                                                 const char **key_str,
                                                 const char **compose,
                                                 int *keycode);

typedef struct _Ecore_Evas_Vnc_Server {
   char *frame_buffer;
   rfbScreenInfoPtr vnc_screen;
   Ecore_Fd_Handler *vnc_listen_handler;
   Ecore_Fd_Handler *vnc_listen6_handler;
   Ecore_Evas_Vnc_Client_Accept_Cb accept_cb;
   Ecore_Evas_Vnc_Client_Disconnected_Cb disc_cb;
   void *cb_data;
   Evas_Object *snapshot;
   Ecore_Evas *ee;
   Eina_Tiler *t;
   Ecore_Evas_Vnc_Key_Info_Get key_info_get_func;
   double double_click_time;
   int last_w;
   int last_h;
} Ecore_Evas_Vnc_Server;

typedef struct _Ecore_Evas_Vnc_Server_Client_Data {
   Ecore_Fd_Handler *handler;
   Evas_Device *keyboard;
   Evas_Device *mouse;
   Evas_Device *seat;
   unsigned int key_modifiers;
   time_t last_mouse_button_down;
   Eina_Bool double_click;
   Eina_Bool triple_click;
} Ecore_Evas_Vnc_Server_Client_Data;

#define VNC_BITS_PER_SAMPLE (8)
#define VNC_SAMPLES_PER_PIXEL (3)
#define VNC_BYTES_PER_PIXEL (4)


static void _ecore_evas_vnc_server_ecore_event_generic_free(void *user_data,
                                                            void *func_data);

static void
_ecore_evas_vnc_server_update_clients(rfbScreenInfoPtr vnc_screen)
{
   rfbClientIteratorPtr itr;
   rfbClientRec *client;

   itr = rfbGetClientIterator(vnc_screen);

   //No clients.
   if (!itr) return;

   while ((client = rfbClientIteratorNext(itr))) {
      rfbBool r;

      r = rfbUpdateClient(client);

      if (!r)
        {
           Ecore_Evas_Vnc_Server_Client_Data *cdata = client->clientData;

           WRN("Could not update the VNC client on seat '%s'\n",
               evas_device_name_get(cdata->seat));
        }

      //Client disconnected
      if (client->sock == -1) rfbClientConnectionGone(client);
   }

   rfbReleaseClientIterator(itr);
}

static void
_ecore_evas_vnc_server_format_setup(rfbScreenInfoPtr vnc_screen)
{
   int aux;

   //FIXME: Using BGR - Is there a better way to do this?
   aux = vnc_screen->serverFormat.redShift;
   vnc_screen->serverFormat.redShift = vnc_screen->serverFormat.blueShift;
   vnc_screen->serverFormat.blueShift = aux;
}

static Eina_Bool
_ecore_evas_vnc_server_socket_listen_activity(void *data,
                                              Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   rfbProcessNewConnection(data);
   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_evas_vnc_server_mouse_inout_emit(Ecore_Evas_Vnc_Server *server,
                                        Evas_Device *dev, int event_type)
{
   Ecore_Event_Mouse_IO *io = calloc(1, sizeof(Ecore_Event_Mouse_IO));
   EINA_SAFETY_ON_NULL_RETURN(io);
   io->timestamp = time(NULL);
   io->dev = efl_ref(dev);
   io->event_window = io->window = server->ee->prop.window;
   ecore_event_add(event_type, io,
                   _ecore_evas_vnc_server_ecore_event_generic_free, dev);
}

static void
_ecore_evas_vnc_server_client_gone(rfbClientRec *client)
{
   Ecore_Evas_Vnc_Server *server = client->screen->screenData;
   Ecore_Evas_Vnc_Server_Client_Data *cdata = client->clientData;

   DBG("VNC client on seat '%s' gone", evas_device_name_get(cdata->seat));

   if (server->disc_cb)
     server->disc_cb(server->cb_data, server->ee, client->host);
   _ecore_evas_vnc_server_mouse_inout_emit(server, cdata->mouse,
                                           ECORE_EVENT_MOUSE_OUT);
   _ecore_evas_mouse_inout_set(server->ee, cdata->mouse, EINA_FALSE,
                               EINA_FALSE);
   ecore_evas_focus_device_set(server->ee, cdata->seat, EINA_FALSE);
   ecore_main_fd_handler_del(cdata->handler);
   evas_device_del(cdata->keyboard);
   evas_device_del(cdata->mouse);
   evas_device_del(cdata->seat);
   free(cdata);
   _available_seat--;
}

static Eina_Bool
_ecore_evas_vnc_server_client_activity(void *data,
                                       Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   rfbClientRec *client = data;
   rfbScreenInfoPtr screen = client->screen;

   rfbProcessClientMessage(client);

   //macro from rfb.h
   if (screen->frameBuffer && FB_UPDATE_PENDING(client))
     rfbSendFramebufferUpdate(client, client->modifiedRegion);

   //Client disconnected.
   if (client->sock == -1)
     {
        rfbClientConnectionGone(client);
        return ECORE_CALLBACK_DONE;
     }

   return ECORE_CALLBACK_RENEW;
}

static enum rfbNewClientAction
_ecore_evas_vnc_server_client_connection_new(rfbClientRec *client)
{
   Ecore_Evas_Vnc_Server *server;
   Ecore_Evas_Vnc_Server_Client_Data *cdata;
   char buf[32];

   EINA_SAFETY_ON_TRUE_RETURN_VAL(_available_seat == UINT_MAX,
                                  RFB_CLIENT_REFUSE);

   server = client->screen->screenData;

   if (server->accept_cb && !server->accept_cb(server->cb_data,
                                               server->ee, client->host))
     return RFB_CLIENT_REFUSE;

   cdata = calloc(1, sizeof(Ecore_Evas_Vnc_Server_Client_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(cdata, RFB_CLIENT_REFUSE);

   cdata->handler = ecore_main_fd_handler_add(client->sock, ECORE_FD_READ,
                                              _ecore_evas_vnc_server_client_activity,
                                              client, NULL, NULL);
   EINA_SAFETY_ON_NULL_GOTO(cdata->handler, err_handler);

   snprintf(buf, sizeof(buf), "seat-%u", _available_seat);

   cdata->seat = evas_device_add_full(server->ee->evas, buf,
                                      "A remote VNC seat",
                                      NULL, NULL, EVAS_DEVICE_CLASS_SEAT,
                                      EVAS_DEVICE_SUBCLASS_NONE);
   EINA_SAFETY_ON_NULL_GOTO(cdata->seat, err_handler);
   evas_device_seat_id_set(cdata->seat, 2);
   cdata->keyboard = evas_device_add_full(server->ee->evas, "Keyboard",
                                          "A remote VNC keyboard",
                                          cdata->seat, NULL,
                                          EVAS_DEVICE_CLASS_KEYBOARD,
                                          EVAS_DEVICE_SUBCLASS_NONE);
   EINA_SAFETY_ON_NULL_GOTO(cdata->keyboard, err_dev);
   cdata->mouse = evas_device_add_full(server->ee->evas, "Mouse",
                                       "A remote VNC mouse",
                                       cdata->seat, NULL,
                                       EVAS_DEVICE_CLASS_MOUSE,
                                       EVAS_DEVICE_SUBCLASS_NONE);
   EINA_SAFETY_ON_NULL_GOTO(cdata->mouse, err_mouse);
   client->clientGoneHook = _ecore_evas_vnc_server_client_gone;
   client->clientData = cdata;

   DBG("New VNC client on seat '%u'", _available_seat);
   _available_seat++;
   _ecore_evas_vnc_server_mouse_inout_emit(server, cdata->mouse,
                                           ECORE_EVENT_MOUSE_IN);
   _ecore_evas_mouse_inout_set(server->ee, cdata->mouse, EINA_TRUE,
                               EINA_FALSE);
   ecore_evas_focus_device_set(server->ee, cdata->seat, EINA_TRUE);
   return RFB_CLIENT_ACCEPT;

 err_mouse:
   evas_device_del(cdata->keyboard);
 err_dev:
   evas_device_del(cdata->seat);
 err_handler:
   free(cdata);
   return RFB_CLIENT_REFUSE;
}

static unsigned int
_ecore_evas_vnc_server_modifier_to_ecore_modifier(int mod, Eina_Bool *is_lock)
{
   *is_lock = EINA_FALSE;
   if (mod == XK_Shift_L || mod == XK_Shift_R)
     return ECORE_EVENT_MODIFIER_SHIFT;
   if (mod == XK_Control_L || mod == XK_Control_R)
     return ECORE_EVENT_MODIFIER_CTRL;
   if (mod == XK_Alt_L || mod == XK_Alt_R)
     return ECORE_EVENT_MODIFIER_ALT;
   if (mod == XK_Super_L || mod == XK_Super_R)
     return ECORE_EVENT_MODIFIER_WIN;
   if (mod == XK_Scroll_Lock)
     {
        *is_lock = EINA_TRUE;
        return ECORE_EVENT_LOCK_SCROLL;
     }
   if (mod == XK_Num_Lock)
     {
        *is_lock = EINA_TRUE;
        return ECORE_EVENT_LOCK_NUM;
     }
   if (mod == XK_Caps_Lock)
     {
        *is_lock = EINA_TRUE;
        return ECORE_EVENT_LOCK_CAPS;
     }
   if (mod == XK_Shift_Lock)
     {
        *is_lock = EINA_TRUE;
        return ECORE_EVENT_LOCK_SHIFT;
     }
   return 0;
}

static void
_ecore_evas_vnc_server_ecore_event_generic_free(void *user_data,
                                                void *func_data)
{
   efl_unref(user_data);
   free(func_data);
}

static Eina_Bool
_ecore_evas_vnc_server_fb_key_info_get(rfbKeySym key,
                                       const char **key_name,
                                       const char **key_str,
                                       const char **compose,
                                       int *keycode EINA_UNUSED)
{
   return ecore_evas_vnc_server_keysym_to_fb_translate(key,
                                                       key_name, key_str,
                                                       compose);
}

static void
_ecore_evas_vnc_server_client_keyboard_event(rfbBool down,
                                             rfbKeySym key,
                                             rfbClientRec *client)
{
   Ecore_Event_Key *e;
   Ecore_Evas_Vnc_Server_Client_Data *cdata = client->clientData;
   rfbScreenInfoPtr screen = client->screen;
   Ecore_Evas_Vnc_Server *server = screen->screenData;
   const char *key_str, *compose, *key_name;
   int keycode = 0;
   Eina_Bool r;
   char buf[10];

   if (key >= XK_Shift_L && key <= XK_Hyper_R)
     {
        Eina_Bool is_lock;
        int mod = _ecore_evas_vnc_server_modifier_to_ecore_modifier(key,
                                                                    &is_lock);

        if (down)
          {
             if (!is_lock || !(cdata->key_modifiers & mod))
               cdata->key_modifiers |= mod;
             else
               cdata->key_modifiers &= ~mod;
          }
        else if (!is_lock)
          cdata->key_modifiers &= ~mod;
     }

   if (server->ee->ignore_events)
     return;

   r = server->key_info_get_func(key, &key_str, &key_name,
                                 &compose, &keycode);
   EINA_SAFETY_ON_FALSE_RETURN(r);

   snprintf(buf, sizeof(buf), "%lc", key);

   e = calloc(1, sizeof(Ecore_Event_Key) + (compose ? 0 : strlen(buf) + 1));
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->timestamp = (unsigned int)time(NULL);
   e->modifiers = cdata->key_modifiers;
   e->same_screen = 1;
   e->window = e->root_window = e->event_window =
     server->ee->prop.window;
   e->dev = cdata->keyboard;
   efl_ref(cdata->keyboard);
   e->keyname = key_name;
   e->key = key_str;
   e->keycode = keycode;
   e->compose = (char *)(e + 1);
   if (compose)
     e->compose = compose;
   else
     strcpy((char *)e->compose, buf);
   e->string = e->compose;

   ecore_event_add(down ? ECORE_EVENT_KEY_DOWN : ECORE_EVENT_KEY_UP,
                   e, _ecore_evas_vnc_server_ecore_event_generic_free,
                   cdata->keyboard);
}

static int
_ecore_evas_vnc_server_pointer_button_get(int mask)
{
    int i;
    for (i = 0; i < 32; i++)
        if (mask >> i & 1)
            return i + 1;
    return 0;
}

static void
_ecore_evas_vnc_server_client_pointer_event(int button_mask,
                                            int x, int y,
                                            rfbClientPtr client)
{
   Ecore_Evas_Vnc_Server_Client_Data *cdata = client->clientData;
   rfbScreenInfoPtr screen = client->screen;
   Ecore_Evas_Vnc_Server *server = screen->screenData;
   Ecore_Event_Mouse_Move *move_event;
   Ecore_Event_Mouse_Button *button_event;
   Ecore_Event_Mouse_Wheel *wheel_event;
   int button_changed, button, event, z = 0, direction = 0;
   time_t now = time(NULL);

   if (server->ee->ignore_events)
     return;

   if (client->lastPtrX != x || client->lastPtrY != y)
     {
        move_event = calloc(1, sizeof(Ecore_Event_Mouse_Move));
        EINA_SAFETY_ON_NULL_RETURN(move_event);

        move_event->x = move_event->multi.x = x;
        move_event->y = move_event->multi.y = y;
        move_event->same_screen = 1;
        move_event->timestamp = (unsigned int)now;
        move_event->window = move_event->event_window =
          move_event->root_window = server->ee->prop.window;
        move_event->multi.pressure = 1.0;
        move_event->modifiers = cdata->key_modifiers;
        move_event->dev = cdata->mouse;
        efl_ref(cdata->mouse);
        ecore_event_add(ECORE_EVENT_MOUSE_MOVE, move_event,
                        _ecore_evas_vnc_server_ecore_event_generic_free,
                        cdata->mouse);
        client->lastPtrX = x;
        client->lastPtrY = y;
     }

   button_changed = button_mask - client->lastPtrButtons;

   if (button_changed > 0)
     {
        button = _ecore_evas_vnc_server_pointer_button_get(button_changed);

        switch (button)
          {
           case 4:
              event = ECORE_EVENT_MOUSE_WHEEL;
              direction = 0; //Vertical
              z = -1; //Up
              break;
           case 5:
              event = ECORE_EVENT_MOUSE_WHEEL;
              direction = 0;
              z = 1; //Down
              break;
           case 6:
              event = ECORE_EVENT_MOUSE_WHEEL;
              direction = 1; //Horizontal
              z = -1;
              break;
           case 7:
              event = ECORE_EVENT_MOUSE_WHEEL;
              direction = 1;
              z = 1;
              break;
           default:
              event = ECORE_EVENT_MOUSE_BUTTON_DOWN;
          }

        if (now - cdata->last_mouse_button_down <= 1000 * server->double_click_time)
          cdata->double_click = EINA_TRUE;
        else
          cdata->double_click = cdata->triple_click = EINA_FALSE;

        if (now - cdata->last_mouse_button_down <= 2000 * server->double_click_time)
          cdata->triple_click = EINA_TRUE;
        else
          cdata->triple_click = EINA_FALSE;

        cdata->last_mouse_button_down = now;
     }
   else if (button_changed < 0)
     {
        button = _ecore_evas_vnc_server_pointer_button_get(-button_changed);
        //Ignore, it was already report.
        if (button > 3 && button < 8)
          return;
        event = ECORE_EVENT_MOUSE_BUTTON_UP;
     }
   else
     return;

   if (event == ECORE_EVENT_MOUSE_BUTTON_DOWN ||
       event == ECORE_EVENT_MOUSE_BUTTON_UP)
     {
        button_event = calloc(1, sizeof(Ecore_Event_Mouse_Button));
        EINA_SAFETY_ON_NULL_RETURN(button_event);

        button_event->timestamp = (unsigned int)now;
        button_event->window = button_event->event_window =
          button_event->root_window = server->ee->prop.window;
        button_event->x = button_event->multi.x = x;
        button_event->y = button_event->multi.y = y;
        button_event->multi.pressure = 1.0;
        button_event->same_screen = 1;
        button_event->buttons = button;
        button_event->modifiers = cdata->key_modifiers;
        button_event->double_click = cdata->double_click ? 1 : 0;
        button_event->triple_click = cdata->triple_click ? 1 : 0;
        button_event->dev = cdata->mouse;
        efl_ref(cdata->mouse);

        ecore_event_add(event, button_event,
                        _ecore_evas_vnc_server_ecore_event_generic_free,
                        cdata->mouse);
        return;
     }

   //Mouse wheel
   wheel_event = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
   EINA_SAFETY_ON_NULL_RETURN(wheel_event);
   wheel_event->dev = cdata->mouse;
   efl_ref(cdata->mouse);
   wheel_event->window = wheel_event->event_window =
     wheel_event->root_window = server->ee->prop.window;
   wheel_event->same_screen = 1;
   wheel_event->modifiers = cdata->key_modifiers;
   wheel_event->x = x;
   wheel_event->y = y;
   wheel_event->direction = direction;
   wheel_event->z = z;

   ecore_event_add(event, wheel_event,
                   _ecore_evas_vnc_server_ecore_event_generic_free,
                   cdata->mouse);
}

static Eina_Bool
_ecore_evas_vnc_server_init(void)
{
   if (!eina_init())
     {
        EINA_LOG_ERR("Could not init Eina");
        return EINA_FALSE;
     }

   if (!ecore_init())
     {
        EINA_LOG_ERR("Could not init Ecore");
        goto err_ecore;
     }

   if (!ecore_evas_init())
     {
        EINA_LOG_ERR("Could not init Ecore_Evas");
        goto err_ecore_evas;
     }

   if (!ecore_event_init())
     {
        EINA_LOG_ERR("Could not init Ecore_Event");
        goto err_ecore_event;
     }

   _ecore_evas_vnc_server_log_dom = eina_log_domain_register("Ecore_Evas_Vnc_Server",
                                                             EINA_COLOR_LIGHTBLUE);
   if (_ecore_evas_vnc_server_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: Ecore_Evas_Vnc_Server");
        goto err_domain;
     }

   return EINA_TRUE;

 err_domain:
   ecore_event_shutdown();
 err_ecore_event:
   ecore_evas_shutdown();
 err_ecore_evas:
   ecore_shutdown();
 err_ecore:
   eina_shutdown();
   return EINA_FALSE;
}

static void
_ecore_evas_vnc_server_shutdown(void)
{
   eina_log_domain_unregister(_ecore_evas_vnc_server_log_dom);
   _ecore_evas_vnc_server_log_dom = -1;
   ecore_event_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   eina_shutdown();
}

static inline int
align4(int v)
{
   return ((v / 4) + (v % 4 ? 1 : 0)) * 4;
}

static void
_ecore_evas_vnc_server_draw(void *data, Evas *e EINA_UNUSED, void *event_info)
{
   Evas_Event_Render_Post *post = event_info;
   Evas_Object *snapshot = data;
   Ecore_Evas_Vnc_Server *server;
   const void *pixels;
   Eina_List *l;
   Eina_Rectangle *r;
   size_t size;
   Eina_Bool new_buf = EINA_FALSE;
   Eina_Rectangle snapshot_pos;

   // Nothing was updated, so let's not bother sending nothingness
   if (!post->updated_area) return ;

   server = evas_object_data_get(snapshot, "_ecore_evas.vnc");
   EINA_SAFETY_ON_NULL_RETURN(server);

   pixels = evas_object_image_data_get(snapshot, EINA_FALSE);
   evas_object_geometry_get(snapshot,
                            &snapshot_pos.x,
                            &snapshot_pos.y,
                            &snapshot_pos.w,
                            &snapshot_pos.h);

   // Align size on 4 pixels for vnc library stability
   snapshot_pos.w = align4(snapshot_pos.w);
   snapshot_pos.h = align4(snapshot_pos.h);

   DBG("Preparing sending of buffer {%i, %i} with %i updates.", snapshot_pos.w, snapshot_pos.h, eina_list_count(post->updated_area));

   if (!server->frame_buffer ||
       server->last_w != snapshot_pos.w ||
       server->last_h != snapshot_pos.h)
     {
        Eina_Rectangle tmp = { 0, 0, snapshot_pos.w, snapshot_pos.h };
        char *new_fb;

        size = snapshot_pos.w * snapshot_pos.h * VNC_BYTES_PER_PIXEL;
        new_fb = malloc(size);
        EINA_SAFETY_ON_NULL_RETURN(new_fb);
        free(server->frame_buffer);
        server->frame_buffer = new_fb;
        server->last_w = snapshot_pos.w;
        server->last_h = snapshot_pos.h;
        new_buf = EINA_TRUE;
        eina_tiler_area_size_set(server->t, snapshot_pos.w, snapshot_pos.h);
        eina_tiler_rect_add(server->t, &tmp);

        rfbNewFramebuffer(server->vnc_screen, server->frame_buffer,
                          snapshot_pos.w, snapshot_pos.h,
                          VNC_BITS_PER_SAMPLE, VNC_SAMPLES_PER_PIXEL, VNC_BYTES_PER_PIXEL);
        _ecore_evas_vnc_server_format_setup(server->vnc_screen);
     }

   EINA_LIST_FOREACH(post->updated_area, l, r)
     {
        Eina_Rectangle tmp = *r;
        size_t src_stride;
        int dy;

        if (tmp.x > snapshot_pos.w ||
            tmp.y > snapshot_pos.h)
          continue ;

        if (!eina_rectangle_intersection(&tmp, &snapshot_pos))
          continue ;

        src_stride = tmp.w * VNC_BYTES_PER_PIXEL;

        for (dy = 0; dy < tmp.h; dy++)
          {
             memcpy(server->frame_buffer + (tmp.x * VNC_BYTES_PER_PIXEL)
                    + ((dy + tmp.y) * (snapshot_pos.w * VNC_BYTES_PER_PIXEL)),
                    (char *)pixels + (dy * src_stride), src_stride);
          }

        rfbMarkRectAsModified(server->vnc_screen,
                              tmp.x, tmp.y, tmp.x + tmp.w, tmp.y + tmp.h);

        if (new_buf) eina_tiler_rect_del(server->t, &tmp);
     }

   //We did not receive the whole buffer yet, zero the missing bytes for now.
   if (new_buf)
     {
        Eina_Iterator *it;

        it = eina_tiler_iterator_new(server->t);
        EINA_ITERATOR_FOREACH(it, r)
          {
             Eina_Rectangle tmp = *r;
             size_t src_stride;
             int dy;

             src_stride = tmp.w * VNC_BYTES_PER_PIXEL;

             for (dy = 0; dy < tmp.h; dy++)
               {
                  memset(server->frame_buffer + (tmp.x * VNC_BYTES_PER_PIXEL)
                         + ((dy + tmp.y) * (snapshot_pos.w * VNC_BYTES_PER_PIXEL)),
                         0, src_stride);
               }
          }
        eina_iterator_free(it);
        eina_tiler_clear(server->t);
     }

   _ecore_evas_vnc_server_update_clients(server->vnc_screen);
}

static void
_ecore_evas_vnc_server_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Ecore_Evas_Vnc_Server *server = data;

   ecore_main_fd_handler_del(server->vnc_listen6_handler);
   ecore_main_fd_handler_del(server->vnc_listen_handler);
   evas_object_del(server->snapshot);
   rfbShutdownServer(server->vnc_screen, TRUE);
   free(server->frame_buffer);
   rfbScreenCleanup(server->vnc_screen);
   free(server);
}

EAPI Evas_Object *
ecore_evas_vnc_server_new(Ecore_Evas *ee, int port, const char *addr,
                          Ecore_Evas_Vnc_Client_Accept_Cb accept_cb,
                          Ecore_Evas_Vnc_Client_Disconnected_Cb disc_cb,
                          void *data)
{
   Ecore_Evas_Vnc_Server *server;
   Evas_Object *snapshot;
   Eina_Bool can_listen = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ee, NULL);

   server = calloc(1, sizeof(Ecore_Evas_Vnc_Server));
   EINA_SAFETY_ON_NULL_RETURN_VAL(server, NULL);

   snapshot = evas_object_image_filled_add(ee->evas);
   EINA_SAFETY_ON_NULL_RETURN_VAL(snapshot, NULL);
   evas_object_image_snapshot_set(snapshot, EINA_TRUE);
   efl_event_callback_del(snapshot, EFL_EVENT_DEL, _ecore_evas_vnc_server_del, server);

   server->key_info_get_func = _ecore_evas_vnc_server_fb_key_info_get;

   server->vnc_screen = rfbGetScreen(0, NULL, 4, 4, VNC_BITS_PER_SAMPLE,
                                     VNC_SAMPLES_PER_PIXEL, VNC_BYTES_PER_PIXEL);
   EINA_SAFETY_ON_NULL_GOTO(server->vnc_screen, err_screen);

   server->vnc_screen->newClientHook = _ecore_evas_vnc_server_client_connection_new;
   server->vnc_screen->kbdAddEvent = _ecore_evas_vnc_server_client_keyboard_event;
   server->vnc_screen->ptrAddEvent = _ecore_evas_vnc_server_client_pointer_event;

   //This enables multiple client connections at the same time.
   server->vnc_screen->alwaysShared = TRUE;
   server->vnc_screen->frameBuffer = server->frame_buffer;

   _ecore_evas_vnc_server_format_setup(server->vnc_screen);

   if (port > 0)
     server->vnc_screen->port = server->vnc_screen->ipv6port= port;

   if (addr)
     {
        int err;

        //rfbStringToAddr() does not change the addr contents.
        err = rfbStringToAddr((char *)addr, &server->vnc_screen->listenInterface);
        EINA_SAFETY_ON_TRUE_GOTO(err == 0, err_addr);
     }

   rfbInitServer(server->vnc_screen);
   if (server->vnc_screen->listenSock >= 0)
     {
        server->vnc_listen_handler = ecore_main_fd_handler_add(server->vnc_screen->listenSock,
                                                               ECORE_FD_READ,
                                                               _ecore_evas_vnc_server_socket_listen_activity,
                                                               server->vnc_screen,
                                                               NULL, NULL);
        EINA_SAFETY_ON_NULL_GOTO(server->vnc_listen_handler, err_listen);
        can_listen = EINA_TRUE;
     }

   if (server->vnc_screen->listen6Sock >= 0)
     {
        server->vnc_listen6_handler = ecore_main_fd_handler_add(server->vnc_screen->listen6Sock,
                                                                ECORE_FD_READ,
                                                                _ecore_evas_vnc_server_socket_listen_activity,
                                                                server->vnc_screen,
                                                                NULL, NULL);
        EINA_SAFETY_ON_NULL_GOTO(server->vnc_listen6_handler, err_listen6);
        can_listen = EINA_TRUE;
     }

   //rfbInitServer() failed and could not setup the sockets.
   EINA_SAFETY_ON_FALSE_GOTO(can_listen, err_engine);

   server->ee = ee;
   server->vnc_screen->screenData = server;
   server->cb_data = data;
   server->accept_cb = accept_cb;
   server->disc_cb = disc_cb;
   server->snapshot = snapshot;
   server->t = eina_tiler_new(1, 1);
   eina_tiler_tile_size_set(server->t, 1, 1);
   eina_tiler_strict_set(server->t, EINA_TRUE);

   evas_object_data_set(snapshot, "_ecore_evas.vnc", server);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST, _ecore_evas_vnc_server_draw, snapshot);

   return snapshot;

 err_engine:
   ecore_main_fd_handler_del(server->vnc_listen6_handler);
 err_listen6:
   ecore_main_fd_handler_del(server->vnc_listen_handler);
 err_listen:
   rfbShutdownServer(server->vnc_screen, TRUE);
 err_addr:
   rfbScreenCleanup(server->vnc_screen);
 err_screen:
   free(server);
   return NULL;
}

EAPI Eina_Bool
ecore_evas_vnc_server_pointer_xy_get(const Evas_Object *snapshot,
                                     const Evas_Device *pointer,
                                     Evas_Coord *x, Evas_Coord *y)
{
   Ecore_Evas_Vnc_Server *server;
   rfbClientRec *client;
   Ecore_Evas_Vnc_Server_Client_Data *cdata;
   rfbClientIteratorPtr itr;

   EINA_SAFETY_ON_NULL_RETURN_VAL(snapshot, EINA_FALSE);
   server = evas_object_data_get(snapshot, "_ecore_evas.vnc");

   EINA_SAFETY_ON_NULL_RETURN_VAL(server, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pointer, EINA_FALSE);

   itr = rfbGetClientIterator(server->vnc_screen);

   while ((client = rfbClientIteratorNext(itr)))
     {
        cdata = client->clientData;

        if (cdata->mouse == pointer)
          {
             if (x) *x = client->lastPtrX;
             if (y) *y = client->lastPtrY;
             return EINA_TRUE;
          }
     }

   rfbReleaseClientIterator(itr);
   return EINA_FALSE;
}

EINA_MODULE_INIT(_ecore_evas_vnc_server_init);
EINA_MODULE_SHUTDOWN(_ecore_evas_vnc_server_shutdown);
