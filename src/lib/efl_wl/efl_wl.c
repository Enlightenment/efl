#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(__clang__)
# pragma clang diagnostic ignored "-Wunused-parameter"
#elif (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "Ecore_Evas.h"
#include "Ecore_Wl2.h"
#include "Ecore_Input.h"
#include "Evas_GL.h"

/* We have to include the wayland server stuff after any wayland client stuff
 * like Ecore_Wl2.h or we'll get complaints about struct wl_buffer being
 * deprecated.
 * That's because its deprecated in server code - it's still the name of
 * the opaque struct client side.
 */
#include <wayland-server.h>
#include "xdg-shell-server-protocol.h"
#include "efl-hints-server-protocol.h"
#include "dmabuf.h"

# ifdef HAVE_ECORE_X
#include "Ecore_X.h"
#endif

#include "Efl_Wl.h"

#undef COORDS_INSIDE
#define COORDS_INSIDE(x, y, xx, yy, ww, hh) \
  (((x) < ((xx) + (ww))) && ((y) < ((yy) + (hh))) && ((x) >= (xx)) && ((y) >= (yy)))

#ifdef __linux__
# include <linux/input.h>
#else
# define BTN_LEFT 0x110
# define BTN_RIGHT 0x111
# define BTN_MIDDLE 0x112
# define BTN_SIDE 0x113
# define BTN_EXTRA 0x114
# define BTN_FORWARD 0x115
# define BTN_BACK 0x116
#endif

#undef container_of
#  define container_of(ptr, type, member) \
   ({ \
      const __typeof__( ((type *)0)->member ) *__mptr = (ptr); \
      (type *)(void *)( (char *)__mptr - offsetof(type,member) ); \
   })

#ifndef EGL_HEIGHT
# define EGL_HEIGHT			0x3056
#endif
#ifndef EGL_WIDTH
# define EGL_WIDTH			0x3057
#endif

#ifndef EGL_TEXTURE_FORMAT
# define EGL_TEXTURE_FORMAT		0x3080
#endif
#ifndef EGL_TEXTURE_RGBA
# define EGL_TEXTURE_RGBA		0x305E
#endif
#ifndef DRM_FORMAT_ARGB8888
# define DRM_FORMAT_ARGB8888           0x34325241
#endif
#ifndef DRM_FORMAT_XRGB8888
# define DRM_FORMAT_XRGB8888           0x34325258
#endif

#define ALL_ACTIONS (WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY | \
		     WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE | \
		     WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK)


typedef struct Input_Sequence
{
   EINA_INLIST;
   uint32_t id;
   uint32_t down_serial;
   uint32_t down_time;
   uint32_t up_serial;
   uint32_t up_time;
   Eina_Bool pass : 1;
} Input_Sequence;

typedef struct Comp_Subsurface Comp_Subsurface;
typedef struct Comp_Surface Comp_Surface;

typedef struct Comp_Buffer
{
   struct wl_resource *res;
   Comp_Surface *cs;
   Eina_List *renders;
   Eina_List *post_renders;
   int x, y, w, h;
   struct wl_listener destroy_listener;
   struct wl_shm_buffer *shm_buffer;
   struct wl_shm_pool *pool;
   struct linux_dmabuf_buffer *dmabuf_buffer;
   Eina_Bool dbg : 1;
} Comp_Buffer;

typedef struct Comp
{
   Efl_Wl_Rotation rotation;
   double scale;
   char *env;
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Display *parent_disp;
   Ecore_Wl2_Display *client_disp;
   struct wl_display *display;

   double wayland_time_base;
   Evas_Object *obj;
   Evas_Object *clip;
   Evas_Object *events;

   Eina_Hash *exes;
   Ecore_Event_Handler *exe_handler;

   Eina_Inlist *surfaces;
   unsigned int surfaces_count;
   Eina_Hash *client_surfaces;
   Comp_Surface *active_surface;

   Eina_Inlist *shells;
   Eina_List *render_queue;
   Eina_List *post_render_queue;
   Evas *evas;
   Evas_GL *gl;
   Evas_GL_Config *glcfg;
   Evas_GL_Context *glctx;
   Evas_GL_Surface *glsfc;
   Evas_GL_API *glapi;
   Eina_List *output_resources;
   Eina_Inlist *seats;
   Eina_Bool rendering : 1;
   Eina_Bool data_device_proxy : 1;
   Eina_Bool x11_selection : 1;
   Eina_Bool rtl : 1;
   Eina_Bool aspect : 1;
   Eina_Bool minmax : 1;
} Comp;

typedef struct Comp_Data_Device_Source Comp_Data_Device_Source;

typedef struct Comp_Seat
{
   EINA_INLIST;
   Comp *c;
   Eina_Stringshare *name;
   struct wl_global *global;

   Ecore_Wl2_Input *seat;
   Ecore_Wl2_Input *client_seat;
   Ecore_Wl2_Offer *client_offer;
   uint32_t client_selection_serial;
   Eo *dev;
   Eina_List *resources;

   Eina_Hash *data_devices;
   struct
   {
      struct wl_resource *res;
      Comp_Data_Device_Source *source;
      Comp_Surface *surface;
      Comp_Surface *enter;
      uint32_t id;
      Ecore_Evas *proxy_win;
      Ecore_Window x11_owner;
      Eina_List *x11_types;
      Eina_Bool tch : 1;
   } drag;
   Comp_Data_Device_Source *selection_source;
   uint32_t selection_serial;
   Ecore_Window x11_selection_owner;

   struct wl_client *active_client;
   Comp_Surface *grab;

   struct
   {
      struct wl_array keys;
      struct wl_array *keys_external;
      struct
      {
         xkb_mod_mask_t depressed;
         xkb_mod_mask_t latched;
         xkb_mod_mask_t locked;
         xkb_layout_index_t group;
         Eina_Bool changed : 1;
      } mods;
      struct xkb_context *context;
      struct xkb_keymap *keymap;
      struct xkb_state *state;
      char *keymap_mem;
      int keymap_mem_size;
      int keymap_fd;
      int repeat_rate;
      int repeat_delay;
      Eina_Hash *resources;
      Comp_Surface *enter;
      Eina_Bool external : 1;
   } kbd;

   struct
   {
      Eina_Hash *resources;
      uint32_t button_mask;
      uint32_t enter_serial;
      Eina_Inlist *events;
      Comp_Surface *enter;
      struct
      {
         Comp_Surface *surface;
         int x, y;
      } cursor;
      struct
      {
         Evas_Object *obj;
         int layer;
         int x, y;
      } efl;
      Evas_Point pos;
      Eina_Bool in : 1;
   } ptr;

   struct
   {
      Eina_Hash *resources;
      Eina_Inlist *events;
      Comp_Surface *enter;
      Evas_Point pos;
   } tch;

   Eina_Bool pointer : 1;
   Eina_Bool keyboard : 1;
   Eina_Bool touch : 1;
   Eina_Bool focused : 1;
   Eina_Bool selection_changed : 1;
   Eina_Bool selection_exists : 1;
   Eina_Bool event_propagate : 1;
} Comp_Seat;

typedef struct Comp_Buffer_State
{
   Comp_Buffer *buffer;
   Eina_Tiler *opaque;
   Eina_Tiler *damages;
   Eina_Tiler *input;
   Eina_List *frames;
   Eina_Bool attach : 1;
   Eina_Bool set_opaque : 1;
   Eina_Bool set_input : 1;
} Comp_Buffer_State;

typedef struct Shell_Data
{
   EINA_INLIST;
   Comp *c;
   struct wl_resource *res;
   Eina_List *surfaces;
   Eina_Inlist *positioners;
   Eina_Bool ping : 1;
} Shell_Data;


typedef struct Shell_Positioner
{
   EINA_INLIST;
   Shell_Data *sd;
   struct wl_resource *res;
   Evas_Coord_Size size;
   Eina_Rectangle anchor_rect;
   enum xdg_positioner_anchor anchor;
   enum xdg_positioner_gravity gravity;
   enum xdg_positioner_constraint_adjustment constrain;
   Evas_Coord_Point offset;
} Shell_Positioner;

struct Comp_Surface
{
   EINA_INLIST;
   Comp *c;
   Evas_Object *obj;
   Evas_Object *clip;
   Evas_Object *img;
   Eina_Array *input_rects;
   Eina_Array *opaque_rects;
   Eina_List *proxies;
   struct wl_resource *res;
   struct wl_resource *role;
   Comp_Seat *drag; //drag surface
   Comp_Buffer *buffer[2]; // new, prev
   /* subsurface stacking order */
   Eina_List *subsurfaces;
   Eina_List *pending_subsurfaces;
   /* any child surface (xdg or subsurface */
   Eina_Inlist *children;
   Comp_Surface *parent;

   Eina_Tiler *opaque;
   Eina_Tiler *input;
   Eina_List *frames;
   Comp_Subsurface *subsurface;
   Comp_Buffer_State pending;
   struct
   {
      struct wl_resource *surface;
      Eina_Rectangle geom;
      Shell_Data *data;
      Eina_Stringshare *title;
      Eina_Stringshare *app_id;
      Shell_Positioner *positioner;
      Eina_List *grabs;
      Eina_Bool popup : 1;
      Eina_Bool new : 1;
      Eina_Bool activated : 1;
   } shell;
   Eina_Bool mapped : 1;
   Eina_Bool cursor : 1;
   Eina_Bool render_queue : 1;
   Eina_Bool post_render_queue : 1;
   Eina_Bool dead : 1;
   Eina_Bool commit : 1;
   Eina_Bool extracted : 1;
   Eina_Bool hint_set_weight : 1;
};

struct Comp_Subsurface
{
   Comp_Surface *surface;
   Comp_Buffer_State cache;
   Evas_Point offset;
   Evas_Point pending_offset;
   Eina_Bool set_offset : 1;
   Eina_Bool sync : 1;
   Eina_Bool cached : 1;
};

typedef enum Comp_Data_Device_Offer_Type
{
   COMP_DATA_DEVICE_OFFER_TYPE_DND,
   COMP_DATA_DEVICE_OFFER_TYPE_CLIPBOARD,
} Comp_Data_Device_Offer_Type;

typedef struct Comp_Data_Device_Offer
{
   Comp_Data_Device_Offer_Type type;
   struct wl_resource *res;
   Comp_Data_Device_Source *source;
   Ecore_Wl2_Offer *proxy_offer;
   uint32_t dnd_actions;
   uint32_t preferred_dnd_action;
   Eina_Bool in_ask : 1;
   Eina_Bool proxy : 1;
} Comp_Data_Device_Offer;

typedef struct Comp_Data_Device_Source
{
   struct wl_resource *res;
   Comp_Seat *seat;
   Comp_Data_Device_Offer *offer;
   Ecore_Window x11_owner;
   Eina_Inlist *transfers;
   Eina_Binbuf *reader_data;
   Ecore_Fd_Handler *reader;
   Eina_List *mime_types;
   uint32_t dnd_actions;
   uint32_t current_dnd_action;
   uint32_t compositor_action;
   Ecore_Event_Handler *proxy_send_handler;
   uint32_t proxy_serial;
   Eina_Bool actions_set : 1;
   Eina_Bool accepted : 1;
   Eina_Bool proxy : 1;
} Comp_Data_Device_Source;

typedef struct Comp_Data_Device_Transfer
{
   EINA_INLIST;
   Comp_Data_Device_Offer_Type type;
   Ecore_Fd_Handler *fdh;
   size_t offset;
   Eina_Stringshare *mime_type;
   Comp_Data_Device_Source *source;
} Comp_Data_Device_Transfer;

static Eina_List *comps;
static Eina_List *handlers;

static inline Eina_Tiler *
tiler_new(void)
{
   Eina_Tiler *t;

   t = eina_tiler_new(65535, 65535);
   eina_tiler_tile_size_set(t, 1, 1);
   return t;
}

static inline void
fdh_del(Ecore_Fd_Handler *fdh)
{
   int fd;
   if (!fdh) return;
   fd = ecore_main_fd_handler_fd_get(fdh);
   if (fd >= 0)
     close(fd);
   ecore_main_fd_handler_del(fdh);
}

#define PTR_SWAP(A, B) ptr_swap((void**)A, (void**)B)

static inline void
ptr_swap(void **a, void **b)
{
   void *c;

   c = *a;
   *a = *b;
   *b = c;
}

static inline void
array_clear(Eina_Array **arr)
{
   Eina_Array *a = *arr;

   if (!a) return;
   while (eina_array_count(a))
     evas_object_del(eina_array_pop(a));
   eina_array_free(a);
   *arr = NULL;
}

static inline Eina_Bool
client_allowed_check(Comp *c, struct wl_client *client)
{
   pid_t p;
   int32_t pid;
   Eina_Bool err;

   wl_client_get_credentials(client, &p, NULL, NULL);
   if (p == getpid()) return EINA_TRUE;
   pid = p;
   err = (!c->exes) || !eina_hash_find(c->exes, &pid);
   if (err)
     wl_client_post_no_memory(client);
   return !err;
}

static inline void
comp_data_device_source_reader_clear(Comp_Data_Device_Source *ds)
{
   fdh_del(ds->reader);
   ds->reader = NULL;
   eina_binbuf_free(ds->reader_data);
   ds->reader_data = NULL;
}

static inline void
comp_surface_reparent(Comp_Surface *cs, Comp_Surface *pcs)
{
   if (cs->parent == pcs) return;
   if (!cs->extracted)
     evas_object_smart_member_del(cs->obj);
   if (cs->parent)
     cs->parent->children = eina_inlist_remove(cs->parent->children, EINA_INLIST_GET(cs));
   if (pcs)
     {
        cs->c->surfaces = eina_inlist_remove(cs->c->surfaces, EINA_INLIST_GET(cs));
        cs->c->surfaces_count--;
        if (!cs->extracted)
          evas_object_smart_member_add(cs->obj, pcs->obj);
        pcs->children = eina_inlist_append(pcs->children, EINA_INLIST_GET(cs));
     }
   else
     {
        if (!cs->extracted)
          evas_object_smart_member_add(cs->obj, cs->c->obj);
        cs->c->surfaces = eina_inlist_append(cs->c->surfaces, EINA_INLIST_GET(cs));
        cs->c->surfaces_count++;
     }
   cs->parent = pcs;
}

static inline struct wl_resource *
data_device_find(Comp_Seat *s, struct wl_resource *resource)
{
   struct wl_client *client = wl_resource_get_client(resource);
   return eina_hash_find(s->data_devices, &client);
}

static inline void
seat_drag_end(Comp_Seat *s)
{
   s->drag.tch = 0;
   s->drag.id = 0;
   s->drag.surface = NULL;
   s->drag.res = NULL;
   s->drag.enter = NULL;
}

static inline Comp_Seat *
seat_find(Comp_Surface *cs, const Eo *dev)
{
   const Eo *seat = dev;
   Comp_Seat *s;

   if (!cs->c->parent_disp) return EINA_INLIST_CONTAINER_GET(cs->c->seats, Comp_Seat);
   if (evas_device_class_get(seat) != EVAS_DEVICE_CLASS_SEAT)
     seat = evas_device_parent_get(seat);
   EINA_INLIST_FOREACH(cs->c->seats, s)
     if (s->dev == seat) return s;
   return NULL;
}

static inline Comp_Seat *
comp_seat_find(Comp *c, const Eo *dev)
{
   const Eo *seat = dev;
   Comp_Seat *s;

   if (!c->parent_disp) return EINA_INLIST_CONTAINER_GET(c->seats, Comp_Seat);
   if (evas_device_class_get(seat) != EVAS_DEVICE_CLASS_SEAT)
     seat = evas_device_parent_get(seat);
   EINA_INLIST_FOREACH(c->seats, s)
     if (s->dev == seat) return s;
   return NULL;
}

static inline Eina_List *
seat_kbd_active_resources_get(Comp_Seat *s)
{
   Eina_List *l, *lcs, *llcs;
   Comp_Surface *cs;

   if (!s->active_client) return NULL;
   if (!s->kbd.resources) return NULL;
   l = eina_hash_find(s->kbd.resources, &s->active_client);
   if (!l) return NULL;
   lcs = eina_hash_find(s->c->client_surfaces, &s->active_client);
   if (!lcs) return NULL;
   EINA_LIST_REVERSE_FOREACH(lcs, llcs, cs)
     if (cs->role && (!cs->shell.popup)) return l;
   return NULL;
}

static inline Eina_List *
seat_ptr_resources_get(Comp_Seat *s, struct wl_client *client)
{
   return s->ptr.resources ? eina_hash_find(s->ptr.resources, &client) : NULL;
}

static inline Eina_List *
seat_tch_resources_get(Comp_Seat *s, struct wl_client *client)
{
   return s->tch.resources ? eina_hash_find(s->tch.resources, &client) : NULL;
}

static void comp_render_pre_proxied(Evas_Object *o, Evas *e, void *event_info);
static void comp_render_post_proxied(Comp_Surface *cs, Evas *e, void *event_info);
static void comp_surface_commit_image_state(Comp_Surface *cs, Comp_Buffer *buffer, Evas_Object *o);

static void
comp_surface_proxy_del(void *data, Evas *e, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Comp_Surface *cs = data;
   int i;

   cs->proxies = eina_list_remove(cs->proxies, obj);
   for (i = 0; i < 2; i++)
     {
        Comp_Buffer *buffer = cs->buffer[i];
        if (!buffer) continue;
        if (buffer->renders) buffer->renders = eina_list_remove(buffer->renders, e);
        if (buffer->post_renders) buffer->post_renders = eina_list_remove(buffer->post_renders, e);
     }
   evas_event_callback_del_full(e, EVAS_CALLBACK_RENDER_PRE, (Evas_Event_Cb)comp_render_pre_proxied, obj);
   evas_event_callback_del_full(e, EVAS_CALLBACK_RENDER_POST, (Evas_Event_Cb)comp_render_post_proxied, cs);
#ifdef HAVE_ECORE_X
   if (ecore_x_display_get())
     ecore_x_dnd_callback_pos_update_set(NULL, NULL);
#endif
}

static void
comp_surface_proxy_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   ecore_evas_resize(data, w, h);
}

static Eina_Bool
comp_surface_is_alpha(Comp_Surface *cs, Comp_Buffer *buffer)
{
   int format;
   if (buffer->shm_buffer)
     format = wl_shm_buffer_get_format(buffer->shm_buffer);
   else if (buffer->dmabuf_buffer)
     format = buffer->dmabuf_buffer->attributes.format;
   else if (cs->c->gl)
     cs->c->glapi->evasglQueryWaylandBuffer(cs->c->gl, buffer->res, EGL_TEXTURE_FORMAT, &format);
   else return EINA_FALSE; //not a real case

   switch (format)
     {
      case DRM_FORMAT_ARGB8888:
      case WL_SHM_FORMAT_ARGB8888:
      case EGL_TEXTURE_RGBA:
        return EINA_TRUE;
      default:
        break;
     }
   return EINA_FALSE;
}

static void
comp_surface_proxy_win_del(Ecore_Evas *ee)
{
   Comp_Seat *s = ecore_evas_data_get(ee, "comp_seat");
   if (!s) return;

   s->drag.proxy_win = NULL;
   //fprintf(stderr, "PROXY WIN DEL\n");
}

static void
seat_drag_proxy_win_add(Comp_Seat *s)
{
   Evas_Object *o;

   if (s->drag.proxy_win) abort();
   evas_object_hide(s->drag.surface->obj);
   s->drag.proxy_win = ecore_evas_new(NULL, 0, 0, 1, 1, NULL);
   ecore_evas_data_set(s->drag.proxy_win, "comp_seat", s);
   ecore_evas_callback_destroy_set(s->drag.proxy_win, comp_surface_proxy_win_del);
   ecore_evas_alpha_set(s->drag.proxy_win, 1);
   ecore_evas_borderless_set(s->drag.proxy_win, 1);
   ecore_evas_override_set(s->drag.proxy_win, 1);
   ecore_evas_show(s->drag.proxy_win);
   o = evas_object_image_filled_add(ecore_evas_get(s->drag.proxy_win));
   evas_object_data_set(o, "comp_surface", s->drag.surface);
   evas_event_callback_add(ecore_evas_get(s->drag.proxy_win),
     EVAS_CALLBACK_RENDER_PRE, (Evas_Event_Cb)comp_render_pre_proxied, o);
   evas_event_callback_add(ecore_evas_get(s->drag.proxy_win),
     EVAS_CALLBACK_RENDER_POST, (Evas_Event_Cb)comp_render_post_proxied, s->drag.surface);
   evas_object_image_border_center_fill_set(o, EVAS_BORDER_FILL_SOLID);
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);
   if (!s->drag.surface->render_queue)
     {
        comp_surface_commit_image_state(s->drag.surface, s->drag.surface->buffer[1], o);
        evas_object_image_alpha_set(o,
          comp_surface_is_alpha(s->drag.surface, s->drag.surface->buffer[1]));
        evas_object_image_data_update_add(o, 0, 0, 9999, 9999);
     }
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL,
     comp_surface_proxy_del, s->drag.surface);
   evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE,
     comp_surface_proxy_resize, s->drag.proxy_win);
   evas_object_resize(o,
     s->drag.surface->buffer[!s->drag.surface->render_queue]->w,
     s->drag.surface->buffer[!s->drag.surface->render_queue]->h);
   s->drag.surface->proxies = eina_list_append(s->drag.surface->proxies, o);
}

static void comp_surface_send_data_device_leave(Comp_Surface *cs, Comp_Seat *s);
static void comp_surface_send_data_device_enter(Comp_Surface *cs, Comp_Seat *s);

static void
dnd_motion(Comp_Seat *s, int ex, int ey)
{
   struct wl_resource *res;
   Eina_List *l, *lcs;
   Comp_Surface *cs;

   s->ptr.pos.x = ex;
   s->ptr.pos.y = ey;
   if (!s->active_client) return;
   l = eina_hash_find(s->c->client_surfaces, &s->active_client);
   EINA_LIST_REVERSE_FOREACH(l, lcs, cs)
     {
        int x, y, w, h;
        if ((!cs->mapped) || (!cs->role)) continue;
        evas_object_geometry_get(cs->obj, &x, &y, &w, &h);
        if (!COORDS_INSIDE(ex, ey, x, y, w, h)) continue;
        if (s->drag.enter != cs)
          {
             if (s->drag.enter)
               comp_surface_send_data_device_leave(s->drag.enter, s);
             s->drag.enter = cs;
             if (s->drag.source)
               comp_surface_send_data_device_enter(cs, s);
          }
        if (!s->drag.source) break;
        res = eina_hash_find(s->data_devices, &s->active_client);
        wl_data_device_send_motion(res,
          (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff),
          wl_fixed_from_int(ex - x), wl_fixed_from_int(ey - y));
        break;
     }
}
#ifdef HAVE_ECORE_X
static void x11_send_send(Comp_Data_Device_Source *source, const char* mime_type, int32_t fd, Comp_Data_Device_Offer_Type type);
#endif
#include "copiedfromweston.x"
#ifdef HAVE_ECORE_X
# include "x11.x"
#endif

static void
resource_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}

#define CONTAINS(x, y, w, h, xx, yy, ww, hh) \
  (((xx) >= (x)) && (((x) + (w)) >= ((xx) + (ww))) && ((yy) >= (y)) && (((y) + (h)) >= ((yy) + (hh))))
#define CONSTRAINED(W, H, X, Y) \
   !CONTAINS(zx, zy, zw, zh, (X), (Y), (W), (H))

static int
_apply_positioner_x(int x, Shell_Positioner *sp, Eina_Bool invert)
{
   enum xdg_positioner_anchor an = XDG_POSITIONER_ANCHOR_NONE;
   enum xdg_positioner_gravity grav = XDG_POSITIONER_GRAVITY_NONE;

   if (invert)
     {
        if (sp->anchor == XDG_POSITIONER_ANCHOR_LEFT)
          an = XDG_POSITIONER_ANCHOR_RIGHT;
        else if (sp->anchor == XDG_POSITIONER_ANCHOR_RIGHT)
          an = XDG_POSITIONER_ANCHOR_LEFT;
        if (sp->gravity & XDG_POSITIONER_GRAVITY_LEFT)
          grav |= XDG_POSITIONER_GRAVITY_RIGHT;
        else if (sp->gravity & XDG_POSITIONER_GRAVITY_RIGHT)
          grav |= XDG_POSITIONER_GRAVITY_LEFT;
     }
   else
     {
        an = sp->anchor;
        grav = sp->gravity;
     }

   /* left edge */
   if (an == XDG_POSITIONER_ANCHOR_LEFT)
     x += sp->anchor_rect.x;
   /* right edge */
   else if (an == XDG_POSITIONER_ANCHOR_RIGHT)
     x += sp->anchor_rect.x + sp->anchor_rect.w;
   /* center */
   else
     x += sp->anchor_rect.x + (sp->anchor_rect.w / 2);

   /* flip left over anchor */
   if (grav & XDG_POSITIONER_GRAVITY_LEFT)
     x -= sp->size.w;
   /* center on anchor */
   else if (!(grav & XDG_POSITIONER_GRAVITY_RIGHT))
     x -= sp->size.w / 2;
   return x;
}

static int
_apply_positioner_y(int y, Shell_Positioner *sp, Eina_Bool invert)
{
   enum xdg_positioner_anchor an = XDG_POSITIONER_ANCHOR_NONE;
   enum xdg_positioner_gravity grav = XDG_POSITIONER_GRAVITY_NONE;

   if (invert)
     {
        if (sp->anchor == XDG_POSITIONER_ANCHOR_TOP)
          an = XDG_POSITIONER_ANCHOR_BOTTOM;
        else if (sp->anchor == XDG_POSITIONER_ANCHOR_BOTTOM)
          an = XDG_POSITIONER_ANCHOR_TOP;
        if (sp->gravity & XDG_POSITIONER_GRAVITY_TOP)
          grav |= XDG_POSITIONER_GRAVITY_BOTTOM;
        else if (sp->gravity & XDG_POSITIONER_GRAVITY_BOTTOM)
          grav |= XDG_POSITIONER_GRAVITY_TOP;
     }
   else
     {
        an = sp->anchor;
        grav = sp->gravity;
     }

   /* up edge */
   if (an == XDG_POSITIONER_ANCHOR_TOP)
     y += sp->anchor_rect.y;
   /* bottom edge */
   else if (an == XDG_POSITIONER_ANCHOR_BOTTOM)
     y += sp->anchor_rect.y + sp->anchor_rect.h;
   /* center */
   else
     y += sp->anchor_rect.y + (sp->anchor_rect.h / 2);

   /* flip up over anchor */
   if (grav & XDG_POSITIONER_GRAVITY_TOP)
     y -= sp->size.h;
   /* center on anchor */
   else if (!(grav & XDG_POSITIONER_GRAVITY_BOTTOM))
     y -= sp->size.h / 2;
   return y;
}

static Eina_Bool
_apply_positioner_slide(Comp_Surface *cs, Shell_Positioner *sp, int *x, int *y, int *w, int *h, int zx, int zy, int zw, int zh)
{
   int px, py, cx, cy;

   evas_object_geometry_get(cs->parent->obj, &px, &py, NULL, NULL);
   evas_object_geometry_get(cs->c->obj, &cx, &cy, NULL, NULL);
   px -= cx, py -= cy;
   if ((sp->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X) &&
       (!CONTAINS(zx, zy, zw, zh, *x, zy, *w, 1)))
     {
        int sx = *x;

        if (sp->gravity & XDG_POSITIONER_GRAVITY_LEFT)
          {
             if (*x + *w > zx + zw)
               sx = MAX(zx + zw - *w, px + sp->anchor_rect.x - *w);
             else if (*x < zx)
               sx = MIN(zx, px + sp->anchor_rect.x + sp->anchor_rect.w);
          }
        else if (sp->gravity & XDG_POSITIONER_GRAVITY_RIGHT)
          {
             if (*x < zx)
               sx = MIN(zx, *x + sp->anchor_rect.x + sp->anchor_rect.w);
             else if (*x + *w > zx + zw)
               sx = MAX(zx + zw - *w, px + sp->anchor_rect.x - *w);
          }
        if (CONTAINS(zx, zy, zw, zh, sx, zy, *w, 1))
          *x = sx;
     }
   if (!CONSTRAINED(*w, *h, *x, *y)) return EINA_TRUE;
   if ((sp->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y) &&
       (!CONTAINS(zx, zy, zw, zh, zx, *y, 1, *h)))
     {
        int sy = *y;

        if (sp->gravity & XDG_POSITIONER_GRAVITY_TOP)
          {
             if (*y + *h > zy + zh)
               sy = MAX(zy + zh - *h, py + sp->anchor_rect.y - *h);
             else if (*y < zy)
               sy = MIN(zy, py + sp->anchor_rect.y + sp->anchor_rect.h);
          }
        else if (sp->gravity & XDG_POSITIONER_GRAVITY_BOTTOM)
          {
             if (*y < zy)
               sy = MIN(zy, py + sp->anchor_rect.y + sp->anchor_rect.h);
             else if (*y + *h > zy + zh)
               sy = MAX(zy + zh - *h, py + sp->anchor_rect.y - *h);
          }
        if (CONTAINS(zx, zy, zw, zh, zx, sy, 1, *h))
          *y = sy;
     }
   return !CONSTRAINED(*w, *h, *x, *y);
}

static void
_apply_positioner(Comp_Surface *cs, Shell_Positioner *sp)
{
   int ix, iy, x, y, w = -1, h = -1, px, py, cx, cy;
   int zx, zy, zw, zh;
   /* apply base geometry:
    * coords are relative to parent
    */
   evas_object_geometry_get(cs->parent->obj, &px, &py, NULL, NULL);
   evas_object_geometry_get(cs->c->obj, &cx, &cy, NULL, NULL);
   px -= cx, py -= cy;
   ix = x = px + sp->offset.x;
   iy = y = py + sp->offset.y;

   if (sp->size.w && sp->size.h)
     {
        w = sp->size.w;
        h = sp->size.h;
     }

   /* apply policies in order:
    - anchor (add anchor_rect using anchor point)
    - gravity (perform flips if gravity is not right|bottom)
    - constrain (adjust if popup does not fit)
    */
   x = _apply_positioner_x(x, sp, 0);
   y = _apply_positioner_y(y, sp, 0);

   evas_object_geometry_get(cs->c->obj, &zx, &zy, &zw, &zh);

   if (!CONSTRAINED(w, h, x, y))
     {
        evas_object_move(cs->obj, zx + x, zy + y);
        if (w > 0) evas_object_resize(cs->obj, w, h);
        return;
     }

   /* assume smart placement:
    - flip
    - slide
    - resize
    */
   if ((sp->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X) &&
       (!CONTAINS(zx, zy, zw, zh, x, zy, w, 1)))
     {
        int fx;

        fx = _apply_positioner_x(ix, sp, 1);
        if (CONTAINS(zx, zy, zw, zh, fx, zy, w, 1))
          x = fx;
     }
   if (!CONSTRAINED(w, h, x, y))
     {
        evas_object_move(cs->obj, zx + x, zy + y);
        if (w > 0) evas_object_resize(cs->obj, w, h);
        return;
     }
   if ((sp->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y) &&
       (!CONTAINS(zx, zy, zw, zh, zx, y, 1, h)))
     {
        int fy;

        fy = _apply_positioner_y(iy, sp, 1);
        if (CONTAINS(zx, zy, zw, zh, zx, fy, 1, h))
          y = fy;
     }
   if (!CONSTRAINED(w, h, x, y))
     {
        evas_object_move(cs->obj, zx + x, zy + y);
        if (w > 0) evas_object_resize(cs->obj, w, h);
        return;
     }
   if (_apply_positioner_slide(cs, sp, &x, &y, &w, &h, zx, zy, zw, zh))
     {
        evas_object_move(cs->obj, zx + x, zy + y);
        if (w > 0) evas_object_resize(cs->obj, w, h);
        return;
     }

   if (!CONSTRAINED(w, h, x, y))
     {
        evas_object_move(cs->obj, zx + x, zy + y);
        if (w > 0) evas_object_resize(cs->obj, w, h);
        return;
     }

   if ((sp->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X) &&
       (!CONTAINS(zx, zy, zw, zh, x, zy, w, 1)))
     {
        w = zx + zw - x;
        if (!CONSTRAINED(w, h, x, y))
          {
             evas_object_move(cs->obj, zx + x, zy + y);
             if (w > 0) evas_object_resize(cs->obj, w, h);
             return;
          }
     }
   if ((sp->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y) &&
       (!CONTAINS(zx, zy, zw, zh, zx, y, 1, h)))
     {
        h = zy + zh - y;
     }
   evas_object_move(cs->obj, zx + x, zy + y);
   if (w > 0) evas_object_resize(cs->obj, w, h);
}

static const struct wl_data_offer_interface data_device_offer_interface =
{
   data_device_offer_accept,
   data_device_offer_receive,
   resource_destroy,
   data_device_offer_finish,
   data_device_offer_set_actions,
};

static void
data_device_offer_create(Comp_Data_Device_Source *source, struct wl_resource *resource)
{
   Comp_Data_Device_Offer *off;
   Eina_List *l;
   Eina_Stringshare *type;

   off = calloc(1, sizeof(Comp_Data_Device_Offer));
   off->res = wl_resource_create(wl_resource_get_client(resource), &wl_data_offer_interface,
     wl_resource_get_version(resource), 0);
   wl_resource_set_implementation(off->res, &data_device_offer_interface, off, data_device_offer_impl_destroy);
   off->source = source;
   source->offer = off;
   off->proxy = source->proxy;
   wl_data_device_send_data_offer(resource, off->res);
   EINA_LIST_FOREACH(source->mime_types, l, type)
     wl_data_offer_send_offer(off->res, type);
}

static void
comp_buffer_state_alloc(Comp_Buffer_State *cbs)
{
   cbs->damages = tiler_new();
   cbs->opaque = tiler_new();
   cbs->input = tiler_new();
}

static void
comp_buffer_state_clear(Comp_Buffer_State *cbs)
{
   eina_tiler_free(cbs->damages);
   eina_tiler_free(cbs->opaque);
   eina_tiler_free(cbs->input);
   while (cbs->frames)
     wl_resource_destroy(eina_list_data_get(cbs->frames));
}

static void
comp_seat_send_modifiers(Comp_Seat *s, struct wl_resource *res, uint32_t serial)
{
   wl_keyboard_send_modifiers(res, serial, s->kbd.mods.depressed,
                              s->kbd.mods.latched,
                              s->kbd.mods.locked,
                              s->kbd.mods.group);
   s->kbd.mods.changed = 0;
}

static Eina_Bool
data_device_selection_read(void *d, Ecore_Fd_Handler *fdh)
{
   Comp_Data_Device_Source *ds = d;
   int len = -1;

   do
     {
        unsigned char buf[2048];
        int fd;

        fd = ecore_main_fd_handler_fd_get(fdh);
        if (fd < 0) break;
        len = read(fd, buf, sizeof(buf));
        if (len > 0)
          {
             if (!ds->reader_data)
               ds->reader_data = eina_binbuf_new();
             eina_binbuf_append_length(ds->reader_data, buf, len);
             return ECORE_CALLBACK_RENEW;
          }
     } while (0);
   fdh_del(fdh);
   ds->reader = NULL;
   if (len < 0)
     {
        eina_binbuf_free(ds->reader_data);
        ds->reader_data = NULL;
     }
   return ECORE_CALLBACK_RENEW;
}

static void
comp_seat_kbd_data_device_enter(Comp_Seat *s)
{
   struct wl_resource *res;
   Comp_Data_Device_Source *ds = s->selection_source;
   int fd[2];

   if (!s->kbd.enter) return;
   res = data_device_find(s, s->kbd.enter->res);
   if (!res) return;
   if (ds)
     {
        data_device_offer_create(ds, res);
        ds->offer->type = COMP_DATA_DEVICE_OFFER_TYPE_CLIPBOARD;
     }
   if (ds)
     wl_data_device_send_selection(res, ds->offer ? ds->offer->res : NULL);
   if ((!ds) || (!ds->mime_types)) return;
   EINA_SAFETY_ON_TRUE_RETURN(pipe2(fd, O_CLOEXEC) < 0);
   wl_data_source_send_send(ds->res, eina_list_data_get(ds->mime_types), fd[1]);
   close(fd[1]);
   ds->reader = ecore_main_fd_handler_file_add(fd[0], ECORE_FD_READ | ECORE_FD_ERROR,
     data_device_selection_read, ds, NULL, NULL);
}

static void
comp_seats_redo_enter(Comp *c, Comp_Surface *cs)
{
   Comp_Seat *s;
   uint32_t serial;
   struct wl_client *client = NULL;

   serial = wl_display_next_serial(c->display);
   if (cs)
     client = wl_resource_get_client(cs->res);
   EINA_INLIST_FOREACH(c->seats, s)
     {
        Eina_List *l, *ll;
        struct wl_resource *res;
        Eina_Bool same = s->kbd.enter == cs;

        if (c->active_surface && s->kbd.enter && (!same))
          {
             l = seat_kbd_active_resources_get(s);
             EINA_LIST_FOREACH(l, ll, res)
               wl_keyboard_send_leave(res, serial, s->kbd.enter->res);
          }
        s->active_client = client;
        if (cs)
          {
             l = seat_kbd_active_resources_get(s);
             EINA_LIST_FOREACH(l, ll, res)
               {
                  if (!same)
                    wl_keyboard_send_enter(res, serial, cs->res, &s->kbd.keys);
                  comp_seat_send_modifiers(s, res, serial);
               }
          }
        s->kbd.enter = cs;
        if (s->kbd.enter && s->selection_source && (!same))
          comp_seat_kbd_data_device_enter(s);
     }
   c->active_surface = cs;
}

static void shell_surface_send_configure(Comp_Surface *cs);

static void
shell_surface_deactivate_recurse(Comp_Surface *cs)
{
   Comp_Surface *ccs;

   EINA_INLIST_FOREACH(cs->children, ccs)
     {
        if (!ccs->shell.popup) continue;
        shell_surface_deactivate_recurse(ccs);
        if (ccs->shell.grabs) evas_object_hide(ccs->obj);
     }
}

static void
shell_surface_activate_recurse(Comp_Surface *cs)
{
   Comp_Surface *lcs, *parent = cs->parent;
   Eina_List *parents = NULL;
   Eina_Inlist *i;

   if (parent)
     {
        /* remove focus from parents */
        while (parent)
          {
             parents = eina_list_append(parents, parent);
             if (!parent->shell.popup) break;
             parent = parent->parent;
          }
     }
   EINA_INLIST_FOREACH_SAFE(cs->c->surfaces, i, lcs)
     if (lcs->shell.activated && (lcs != cs))
       {
          if ((!parents) || (!eina_list_data_find(parents, lcs)))
            {
               lcs->shell.activated = 0;
               shell_surface_send_configure(lcs);
               cs->c->surfaces = eina_inlist_promote(cs->c->surfaces, EINA_INLIST_GET(lcs));
            }
       }
   eina_list_free(parents);
}

static void
shell_surface_minmax_update(Comp_Surface *cs)
{
   int w, h;

   if (!cs) return;
   if (!cs->c->minmax) return;
   if (cs->extracted) return;
   if (cs->parent) return;
   evas_object_size_hint_min_get(cs->obj, &w, &h);
   evas_object_size_hint_min_set(cs->c->obj, w, h);
   evas_object_size_hint_max_get(cs->obj, &w, &h);
   if (!w) w = -1;
   if (!h) h = -1;
   evas_object_size_hint_max_set(cs->c->obj, w, h);
}

static void
shell_surface_aspect_update(Comp_Surface *cs)
{
   Evas_Aspect_Control aspect;
   int w, h;

   if (!cs) return;
   if (!cs->c->aspect) return;
   if (cs->extracted) return;
   if (cs->parent) return;
   evas_object_size_hint_aspect_get(cs->obj, &aspect, &w, &h);
   evas_object_size_hint_aspect_set(cs->c->obj, aspect, w, h);
}

static void
shell_surface_send_configure(Comp_Surface *cs)
{
   uint32_t serial, *s;
   struct wl_array states;
   int w, h;

   cs->shell.new = 0;
   if (cs->shell.popup)
     {
        int x, y, px, py;
        evas_object_geometry_get(cs->obj, &x, &y, &w, &h);
        evas_object_geometry_get(cs->parent->obj, &px, &py, NULL, NULL);
        serial = wl_display_next_serial(cs->c->display);
        xdg_popup_send_configure(cs->role, x - px, y - py, w, h);
        xdg_surface_send_configure(cs->shell.surface, serial);
        return;
     }
   wl_array_init(&states);
   s = wl_array_add(&states, sizeof(uint32_t));
   *s = XDG_TOPLEVEL_STATE_FULLSCREEN;
   if (cs->shell.activated)
     {
        s = wl_array_add(&states, sizeof(uint32_t));
        *s = XDG_TOPLEVEL_STATE_ACTIVATED;
        if (!cs->extracted)
          evas_object_raise(cs->obj);
        if (cs->parent)
          cs->parent->children = eina_inlist_demote(cs->parent->children, EINA_INLIST_GET(cs));
        else
          cs->c->surfaces = eina_inlist_demote(cs->c->surfaces, EINA_INLIST_GET(cs));
        shell_surface_activate_recurse(cs);
     }
   serial = wl_display_next_serial(cs->c->display);
   if (cs->extracted)
     evas_object_geometry_get(cs->obj, NULL, NULL, &w, &h);
   else
     evas_object_geometry_get(cs->c->clip, NULL, NULL, &w, &h);
   xdg_toplevel_send_configure(cs->role, w, h, &states);
   xdg_surface_send_configure(cs->shell.surface, serial);
   wl_array_release(&states);
   if (cs->shell.activated)
     {
        Comp_Surface *ccs;

        /* apply activation to already-mapped surface */
        if (cs->mapped)
          {
             comp_seats_redo_enter(cs->c, cs);
             shell_surface_aspect_update(cs);
             shell_surface_minmax_update(cs);
          }
        EINA_INLIST_FOREACH(cs->children, ccs)
          if (ccs->shell.surface && ccs->role && ccs->shell.popup)
            ccs->shell.activated = cs->shell.activated;
     }
   else
     shell_surface_deactivate_recurse(cs);
}

static void
shell_surface_init(Comp_Surface *cs)
{
   /* update activate status: newest window is always activated */
   Comp_Surface *parent = cs->parent;

   if (cs->c->active_surface && parent && (!parent->shell.activated))
     {
        /* child windows/popups cannot steal focus from toplevel */
        shell_surface_send_configure(cs);
        return;
     }

   cs->shell.activated = 1;
   shell_surface_send_configure(cs);
}

static void
comp_surface_output_leave(Comp_Surface *cs)
{
   Eina_List *l;
   struct wl_resource *res;

   EINA_LIST_FOREACH(cs->c->output_resources, l, res)
     if (wl_resource_get_client(res) == wl_resource_get_client(cs->res))
       wl_surface_send_leave(cs->res, res);
}

static void
comp_surface_output_enter(Comp_Surface *cs)
{
   Eina_List *l;
   struct wl_resource *res;

   EINA_LIST_FOREACH(cs->c->output_resources, l, res)
     if (wl_resource_get_client(res) == wl_resource_get_client(cs->res))
       wl_surface_send_enter(cs->res, res);
}

static void
comp_surface_buffer_detach(Comp_Buffer **pbuffer)
{
   Comp_Buffer *buffer;

   buffer = *pbuffer;
   if (!buffer) return;
   if (buffer->post_renders)
     fprintf(stderr, "CRASH %u\n", wl_resource_get_id(buffer->res));
   eina_list_free(buffer->renders);
   wl_list_remove(&buffer->destroy_listener.link);
   //if (buffer->dbg) fprintf(stderr, "BUFFER(%d) RELEASE\n", wl_resource_get_id(buffer->res));
   if (buffer->pool) wl_shm_pool_unref(buffer->pool);
   wl_buffer_send_release(buffer->res);
   free(buffer);
   *pbuffer = NULL;
}

static void
comp_surface_buffer_post_render(Comp_Surface *cs)
{
   double t = ecore_loop_time_get() - cs->c->wayland_time_base;

   //if (cs->subsurface)
     //fprintf(stderr, "FRAME(%d)\n", wl_resource_get_id(cs->res));
   while (cs->frames)
     {
        struct wl_resource *frame = eina_list_data_get(cs->frames);

        wl_callback_send_done(frame, t * 1000);
        wl_resource_destroy(frame);
     }
}

static void
comp_surface_pixels_get(void *data, Evas_Object *obj)
{
   Comp_Surface *cs = data;
   Comp_Buffer *buffer;

   evas_object_image_pixels_dirty_set(obj, 0);
   evas_object_image_pixels_get_callback_set(obj, NULL, NULL);
   buffer = cs->buffer[!cs->render_queue];
   //if (cs->cursor || (buffer->w == 32)) fprintf(stderr, "RENDER(%d) %dx%d\n", wl_resource_get_id(buffer->res), buffer->w, buffer->h);
   evas_object_image_size_set(obj, buffer->w, buffer->h);
   evas_object_image_data_set(obj, wl_shm_buffer_get_data(buffer->shm_buffer));
}

static void
comp_surface_commit_image_state(Comp_Surface *cs, Comp_Buffer *buffer, Evas_Object *o)
{
   if ((!buffer->renders) || (!eina_list_data_find(buffer->renders, evas_object_evas_get(o))))
     buffer->renders = eina_list_append(buffer->renders, evas_object_evas_get(o));
   evas_object_image_pixels_dirty_set(o, 1);

   if (buffer->shm_buffer)
     {
        //if (cs->subsurface)
          //fprintf(stderr, "SET CB\n");
        evas_object_image_pixels_get_callback_set(o, comp_surface_pixels_get, cs);
        buffer->pool = wl_shm_buffer_ref_pool(buffer->shm_buffer);
     }
   else
     {
        Evas_Native_Surface ns;

        evas_object_image_pixels_get_callback_set(o, NULL, NULL);
        if (buffer->dmabuf_buffer)
          {
             ns.type = EVAS_NATIVE_SURFACE_WL_DMABUF;
             ns.version = EVAS_NATIVE_SURFACE_VERSION;

             ns.data.wl_dmabuf.attr = &buffer->dmabuf_buffer->attributes;
             ns.data.wl_dmabuf.resource = buffer->res;
             ns.data.wl_dmabuf.scanout.handler = NULL;
             ns.data.wl_dmabuf.scanout.data = NULL;
          }
        else
          {
             ns.type = EVAS_NATIVE_SURFACE_WL;
             ns.version = EVAS_NATIVE_SURFACE_VERSION;
             ns.data.wl.legacy_buffer = buffer->res;
          }
        evas_object_image_native_surface_set(o, &ns);
     }
}

static void
shell_surface_reset(Comp_Surface *cs)
{
   EINA_RECTANGLE_SET(&cs->shell.geom, 0, 0, 0, 0);
   eina_stringshare_replace(&cs->shell.title, NULL);
   eina_stringshare_replace(&cs->shell.app_id, NULL);
   cs->shell.activated = 0;
}

static void
comp_surface_commit_state(Comp_Surface *cs, Comp_Buffer_State *state)
{
   int x, y;
   Eina_List *l;
   Evas_Object *o;
   Comp_Buffer *buffer = NULL;
   Eina_Bool newly_new = EINA_FALSE;

   if (state->attach)
     {
        comp_surface_buffer_detach(&cs->buffer[0]);
        buffer = cs->buffer[0] = state->buffer;

        if (buffer)
          {
             //if (cs->subsurface)
               //fprintf(stderr, "BUFFER(%d) COMMIT %d\n", wl_resource_get_id(buffer->res), wl_resource_get_id(cs->res));
             if ((!cs->c->rendering) && (!cs->post_render_queue) &&
               ((!cs->buffer[1]) || (!cs->buffer[1]->post_renders)))
               comp_surface_buffer_detach(&cs->buffer[1]);
          }
        else
          {
             if (!cs->extracted)
               evas_object_hide(cs->obj);
             EINA_LIST_FOREACH(cs->proxies, l, o)
               evas_object_hide(o);
             if (cs->shell.surface)
               {
                  cs->shell.new = 1;
                  newly_new = EINA_TRUE;
                  shell_surface_reset(cs);
               }
          }
     }
   evas_object_geometry_get(cs->obj, &x, &y, NULL, NULL);

   if (buffer && (!cs->mapped))
     {
        if (cs->role && (!cs->extracted))
          evas_object_show(cs->obj);
        /* apply activation to activated surface on map */
        if (cs->role && cs->shell.surface && cs->shell.activated && (!cs->shell.popup))
          {
             comp_seats_redo_enter(cs->c, cs);
             shell_surface_aspect_update(cs);
             shell_surface_minmax_update(cs);
          }
     }

   if (state->attach && state->buffer)
     {
        evas_object_move(cs->img, x + buffer->x, y + buffer->y);
        evas_object_resize(cs->obj, buffer->w, buffer->h);
     }
   else if (cs->shell.new && (!newly_new))
     shell_surface_init(cs);

   state->attach = 0;
   state->buffer = NULL;

   cs->frames = eina_list_merge(cs->frames, state->frames);
   state->frames = NULL;

   if (eina_tiler_empty(state->damages))
     {
        comp_surface_buffer_detach(&buffer);
        comp_surface_buffer_post_render(cs);
        if (!cs->post_render_queue)
          {
             evas_object_image_pixels_dirty_set(cs->img, 0);
             EINA_LIST_FOREACH(cs->proxies, l, o)
               evas_object_image_pixels_dirty_set(o, 0);
          }
     }
   else if (buffer)
     {
        Eina_Iterator *it;
        Eina_Rectangle *rect;

        comp_surface_commit_image_state(cs, buffer, cs->img);
        EINA_LIST_FOREACH(cs->proxies, l, o)
          comp_surface_commit_image_state(cs, buffer, o);

        it = eina_tiler_iterator_new(state->damages);
        EINA_ITERATOR_FOREACH(it, rect)
          {
               //if (cs->subsurface) fprintf(stderr, "BUFFER(%d) DAMAGE %d\n", wl_resource_get_id(buffer->res), wl_resource_get_id(cs->res));
             evas_object_image_data_update_add(cs->img, rect->x, rect->y, rect->w, rect->h);
             EINA_LIST_FOREACH(cs->proxies, l, o)
               evas_object_image_data_update_add(o, rect->x, rect->y, rect->w, rect->h);
          }
        eina_iterator_free(it);
        if (!cs->render_queue)
          cs->c->render_queue = eina_list_append(cs->c->render_queue, cs);
        cs->render_queue = 1;
     }
   eina_tiler_clear(state->damages);

   if (state->set_opaque && (!eina_tiler_equal(cs->opaque, state->opaque)))
     {
        array_clear(&cs->opaque_rects);
        if (!eina_tiler_empty(state->opaque))
          /* FIXME: proxied opaque regions */
          {
             Eina_Iterator *it;
             Eina_Rectangle *rect;
             Evas_Object *r;

             it = eina_tiler_iterator_new(state->opaque);
             cs->opaque_rects  = eina_array_new(1);
             EINA_ITERATOR_FOREACH(it, rect)
               {
                  r = evas_object_rectangle_add(cs->c->evas);
                  evas_object_name_set(r, "opaque_rect");
                  evas_object_pass_events_set(r, 1);
                  evas_object_color_set(r, 0, 0, 0, 255);
                  evas_object_smart_member_add(r, cs->obj);
                  evas_object_geometry_set(r, x + rect->x, y + rect->y, rect->w, rect->h);
                  evas_object_stack_below(r, cs->img);
                  evas_object_show(r);
                  eina_array_push(cs->opaque_rects, r);
               }
             /* FIXME: maybe use image border here */

             eina_iterator_free(it);
          }
        PTR_SWAP(&cs->opaque, &state->opaque);
     }
   eina_tiler_clear(state->opaque);
   state->set_opaque = 0;

   if (state->set_input)
     {
        if (eina_tiler_empty(state->input))
          {
             array_clear(&cs->input_rects);
             evas_object_pass_events_set(cs->img, 0);
             evas_object_pointer_mode_set(cs->img, EVAS_OBJECT_POINTER_MODE_NOGRAB);
          }
        else if (!eina_tiler_equal(cs->input, state->input))
          {
             Eina_Iterator *it;
             Eina_Rectangle *rect;
             Evas_Object *r;

             array_clear(&cs->input_rects);
             it = eina_tiler_iterator_new(state->input);
             cs->input_rects  = eina_array_new(1);
             EINA_ITERATOR_FOREACH(it, rect)
               {
                  r = evas_object_rectangle_add(cs->c->evas);
                  evas_object_name_set(r, "input_rect");
                  evas_object_color_set(r, 0, 0, 0, 0);
                  evas_object_smart_member_add(r, cs->obj);
                  evas_object_geometry_set(r, x + rect->x, y + rect->y, rect->w, rect->h);
                  evas_object_stack_above(r, cs->img);
                  evas_object_pointer_mode_set(r, EVAS_OBJECT_POINTER_MODE_NOGRAB);
                  evas_object_show(r);
                  evas_object_show(r);
                  eina_array_push(cs->input_rects, r);
               }
             evas_object_pass_events_set(cs->img, 1);
             eina_iterator_free(it);
          }
        PTR_SWAP(&cs->input, &state->input);
     }
   eina_tiler_clear(state->input);
   state->set_input = 0;

   if (cs->pending_subsurfaces)
     {
        cs->subsurfaces = eina_list_free(cs->subsurfaces);
        PTR_SWAP(&cs->subsurfaces, &cs->pending_subsurfaces);
     }
   if (cs->subsurface)
     {
        if (cs->subsurface->set_offset)
          {
             cs->subsurface->offset.x = cs->subsurface->pending_offset.x;
             cs->subsurface->offset.y = cs->subsurface->pending_offset.y;
             cs->subsurface->pending_offset.x = cs->subsurface->pending_offset.y = 0;
             cs->subsurface->set_offset = 0;
             evas_object_geometry_get(cs->parent->obj, &x, &y, NULL, NULL);
             evas_object_move(cs->obj, x + cs->subsurface->offset.x, y + cs->subsurface->offset.y);
          }
     }
}

static void
comp_surface_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   if (cs)
     {
        if (cs->post_render_queue)
          {
             cs->dead = 1;
             evas_object_pass_events_set(cs->obj, 1);
             return;
          }
        cs->res = NULL;
     }
   wl_resource_destroy(resource);
}

static void
comp_surface_buffer_destroy(struct wl_listener *listener, void *data EINA_UNUSED)
{
   Comp_Buffer *buffer;
   Comp_Surface *cs;

   buffer = container_of(listener, Comp_Buffer, destroy_listener);
   cs = buffer->cs;
   if (cs)
     {
        if (cs->buffer[0] == buffer) cs->buffer[0] = NULL;
        else if (cs->buffer[1] == buffer) cs->buffer[1] = NULL;
        else if (cs->pending.buffer == buffer) cs->pending.buffer = NULL;
        else if (cs->subsurface)
          {
             if (cs->subsurface->cache.buffer == buffer) cs->subsurface->cache.buffer = NULL;
          }
     }
   free(buffer);
}

static void
comp_surface_attach(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *buffer_resource, int32_t x, int32_t y)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   Comp_Buffer *buffer;
   struct wl_shm_buffer *shmbuff;
   struct linux_dmabuf_buffer *dmabuf;

   if (cs->shell.new)
     {
        wl_resource_post_error(cs->shell.surface, XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER,
                               "buffer attached/committed before configure");
        return;
     }

   comp_surface_buffer_detach(&cs->pending.buffer);
   cs->pending.attach = 1;
   if (!buffer_resource) return;

   buffer = calloc(1, sizeof(Comp_Buffer));
   if (cs->subsurface)
     {
        //fprintf(stderr, "BUFFER(%d) HELD BY %d\n", wl_resource_get_id(buffer_resource), wl_resource_get_id(resource));
        buffer->dbg = 1;
     }
   buffer->cs = cs;
   buffer->x = x;
   buffer->y = y;
   shmbuff = wl_shm_buffer_get(buffer_resource);
   dmabuf = linux_dmabuf_buffer_get(buffer_resource);
   if (shmbuff)
     {
        buffer->w = wl_shm_buffer_get_width(shmbuff);
        buffer->h = wl_shm_buffer_get_height(shmbuff);
     }
   else if (dmabuf)
     {
        buffer->w = dmabuf->attributes.width;
        buffer->h = dmabuf->attributes.height;
     }
   else if (cs->c->gl)
     {
        cs->c->glapi->evasglQueryWaylandBuffer(cs->c->gl, buffer_resource, EGL_WIDTH, &buffer->w);
        cs->c->glapi->evasglQueryWaylandBuffer(cs->c->gl, buffer_resource, EGL_HEIGHT, &buffer->h);
     }
   buffer->shm_buffer = shmbuff;
   buffer->dmabuf_buffer = dmabuf;

   buffer->res = buffer_resource;
   buffer->destroy_listener.notify = comp_surface_buffer_destroy;
   wl_resource_add_destroy_listener(buffer_resource, &buffer->destroy_listener);
   cs->pending.buffer = buffer;
}

static void
comp_surface_damage_buffer(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   eina_tiler_rect_add(cs->pending.damages, &(Eina_Rectangle){x, y, w, h});
}

/*
 * Currently damage and damage_buffer are the same because we don't support
 * buffer_scale, transform, or viewport.  Once we support those we'll have
 * to make surface_cb_damage handle damage in surface co-ordinates.
 */
static void
comp_surface_damage(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   comp_surface_damage_buffer(client, resource, x, y, w, h);
}

static void
comp_surface_frame_impl_destroy(struct wl_resource *resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);

   if (!cs) return;
   if (cs->frames)
     cs->frames = eina_list_remove(cs->frames, resource);
   if (cs->pending.frames)
     cs->pending.frames = eina_list_remove(cs->pending.frames, resource);

   if (cs->subsurface && cs->subsurface->cached)
     cs->subsurface->cache.frames = eina_list_remove(cs->subsurface->cache.frames, resource);
}

static void
comp_surface_frame(struct wl_client *client, struct wl_resource *resource, uint32_t callback)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   struct wl_resource *res;

   res = wl_resource_create(client, &wl_callback_interface, 1, callback);
   wl_resource_set_implementation(res, NULL, cs, comp_surface_frame_impl_destroy);
   cs->pending.frames = eina_list_append(cs->pending.frames, res);
}

static void
comp_surface_set_opaque_region(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *region_resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);

   cs->pending.set_opaque = 1;
   eina_tiler_clear(cs->pending.opaque);
   if (region_resource)
     eina_tiler_union(cs->pending.opaque, wl_resource_get_user_data(region_resource));
}

static void
comp_surface_set_input_region(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *region_resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);

   if (cs->cursor) return;

   cs->pending.set_input = 1;
   eina_tiler_clear(cs->pending.input);
   if (region_resource)
     eina_tiler_union(cs->pending.input, wl_resource_get_user_data(region_resource));
}

static void
subcomp_subsurface_cache_merge(Comp_Subsurface *css)
{
   //fprintf(stderr, "CACHE MERGE\n");
   css->cached = 1;
   if (css->cache.frames || css->surface->pending.frames)
     css->cache.frames = eina_list_merge(css->cache.frames, css->surface->pending.frames);
   css->surface->pending.frames = NULL;
   eina_tiler_union(css->cache.damages, css->surface->pending.damages);
   eina_tiler_clear(css->surface->pending.damages);
   css->cache.set_input = css->surface->pending.set_input;
   if (css->surface->pending.set_input)
     {
        eina_tiler_clear(css->cache.input);
        PTR_SWAP(&css->cache.input, &css->surface->pending.input);
     }
   css->cache.set_opaque = css->surface->pending.set_opaque;
   if (css->surface->pending.set_opaque)
     {
        eina_tiler_clear(css->cache.opaque);
        PTR_SWAP(&css->cache.opaque, &css->surface->pending.opaque);
     }
   css->surface->pending.set_input = 0;
   css->surface->pending.set_opaque = 0;
   if (!css->surface->pending.attach) return;
   css->cache.attach = 1;
   comp_surface_buffer_detach(&css->cache.buffer);
   PTR_SWAP(&css->cache.buffer, &css->surface->pending.buffer);
   css->surface->pending.attach = 0;
}

static void
comp_surface_commit(struct wl_client *client, struct wl_resource *resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   Comp_Subsurface *css;
   Eina_List *l;
   Comp_Buffer_State *cbs = &cs->pending;

   if (cs->shell.popup && (!cs->parent))
     {
        wl_resource_post_error(cs->shell.surface, XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
                               "popup surface has no parent");
        return;
     }

   cs->commit = 1;
   if (cs->subsurface)
     {
        Comp_Surface *parent;

        css = cs->subsurface;
        parent = cs->parent;
        if ((!parent->commit) && css->sync)
          {
             subcomp_subsurface_cache_merge(css);
             return;
          }
        while (parent && (!parent->commit) && parent->subsurface)
          {
             Comp_Subsurface *pss = parent->subsurface;

             if (pss->sync)
               {
                  subcomp_subsurface_cache_merge(css);
                  return;
               }
             parent = parent->parent;
          }


        subcomp_subsurface_cache_merge(css);
        cbs = &css->cache;
     }

   comp_surface_commit_state(cs, cbs);
   EINA_LIST_FOREACH(cs->subsurfaces, l, css)
     if (css->sync) comp_surface_commit(client, css->surface->res);
   cs->commit = 0;
}

static void
comp_surface_set_buffer_transform(struct wl_client *client, struct wl_resource *resource, int32_t transform)
{
   //Comp_Surface *cs = wl_resource_get_user_data(resource);
}

static void
comp_surface_set_buffer_scale(struct wl_client *client, struct wl_resource *resource, int32_t scale)
{
   //Comp_Surface *cs = wl_resource_get_user_data(resource);
}

static const struct wl_surface_interface comp_surface_interface =
{
   comp_surface_destroy,
   comp_surface_attach,
   comp_surface_damage,
   comp_surface_frame,
   comp_surface_set_opaque_region,
   comp_surface_set_input_region,
   comp_surface_commit,
   comp_surface_set_buffer_transform,
   comp_surface_set_buffer_scale,
   comp_surface_damage_buffer
};

static void
comp_surface_impl_destroy(struct wl_resource *resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   Eina_List *subsurfaces;
   Comp_Subsurface *css;
   Comp_Seat *s;
   struct wl_client *client = wl_resource_get_client(resource);

   EINA_INLIST_FOREACH(cs->c->seats, s)
     {
        if (s->kbd.enter == cs) s->kbd.enter = NULL;
        if (s->ptr.enter == cs) s->ptr.enter = NULL;
        if (s->ptr.cursor.surface == cs)
          {
             if (s->ptr.in)
               {
                  const Eina_List *l;
                  Eo *dev;
                  Ecore_Evas *ee = ecore_evas_ecore_evas_get(s->c->evas);
                  EINA_LIST_FOREACH(evas_device_list(s->c->evas, s->dev), l, dev)
                    if (evas_device_class_get(dev) == EVAS_DEVICE_CLASS_MOUSE)
                      ecore_evas_cursor_device_unset(ee, dev);
               }
             s->ptr.cursor.surface = NULL;
          }
        if (s->drag.surface == cs) s->drag.surface = NULL;
     }
   eina_hash_list_remove(cs->c->client_surfaces, &client, cs);
   if (cs->render_queue)
     cs->c->render_queue = eina_list_remove(cs->c->render_queue, cs);
   subsurfaces = cs->pending_subsurfaces ?: cs->subsurfaces;
   EINA_LIST_FREE(subsurfaces, css)
     {
        evas_object_hide(css->surface->obj);
        comp_surface_reparent(css->surface, NULL);
     }
   if (cs->pending_subsurfaces) eina_list_free(cs->subsurfaces);
   cs->pending_subsurfaces = cs->subsurfaces = NULL;
   comp_surface_buffer_detach(&cs->buffer[0]);
   cs->res = NULL;
   if (cs->post_render_queue && (!cs->dead))
     {
        Eina_List *l;
        Evas_Object *o;

        cs->dead = 1;
        evas_object_hide(cs->obj);
        EINA_LIST_FOREACH(cs->proxies, l, o)
          evas_object_hide(o);
     }
   else
     {
        comp_surface_buffer_detach(&cs->buffer[1]);
        if (!cs->dead) evas_object_del(cs->obj);
     }
}


static Evas_Smart *comp_surface_smart = NULL;

static inline Eina_Bool
comp_surface_check_grab(Comp_Surface *cs, Comp_Seat *s)
{
   Comp_Surface *parent;
   if (!s->grab) return EINA_TRUE;
   if (cs == s->grab) return EINA_TRUE;
   parent = s->grab->parent;
   while (parent)
     {
        if (cs == parent) return EINA_TRUE;
        parent = parent->parent;
     }
   return EINA_FALSE;
}

static void
comp_surface_input_event(Eina_Inlist **list, uint32_t id, uint32_t serial, uint32_t time, Eina_Bool up)
{
   Input_Sequence *ev;

   if (up)
     {
        EINA_INLIST_FOREACH(*list, ev)
          if (ev->id == id)
            {
               ev->up_serial = serial;
               ev->up_time = time;
               return;
            }
        return;
     }
   ev = calloc(1, sizeof(Input_Sequence));
   ev->id = id;
   ev->down_serial = serial;
   ev->down_time = time;
   *list = eina_inlist_append(*list, EINA_INLIST_GET(ev));
}

static void
comp_surface_send_data_device_enter(Comp_Surface *cs, Comp_Seat *s)
{
   struct wl_resource *offer = NULL;
   int x, y, cx, cy;
   uint32_t serial;
   struct wl_resource *res = data_device_find(s, cs->res);


   if (!res) return;
   evas_object_geometry_get(cs->obj, &x, &y, NULL, NULL);
   if (s->drag.tch)
     cx = s->tch.pos.x, cy = s->tch.pos.y;
   else
     cx = s->ptr.pos.x, cy = s->ptr.pos.y;
   if (s->drag.source)
     {
        data_device_offer_create(s->drag.source, res);
        s->drag.source->offer->type = COMP_DATA_DEVICE_OFFER_TYPE_DND;
        offer = s->drag.source->offer->res;
     }
   s->drag.enter = cs;
   s->ptr.enter_serial = serial = wl_display_next_serial(cs->c->display);
   wl_data_device_send_enter(res, serial, cs->res,
       wl_fixed_from_int(cx - x), wl_fixed_from_int(cy - y), offer);
}

static Eina_Bool
comp_surface_send_pointer_enter(Comp_Surface *cs, Comp_Seat *s, int cx, int cy)
{
   Eina_List *l, *ll;
   struct wl_resource *res;
   uint32_t serial;
   int x, y;

   if (s->ptr.enter && (cs != s->grab)) return EINA_FALSE;
   if (!comp_surface_check_grab(cs, s)) return EINA_FALSE;
   s->ptr.enter = cs;
   if (cs->dead) return EINA_FALSE;
   if (s->drag.res && (!s->drag.tch))
     {
        comp_surface_send_data_device_enter(cs, s);
        return EINA_TRUE;
     }
   l = seat_ptr_resources_get(s, wl_resource_get_client(cs->res));
   if (!l) return EINA_FALSE;
   s->ptr.enter_serial = serial = wl_display_next_serial(cs->c->display);
   //fprintf(stderr, "ENTER %s\n", cs->shell.popup ? "POPUP" : "TOPLEVEL");
   evas_object_geometry_get(cs->obj, &x, &y, NULL, NULL);
   EINA_LIST_FOREACH(l, ll, res)
     wl_pointer_send_enter(res, serial, cs->res,
       wl_fixed_from_int(cx - x), wl_fixed_from_int(cy - y));
   return EINA_TRUE;
}

static void
comp_surface_mouse_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_In *ev = event_info;
   Comp_Seat *s;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   s = seat_find(data, ev->dev);
   if (comp_surface_send_pointer_enter(data, s, ev->canvas.x, ev->canvas.y))
     {
        s->event_propagate = 1;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
}

static void
comp_surface_send_data_device_leave(Comp_Surface *cs, Comp_Seat *s)
{
   struct wl_resource *res = data_device_find(s, cs->res);
   if (!res) return;
   if (s->drag.source)
     {
        s->drag.source->offer->source = NULL;
        s->drag.source->offer = NULL;
     }
   s->drag.enter = NULL;
   wl_data_device_send_leave(res);
}

static Eina_Bool
comp_surface_send_pointer_leave(Comp_Surface *cs, Comp_Seat *s)
{
   Eina_List *l, *ll;
   struct wl_resource *res;
   uint32_t serial;

   if (s->ptr.enter != cs) return EINA_FALSE;
   if (!comp_surface_check_grab(cs, s)) return EINA_FALSE;
   s->ptr.enter = NULL;
   if (cs->dead) return EINA_FALSE;
   if (s->drag.res)
     {
        comp_surface_send_data_device_leave(cs, s);
        return EINA_TRUE;
     }
   l = seat_ptr_resources_get(s, wl_resource_get_client(cs->res));
   if (!l) return EINA_FALSE;
   serial = wl_display_next_serial(cs->c->display);
   //fprintf(stderr, "LEAVE %s\n", cs->shell.popup ? "POPUP" : "TOPLEVEL");
   EINA_LIST_FOREACH(l, ll, res)
     wl_pointer_send_leave(res, serial, cs->res);
   return EINA_TRUE;
}

static void
comp_surface_mouse_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Out *ev = event_info;
   Comp_Seat *s;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   s = seat_find(data, ev->dev);
   if (comp_surface_send_pointer_leave(data, s))
     {
        s->event_propagate = 1;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
}

static Eina_Bool
comp_surface_mouse_button(Comp_Surface *cs, Comp_Seat *s, uint32_t timestamp, uint32_t button_id, uint32_t state)
{
   uint32_t serial, btn;
   Eina_List *l, *ll;
   struct wl_resource *res;

   switch (button_id)
     {
      case 1:
        btn = BTN_LEFT;
        break;
      case 2:
        btn = BTN_MIDDLE;
        break;
      case 3:
        btn = BTN_RIGHT;
        break;
      case 4:
        btn = BTN_SIDE;
        break;
      case 5:
        btn = BTN_EXTRA;
        break;
      case 6:
        btn = BTN_FORWARD;
        break;
      case 7:
        btn = BTN_BACK;
        break;
      default:
        btn = button_id + BTN_SIDE - 8;
        break;
     }
   if (s->ptr.enter != cs) return EINA_FALSE;
   if (!comp_surface_check_grab(cs, s)) return EINA_FALSE;
   if (state == WL_POINTER_BUTTON_STATE_PRESSED)
     s->ptr.button_mask |= 1 << button_id;
   else
     {
        if (!(s->ptr.button_mask & (1 << button_id))) return EINA_FALSE;
        s->ptr.button_mask &= ~(1 << button_id);
        if (s->drag.res && (!s->drag.tch))
          {
             drag_grab_button(s, timestamp, button_id, WL_POINTER_BUTTON_STATE_RELEASED);
             comp_surface_input_event(&s->ptr.events, button_id, 0, timestamp, state == WL_POINTER_BUTTON_STATE_RELEASED);
             s->ptr.enter = NULL;
             comp_surface_send_pointer_enter(cs, s, s->ptr.pos.x, s->ptr.pos.y);
             return EINA_TRUE;
          }
     }

   if (cs->dead)
     {
        comp_surface_input_event(&s->ptr.events, button_id, 0, timestamp, state == WL_POINTER_BUTTON_STATE_RELEASED);
        return EINA_TRUE;
     }

   l = seat_ptr_resources_get(s, wl_resource_get_client(cs->res));
   if (!l) return EINA_FALSE;
   serial = wl_display_next_serial(s->c->display);
   comp_surface_input_event(&s->ptr.events, button_id, serial, timestamp, state == WL_POINTER_BUTTON_STATE_RELEASED);

   EINA_LIST_FOREACH(l, ll, res)
     wl_pointer_send_button(res, serial, timestamp, btn, state);
   return EINA_TRUE;
}

static void
comp_surface_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Comp_Seat *s;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   s = seat_find(data, ev->dev);
   if (comp_surface_mouse_button(data, s, ev->timestamp, ev->button, WL_POINTER_BUTTON_STATE_PRESSED))
     {
        s->event_propagate = 1;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
}

static void
comp_surface_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Comp_Seat *s;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   s = seat_find(data, ev->dev);
   if (comp_surface_mouse_button(data, s, ev->timestamp, ev->button, WL_POINTER_BUTTON_STATE_RELEASED))
     {
        s->event_propagate = 1;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
}

static void
comp_surface_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Comp_Surface *cs = data;
   Eina_List *l = NULL, *ll;
   struct wl_resource *res;
   int x, y;
   Comp_Seat *s;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (cs->dead) return;
   s = seat_find(data, ev->dev);
   if (s->ptr.enter != cs) return;
   if (!comp_surface_check_grab(cs, s)) return;
   if (s->drag.res)
     {
        if (s->drag.enter != cs) return;
        res = data_device_find(s, cs->res);
        if (!res) return;
     }
   else
     {
        l = seat_ptr_resources_get(s, wl_resource_get_client(cs->res));
        if (!l) return;
     }
   evas_object_geometry_get(cs->obj, &x, &y, NULL, NULL);
   //fprintf(stderr, "MOTION %s\n", cs->shell.popup ? "POPUP" : "TOPLEVEL");
   if (s->drag.res)
     wl_data_device_send_motion(res, ev->timestamp,
            wl_fixed_from_int(ev->cur.canvas.x - x), wl_fixed_from_int(ev->cur.canvas.y - y));
   else
     {
      //int n = 0;
        EINA_LIST_FOREACH(l, ll, res)
          {
           //fprintf(stderr, "motion %d\n", n++);
             wl_pointer_send_motion(res, ev->timestamp,
               wl_fixed_from_int(ev->cur.canvas.x - x), wl_fixed_from_int(ev->cur.canvas.y - y));
          }
     }
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   s->event_propagate = 1;
}

static void
comp_surface_mouse_wheel(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Evas_Event_Mouse_Wheel *ev = event;
   Comp_Surface *cs = data;
   Eina_List *l, *ll;
   struct wl_resource *res;
   uint32_t axis, dir;
   Comp_Seat *s;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (ev->direction == 0)
     axis = WL_POINTER_AXIS_VERTICAL_SCROLL;
   else
     axis = WL_POINTER_AXIS_HORIZONTAL_SCROLL;

   if (ev->z < 0)
     dir = -wl_fixed_from_int(abs(10 * ev->z));
   else
     dir = wl_fixed_from_int(10 * ev->z);

   if (cs->dead) return;
   s = seat_find(data, ev->dev);
   if (s->ptr.enter != cs) return;
   if (!comp_surface_check_grab(cs, s)) return;
   l = seat_ptr_resources_get(s, wl_resource_get_client(cs->res));
   if (!l) return;
   EINA_LIST_FOREACH(l, ll, res)
     wl_pointer_send_axis(res, ev->timestamp, axis, dir);
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   s->event_propagate = 1;
}

static void
comp_surface_multi_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Multi_Down *ev = event_info;
   Comp_Surface *cs = data;
   Eina_List *l, *ll;
   struct wl_resource *res;
   uint32_t serial;
   int x, y;
   Comp_Seat *s;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (cs->dead) return;
   s = seat_find(data, ev->dev);
   if (!comp_surface_check_grab(cs, s)) return;
   s->tch.enter = cs;
   l = seat_tch_resources_get(s, wl_resource_get_client(cs->res));
   if (!l)
     {
        comp_surface_input_event(&s->tch.events, ev->device, 0, ev->timestamp, 0);
        s->event_propagate = 1;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return;
     }
   evas_object_geometry_get(cs->obj, &x, &y, NULL, NULL);
   serial = wl_display_next_serial(cs->c->display);
   comp_surface_input_event(&s->tch.events, ev->device, serial, ev->timestamp, 0);
   EINA_LIST_FOREACH(l, ll, res)
     wl_touch_send_down(res, serial, ev->timestamp, cs->res, ev->device,
       wl_fixed_from_int(ev->canvas.x - x), wl_fixed_from_int(ev->canvas.y - y));
   s->event_propagate = 1;
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static void
comp_surface_multi_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Multi_Up *ev = event_info;
   Comp_Surface *cs = data;
   Eina_List *l, *ll;
   struct wl_resource *res;
   uint32_t serial;
   Comp_Seat *s;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (cs->dead) return;
   s = seat_find(data, ev->dev);
   if (!comp_surface_check_grab(cs, s)) return;
   if (!ev->device)
     s->tch.enter = NULL;
   l = seat_tch_resources_get(s, wl_resource_get_client(cs->res));
   if ((!l) || (s->drag.tch && ((uint32_t)ev->device == s->drag.id)))
     {
        if (s->drag.tch)
          {
             res = data_device_find(s, cs->res);
             if (!res) return;

             wl_data_device_send_drop(res);
          }
        comp_surface_input_event(&s->tch.events, ev->device, 0, ev->timestamp, 1);
        s->event_propagate = 1;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return;
     }
   serial = wl_display_next_serial(cs->c->display);
   comp_surface_input_event(&s->tch.events, ev->device, serial, ev->timestamp, 1);
   EINA_LIST_FOREACH(l, ll, res)
     wl_touch_send_up(res, serial, ev->timestamp, ev->device);
   s->event_propagate = 1;
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static void
comp_surface_multi_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Multi_Move *ev = event_info;
   Comp_Surface *cs = data;
   struct wl_resource *res;
   int x, y;
   Comp_Seat *s;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (cs->dead) return;
   s = seat_find(data, ev->dev);
   if (!comp_surface_check_grab(cs, s)) return;
   evas_object_geometry_get(cs->obj, &x, &y, NULL, NULL);
   if (s->drag.tch)
     {
        if (s->drag.enter != cs) return;
        res = data_device_find(s, cs->res);
        if (res)
          {
             wl_data_device_send_motion(res, ev->timestamp,
               wl_fixed_from_int(ev->cur.canvas.x - x), wl_fixed_from_int(ev->cur.canvas.y - y));
             s->event_propagate = 1;
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
          }
        return;
     }
   else
     {
        Eina_List *l, *ll;

        l = seat_tch_resources_get(s, wl_resource_get_client(cs->res));
        if (!l) return;
        EINA_LIST_FOREACH(l, ll, res)
          wl_touch_send_motion(res, ev->timestamp, ev->device,
            wl_fixed_from_int(ev->cur.canvas.x - x), wl_fixed_from_int(ev->cur.canvas.y - y));
        s->event_propagate = 1;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
}

static void
comp_surface_smart_add(Evas_Object *obj)
{
   Comp_Surface *cs;
   Evas *e;

   cs = calloc(1, sizeof(Comp_Surface));
   evas_object_smart_data_set(obj, cs);
   cs->obj = obj;
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_MOUSE_DOWN, comp_surface_mouse_down, cs);
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_MOUSE_UP, comp_surface_mouse_up, cs);
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_MOUSE_IN, comp_surface_mouse_in, cs);
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_MOUSE_OUT, comp_surface_mouse_out, cs);
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_MOUSE_MOVE, comp_surface_mouse_move, cs);
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_MOUSE_WHEEL, comp_surface_mouse_wheel, cs);
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_MULTI_DOWN, comp_surface_multi_down, cs);
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_MULTI_UP, comp_surface_multi_up, cs);
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_MULTI_MOVE, comp_surface_multi_move, cs);
   e = evas_object_evas_get(obj);
   cs->img = evas_object_image_filled_add(e);
   evas_object_show(cs->img);
   cs->clip = evas_object_rectangle_add(e);

   evas_object_smart_member_add(cs->img, cs->obj);
   evas_object_smart_member_add(cs->clip, cs->obj);
   evas_object_image_border_center_fill_set(cs->img, EVAS_BORDER_FILL_SOLID);
   evas_object_image_colorspace_set(cs->img, EVAS_COLORSPACE_ARGB8888);
}

static void
comp_surface_smart_del(Evas_Object *obj)
{
   Comp_Surface *cs = evas_object_smart_data_get(obj);

   array_clear(&cs->input_rects);
   array_clear(&cs->opaque_rects);
   eina_tiler_free(cs->opaque);
   eina_tiler_free(cs->input);
   comp_buffer_state_clear(&cs->pending);
   while (cs->proxies)
     evas_object_del(eina_list_data_get(cs->proxies));
   if (cs->res)
     {
        cs->dead = 1;
        wl_resource_destroy(cs->res);
     }
   evas_object_del(cs->img);
   evas_object_del(cs->clip);
   if (cs->shell.surface)
     {
        if (cs->role)
          wl_resource_destroy(cs->role);
        wl_resource_destroy(cs->shell.surface);
     }
   cs->c->surfaces = eina_inlist_remove(cs->c->surfaces, EINA_INLIST_GET(cs));
   cs->c->surfaces_count--;
   free(cs);
}

static void
comp_surface_smart_move(Evas_Object *obj, int x, int y)
{
   Eina_List *l;
   Evas_Object *o;
   int px, py, cx, cy;

   evas_object_geometry_get(obj, &px, &py, NULL, NULL);
   //{
      //Comp_Surface *cs = evas_object_smart_data_get(obj);
      //if (cs->cursor)
      //fprintf(stderr, "COMP %sSURFACE(%p) %d,%d\n", cs->subsurface ? "SUB" : "", cs, x, y);
   //}
   l = evas_object_smart_members_get(obj);
   EINA_LIST_FREE(l, o)
     {
        evas_object_geometry_get(o, &cx, &cy, NULL, NULL);
        evas_object_move(o, x + (cx - px), y + (cy - py));
        //fprintf(stderr, "SUBOBJ %d,%d\n", x + (cx - px), y + (cy - py));
     }
}

static void
comp_surface_smart_resize(Evas_Object *obj, int w, int h)
{
   Comp_Surface *cs = evas_object_smart_data_get(obj);
   evas_object_resize(cs->clip, w, h);
   //if (cs->cursor) fprintf(stderr, "COMP %sSURFACE(%p) %dx%d\n", cs->subsurface ? "SUB" : "", cs, w, h);
   if (cs->drag)
     evas_object_move(cs->obj, cs->drag->ptr.pos.x, cs->drag->ptr.pos.y);
}

static void
comp_surface_smart_show(Evas_Object *obj)
{
   Comp_Surface *cs = evas_object_smart_data_get(obj);
   evas_object_show(cs->clip);
   cs->mapped = 1;
}

static void
comp_surface_smart_hide(Evas_Object *obj)
{
   Comp_Surface *pcs = NULL, *lcs, *cs = evas_object_smart_data_get(obj);

   evas_object_hide(cs->clip);
   cs->mapped = 0;

   if (!cs->shell.activated) return;
   cs->shell.activated = 0;
   if (cs->shell.popup && cs->role)
     xdg_popup_send_popup_done(cs->role);
   if (cs->parent && cs->shell.popup) return;
   /* attempt to revert focus based on stacking order */
   if (cs->parent)
     {
        EINA_INLIST_REVERSE_FOREACH(cs->parent->children, lcs)
          {
             if (lcs == cs) continue;
             if (!evas_object_visible_get(lcs->obj)) continue;
             if ((!lcs->shell.surface) || (!lcs->role)) continue;
             lcs->shell.activated = 1;
             if (lcs->shell.popup)
               {
                  if (!lcs->extracted)
                    evas_object_raise(lcs->obj);
               }
             else
               shell_surface_send_configure(lcs);
             return;
          }
        if (!cs->parent->shell.popup)
          {
             pcs = cs->parent;
             if (!pcs->mapped) pcs = NULL;
          }
     }
   if (cs->c->seats)
     comp_seats_redo_enter(cs->c, pcs);
}

static void
comp_surface_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   Comp_Surface *cs = evas_object_smart_data_get(obj);
   evas_object_clip_set(cs->clip, clip);
}

static void
comp_surface_smart_clip_unset(Evas_Object *obj)
{
   Comp_Surface *cs = evas_object_smart_data_get(obj);
   evas_object_clip_unset(cs->clip);
}

static void
comp_surface_smart_member_add(Evas_Object *obj, Evas_Object *child)
{
   Comp_Surface *cs = evas_object_smart_data_get(obj);
   if (child != cs->clip) evas_object_clip_set(child, cs->clip);
}

static void
comp_surface_smart_member_del(Evas_Object *obj, Evas_Object *child)
{
   Comp_Surface *cs = evas_object_smart_data_get(obj);
   if (child != cs->clip) evas_object_clip_unset(child);
}

static void
comp_surface_smart_init(void)
{
   if (comp_surface_smart) return;
   {
      static const Evas_Smart_Class sc =
      {
         "comp_surface",
         EVAS_SMART_CLASS_VERSION,
         comp_surface_smart_add,
         comp_surface_smart_del,
         comp_surface_smart_move,
         comp_surface_smart_resize,
         comp_surface_smart_show,
         comp_surface_smart_hide,
         NULL, //color_set
         comp_surface_smart_clip_set,
         comp_surface_smart_clip_unset,
         NULL,
         comp_surface_smart_member_add,
         comp_surface_smart_member_del,

         NULL,
         NULL,
         NULL,
         NULL
      };
      comp_surface_smart = evas_smart_class_new(&sc);
   }
}

static void
comp_surface_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   struct wl_resource *res;
   Comp_Surface *cs;
   Comp *c = wl_resource_get_user_data(resource);
   Evas_Object *obj;
   int x, y;

   res = wl_resource_create(client, &wl_surface_interface, wl_resource_get_version(resource), id);
   comp_surface_smart_init();
   obj = evas_object_smart_add(c->evas, comp_surface_smart);
   cs = evas_object_smart_data_get(obj);
   cs->res = res;
   evas_object_smart_member_add(cs->obj, c->obj);
   cs->c = c;
   evas_object_geometry_get(c->obj, &x, &y, NULL, NULL);
   evas_object_move(cs->obj, x, y);

   c->surfaces = eina_inlist_prepend(c->surfaces, EINA_INLIST_GET(cs));
   c->surfaces_count++;
   eina_hash_list_append(c->client_surfaces, &client, cs);
   if (evas_object_visible_get(cs->c->clip))
     comp_surface_output_enter(cs);
   else
     comp_surface_output_leave(cs);

   cs->opaque = tiler_new();
   cs->input = tiler_new();
   comp_buffer_state_alloc(&cs->pending);
   cs->pending.set_input = 1;
   eina_tiler_rect_add(cs->pending.input, &(Eina_Rectangle){0, 0, 65535, 65535});

   wl_resource_set_implementation(res, &comp_surface_interface, cs, comp_surface_impl_destroy);
}

/////////////////////////////////////////////////////////////////

static void
comp_region_add(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   Eina_Tiler *tiler = wl_resource_get_user_data(resource);
   eina_tiler_rect_add(tiler, &(Eina_Rectangle){x, y, w, h});
}

static void
comp_region_subtract(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   Eina_Tiler *tiler = wl_resource_get_user_data(resource);
   eina_tiler_rect_del(tiler, &(Eina_Rectangle){x, y, w, h});
}

static const struct wl_region_interface comp_region_interface =
{
   resource_destroy,
   comp_region_add,
   comp_region_subtract
};

static void
comp_region_impl_destroy(struct wl_resource *resource)
{
   eina_tiler_free(wl_resource_get_user_data(resource));
}

static void
comp_region_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   Eina_Tiler *tiler;
   struct wl_resource *res;

   tiler = tiler_new();
   res = wl_resource_create(client, &wl_region_interface, 1, id);
   wl_resource_set_implementation(res, &comp_region_interface, tiler, comp_region_impl_destroy);
}

/////////////////////////////////////////////////////////////////

static const struct wl_compositor_interface comp_interface =
{
   comp_surface_create,
   comp_region_create,
};

static void
comp_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   struct wl_resource *res;

   if (!client_allowed_check(data, client)) return;

   res = wl_resource_create(client, &wl_compositor_interface, version, id);
   wl_resource_set_implementation(res, &comp_interface, data, NULL);
}

/////////////////////////////////////////////////////////////////


static void
subcomp_subsurface_impl_destroy(struct wl_resource *resource)
{
   Comp_Subsurface *css = wl_resource_get_user_data(resource);

   evas_object_hide(css->surface->obj);
   if (css->surface->parent)
     {
        css->surface->parent->subsurfaces = eina_list_remove(css->surface->parent->subsurfaces, css);
        if (css->surface->parent->pending_subsurfaces)
          css->surface->parent->pending_subsurfaces = eina_list_remove(css->surface->parent->pending_subsurfaces, css);
     }
   comp_surface_reparent(css->surface, NULL);
   comp_buffer_state_clear(&css->cache);
   comp_surface_buffer_detach(&css->cache.buffer);
   css->surface->subsurface = NULL;
   css->surface->role = NULL;
   free(css);
}

static void
subcomp_subsurface_set_position(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y)
{
   Comp_Subsurface *css = wl_resource_get_user_data(resource);

   css->pending_offset.x = x;
   css->pending_offset.y = y;
   css->set_offset = 1;
}

static void
subcomp_subsurface_place_above(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *sibling_resource)
{
   Comp_Subsurface *css = wl_resource_get_user_data(resource);
   Comp_Subsurface *css2 = wl_resource_get_user_data(sibling_resource);

   if ((!css) || (!css2))
     {
        wl_resource_post_error( (!css) ? resource : sibling_resource,
                               WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "surface is not a valid subsurface");
        return;
     }

   if (!css->surface->parent->pending_subsurfaces)
     css->surface->parent->pending_subsurfaces = eina_list_clone(css->surface->parent->subsurfaces);
   css->surface->parent->pending_subsurfaces = eina_list_remove(css->surface->parent->pending_subsurfaces, css);
   css->surface->parent->pending_subsurfaces = eina_list_append_relative(css->surface->parent->pending_subsurfaces, css, css2);
}

static void
subcomp_subsurface_place_below(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *sibling_resource)
{
   Comp_Subsurface *css = wl_resource_get_user_data(resource);
   Comp_Subsurface *css2 = wl_resource_get_user_data(sibling_resource);

   if ((!css) || (!css2))
     {
        wl_resource_post_error( (!css) ? resource : sibling_resource,
                               WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "surface is not a valid subsurface");
        return;
     }

   if (!css->surface->parent->pending_subsurfaces)
     css->surface->parent->pending_subsurfaces = eina_list_clone(css->surface->parent->subsurfaces);
   css->surface->parent->pending_subsurfaces = eina_list_remove(css->surface->parent->pending_subsurfaces, css);
   css->surface->parent->pending_subsurfaces = eina_list_prepend_relative(css->surface->parent->pending_subsurfaces, css, css2);
}

static void
subcomp_subsurface_set_sync(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   Comp_Subsurface *css = wl_resource_get_user_data(resource);
   css->sync = 1;
}

static void
subcomp_subsurface_set_desync(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   Comp_Subsurface *css = wl_resource_get_user_data(resource);
   css->sync = 0;
}

static const struct wl_subsurface_interface subcomp_subsurface_interface =
{
   resource_destroy,
   subcomp_subsurface_set_position,
   subcomp_subsurface_place_above,
   subcomp_subsurface_place_below,
   subcomp_subsurface_set_sync,
   subcomp_subsurface_set_desync
};

static void
subcomp_subsurface_create(struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *surface_resource, struct wl_resource *parent_resource)
{
   Comp_Subsurface *css;
   Comp_Surface *cs = wl_resource_get_user_data(surface_resource);
   Comp_Surface *pcs = wl_resource_get_user_data(parent_resource);

   if (surface_resource == parent_resource)
     {
        wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE,
          "cannot create subsurface as its own child");
        return;
     }

   if (cs->role)
     {
        wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE,
                               "surface already has a role");
        return;
     }

   css = cs->subsurface = calloc(1, sizeof(Comp_Subsurface));
   comp_buffer_state_alloc(&css->cache);
   css->sync = 1;
   evas_object_name_set(cs->img, "subsurface");
   css->surface = cs;
   if (!pcs->pending_subsurfaces)
     pcs->pending_subsurfaces = eina_list_clone(pcs->subsurfaces);
   pcs->pending_subsurfaces = eina_list_append(pcs->pending_subsurfaces, css);
   comp_surface_reparent(cs, pcs);

   cs->role = wl_resource_create(client, &wl_subsurface_interface, 1, id);
   wl_resource_set_implementation(cs->role, &subcomp_subsurface_interface, css, subcomp_subsurface_impl_destroy);
}

static const struct wl_subcompositor_interface subcomp_interface =
{
   resource_destroy,
   subcomp_subsurface_create,
};

static void
subcomp_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   struct wl_resource *res;

   if (!client_allowed_check(data, client)) return;
   res = wl_resource_create(client, &wl_subcompositor_interface, version, id);
   wl_resource_set_implementation(res, &subcomp_interface, data, NULL);
}

/////////////////////////////////////////////////////////////////

static void
data_device_source_offer(struct wl_client *client, struct wl_resource *resource, const char *type)
{
   Comp_Data_Device_Source *ds = wl_resource_get_user_data(resource);

   if ((!ds->proxy) || (!ds->x11_owner))
     ds->mime_types = eina_list_append(ds->mime_types, eina_stringshare_add(type));
}

static void
data_device_source_set_actions(struct wl_client *client, struct wl_resource *resource, uint32_t dnd_actions)
{
   Comp_Data_Device_Source *ds = wl_resource_get_user_data(resource);

   if (ds->actions_set)
     {
        wl_resource_post_error(ds->res, WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
          "dnd_actions already set");
        return;
     }

    if (dnd_actions & ~ALL_ACTIONS)
      {
         wl_resource_post_error(ds->res, WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
           "invalid dnd_actions");
         return;
      }

   if (ds->seat && (!ds->proxy))
     {
        wl_resource_post_error(ds->res, WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
        "drag already begun");
        return;
     }

   ds->dnd_actions = dnd_actions;
   ds->actions_set = 1;
}

static void
data_device_source_impl_destroy(struct wl_resource *resource)
{
   Comp_Data_Device_Source *ds = wl_resource_get_user_data(resource);
   Eina_Stringshare *type;

   EINA_LIST_FREE(ds->mime_types, type) eina_stringshare_del(type);
   while (ds->transfers)
     {
        Comp_Data_Device_Transfer *dt = EINA_INLIST_CONTAINER_GET(ds->transfers, Comp_Data_Device_Transfer);
        fdh_del(dt->fdh);
        ds->transfers = eina_inlist_remove(ds->transfers, EINA_INLIST_GET(dt));
        free(dt);
     }
   if (ds->seat && (ds->seat->selection_source == ds)) ds->seat->selection_source = NULL;
   if (ds->seat && (ds->seat->drag.source == ds)) ds->seat->drag.source = NULL;
   comp_data_device_source_reader_clear(ds);
   ecore_event_handler_del(ds->proxy_send_handler);
   if (ds->offer) ds->offer->source = NULL;
   free(ds);
}

static const struct wl_data_source_interface data_device_source_interface =
{
   data_device_source_offer,
   resource_destroy,
   data_device_source_set_actions,
};

static void
data_device_manager_source_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   Comp_Data_Device_Source *ds;
   pid_t pid;
   Comp_Seat *s;
   Comp *c;

   ds = calloc(1, sizeof(Comp_Data_Device_Source));
   c = wl_resource_get_user_data(resource);
   wl_client_get_credentials(client, &pid, NULL, NULL);
   ds->proxy = c->client_disp && (pid == getpid());
   ds->res = wl_resource_create(client, &wl_data_source_interface, MIN(wl_resource_get_version(resource), 3), id);
   wl_resource_set_implementation(ds->res, &data_device_source_interface, ds, data_device_source_impl_destroy);
   if (!ds->proxy) return;
   EINA_INLIST_FOREACH(c->seats, s)
     {
        int x, y;
        struct wl_resource *res;
        Eina_Array *arr;
        Eina_Array_Iterator it;
        unsigned int i;
        char *type;
        if (((!s->client_offer) && (!s->drag.x11_owner)) || s->drag.res) continue;

        //proxied drag
        s->drag.res = resource;
        s->drag.source = ds;
        ds->seat = s;
        if (!s->drag.enter) return;
        evas_object_geometry_get(s->drag.enter->obj, &x, &y, NULL, NULL);
        if (s->client_offer)
          {
             arr = ecore_wl2_offer_mimes_get(s->client_offer);
             EINA_ARRAY_ITER_NEXT(arr, i, type, it)
               ds->mime_types = eina_list_append(ds->mime_types, eina_stringshare_add(type));
          }
        else if (s->drag.x11_owner)
          {
             PTR_SWAP(&s->drag.x11_types, &ds->mime_types);
             ds->x11_owner = s->drag.x11_owner;
             s->drag.x11_owner = 0;
          }
        comp_surface_send_data_device_enter(s->drag.enter, s);
        res = data_device_find(s, s->drag.enter->res);
        if (!res) return;
        wl_data_device_send_motion(res,
          (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff),
          wl_fixed_from_int(s->ptr.pos.x - x), wl_fixed_from_int(s->ptr.pos.y - y));
     }
}

static void
data_device_manager_device_impl_destroy(struct wl_resource *resource)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   struct wl_client *client = wl_resource_get_client(resource);
   eina_hash_del_by_key(s->data_devices, &client);
}

static void
data_device_start_drag(struct wl_client *client, struct wl_resource *resource, struct wl_resource *source_resource, struct wl_resource *origin_resource, struct wl_resource *icon_resource, uint32_t serial)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   Comp_Data_Device_Source *ds = NULL;
   Comp_Surface *cs = wl_resource_get_user_data(origin_resource);
   Comp_Surface *ics = wl_resource_get_user_data(icon_resource);
   Comp_Surface *enter;
   Input_Sequence *ev;
   Eina_Bool found = EINA_FALSE, up = EINA_FALSE;
   int cx, cy;

   if (source_resource) ds = wl_resource_get_user_data(source_resource);
   if ((cs != s->ptr.enter) && (cs != s->tch.enter)) return;
   enter = s->ptr.enter;
   if (enter) comp_surface_send_pointer_leave(enter, s);
   EINA_INLIST_FOREACH(s->tch.events, ev)
     {
        if (ev->down_serial == serial)
          {
             s->drag.tch = 1;
             up = !!ev->up_time;
             found = 1;
             s->drag.id = ev->id;
             break;
          }
     }
   if (!found)
     {
        EINA_INLIST_FOREACH(s->ptr.events, ev)
          {
             if (ev->down_serial == serial)
               {
                  up = !!ev->up_time;
                  found = 1;
                  s->drag.id = ev->id;
                  break;
               }
          }
     }
   if (!found) return;
   if (s->ptr.enter && (!s->drag.tch))
     comp_surface_send_pointer_leave(s->ptr.enter, s);
   s->drag.res = resource;
   s->drag.source = ds;
   if (ds) ds->seat = s;
   s->drag.surface = ics;
   if (ds && ds->proxy && s->drag.x11_owner)
     ds->x11_owner = s->drag.x11_owner;
   s->drag.x11_owner = 0;

   if (s->drag.tch)
     {
        cx = s->tch.pos.x;
        cy = s->tch.pos.y;
     }
   else
     {
        cx = s->ptr.pos.x;
        cy = s->ptr.pos.y;
     }
   if (ics)
     {
        int w, h;

        ics->cursor = 1;
        ics->drag = s;
        ics->pending.set_input = 1;
        eina_tiler_clear(ics->pending.input);
        evas_object_smart_member_del(ics->obj);
        evas_object_pass_events_set(ics->obj, 1);
        evas_object_layer_set(ics->obj, EVAS_LAYER_MAX - 1);
        evas_object_geometry_get(ics->obj, NULL, NULL, &w, &h);
        evas_object_move(ics->obj, cx, cy);
        evas_object_show(ics->obj);
     }
   if (s->drag.tch)
     {
        if (s->tch.enter)
          comp_surface_send_data_device_enter(s->tch.enter, s);
     }
   else if (enter)
     comp_surface_send_pointer_enter(enter, s, cx, cy);
#ifdef HAVE_ECORE_X
   if (ecore_x_display_get())
     ecore_x_pointer_grab(ecore_evas_window_get(ecore_evas_ecore_evas_get(s->c->evas)));
#endif
   if (up) drag_grab_button(s, ev->up_time, ev->id, WL_POINTER_BUTTON_STATE_RELEASED);
}

static Eina_Bool
data_device_proxy_send_send(void *d, int t EINA_UNUSED, void *event)
{
   Comp_Data_Device_Source *ds = d;
   Ecore_Wl2_Event_Data_Source_Send *ev = event;

   if (ds->proxy_serial != ev->serial) return ECORE_CALLBACK_RENEW;
   if (ev->display == ds->seat->c->parent_disp)
     {
        if (ecore_wl2_input_seat_id_get(ds->seat->seat) != ev->seat) return ECORE_CALLBACK_RENEW;
        wl_data_source_send_send(ds->res, ev->type, ev->fd);
        close(ev->fd);
     }
   return ECORE_CALLBACK_RENEW;
}

static void
data_device_set_selection(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *source_resource, uint32_t serial)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   Comp_Data_Device_Source *ds = NULL;

   if (source_resource)
     ds = wl_resource_get_user_data(source_resource);

   if (ds && ds->actions_set)
     {
        wl_resource_post_error(source_resource, WL_DATA_SOURCE_ERROR_INVALID_SOURCE, "invalid source");
        return;
     }
   if (s->selection_source && (s->selection_serial - serial < UINT32_MAX / 2))
     return;
   if (s->selection_source)
     {
        if (!s->selection_source->transfers)
          comp_data_device_source_reader_clear(s->selection_source);
        ecore_event_handler_del(s->selection_source->proxy_send_handler);
        s->selection_source->proxy_send_handler = NULL;
     }
   if (ds) ds->seat = s;
   s->selection_source = ds;
   s->selection_serial = serial;
   comp_seat_kbd_data_device_enter(s);
   if (ds && ds->proxy && s->x11_selection_owner)
     ds->x11_owner = s->x11_selection_owner;
   s->x11_selection_owner = 0;
   if (1)
   //if (s->c->data_device_proxy)
     {
        if (s->c->parent_disp) //wayland
          {
             if (ds && ds->mime_types)
               {
                  char *t, *types[eina_list_count(ds->mime_types) + 1];
                  Eina_List *l;
                  int i = 0;

                  EINA_LIST_FOREACH(ds->mime_types, l, t)
                    types[i++] = t;
                  types[i] = NULL;
                  ds->proxy_serial = ecore_wl2_dnd_selection_set(s->seat, (const char**)types);
                  ds->proxy_send_handler =
                    ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND,
                      data_device_proxy_send_send, ds);
               }
             else if (ds)
               ds->proxy_serial = ecore_wl2_dnd_selection_clear(s->seat);
          }
#ifdef HAVE_ECORE_X
        else if (ds && (!ds->proxy) && ecore_x_display_get())
          {
             Eina_List *l;
             Comp *c;
             Ecore_X_Time t = ecore_x_current_time_get();

             EINA_LIST_FOREACH(comps, l, c)
               c->x11_selection = 0;
             s->c->x11_selection = 1;
             ecore_x_selection_owner_set(ecore_evas_window_get(ecore_evas_ecore_evas_get(s->c->evas)),
               ECORE_X_ATOM_SELECTION_CLIPBOARD, t);
          }
#endif
     }
   s->client_selection_serial = 0;
}

static const struct wl_data_device_interface data_device_interface =
{
   data_device_start_drag,
   data_device_set_selection,
   resource_destroy,
};

static void
data_device_manager_device_create(struct wl_client *client, struct wl_resource *manager_resource, uint32_t id, struct wl_resource *seat_resource)
{
   Comp_Seat *s = wl_resource_get_user_data(seat_resource);
   struct wl_resource *res;

   res = wl_resource_create(client, &wl_data_device_interface, wl_resource_get_version(manager_resource), id);
   wl_resource_set_implementation(res, &data_device_interface, s, data_device_manager_device_impl_destroy);
   eina_hash_add(s->data_devices, &client, res);
}

static const struct wl_data_device_manager_interface data_device_manager_interface =
{
   data_device_manager_source_create,
   data_device_manager_device_create
};

static void
data_device_manager_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   struct wl_resource *res;

   if (!client_allowed_check(data, client)) return;
   res = wl_resource_create(client, &wl_data_device_manager_interface, MIN(3, version), id);
   wl_resource_set_implementation(res, &data_device_manager_interface, data, NULL);
}

/////////////////////////////////////////////////////////////////

static void
output_resize(Comp *c, struct wl_resource *res)
{
   int w, h;
   int rot[][4] =
   {
    {
     [EFL_WL_ROTATION_0] = WL_OUTPUT_TRANSFORM_NORMAL,
     [EFL_WL_ROTATION_90] = WL_OUTPUT_TRANSFORM_90,
     [EFL_WL_ROTATION_180] = WL_OUTPUT_TRANSFORM_180,
     [EFL_WL_ROTATION_270] = WL_OUTPUT_TRANSFORM_270,
    },
    {
     [EFL_WL_ROTATION_0] = WL_OUTPUT_TRANSFORM_FLIPPED,
     [EFL_WL_ROTATION_90] = WL_OUTPUT_TRANSFORM_FLIPPED_90,
     [EFL_WL_ROTATION_180] = WL_OUTPUT_TRANSFORM_FLIPPED_180,
     [EFL_WL_ROTATION_270] = WL_OUTPUT_TRANSFORM_FLIPPED_270,
    },
   };

   evas_object_geometry_get(c->clip, NULL, NULL, &w, &h);
   /* FIXME: transform */
   wl_output_send_geometry(res, 0, 0, w, h, 0, "", "", rot[c->rtl][c->rotation]);
   wl_output_send_mode(res, WL_OUTPUT_MODE_CURRENT, w, h, 60 * 1000);
   if (wl_resource_get_version(res) >= WL_OUTPUT_DONE_SINCE_VERSION)
     wl_output_send_done(res);
}

static void
output_unbind(struct wl_resource *resource)
{
   Comp *c = wl_resource_get_user_data(resource);
   c->output_resources = eina_list_remove(c->output_resources, resource);
}

static void
output_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   Comp *c = data;
   Comp_Surface *cs;
   struct wl_resource *res;

   if (!client_allowed_check(data, client)) return;
   res = wl_resource_create(client, &wl_output_interface, version, id);
   c->output_resources = eina_list_append(c->output_resources, res);
   wl_resource_set_implementation(res, NULL, data, output_unbind);
   if (wl_resource_get_version(res) >= WL_OUTPUT_SCALE_SINCE_VERSION)
     wl_output_send_scale(res, lround(c->scale));
   output_resize(c, res);
   EINA_INLIST_FOREACH(c->surfaces, cs)
     if (wl_resource_get_client(cs->res) == client)
       {
          if (evas_object_visible_get(c->clip))
            comp_surface_output_enter(cs);
          else
            comp_surface_output_leave(cs);
       }
}

/////////////////////////////////////////////////////////////////

static void
shell_surface_toplevel_impl_destroy(struct wl_resource *resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);

   cs->role = NULL;
   evas_object_hide(cs->obj);
   shell_surface_reset(cs);
   if (!cs->parent) return;
   comp_surface_reparent(cs, NULL);
}

static void
shell_surface_toplevel_set_parent(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *parent_resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   Comp_Surface *pcs = NULL;

   if (parent_resource) pcs = wl_resource_get_user_data(parent_resource);

   comp_surface_reparent(cs, pcs);
   evas_object_smart_callback_call(cs->c->obj, "child_added", cs->obj);
}

static void
shell_surface_toplevel_set_title(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, const char *title)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);

   eina_stringshare_replace(&cs->shell.title, title);
}

static void
shell_surface_toplevel_set_app_id(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, const char *id)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);

   eina_stringshare_replace(&cs->shell.app_id, id);
}

static void
shell_surface_toplevel_show_window_menu(){}
static void
shell_surface_toplevel_move(){}
static void
shell_surface_toplevel_resize(){}

static void
shell_surface_toplevel_set_max_size(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t w, int32_t h)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   evas_object_size_hint_max_set(cs->obj, w, h);
   if (cs == cs->c->active_surface)
     shell_surface_minmax_update(cs);
}

static void
shell_surface_toplevel_set_min_size(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t w, int32_t h)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   evas_object_size_hint_min_set(cs->obj, w, h);
   if (cs == cs->c->active_surface)
     shell_surface_minmax_update(cs);
}

static void
shell_surface_toplevel_set_maximized(){}
static void
shell_surface_toplevel_unset_maximized(){}
static void
shell_surface_toplevel_set_fullscreen(){}
static void
shell_surface_toplevel_unset_fullscreen(){}
static void
shell_surface_toplevel_set_minimized(){}

static const struct xdg_toplevel_interface shell_surface_toplevel_interface =
{
   resource_destroy,
   shell_surface_toplevel_set_parent,
   shell_surface_toplevel_set_title,
   shell_surface_toplevel_set_app_id,
   shell_surface_toplevel_show_window_menu,
   shell_surface_toplevel_move,
   shell_surface_toplevel_resize,
   shell_surface_toplevel_set_max_size,
   shell_surface_toplevel_set_min_size,
   shell_surface_toplevel_set_maximized,
   shell_surface_toplevel_unset_maximized,
   shell_surface_toplevel_set_fullscreen,
   shell_surface_toplevel_unset_fullscreen,
   shell_surface_toplevel_set_minimized,
};

static void
shell_surface_toplevel_create(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t id)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);

   if (cs->buffer[0] || cs->pending.buffer)
     {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER,
                               "buffer attached/committed before configure");
        return;
     }
   if (cs->role)
     {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_ROLE,
                               "surface already has assigned role");
        return;
     }

   cs->role = wl_resource_create(client, &xdg_toplevel_interface, 1, id);
   wl_resource_set_implementation(cs->role, &shell_surface_toplevel_interface, cs, shell_surface_toplevel_impl_destroy);
   cs->shell.new = 1;
}

static void
shell_surface_popup_grab(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *seat, uint32_t serial EINA_UNUSED)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   Comp_Seat *s = wl_resource_get_user_data(seat);

   if (cs->dead || (!cs->role) || (!cs->shell.surface))
     {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT, "can't grab for this resource");
        return;
     }
   if (cs->mapped)
     {
        wl_resource_post_error(resource, XDG_POPUP_ERROR_INVALID_GRAB,
                                "grab requested on mapped popup");
        return;
     }

   if (cs->parent->shell.popup && (s->grab != cs->parent))
     {
        wl_resource_post_error(resource, XDG_POPUP_ERROR_INVALID_GRAB,
                                "grab requested on ungrabbed nested popup");
        return;
     }
   s->grab = cs;
   cs->shell.grabs = eina_list_append(cs->shell.grabs, s);
}

static const struct xdg_popup_interface shell_surface_popup_interface =
{
   resource_destroy,
   shell_surface_popup_grab,
};

static void
shell_surface_popup_impl_destroy(struct wl_resource *resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);
   Comp_Seat *s;

   cs->role = NULL;
   evas_object_hide(cs->obj);
   cs->shell.popup = 0;
   shell_surface_reset(cs);
   EINA_LIST_FREE(cs->shell.grabs, s)
     if (s->grab == cs)
       {
          if (cs->parent->shell.grabs &&
              eina_list_data_find(cs->parent->shell.grabs, s))
            s->grab = cs->parent;
          else
            s->grab = NULL;
       }
   if (cs->children)
     wl_resource_post_error(cs->shell.surface, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES,
                            "popups dismissed out of order");
   if (cs->parent)
     comp_surface_reparent(cs, NULL);
}

static void
shell_surface_popup_create(struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *parent_resource, struct wl_resource *positioner_resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);

   if (cs->buffer[0] || cs->pending.buffer)
     {
        wl_resource_post_error(resource, XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER,
                               "buffer attached/committed before configure");
        return;
     }
   if (cs->role)
     {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_ROLE,
                               "surface already has assigned role");
        return;
     }

   cs->role = wl_resource_create(client, &xdg_popup_interface, 1, id);
   wl_resource_set_implementation(cs->role, &shell_surface_popup_interface, cs, shell_surface_popup_impl_destroy);
   cs->shell.new = 1;
   cs->shell.popup = 1;
   if(parent_resource)
     comp_surface_reparent(cs, wl_resource_get_user_data(parent_resource));
   cs->shell.positioner = wl_resource_get_user_data(positioner_resource);
   _apply_positioner(cs, cs->shell.positioner);
   evas_object_smart_callback_call(cs->c->obj, "popup_added", cs->obj);
}

static void
_validate_size(struct wl_resource *resource, int32_t value)
{
   if (value <= 0)
     wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "Invalid size passed");
}

static void
_validate_size_negative(struct wl_resource *resource, int32_t value)
{
   if (value < 0)
     wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "Invalid size passed");
}

static void
shell_positioner_set_size(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, int32_t w, int32_t h)
{
   Shell_Positioner *p = wl_resource_get_user_data(resource);

   _validate_size(resource, w);
   _validate_size(resource, h);

   p->size.w = w;
   p->size.h = h;
}

static void
shell_positioner_set_anchor_rect(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   Shell_Positioner *p = wl_resource_get_user_data(resource);

   _validate_size_negative(resource, w);
   _validate_size_negative(resource, h);

   EINA_RECTANGLE_SET(&p->anchor_rect, x, y, w, h);
}

static void
shell_positioner_set_anchor(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, enum xdg_positioner_anchor anchor)
{
   Shell_Positioner *p = wl_resource_get_user_data(resource);

   p->anchor = anchor;
}

static void
shell_positioner_set_gravity(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, enum xdg_positioner_gravity gravity)
{
   Shell_Positioner *p = wl_resource_get_user_data(resource);

   if ((gravity & (XDG_POSITIONER_GRAVITY_TOP | XDG_POSITIONER_GRAVITY_BOTTOM)) ==
       (XDG_POSITIONER_GRAVITY_TOP | XDG_POSITIONER_GRAVITY_BOTTOM))
     wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "Invalid gravity values passed");
   else if ((gravity & (XDG_POSITIONER_GRAVITY_LEFT | XDG_POSITIONER_GRAVITY_RIGHT)) ==
       (XDG_POSITIONER_GRAVITY_LEFT | XDG_POSITIONER_GRAVITY_RIGHT))
     wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "Invalid gravity values passed");
   else
     p->gravity = gravity;
}

static void
shell_positioner_set_constraint_adjustment(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, enum xdg_positioner_constraint_adjustment constraint_adjustment)
{
   Shell_Positioner *p = wl_resource_get_user_data(resource);

   p->constrain = constraint_adjustment;
}

static void
shell_positioner_set_offset(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y)
{
   Shell_Positioner *p = wl_resource_get_user_data(resource);

   p->offset.x = x;
   p->offset.y = y;
}

static const struct xdg_positioner_interface shell_positioner_interface =
{
   resource_destroy,
   shell_positioner_set_size,
   shell_positioner_set_anchor_rect,
   shell_positioner_set_anchor,
   shell_positioner_set_gravity,
   shell_positioner_set_constraint_adjustment,
   shell_positioner_set_offset,
};

static void
shell_positioner_impl_destroy(struct wl_resource *resource)
{
   Shell_Positioner *sp;

   sp = wl_resource_get_user_data(resource);
   if (!sp) return;
   if (sp->sd) sp->sd->positioners = eina_inlist_remove(sp->sd->positioners, EINA_INLIST_GET(sp));
   free(sp);
}

static void
shell_positioner_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   struct wl_resource *res;
   Shell_Data *sd;
   Shell_Positioner *sp;

   sd = wl_resource_get_user_data(resource);
   res = wl_resource_create(client, &xdg_positioner_interface, 1, id);
   sp = calloc(1, sizeof(Shell_Positioner));
   sp->anchor_rect.w = sp->anchor_rect.h = -1;
   sp->sd = sd;
   sp->res = res;
   sd->positioners = eina_inlist_append(sd->positioners, EINA_INLIST_GET(sp));
   wl_resource_set_implementation(res, &shell_positioner_interface, sp, shell_positioner_impl_destroy);
}

static void
shell_surface_set_window_geometry(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   Comp_Surface *cs = wl_resource_get_user_data(resource);

   EINA_RECTANGLE_SET(&cs->shell.geom, x, y, w, h);
}

static void
shell_surface_ack_configure(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial)
{
}

static const struct xdg_surface_interface shell_surface_interface =
{
   resource_destroy,
   shell_surface_toplevel_create,
   shell_surface_popup_create,
   shell_surface_set_window_geometry,
   shell_surface_ack_configure,
};

static void
shell_surface_impl_destroy(struct wl_resource *resource)
{
   Comp_Surface *ccs, *cs = wl_resource_get_user_data(resource);

   if (cs->role)
     {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES, "shell surface destroyed before role surfaces");
        wl_resource_destroy(cs->role);
     }
   cs->shell.surface = NULL;
   cs->shell.data->surfaces = eina_list_remove(cs->shell.data->surfaces, cs);
   cs->shell.data = NULL;
   shell_surface_reset(cs);
   while (cs->children)
     {
        ccs = EINA_INLIST_CONTAINER_GET(cs->children, Comp_Surface);
        evas_object_hide(ccs->obj);
        comp_surface_reparent(ccs, cs->parent);
     }
}

static void
shell_surface_create(struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *surface_resource)
{
   Shell_Data *sd = wl_resource_get_user_data(resource);
   Comp_Surface *cs = wl_resource_get_user_data(surface_resource);

   if (cs->role || cs->shell.surface)
     {
        wl_resource_post_error(surface_resource, WL_DISPLAY_ERROR_INVALID_OBJECT, "surface already has role");
        return;
     }

   cs->shell.surface = wl_resource_create(client, &xdg_surface_interface, 1, id);
   cs->shell.data = sd;
   wl_resource_set_implementation(cs->shell.surface, &shell_surface_interface, cs, shell_surface_impl_destroy);
   sd->surfaces = eina_list_append(sd->surfaces, cs);
}

static void
shell_pong(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial EINA_UNUSED)
{
   Shell_Data *sd = wl_resource_get_user_data(resource);

   sd->ping = 0;
}

static const struct xdg_wm_base_interface shell_interface =
{
   resource_destroy,
   shell_positioner_create,
   shell_surface_create,
   shell_pong
};

static void
shell_unbind(struct wl_resource *resource)
{
   Shell_Data *sd = wl_resource_get_user_data(resource);

   sd->c->shells = eina_inlist_remove(sd->c->shells, EINA_INLIST_GET(sd));
   while (sd->surfaces)
     {
        Comp_Surface *cs = eina_list_data_get(sd->surfaces);
        if (cs->shell.surface)
          {
             if (cs->role)
               wl_resource_destroy(cs->role);
             wl_resource_destroy(cs->shell.surface);
          }
     }
   free(sd);
}

static void
shell_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   Comp *c = data;
   struct wl_resource *res;
   Shell_Data *sd;

   if (!client_allowed_check(data, client)) return;
   sd = calloc(1, sizeof(Shell_Data));
   sd->c = c;
   c->shells = eina_inlist_append(c->shells, EINA_INLIST_GET(sd));

   res = wl_resource_create(client, &xdg_wm_base_interface, version, id);
   sd->res = res;
   wl_resource_set_implementation(res, &shell_interface, sd, shell_unbind);
}

/////////////////////////////////////////////////////////////////

static void
seat_update_caps(Comp_Seat *s, struct wl_resource *res)
{
   enum wl_seat_capability caps = 0;
   Eina_List *l;

   if (s->pointer)
     caps |= WL_SEAT_CAPABILITY_POINTER;
   if (s->keyboard)
     caps |= WL_SEAT_CAPABILITY_KEYBOARD;
   if (s->touch)
     caps |= WL_SEAT_CAPABILITY_TOUCH;

   if (!caps) return;
   if (res)
     wl_seat_send_capabilities(res, caps);
   else
     EINA_LIST_FOREACH(s->resources, l, res)
       wl_seat_send_capabilities(res, caps);
}

static void
seat_keymap_send(Comp_Seat *s)
{
   Eina_List *l;
   Eina_Iterator *it;
   it = eina_hash_iterator_data_new(s->kbd.resources);
   EINA_ITERATOR_FOREACH(it, l)
     {
        Eina_List *ll;
        struct wl_resource *res;
        EINA_LIST_FOREACH(l, ll, res)
          wl_keyboard_send_keymap(res, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, s->kbd.keymap_fd, s->kbd.keymap_mem_size);
     }
   eina_iterator_free(it);
}

static Eina_Bool
seat_kbd_mods_update(Comp_Seat *s)
{
   xkb_mod_mask_t mod;
   xkb_layout_index_t grp;

   mod = xkb_state_serialize_mods(s->kbd.state, XKB_STATE_DEPRESSED);
   s->kbd.mods.changed |= mod != s->kbd.mods.depressed;
   s->kbd.mods.depressed = mod;
   mod = xkb_state_serialize_mods(s->kbd.state, XKB_STATE_MODS_LATCHED);
   s->kbd.mods.changed |= mod != s->kbd.mods.latched;
   s->kbd.mods.latched = mod;
   mod = xkb_state_serialize_mods(s->kbd.state, XKB_STATE_MODS_LOCKED);
   s->kbd.mods.changed |= mod != s->kbd.mods.locked;
   s->kbd.mods.locked = mod;
   grp = xkb_state_serialize_layout(s->kbd.state, XKB_STATE_LAYOUT_EFFECTIVE);
   s->kbd.mods.changed |= grp != s->kbd.mods.group;
   s->kbd.mods.group = grp;
   return s->kbd.mods.changed;
}

static void
seat_kbd_external_init(Comp_Seat *s)
{
   Eina_List *l, *ll;
   uint32_t serial;
   struct wl_resource *res;

   seat_keymap_send(s);
   if (!seat_kbd_mods_update(s)) return;
   l = seat_kbd_active_resources_get(s);
   if (!l) return;
   serial = wl_display_next_serial(s->c->display);
   EINA_LIST_FOREACH(l, ll, res)
     comp_seat_send_modifiers(s, res, serial);
}

static void
seat_keymap_update(Comp_Seat *s)
{
   char *str;
   Eina_Tmpstr *file;
   xkb_mod_mask_t latched = 0, locked = 0;

   if (s->kbd.keymap_mem) munmap(s->kbd.keymap_mem, s->kbd.keymap_mem_size);
   if (s->kbd.keymap_fd > -1) close(s->kbd.keymap_fd);

#ifdef HAVE_ECORE_X
   if (!x11_kbd_keymap)
     {
#endif
        if (s->kbd.state)
          {
             latched = xkb_state_serialize_mods(s->kbd.state, XKB_STATE_MODS_LATCHED);
             locked = xkb_state_serialize_mods(s->kbd.state, XKB_STATE_MODS_LOCKED);
             xkb_state_unref(s->kbd.state);
          }
        if (!s->kbd.keymap)
          {
             s->kbd.state = NULL;
             s->kbd.keymap_fd = -1;
             s->kbd.keymap_mem = NULL;
             return;
          }

        s->kbd.state = xkb_state_new(s->kbd.keymap);
        xkb_state_update_mask(s->kbd.state, 0, latched, locked, 0, 0, 0);
#ifdef HAVE_ECORE_X
     }
#endif
   str = xkb_map_get_as_string(s->kbd.keymap);
   s->kbd.keymap_mem_size = strlen(str) + 1;
   s->kbd.keymap_fd = eina_file_mkstemp("comp-keymapXXXXXX", &file);
   if (s->kbd.keymap_fd < 0)
     {
        EINA_LOG_ERR("mkstemp failed!\n");
        s->kbd.keymap_fd = -1;
        xkb_state_unref(s->kbd.state);
        s->kbd.state = NULL;
        return;
     }
   if (!eina_file_close_on_exec(s->kbd.keymap_fd, 1))
     {
        EINA_LOG_ERR("Failed to set CLOEXEC on fd %d\n", s->kbd.keymap_fd);
        close(s->kbd.keymap_fd);
        s->kbd.keymap_fd = -1;
        xkb_state_unref(s->kbd.state);
        s->kbd.state = NULL;
        return;
     }
   ftruncate(s->kbd.keymap_fd, s->kbd.keymap_mem_size);
   eina_file_unlink(file);
   eina_tmpstr_del(file);
   s->kbd.keymap_mem =
     mmap(NULL, s->kbd.keymap_mem_size + 1,
       PROT_READ | PROT_WRITE, MAP_SHARED, s->kbd.keymap_fd, 0);

   memcpy(s->kbd.keymap_mem, str, s->kbd.keymap_mem_size);
   s->kbd.keymap_mem[s->kbd.keymap_mem_size] = 0;
   free(str);

   seat_keymap_send(s);
}

static inline void
seat_kbd_repeat_rate_update(Comp_Seat *s)
{
   double rate, delay;

   if (s->seat)
     {
        if (ecore_wl2_input_keyboard_repeat_get(s->seat, &rate, &delay))
          {
             s->kbd.repeat_rate = lround(1 / rate);
             s->kbd.repeat_delay = lround(delay * 1000);
          }
        else
          s->kbd.repeat_rate = s->kbd.repeat_delay = 0;
     }
   else
     {
        s->kbd.repeat_rate = 40;
        s->kbd.repeat_delay = 400;
     }
}

static void
seat_keymap_create(Comp_Seat *s)
{
   struct xkb_rule_names names;

   memset(&names, 0, sizeof(names));
   names.rules = "evdev";
   names.model = "pc105";
   names.layout = "us";
   s->kbd.context = xkb_context_new(0);
   s->kbd.keymap = xkb_map_new_from_names(s->kbd.context, &names, 0);
}

static const struct wl_keyboard_interface seat_kbd_interface =
{
   resource_destroy
};

static void
seat_kbd_unbind(struct wl_resource *resource)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   struct wl_client *client = wl_resource_get_client(resource);

   eina_hash_list_remove(s->kbd.resources, &client, resource);
}

static void
seat_kbd_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   struct wl_resource *res;
   Eina_List *l, *ll;
   uint32_t serial;

   res = wl_resource_create(client, &wl_keyboard_interface, wl_resource_get_version(resource), id);
   wl_resource_set_implementation(res, &seat_kbd_interface, s, seat_kbd_unbind);
   if (!s->kbd.resources) s->kbd.resources = eina_hash_pointer_new(NULL);
   eina_hash_list_append(s->kbd.resources, &client, res);

   wl_keyboard_send_keymap(res, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, s->kbd.keymap_fd, s->kbd.keymap_mem_size);

   if (wl_resource_get_version(res) >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
     wl_keyboard_send_repeat_info(res, s->kbd.repeat_rate, s->kbd.repeat_delay);

   if (s->active_client != client) return;
   l = seat_kbd_active_resources_get(s);
   if (!l) return;
   serial = wl_display_next_serial(s->c->display);
   EINA_LIST_FOREACH(l, ll, res)
     {
        if (s->c->active_surface)
          wl_keyboard_send_enter(res, serial, s->c->active_surface->res, &s->kbd.keys);
        comp_seat_send_modifiers(s, res, serial);
     }
}

static void
seat_ptr_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Comp_Seat *s = data;

   evas_object_event_callback_del_full(s->ptr.efl.obj, EVAS_CALLBACK_DEL, seat_ptr_del, s);
   s->ptr.efl.obj = NULL;
}

static void
seat_ptr_inherit(Comp_Seat *s, Eo *dev)
{
   ecore_evas_cursor_device_get(ecore_evas_ecore_evas_get(s->c->evas), dev, &s->ptr.efl.obj,
              &s->ptr.efl.layer, &s->ptr.efl.x, &s->ptr.efl.y);
   if (s->ptr.efl.obj)
     evas_object_event_callback_add(s->ptr.efl.obj, EVAS_CALLBACK_DEL, seat_ptr_del, s);
}

static void
seat_ptr_set_cursor(struct wl_client *client, struct wl_resource *resource, uint32_t serial, struct wl_resource *surface_resource, int32_t x, int32_t y)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   Comp_Surface *cs = NULL;

   if (!s->active_client) return;
   if (surface_resource && (s->active_client != wl_resource_get_client(surface_resource))) return;
   if (s->ptr.enter_serial - serial > UINT32_MAX / 2) return;
   if (surface_resource)
     cs = wl_resource_get_user_data(surface_resource);
   if (cs && cs->role && (!cs->cursor))
     {
        wl_resource_post_error(surface_resource,
                               WL_POINTER_ERROR_ROLE, "surface already has role");
        return;
     }
   if (s->ptr.cursor.surface == cs) return;
   if (cs)
     {
        cs->cursor = 1;
        cs->pending.set_input = 1;
        eina_tiler_clear(cs->pending.input);
        evas_object_pass_events_set(cs->obj, 1);
     }
   if (s->ptr.cursor.surface)
     {
        s->ptr.cursor.surface->cursor = 0;
        s->ptr.cursor.surface->role = NULL;
     }

   if (s->ptr.in)
     {
        const Eina_List *l;
        Eo *dev;
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(s->c->evas);

        EINA_LIST_FOREACH(evas_device_list(s->c->evas, s->dev), l, dev)
          if (evas_device_class_get(dev) == EVAS_DEVICE_CLASS_MOUSE)
            {
               if ((!s->ptr.efl.obj) && (!s->ptr.cursor.surface))
                 seat_ptr_inherit(s, dev);
               ecore_evas_cursor_device_unset(ee, dev);
               if (cs)
                 {
                    cs->role = cs->res;
                    ecore_evas_object_cursor_device_set(ee, dev, cs->obj, EVAS_LAYER_MAX, x, y);
                 }
            }
     }
   if (cs)
     evas_object_smart_member_del(cs->obj);
   s->ptr.cursor.surface = cs;
   s->ptr.cursor.x = x;
   s->ptr.cursor.y = y;
}

static const struct wl_pointer_interface seat_ptr_interface =
{
   seat_ptr_set_cursor,
   resource_destroy
};

static void
seat_ptr_unbind(struct wl_resource *resource)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   struct wl_client *client = wl_resource_get_client(resource);

   eina_hash_list_remove(s->ptr.resources, &client, resource);
}

static void
seat_ptr_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   struct wl_resource *res;
   Comp_Surface *cs;
   int x, y;

   res = wl_resource_create(client, &wl_pointer_interface, wl_resource_get_version(resource), id);
   wl_resource_set_implementation(res, &seat_ptr_interface, s, seat_ptr_unbind);
   if (!s->ptr.resources) s->ptr.resources = eina_hash_pointer_new(NULL);
   eina_hash_list_append(s->ptr.resources, &client, res);
   if (!s->ptr.enter) return;
   cs = s->ptr.enter;
   if (wl_resource_get_client(cs->res) != client) return;
   s->ptr.enter = NULL;
   evas_pointer_canvas_xy_get(s->c->evas, &x, &y);
   comp_surface_send_pointer_enter(cs, s, x, y);
}

static const struct wl_touch_interface seat_tch_interface =
{
   resource_destroy
};

static void
seat_tch_unbind(struct wl_resource *resource)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   struct wl_client *client = wl_resource_get_client(resource);

   eina_hash_list_remove(s->tch.resources, &client, resource);
}

static void
seat_tch_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);
   struct wl_resource *res;

   res = wl_resource_create(client, &wl_touch_interface, wl_resource_get_version(resource), id);
   wl_resource_set_implementation(res, &seat_tch_interface, s, seat_tch_unbind);
   if (!s->tch.resources) s->tch.resources = eina_hash_pointer_new(NULL);
   eina_hash_list_append(s->tch.resources, &client, res);
}

static const struct wl_seat_interface seat_interface =
{
   seat_ptr_create,
   seat_kbd_create,
   seat_tch_create,
   resource_destroy,
};

static void
seat_unbind(struct wl_resource *resource)
{
   Comp_Seat *s = wl_resource_get_user_data(resource);

   s->resources = eina_list_remove(s->resources, resource);
}

static void
seat_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   struct wl_resource *res;
   Comp_Seat *s = data;

   if (!client_allowed_check(s->c, client)) return;
   res = wl_resource_create(client, &wl_seat_interface, version, id);
   s->resources = eina_list_append(s->resources, res);
   if (s->c->active_surface)
     s->active_client = wl_resource_get_client(s->c->active_surface->res);

   wl_resource_set_implementation(res, &seat_interface, s, seat_unbind);

   seat_update_caps(s, res);
   if (s->name && (version >= WL_SEAT_NAME_SINCE_VERSION))
     wl_seat_send_name(res, s->name);
}

static void
seat_resource_hash_free(Eina_Hash *h)
{
   Eina_Iterator *it;
   Eina_List *l;
   void **key;

   if (!h) return;
   while (eina_hash_population(h))
     {
        it = eina_hash_iterator_key_new(h);
        EINA_ITERATOR_FOREACH(it, key)
          {
             struct wl_resource *res;
             l = eina_hash_set(h, key, NULL);

             EINA_LIST_FREE(l, res)
               wl_resource_destroy(res);
             break;
          }
        eina_iterator_free(it);
     }
   eina_hash_free(h);
}

static void
seat_kbd_destroy(Comp_Seat *s)
{
   if (s->kbd.external) return;
#ifdef HAVE_ECORE_X
   if (!x11_kbd_keymap)
     {
#endif
        if (s->kbd.state) xkb_state_unref(s->kbd.state);
        if (s->kbd.keymap) xkb_keymap_unref(s->kbd.keymap);
        if (s->kbd.context) xkb_context_unref(s->kbd.context);
#ifdef HAVE_ECORE_X
     }
#endif
   if (s->kbd.keymap_mem) munmap(s->kbd.keymap_mem, s->kbd.keymap_mem_size);
   if (s->kbd.keymap_fd > -1) close(s->kbd.keymap_fd);
   wl_array_release(&s->kbd.keys);
}

static void
seat_destroy(Comp_Seat *s)
{
   Eina_Stringshare *type;
   seat_resource_hash_free(s->kbd.resources);
   seat_resource_hash_free(s->ptr.resources);
   seat_resource_hash_free(s->tch.resources);
   while (s->resources)
     wl_resource_destroy(eina_list_data_get(s->resources));
   eina_stringshare_del(s->name);
   seat_kbd_destroy(s);
   efl_unref(s->dev);
   s->c->seats = eina_inlist_remove(s->c->seats, EINA_INLIST_GET(s));
   eina_hash_free(s->data_devices);
   EINA_LIST_FREE(s->drag.x11_types, type) eina_stringshare_del(type);
   while (s->ptr.events)
     {
        Input_Sequence *ev = EINA_INLIST_CONTAINER_GET(s->ptr.events, Input_Sequence);
        s->ptr.events = eina_inlist_remove(s->ptr.events, s->ptr.events);
        free(ev);
     }
   while (s->tch.events)
     {
        Input_Sequence *ev = EINA_INLIST_CONTAINER_GET(s->tch.events, Input_Sequence);
        s->tch.events = eina_inlist_remove(s->tch.events, s->tch.events);
        free(ev);
     }
   wl_global_destroy(s->global);
   free(s);

}

/////////////////////////////////////////////////////////////////

static void
comp_gl_shutdown(Comp *c)
{
   if (c->glapi && c->glapi->evasglUnbindWaylandDisplay)
     c->glapi->evasglUnbindWaylandDisplay(c->gl, c->display);
   if (c->glsfc)
     evas_gl_surface_destroy(c->gl, c->glsfc);
   if (c->glctx)
     evas_gl_context_destroy(c->gl, c->glctx);
   evas_gl_free(c->gl);
   evas_gl_config_free(c->glcfg);
   c->glsfc = NULL;
   c->glctx = NULL;
   c->glcfg = NULL;
   c->gl = NULL;
}

static void
comp_gl_init(Comp *c)
{
   c->glctx = evas_gl_context_create(c->gl, NULL);
   if (!c->glctx) goto end;
   c->glcfg = evas_gl_config_new();
   if (!c->glcfg) goto end;
   c->glsfc = evas_gl_surface_create(c->gl, c->glcfg, 1, 1);
   if (!c->glsfc) goto end;
   if (!evas_gl_make_current(c->gl, c->glsfc, c->glctx)) goto end;
   c->glapi = evas_gl_context_api_get(c->gl, c->glctx);
   if (c->glapi->evasglBindWaylandDisplay &&
       c->glapi->evasglBindWaylandDisplay(c->gl, c->display))
     return;
end:
   comp_gl_shutdown(c);
}

static void
comp_render_pre(Comp *c, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Comp_Surface *cs;
   Eina_List *l, *ll;

   c->rendering = 1;
   EINA_LIST_FOREACH_SAFE(c->render_queue, l, ll, cs)
     {
        Comp_Buffer *buffer;
//if (cs->subsurface) fprintf(stderr, "RENDER PRE\n");

        comp_surface_buffer_detach(&cs->buffer[1]);
        cs->buffer[1] = cs->buffer[0];
        cs->buffer[0] = NULL;
        cs->render_queue = 0;
        buffer = cs->buffer[1];

        if (!buffer)
          {
             c->render_queue = eina_list_remove_list(c->render_queue, l);
             evas_object_image_pixels_dirty_set(cs->img, 0);
             continue;
          }
        //if (cs->proxies) fprintf(stderr, "RENDER %d\n", wl_resource_get_id(buffer->res));
        cs->post_render_queue = 1;

        if (cs->opaque_rects && (eina_array_count(cs->opaque_rects) == 1))
          {
             Evas_Object *r = eina_array_data_get(cs->opaque_rects, 0);
             int x, y, w, h, ox, oy, ow, oh;

             evas_object_geometry_get(cs->img, &x, &y, &w, &h);
             evas_object_geometry_get(r, &ox, &oy, &ow, &oh);
             evas_object_image_border_set(cs->img, ox - x, ox + ow, oy - y, oy + oh);
             evas_object_image_border_center_fill_set(cs->img, EVAS_BORDER_FILL_SOLID);
          }
        else
          {
             evas_object_image_border_set(cs->img, 0, 0, 0, 0);
             evas_object_image_border_center_fill_set(cs->img, EVAS_BORDER_FILL_DEFAULT);
          }
        evas_object_image_alpha_set(cs->img, comp_surface_is_alpha(cs, buffer));
        evas_object_resize(cs->img, buffer->w, buffer->h);
        evas_object_image_size_set(cs->img, buffer->w, buffer->h);
     }
   c->post_render_queue = c->render_queue;
   c->render_queue = NULL;
}

static void
comp_render_pre_proxied(Evas_Object *o, Evas *e, void *event_info)
{
   Comp_Surface *cs = evas_object_data_get(o, "comp_surface");
   Comp_Buffer *buffer = cs->buffer[!cs->render_queue];

   //fprintf(stderr, "PROXY RENDER_PRE %d\n", buffer ? wl_resource_get_id(buffer->res) : -1);
   if (buffer)
     {
        buffer->renders = eina_list_remove(buffer->renders, e);
        buffer->post_renders = eina_list_append(buffer->post_renders, e);
        evas_object_image_size_set(o, buffer->w, buffer->h);
        evas_object_image_alpha_set(o, comp_surface_is_alpha(cs, buffer));
        evas_object_resize(o, buffer->w, buffer->h);
     }
   else
     evas_object_image_pixels_dirty_set(o, 0);
}

static void
comp_render_post_proxied(Comp_Surface *cs, Evas *e, void *event_info)
{
   Comp_Buffer *buffer;

   buffer = cs->buffer[!cs->render_queue];
   //fprintf(stderr, "PROXY RENDER_POST %d\n", buffer ? wl_resource_get_id(buffer->res) : -1);
   buffer->post_renders = eina_list_remove(buffer->post_renders, e);
   if (buffer->post_renders) return;
   if (cs->buffer[0] || cs->dead) comp_surface_buffer_detach(&cs->buffer[1]);
   comp_surface_buffer_post_render(cs);
}

static void
comp_render_post(Comp *c, Evas *e, void *event_info EINA_UNUSED)
{
   Comp_Surface *cs;
   Comp_Seat *s;

   c->rendering = 0;
   EINA_LIST_FREE(c->post_render_queue, cs)
     {
        //fprintf(stderr, "POST(%d)\n", wl_resource_get_id(cs->res));
        cs->buffer[1]->renders = eina_list_remove(cs->buffer[1]->renders, e);
        if ((!cs->buffer[1]->renders) && (!cs->buffer[1]->post_renders))
          {
             if (cs->buffer[0] || cs->dead) comp_surface_buffer_detach(&cs->buffer[1]);
             comp_surface_buffer_post_render(cs);
          }
        cs->post_render_queue = 0;
        if (!cs->dead) continue;
        if (cs->res) wl_resource_destroy(cs->res);
        else evas_object_del(cs->obj);
     }
   EINA_INLIST_FOREACH(c->seats, s)
     {
        Input_Sequence *ev;
        Eina_Inlist *l;

        EINA_INLIST_FOREACH_SAFE(s->ptr.events, l, ev)
          {
             if (!ev->up_serial) continue;
             if (!ev->pass++) continue;
             s->ptr.events = eina_inlist_remove(s->ptr.events, EINA_INLIST_GET(ev));
             free(ev);
          }
        EINA_INLIST_FOREACH_SAFE(s->tch.events, l, ev)
          {
             if (!ev->up_serial) continue;
             if (!ev->pass++) continue;
             s->tch.events = eina_inlist_remove(s->tch.events, EINA_INLIST_GET(ev));
             free(ev);
          }
     }
}

static Evas_Smart *comp_smart = NULL;

static void
comp_seat_selection_update(Comp_Seat *s)
{
   Ecore_Wl2_Offer *offer;
   Eina_Array *arr;
   unsigned int i;
   Eina_Array_Iterator it;
   char *type;

   if (!s->client_seat) return;
   s->selection_changed = 0;
   if (!s->selection_exists)
     {
        s->client_selection_serial = ecore_wl2_dnd_selection_clear(s->client_seat);
        return;
     }
   if (!s->seat) return; /* x11_fixes_selection_notify() */
   offer = ecore_wl2_dnd_selection_get(s->seat);
   if (!offer) return;
   arr = ecore_wl2_offer_mimes_get(offer);
   {
      char *types[eina_array_count(arr) + 1];

      EINA_ARRAY_ITER_NEXT(arr, i, type, it)
        types[i] = type;
      types[i] = NULL;
      s->client_selection_serial = ecore_wl2_dnd_selection_set(s->client_seat, (const char**)types);
   }
}

static void
seat_drag_offer_update(Comp_Seat *s)
{
   Eina_Array *arr;
   char **types;
   unsigned int i;
   Eina_Array_Iterator it;
   char *type;

    // drag.id = probably double proxied
   if (s->drag.id || (!s->client_offer)) return;
   arr = ecore_wl2_offer_mimes_get(s->client_offer);
   types = alloca(sizeof(void*) * (eina_array_count(arr) + 1));

   EINA_ARRAY_ITER_NEXT(arr, i, type, it)
     types[i] = type;
   types[i] = NULL;

   ecore_wl2_dnd_drag_types_set(s->client_seat, (const char**)types);
   ecore_wl2_dnd_set_actions(s->client_seat);
}

static void
seat_proxy_update(Comp_Seat *s)
{
   Eina_Iterator *it;
   Ecore_Wl2_Input *input;

   if (!s->name) return;
   it = ecore_wl2_display_inputs_get(s->c->client_disp);
   EINA_ITERATOR_FOREACH(it, input)
     {
        const char *name = ecore_wl2_input_name_get(input);

        if (!name) continue;
        if (!eina_streq(s->name, name)) continue;
        s->client_seat = input;
        if (s->selection_changed)
          comp_seat_selection_update(s);
        if (s->client_offer)
          seat_drag_offer_update(s);
        break;
     }
   eina_iterator_free(it);
}

static void
comp_seat_proxy_update(Comp_Seat *s)
{
   Eina_Iterator *it;
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *win;
   int n = 0, i = 0;

   if (s->seat) return;
   win = ecore_evas_wayland2_window_get(ecore_evas_ecore_evas_get(s->c->evas));
   if (win)
     {
        Ecore_Wl2_Display *ewd = ecore_wl2_window_display_get(win);
        it = ecore_wl2_display_inputs_get(ewd);
        EINA_ITERATOR_FOREACH(it, input)
          {
             if (ecore_wl2_input_seat_id_get(input) == evas_device_seat_id_get(s->dev))
               break;
             n++;
          }
        eina_iterator_free(it);
     }
   it = ecore_wl2_display_inputs_get(s->c->parent_disp);
   EINA_ITERATOR_FOREACH(it, input)
     {
        if (!win)
          {
             if (eina_streq(ecore_wl2_input_name_get(input), evas_device_name_get(s->dev)))
               {
                  s->seat = input;
                  break;
               }
          }
        else if (i == n)
          {
             s->seat = input;
             break;
          }
        i++;
     }
   eina_iterator_free(it);
}

static void
comp_device_caps_update(Comp_Seat *s)
{
   const Eina_List *l;
   Eo *dev;
   Eina_Bool kbd;

   if (!s) return;
   kbd = s->keyboard;
   s->keyboard = s->touch = s->pointer = 0;
   EINA_LIST_FOREACH(evas_device_list(s->c->evas, s->dev), l, dev)
     {
        Evas_Device_Class clas = evas_device_class_get(dev);
        s->keyboard |= clas == EVAS_DEVICE_CLASS_KEYBOARD;
        s->touch |= clas == EVAS_DEVICE_CLASS_TOUCH;
        s->pointer |= (clas == EVAS_DEVICE_CLASS_MOUSE ||
                     clas == EVAS_DEVICE_CLASS_TOUCH ||
                     clas == EVAS_DEVICE_CLASS_PEN ||
                     clas == EVAS_DEVICE_CLASS_POINTER ||
                     clas == EVAS_DEVICE_CLASS_WAND);
     }
   if (s->keyboard != kbd)
     {
        if (s->kbd.external)
          seat_kbd_external_init(s);
        else
          {
             if (s->keyboard)
               {
#ifdef HAVE_ECORE_X
                  if ((!s->c->parent_disp) && ecore_x_display_get())
                    x11_kbd_apply(s);
                  else
#endif
                  seat_keymap_create(s);
                  seat_kbd_repeat_rate_update(s);
               }
             else
               {
                  xkb_keymap_unref(s->kbd.keymap);
                  s->kbd.keymap = NULL;
               }
             seat_keymap_update(s);
             s->keyboard = !!s->kbd.state;
          }
     }
   seat_update_caps(s, NULL);
}

static void
comp_seats_proxy(Comp *c)
{
   const Eina_List *l, *ll;
   Eo *dev;

   l = evas_device_list(c->evas, NULL);
   EINA_LIST_FOREACH(l, ll, dev)
     {
        Comp_Seat *s;
        Eina_Bool skip = EINA_FALSE;

        if (evas_device_class_get(dev) != EVAS_DEVICE_CLASS_SEAT) continue;
        EINA_INLIST_FOREACH(c->seats, s)
          if (s->dev == dev)
            {
               skip = EINA_TRUE;
               if (c->parent_disp)
                 comp_seat_proxy_update(s);
               break;
            }
        if (skip) continue;

        s = calloc(1, sizeof(Comp_Seat));
        s->c = c;
        s->dev = dev;
        s->kbd.keymap_fd = -1;
        efl_ref(s->dev);
        if (c->parent_disp)
          comp_seat_proxy_update(s);
        s->name = eina_stringshare_ref(evas_device_name_get(dev));
        s->data_devices = eina_hash_pointer_new(NULL);
        c->seats = eina_inlist_append(c->seats, EINA_INLIST_GET(s));
        wl_array_init(&s->kbd.keys);
        if (s->seat)
          {
             Ecore_Wl2_Seat_Capabilities caps = ecore_wl2_input_seat_capabilities_get(s->seat);
             s->keyboard = !!(caps & ECORE_WL2_SEAT_CAPABILITIES_KEYBOARD);
             s->pointer = !!(caps & ECORE_WL2_SEAT_CAPABILITIES_POINTER);
             s->touch = !!(caps & ECORE_WL2_SEAT_CAPABILITIES_TOUCH);
             if (s->keyboard)
               {
                  if (s->kbd.external)
                    seat_kbd_external_init(s);
                  else
                    {
                       if (s->seat)
                         {
                            s->kbd.keymap = ecore_wl2_input_keymap_get(s->seat);
                            if (s->kbd.keymap) xkb_keymap_ref(s->kbd.keymap);
                         }
                       else
                         {
#ifdef HAVE_ECORE_X
                            if ((!s->c->parent_disp) && ecore_x_display_get())
                              x11_kbd_apply(s);
                            else
#endif
                            seat_keymap_create(s);
                         }
                       seat_keymap_update(s);
                       seat_kbd_repeat_rate_update(s);
                       s->keyboard = !!s->kbd.state;
                    }
               }

          }
        else if (!c->parent_disp)
          comp_device_caps_update(s);
        s->global = wl_global_create(c->display, &wl_seat_interface, 4, s, seat_bind);
        evas_object_smart_callback_call(s->c->obj, "seat_added", dev);
        if (ecore_wl2_display_sync_is_done(c->client_disp))
          seat_proxy_update(s);
     }
}

static void
comp_wayland_features_init(Comp *c)
{
   if (ecore_wl2_display_dmabuf_get(c->parent_disp))
     linux_dmabuf_setup(c->display, c);
}

static Eina_Bool
comp_sync_done(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Sync_Done *ev)
{
   Eina_List *l;
   Comp *c;

   EINA_LIST_FOREACH(comps, l, c)
     {
        if (c->parent_disp == ev->display)
          {
             c->client_disp = ecore_wl2_display_connect(c->env);
             if (!c->seats)
               comp_seats_proxy(c);
             comp_wayland_features_init(c);
          }
        else if (c->client_disp == ev->display)
          {
             Eina_Iterator *it;
             Ecore_Wl2_Input *input;

             it = ecore_wl2_display_inputs_get(c->client_disp);
             EINA_ITERATOR_FOREACH(it, input)
               {
                  Comp_Seat *s;
                  EINA_INLIST_FOREACH(c->seats, s)
                    {
                       if (!eina_streq(s->name, ecore_wl2_input_name_get(input)))
                         continue;
                       s->client_seat = input;
                       if (s->selection_changed)
                         comp_seat_selection_update(s);
                       if (s->client_offer)
                         seat_drag_offer_update(s);
                       break;
                    }
               }
             eina_iterator_free(it);
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
comp_seat_selection_changed(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Seat_Selection *ev)
{
   Eina_List *l;
   Comp *c;
   Comp_Seat *s;

   EINA_LIST_FOREACH(comps, l, c)
     if (c->parent_disp == ev->display)
       EINA_INLIST_FOREACH(c->seats, s)
         {
            if (s->seat && (ecore_wl2_input_seat_id_get(s->seat) != ev->seat)) continue;
            s->selection_changed = 1;
            s->selection_exists = !!ecore_wl2_dnd_selection_get(s->seat);
            if (!s->focused) continue;
            comp_seat_selection_update(s);
         }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
comp_seat_caps_handler(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Seat_Capabilities *ev)
{
   Eina_List *l;
   Comp *c;
   Comp_Seat *s;

   EINA_LIST_FOREACH(comps, l, c)
     if (c->parent_disp == ev->display)
       EINA_INLIST_FOREACH(c->seats, s)
         {
            if (ecore_wl2_input_seat_id_get(s->seat) != ev->id) continue;
            s->pointer = ev->pointer_enabled;
            if (s->keyboard != ev->keyboard_enabled)
              {
                 if (s->kbd.external)
                   seat_kbd_external_init(s);
                 else
                   {
                      if (ev->keyboard_enabled)
                        {
                           s->kbd.keymap = ecore_wl2_input_keymap_get(s->seat);
                           if (s->kbd.keymap) xkb_keymap_ref(s->kbd.keymap);
                           seat_kbd_repeat_rate_update(s);
                        }
                      else
                        {
                           xkb_keymap_unref(s->kbd.keymap);
                           s->kbd.keymap = NULL;
                        }
                      seat_keymap_update(s);
                   }
              }
            s->keyboard = !!ev->keyboard_enabled && !!s->kbd.state;
            s->touch = ev->touch_enabled;
            seat_update_caps(s, NULL);
         }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
comp_seat_name_changed(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Seat_Name *ev)
{
   Eina_List *l;
   Comp *c;
   Comp_Seat *s;

   EINA_LIST_FOREACH(comps, l, c)
     if (c->parent_disp == ev->display)
       EINA_INLIST_FOREACH(c->seats, s)
         {
            Eina_List *lll;
            struct wl_resource *res;

            if (ecore_wl2_input_seat_id_get(s->seat) != ev->id) continue;
            eina_stringshare_replace(&s->name, ev->name);
            EINA_LIST_FOREACH(s->resources, lll, res)
              if (wl_resource_get_version(res) >= WL_SEAT_NAME_SINCE_VERSION)
                wl_seat_send_name(res, s->name);
            seat_proxy_update(s);
            break;
         }
     else if (c->client_disp == ev->display)
       EINA_INLIST_FOREACH(c->seats, s)
         if (!s->client_seat) seat_proxy_update(s);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
comp_seat_keymap_changed(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Seat_Keymap_Changed *ev)
{
   Eina_List *l;
   Comp *c;
   Comp_Seat *s;

   EINA_LIST_FOREACH(comps, l, c)
     if (c->parent_disp == ev->display)
       EINA_INLIST_FOREACH(c->seats, s)
         {
            struct xkb_keymap *keymap;

            if (s->kbd.external) continue;
            if (ecore_wl2_input_seat_id_get(s->seat) != ev->id) continue;

            if (s->kbd.keymap) xkb_map_unref(s->kbd.keymap);
            s->kbd.keymap = NULL;
            keymap = ecore_wl2_input_keymap_get(s->seat);
            if (keymap) s->kbd.keymap = xkb_keymap_ref(keymap);
            seat_keymap_update(s);
            if (s->keyboard != (!!s->kbd.state))
              {
                 s->keyboard = !!s->kbd.state;
                 seat_update_caps(s, NULL);
              }
         }
   return ECORE_CALLBACK_RENEW;
}

static void
seat_kbd_repeat_rate_send(Comp_Seat *s)
{
   Eina_List *ll, *lll;
   struct wl_resource *res;
   Eina_Iterator *it;

   it = eina_hash_iterator_data_new(s->kbd.resources);
   EINA_ITERATOR_FOREACH(it, ll)
     EINA_LIST_FOREACH(ll, lll, res)
       if (wl_resource_get_version(res) >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
         wl_keyboard_send_repeat_info(res, s->kbd.repeat_rate, s->kbd.repeat_delay);
   eina_iterator_free(it);
}

static Eina_Bool
comp_seat_keyboard_repeat_changed(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Seat_Keyboard_Repeat_Changed *ev)
{
   Eina_List *l;
   Comp *c;
   Comp_Seat *s;

   EINA_LIST_FOREACH(comps, l, c)
     if (c->parent_disp == ev->display)
       EINA_INLIST_FOREACH(c->seats, s)
         {
            if (ecore_wl2_input_seat_id_get(s->seat) != ev->id) continue;
            if (s->kbd.external) continue;

            seat_kbd_repeat_rate_update(s);
            seat_kbd_repeat_rate_send(s);
         }
   return ECORE_CALLBACK_RENEW;
}

static void
comp_seat_key_send(Comp_Seat *s, xkb_keycode_t key, int dir, unsigned int timestamp, Eina_Bool mods)
{
   Eina_List *l, *ll;
   struct wl_resource *res;
   uint32_t serial = wl_display_next_serial(s->c->display);
   uint32_t wl[] = { WL_KEYBOARD_KEY_STATE_PRESSED, WL_KEYBOARD_KEY_STATE_RELEASED };

   l = seat_kbd_active_resources_get(s);

   EINA_LIST_FOREACH(l, ll, res)
     {
        wl_keyboard_send_key(res, serial, timestamp, key, wl[dir]);
        if (mods) comp_seat_send_modifiers(s, res, serial);
     }
}

static void
comp_seat_key_update(Comp_Seat *s, xkb_keycode_t key, int dir, unsigned int timestamp)
{
   uint32_t xkb[] = { XKB_KEY_DOWN, XKB_KEY_UP };
   Eina_Bool mods = EINA_FALSE;

   if (s->kbd.external || xkb_state_update_key(s->kbd.state, key + 8, xkb[dir]))
     mods = seat_kbd_mods_update(s);
   if (s->focused)
     comp_seat_key_send(s, key, dir, timestamp, mods);
}

static Eina_Bool
comp_key_down(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Event_Key *ev)
{
   uint32_t keycode;
   Comp *c;
   Eina_List *l;
   Comp_Seat *s;
   const Eo *dev = evas_device_parent_get(ev->dev);

   keycode = (ev->keycode - 8);
   EINA_LIST_FOREACH(comps, l, c)
     EINA_INLIST_FOREACH(c->seats, s)
       {
          if (c->parent_disp && (dev != s->dev)) continue;

          if (s->kbd.external)
            {
               /* only doing passthrough in external mode */
               if (!s->focused) return ECORE_CALLBACK_RENEW;
            }
          else
            {
               uint32_t *end, *k;

               end = (uint32_t*)s->kbd.keys.data + (s->kbd.keys.size / sizeof(uint32_t));
               for (k = s->kbd.keys.data; k < end; k++)
                 if (*k == keycode) return ECORE_CALLBACK_RENEW;

               s->kbd.keys.size = (char*)end - (char*)s->kbd.keys.data;
               k = wl_array_add(&s->kbd.keys, sizeof(uint32_t));
               *k = keycode;
            }
          comp_seat_key_update(s, keycode, 0, ev->timestamp);
       }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
comp_key_up(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Event_Key *ev)
{
   uint32_t keycode;
   Comp *c;
   Eina_List *l;
   Comp_Seat *s;
   const Eo *dev = evas_device_parent_get(ev->dev);

   keycode = (ev->keycode - 8);
   EINA_LIST_FOREACH(comps, l, c)
     EINA_INLIST_FOREACH(c->seats, s)
       {
          if (c->parent_disp && (dev != s->dev)) continue;

          if (s->kbd.external)
            {
               /* only doing passthrough in external mode */
               if (!s->focused) return ECORE_CALLBACK_RENEW;
            }
          else
            {
               uint32_t *end, *k;

               end = (uint32_t*)s->kbd.keys.data + (s->kbd.keys.size / sizeof(uint32_t));

               for (k = s->kbd.keys.data; k < end; k++)
                 if (*k == keycode)
                   {
                      *k = end[-1];
                      s->kbd.keys.size = (char*)end - (char*)s->kbd.keys.data - 1;
                      break;
                   }
            }
          comp_seat_key_update(s, keycode, 1, ev->timestamp);
       }
   return ECORE_CALLBACK_RENEW;
}

static void
comp_device_add(void *d, const Efl_Event *ev)
{
   Comp *c = d;

   if (efl_input_device_type_get(ev->info) == EFL_INPUT_DEVICE_TYPE_SEAT)
     comp_seats_proxy(c);
   else if (!c->parent_disp)
     comp_device_caps_update(comp_seat_find(c, ev->info));
}

static void
comp_device_del(void *d, const Efl_Event *ev)
{
   Comp *c = d;
   Eo *seat;
   Eina_Inlist *l;
   Comp_Seat *s;

   if (efl_input_device_type_get(ev->info) == EFL_INPUT_DEVICE_TYPE_SEAT)
     seat = ev->info;
   else
     seat = efl_input_device_seat_get(ev->info);
   EINA_INLIST_FOREACH_SAFE(c->seats, l, s)
     if (seat == s->dev)
       {
          if (ev->info == seat)
            seat_destroy(s);
          else if (!c->parent_disp)
            comp_device_caps_update(s);
       }
}

static Eina_Bool
comp_data_device_dnd_leave(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Dnd_Leave *ev)
{
   Comp *c;
   Comp_Seat *s;
   Eina_List *l;
   EINA_LIST_FOREACH(comps, l, c)
     {
        if (ev->display != c->parent_disp) continue;
        EINA_INLIST_FOREACH(c->seats, s)
          {
             if (ecore_wl2_input_seat_id_get(s->seat) != ev->seat) continue;
             if (s->drag.id) continue; // probably double proxied
             if (s->drag.source)
               wl_data_source_send_cancelled(s->drag.source->res);
             if (s->client_seat && s->drag.res)
               {
                  ecore_wl2_dnd_drag_end(s->client_seat);
                  s->client_offer = NULL;
               }
             if (s->drag.proxy_win)
               {
                  if (s->seat)
                    ecore_wl2_dnd_drag_end(s->seat);
                  ecore_evas_free(s->drag.proxy_win);
               }
             seat_drag_end(s);
             s->drag.source = NULL;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
comp_data_device_dnd_end(void *d, int t, Ecore_Wl2_Event_Data_Source_End *ev)
{
   Comp *c;
   Comp_Seat *s;
   Eina_List *l;

   EINA_LIST_FOREACH(comps, l, c)
     {
        if (ev->display != c->parent_disp) continue;
        EINA_INLIST_FOREACH(c->seats, s)
          {
             if (ecore_wl2_input_seat_id_get(s->seat) != ev->seat) continue;
             if (!s->drag.source) continue;
             if (s->drag.source->proxy_serial != ev->serial) continue;
             if (ev->cancelled)
               wl_data_source_send_cancelled(s->drag.source->res);
             else
               data_source_notify_finish(s->drag.source);
             if (s->drag.proxy_win)
               {
                  if (s->seat)
                    ecore_wl2_dnd_drag_end(s->seat);
                  ecore_evas_free(s->drag.proxy_win);
               }
             seat_drag_end(s);
             s->drag.source = NULL;
             s->client_offer = NULL;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
comp_data_device_dnd_enter(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Dnd_Enter *ev)
{
   Comp *c;
   Comp_Seat *s;
   Eina_List *l;

   EINA_LIST_FOREACH(comps, l, c)
     {
        if (ev->display != c->parent_disp) continue;
        EINA_INLIST_FOREACH(c->seats, s)
          {
             if (ecore_wl2_input_seat_id_get(s->seat) != ev->seat) continue;
             s->client_offer = ev->offer;
             seat_drag_offer_update(s);
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
comp_data_device_dnd_motion(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Dnd_Motion *ev)
{
   Comp *c;
   Comp_Seat *s;
   Eina_List *l;
   int x, y, w, h, ex, ey;
   Eina_Bool found = EINA_FALSE;

   EINA_LIST_FOREACH(comps, l, c)
     {
        int fx, fy;

        if (ev->display != c->parent_disp) continue;
        evas_output_framespace_get(c->evas, &fx, &fy, NULL, NULL);
        ex = ev->x - fx;
        ey = ev->y - fy;
        evas_object_geometry_get(c->obj, &x, &y, &w, &h);
        if (!COORDS_INSIDE(ex, ey, x, y, w, h)) continue;
        found = EINA_TRUE;
        break;
     }
   if (!found) return ECORE_CALLBACK_RENEW;

   EINA_INLIST_FOREACH(c->seats, s)
     if (ecore_wl2_input_seat_id_get(s->seat) == ev->seat)
       {
          dnd_motion(s, ex, ey);
          break;
       }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
comp_data_device_dnd_drop(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Wl2_Event_Dnd_Drop *ev)
{
   Comp *c;
   Comp_Seat *s;
   Eina_List *l;

   EINA_LIST_FOREACH(comps, l, c)
     {
        if (c->parent_disp != ev->display) continue;
        EINA_INLIST_FOREACH(c->seats, s)
          if (ecore_wl2_input_seat_id_get(s->seat) == ev->seat)
            {
               if (s->drag.enter)
                 {
                    drag_grab_button(s, 0, s->drag.id, WL_POINTER_BUTTON_STATE_RELEASED);
                    if (s->drag.proxy_win)
                      {
                         ecore_wl2_dnd_drag_end(s->seat);
                         ecore_evas_free(s->drag.proxy_win);
                      }
                    return ECORE_CALLBACK_RENEW;
                 }
            }
     }
   return ECORE_CALLBACK_RENEW;
}

static void
comp_handlers_init(void)
{
   Ecore_Event_Handler *h;

   h = ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_CAPABILITIES_CHANGED,
     (Ecore_Event_Handler_Cb)comp_seat_caps_handler, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED,
     (Ecore_Event_Handler_Cb)comp_seat_keymap_changed, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_NAME_CHANGED,
     (Ecore_Event_Handler_Cb)comp_seat_name_changed, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_SYNC_DONE,
            (Ecore_Event_Handler_Cb)comp_sync_done, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED,
            (Ecore_Event_Handler_Cb)comp_seat_keyboard_repeat_changed, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_SELECTION,
            (Ecore_Event_Handler_Cb)comp_seat_selection_changed, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_DND_MOTION,
            (Ecore_Event_Handler_Cb)comp_data_device_dnd_motion, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_DND_ENTER,
            (Ecore_Event_Handler_Cb)comp_data_device_dnd_enter, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_DND_LEAVE,
            (Ecore_Event_Handler_Cb)comp_data_device_dnd_leave, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_END,
            (Ecore_Event_Handler_Cb)comp_data_device_dnd_end, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_WL2_EVENT_DND_DROP,
            (Ecore_Event_Handler_Cb)comp_data_device_dnd_drop, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
            (Ecore_Event_Handler_Cb)comp_key_down, NULL);
   handlers = eina_list_append(handlers, h);
   h = ecore_event_handler_add(ECORE_EVENT_KEY_UP,
            (Ecore_Event_Handler_Cb)comp_key_up, NULL);
   handlers = eina_list_append(handlers, h);
}

static void
comp_focus_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Comp *c = data;
   //Efl_Input_Focus_Data *ev = event_info;
   Comp_Seat *s;

   EINA_INLIST_FOREACH(c->seats, s)
     {
        Eina_List *l, *ll;
        struct wl_resource *res;
        uint32_t serial;

        //if (ev->device != s->dev) continue;
        if (evas_seat_focus_get(c->evas, s->dev) != c->obj) continue;
        s->focused = 1;
        if (s->selection_changed)
          comp_seat_selection_update(s);
        if ((!s->kbd.keymap) && (!s->kbd.state)) continue;
        if (s->kbd.external)
          {
             Eina_Bool mods = seat_kbd_mods_update(s);
             if (!s->focused) return;
             l = seat_kbd_active_resources_get(s);

             EINA_LIST_FOREACH(l, ll, res)
               {
                  uint32_t *end, *k;
                  serial = wl_display_next_serial(s->c->display);

                  end = (uint32_t*)s->kbd.keys_external->data + (s->kbd.keys_external->size / sizeof(uint32_t));
                  if (mods) comp_seat_send_modifiers(s, res, serial);

                  for (k = s->kbd.keys_external->data; k < end; k++)
                    comp_seat_key_send(s, *k, 0, 0, mods);
               }
             return;
          }
        if (!seat_kbd_mods_update(s)) continue;
        l = seat_kbd_active_resources_get(s);
        if (!l) continue;
        serial = wl_display_next_serial(s->c->display);
        EINA_LIST_FOREACH(l, ll, res)
          comp_seat_send_modifiers(s, res, serial);
     }
}

static void
comp_focus_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Comp *c = data;
   //Efl_Input_Focus_Data *ev = event_info;
   Comp_Seat *s;

   EINA_INLIST_FOREACH(c->seats, s)
     {
        //if (ev->device != s->dev) continue;
        if (evas_seat_focus_get(c->evas, s->dev) != c->obj)
          s->focused = 0;
     }
}

static void
comp_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Comp *c = data;
   Evas_Event_Mouse_In *ev = event_info;
   Comp_Seat *s;
   s = comp_seat_find(c, ev->dev);

   if (!s->event_propagate)
     {
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
     }
   s->event_propagate = 0;
   if (s->drag.proxy_win)
     {
        ecore_evas_free(s->drag.proxy_win);
#ifdef HAVE_ECORE_X
        if (ecore_x_display_get())
          {
             ecore_x_dnd_abort(ecore_evas_window_get(ecore_evas_ecore_evas_get(s->c->evas)));
             s->drag.x11_owner = 0;
          }
#endif
        if (s->ptr.cursor.surface)
          evas_object_show(s->ptr.cursor.surface->obj);
     }
   if (s->drag.surface)
     evas_object_show(s->drag.surface->obj);
   s->ptr.in = 1;
   seat_ptr_inherit(s, ev->dev);
   ecore_evas_cursor_device_unset(ecore_evas_ecore_evas_get(e), ev->dev);
   if (s->ptr.efl.obj) evas_object_hide(s->ptr.efl.obj);
   if (!s->ptr.cursor.surface) return;
   s->ptr.cursor.surface->role = s->ptr.cursor.surface->res;
   ecore_evas_object_cursor_device_set(ecore_evas_ecore_evas_get(e), ev->dev,
     s->ptr.cursor.surface->obj, EVAS_LAYER_MAX, s->ptr.cursor.x, s->ptr.cursor.y);
}

static void
comp_multi_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Comp *c = data;
   Evas_Event_Multi_Move *ev = event_info;
   Comp_Seat *s;
   int w, h;

   s = comp_seat_find(c, ev->dev);
   if (!s->event_propagate)
     {
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
     }
   s->event_propagate = 0;
   s->tch.pos.x = ev->cur.canvas.x;
   s->tch.pos.y = ev->cur.canvas.y;
   if ((!s->drag.tch) || (!s->drag.surface)) return;
   evas_object_geometry_get(s->drag.surface->obj, NULL, NULL, &w, &h);
   evas_object_move(s->drag.surface->obj, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
comp_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Comp *c = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Comp_Seat *s;
   int w, h;

   s = comp_seat_find(c, ev->dev);
   if (!s->event_propagate)
     {
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
     }
   s->event_propagate = 0;
   s->ptr.pos.x = ev->cur.canvas.x;
   s->ptr.pos.y = ev->cur.canvas.y;
   if (s->drag.tch || (!s->drag.surface)) return;
   evas_object_geometry_get(s->drag.surface->obj, NULL, NULL, &w, &h);
   evas_object_move(s->drag.surface->obj, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
comp_mouse_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Comp *c = data;
   Evas_Event_Mouse_Out *ev = event_info;
   Comp_Seat *s;
   Eina_List *l;
   const char **types, *type;
   unsigned int i = 0;

   s = comp_seat_find(c, ev->dev);
   if (!s->event_propagate)
     {
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
     }
   s->event_propagate = 0;
   if (!s->ptr.in) return;
   s->ptr.in = 0;
   ecore_evas_cursor_device_unset(ecore_evas_ecore_evas_get(e), ev->dev);
   if (s->ptr.efl.obj)
     {
        ecore_evas_object_cursor_device_set(ecore_evas_ecore_evas_get(e), ev->dev,
          s->ptr.efl.obj, s->ptr.efl.layer, s->ptr.efl.x, s->ptr.efl.y);
        seat_ptr_del(s, NULL, NULL, NULL);
     }
   if (s->ptr.cursor.surface)
     {
        s->ptr.cursor.surface->role = NULL;
        evas_object_hide(s->ptr.cursor.surface->obj);
     }
   if ((!s->drag.res) || (!s->drag.source) || s->drag.source->proxy) return;
   if (s->drag.enter) comp_surface_send_data_device_leave(s->drag.enter, s);
   if (s->drag.surface)
     seat_drag_proxy_win_add(s);
   types = alloca(sizeof(void*) * (eina_list_count(s->drag.source->mime_types) + 1));
   EINA_LIST_FOREACH(s->drag.source->mime_types, l, type)
     types[i++] = type;
   types[i] = NULL;
   if (s->c->parent_disp)
     {
        Ecore_Wl2_Window *win = NULL;

        if (s->drag.proxy_win)
          win = ecore_evas_wayland2_window_get(s->drag.proxy_win);
        ecore_wl2_dnd_drag_types_set(s->seat, types);
        s->drag.source->proxy_serial = ecore_wl2_dnd_drag_start(s->seat,
          ecore_evas_wayland2_window_get(ecore_evas_ecore_evas_get(s->c->evas)),
          win);
        s->drag.source->proxy_send_handler =
          ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND,
                           data_device_proxy_send_send, s->drag.source);
     }
#ifdef HAVE_ECORE_X
   else if (ecore_x_display_get())
     {
        Ecore_Window win = ecore_evas_window_get(ecore_evas_ecore_evas_get(s->c->evas));
        Ecore_Window xwin = ecore_evas_window_get(s->drag.proxy_win);
        Ecore_X_Atom actions[] = { ECORE_X_DND_ACTION_MOVE, ECORE_X_DND_ACTION_COPY };

        if (s->drag.proxy_win && s->drag.surface)
          {
             int x, y, ex, ey;
             ecore_evas_geometry_get(ecore_evas_ecore_evas_get(s->c->evas), &ex, &ey, NULL, NULL);
             evas_object_geometry_get(s->drag.surface->obj, &x, &y, NULL, NULL);
             x += ex, y += ey;
             ecore_x_window_ignore_set(xwin, 1);
             ecore_evas_move(s->drag.proxy_win, x, y);
          }
        s->drag.x11_owner = win;

        ecore_x_dnd_types_set(win, types, i);
        ecore_x_dnd_actions_set(win, actions, EINA_C_ARRAY_LENGTH(actions));
        ecore_x_dnd_self_begin(win, NULL, 0);
        ecore_x_dnd_callback_pos_update_set(x11_dnd_move, s->drag.proxy_win);
     }
#endif
}

EFL_CALLBACKS_ARRAY_DEFINE(comp_device_cbs,
  { EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED, comp_device_add },
  { EFL_CANVAS_SCENE_EVENT_DEVICE_REMOVED, comp_device_del });

static void
hints_set_aspect(struct wl_client *client, struct wl_resource *resource, struct wl_resource *surface, uint32_t width, uint32_t height, uint32_t aspect)
{
   Comp_Surface *cs = wl_resource_get_user_data(surface);
   evas_object_size_hint_aspect_set(cs->obj, aspect, width, height);
   if (cs == cs->c->active_surface)
     shell_surface_aspect_update(cs);
}

static void
hints_set_weight(struct wl_client *client, struct wl_resource *resource, struct wl_resource *surface, int w, int h)
{
   Comp_Surface *cs = wl_resource_get_user_data(surface);
   cs->hint_set_weight = 1;
   evas_object_size_hint_weight_set(cs->obj, w / 100., h / 100.);
}

static const struct efl_hints_interface hints_interface =
{
   hints_set_aspect,
   hints_set_weight,
};

static void
efl_hints_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   struct wl_resource *res;

   if (!client_allowed_check(data, client)) return;
   res = wl_resource_create(client, &efl_hints_interface, version, id);
   wl_resource_set_implementation(res, &hints_interface, data, NULL);
}



static void
comp_smart_add(Evas_Object *obj)
{
   Comp *c;
   char *env, *dbg = NULL;

   c = calloc(1, sizeof(Comp));
   c->wayland_time_base = ecore_loop_time_get();
   c->obj = obj;
   evas_object_smart_data_set(obj, c);
   env = getenv("WAYLAND_DISPLAY");
   if (env) env = strdup(env);

   if (getenv("EFL_WL_DEBUG"))
     {
        dbg = eina_strdup(getenv("WAYLAND_DEBUG"));
        setenv("WAYLAND_DEBUG", "1", 1);
     }
   c->disp = ecore_wl2_display_create(NULL);
   if (getenv("EFL_WL_DEBUG"))
     {
        if (dbg) setenv("WAYLAND_DEBUG", dbg, 1);
        else unsetenv("WAYLAND_DEBUG");
        free(dbg);
     }
   c->env = eina_strdup(getenv("WAYLAND_DISPLAY"));
   if (env) setenv("WAYLAND_DISPLAY", env, 1);
   else unsetenv("WAYLAND_DISPLAY");
   c->display = ecore_wl2_display_get(c->disp);
   c->client_surfaces = eina_hash_pointer_new(NULL);
   c->evas = evas_object_evas_get(obj);
   c->clip = evas_object_rectangle_add(c->evas);
   evas_object_smart_member_add(c->clip, obj);
   c->events = evas_object_rectangle_add(c->evas);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_IN, comp_mouse_in, c);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE, comp_mouse_move, c);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MULTI_MOVE, comp_multi_move, c);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_OUT, comp_mouse_out, c);
   evas_object_color_set(c->events, 0, 0, 0, 0);
   evas_object_smart_member_add(c->events, obj);
   evas_object_show(c->events);
   evas_object_lower(c->events);

   wl_global_create(c->display, &wl_compositor_interface, 4, c, comp_bind);
   wl_global_create(c->display, &wl_subcompositor_interface, 1, c, subcomp_bind);
   wl_global_create(c->display, &wl_output_interface, 2, c, output_bind);
   wl_global_create(c->display, &xdg_wm_base_interface, 1, c, shell_bind);
   wl_global_create(c->display, &wl_data_device_manager_interface, 3, c, data_device_manager_bind);
   wl_global_create(c->display, &efl_hints_interface, 1, c, efl_hints_bind);
   wl_display_init_shm(c->display);

   if (env)
     {
        c->parent_disp = ecore_wl2_display_connect(env);
        if (ecore_wl2_display_sync_is_done(c->parent_disp))
          {
             c->client_disp = ecore_wl2_display_connect(c->env);
             comp_seats_proxy(c);
             comp_wayland_features_init(c);
          }
     }
   else
     {
        c->client_disp = ecore_wl2_display_connect(c->env);
        comp_seats_proxy(c);
#ifdef HAVE_ECORE_X
        if (ecore_x_display_get())
          {
             ecore_x_xkb_track_state();
             // if proxiedallowed
             ecore_x_dnd_aware_set(ecore_evas_window_get(ecore_evas_ecore_evas_get(c->evas)), EINA_TRUE);
             if (!comps) x11_init();
          }
#endif
     }

   c->gl = evas_gl_new(c->evas);
   if (c->gl) comp_gl_init(c);
   evas_event_callback_add(c->evas, EVAS_CALLBACK_RENDER_PRE, (Evas_Event_Cb)comp_render_pre, c);
   evas_event_callback_add(c->evas, EVAS_CALLBACK_RENDER_POST, (Evas_Event_Cb)comp_render_post, c);
   efl_event_callback_array_add(c->evas, comp_device_cbs(), c);

   evas_object_event_callback_add(c->obj, EVAS_CALLBACK_FOCUS_IN, comp_focus_in, c);
   evas_object_event_callback_add(c->obj, EVAS_CALLBACK_FOCUS_OUT, comp_focus_out, c);

   if (!comps) comp_handlers_init();
   comps = eina_list_append(comps, c);
   free(env);
}

static void
comp_smart_del(Evas_Object *obj)
{
   Comp *c = evas_object_smart_data_get(obj);

   evas_object_del(c->clip);
   evas_object_del(c->events);
   free(c->env);
   if (c->gl) comp_gl_shutdown(c);
   while (c->shells)
     {
        Shell_Data *sd = EINA_INLIST_CONTAINER_GET(c->shells, Shell_Data);
        wl_resource_destroy(sd->res);
     }
   while (c->seats)
     {
        Comp_Seat *s = EINA_INLIST_CONTAINER_GET(c->seats, Comp_Seat);
        seat_destroy(s);
     }
   while (c->surfaces)
     {
        Comp_Surface *cs = EINA_INLIST_CONTAINER_GET(c->surfaces, Comp_Surface);
        evas_object_del(cs->obj);
     }
   eina_hash_free(c->client_surfaces);
   if (c->parent_disp) ecore_wl2_display_disconnect(c->parent_disp);
   ecore_wl2_display_destroy(c->disp);
   ecore_wl2_display_disconnect(c->client_disp);

   evas_event_callback_del_full(c->evas, EVAS_CALLBACK_RENDER_PRE, (Evas_Event_Cb)comp_render_pre, c);
   evas_event_callback_del_full(c->evas, EVAS_CALLBACK_RENDER_POST, (Evas_Event_Cb)comp_render_post, c);
   efl_event_callback_array_del(c->evas, comp_device_cbs(), c);
   ecore_event_handler_del(c->exe_handler);
   eina_hash_free(c->exes);
   comps = eina_list_remove(comps, c);
   free(c);
   if (!comps)
     {
        void *h;
        EINA_LIST_FREE(handlers, h)
          ecore_event_handler_del(h);
#ifdef HAVE_ECORE_X
        x11_shutdown();
#endif
     }
   ecore_wl2_shutdown();
}

static void
comp_smart_move(Evas_Object *obj, int x, int y)
{
   Comp_Surface *cs;
   Comp *c = evas_object_smart_data_get(obj);
   int dx, dy;
   evas_object_geometry_get(obj, &dx, &dy, NULL, NULL);
   dx = x - dx;
   dy = y - dy;
   evas_object_move(c->clip, x, y);
   evas_object_move(c->events, x, y);
   EINA_INLIST_FOREACH(c->surfaces, cs)
     {
        int cx, cy;
        evas_object_geometry_get(cs->obj, &cx, &cy, NULL, NULL);
        evas_object_move(cs->obj, cx + dx, cy + dy);
     }
}

static void
comp_smart_resize(Evas_Object *obj, int w, int h)
{
   Comp *c = evas_object_smart_data_get(obj);
   Eina_List *l;
   Comp_Surface *cs;
   struct wl_resource *res;

   evas_object_resize(c->clip, w, h);
   evas_object_resize(c->events, w, h);
   EINA_LIST_FOREACH(c->output_resources, l, res)
     output_resize(c, res);
   //fprintf(stderr, "COMP %dx%d\n", w, h);
   EINA_INLIST_FOREACH(c->surfaces, cs)
     if (cs->shell.surface && cs->role && (!cs->extracted))
       shell_surface_send_configure(cs);
}

static void
comp_smart_show(Evas_Object *obj)
{
   Comp *c = evas_object_smart_data_get(obj);
   Comp_Surface *cs;

   evas_object_show(c->clip);
   EINA_INLIST_FOREACH(c->surfaces, cs)
     comp_surface_output_leave(cs);
}

static void
comp_smart_hide(Evas_Object *obj)
{
   Comp *c = evas_object_smart_data_get(obj);
   Comp_Surface *cs;

   evas_object_hide(c->clip);
   EINA_INLIST_FOREACH(c->surfaces, cs)
     comp_surface_output_leave(cs);
}

static void
comp_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Comp *c = evas_object_smart_data_get(obj);
   evas_object_color_set(c->clip, r, g, b, a);
}

static void
comp_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   Comp *c = evas_object_smart_data_get(obj);
   evas_object_clip_set(c->clip, clip);
}

static void
comp_smart_clip_unset(Evas_Object *obj)
{
   Comp *c = evas_object_smart_data_get(obj);
   evas_object_clip_unset(c->clip);
}

static void
comp_smart_member_add(Evas_Object *obj, Evas_Object *child)
{
   Comp *c = evas_object_smart_data_get(obj);
   if (child != c->clip) evas_object_clip_set(child, c->clip);
}

static void
comp_smart_member_del(Evas_Object *obj, Evas_Object *child)
{
   Comp *c = evas_object_smart_data_get(obj);
   if (child != c->clip) evas_object_clip_unset(child);
}

static void
comp_smart_init(void)
{
   if (comp_smart) return;
   {
      static const Evas_Smart_Class sc =
      {
         "comp",
         EVAS_SMART_CLASS_VERSION,
         comp_smart_add,
         comp_smart_del,
         comp_smart_move,
         comp_smart_resize,
         comp_smart_show,
         comp_smart_hide,
         comp_smart_color_set,
         comp_smart_clip_set,
         comp_smart_clip_unset,
         NULL,
         comp_smart_member_add,
         comp_smart_member_del,

         NULL,
         NULL,
         NULL,
         NULL
      };
      comp_smart = evas_smart_class_new(&sc);
   }
}

static Eina_Bool
exe_event_del(void *data, int t EINA_UNUSED, Ecore_Exe_Event_Del *ev)
{
   Comp *c = data;
   int32_t pid = ev->pid;

   eina_hash_del_by_key(c->exes, &pid);
   return ECORE_CALLBACK_RENEW;
}

# ifdef __GNUC__
#  if __GNUC__ >= 4
__attribute__ ((visibility("hidden")))
# endif
#endif
Eina_Bool
comp_dmabuf_test(struct linux_dmabuf_buffer *dmabuf)
{
   Evas_Native_Surface ns;
   Evas_Object *test;
   int size;
   void *data;
   Comp *c = dmabuf->compositor;

   if (c->gl)
     {
        Eina_Bool ret;
        ns.type = EVAS_NATIVE_SURFACE_WL_DMABUF;
        ns.version = EVAS_NATIVE_SURFACE_VERSION;
        ns.data.wl_dmabuf.attr = &dmabuf->attributes;
        ns.data.wl_dmabuf.resource = NULL;
        ns.data.wl_dmabuf.scanout.handler = NULL;
        ns.data.wl_dmabuf.scanout.data = NULL;
        test = evas_object_image_add(c->evas);
        evas_object_image_native_surface_set(test, &ns);
        ret = evas_object_image_load_error_get(test) == EVAS_LOAD_ERROR_NONE;
        evas_object_del(test);
        if (!ns.data.wl_dmabuf.attr) return EINA_FALSE;
        return ret;
     }

   /* TODO: Software rendering for multi-plane formats */
   if (dmabuf->attributes.n_planes != 1) return EINA_FALSE;
   if (dmabuf->attributes.format != DRM_FORMAT_ARGB8888 &&
       dmabuf->attributes.format != DRM_FORMAT_XRGB8888) return EINA_FALSE;

   /* This is only legit for ARGB8888 */
   size = dmabuf->attributes.height * dmabuf->attributes.stride[0];
   data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dmabuf->attributes.fd[0], 0);
   if (data == MAP_FAILED) return EINA_FALSE;
   munmap(data, size);

   return EINA_TRUE;
}

void
comp_dmabuf_formats_query(void *compositor EINA_UNUSED, int **formats EINA_UNUSED, int *num_formats)
{
   *num_formats = 0;
}

void
comp_dmabuf_modifiers_query(void *compositor EINA_UNUSED, int format EINA_UNUSED, uint64_t **modifiers EINA_UNUSED, int *num_modifiers)
{
   *num_modifiers = 0;
}

Evas_Object *
efl_wl_add(Evas *e)
{
   comp_smart_init();
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!ecore_wl2_init(), NULL);
   return evas_object_smart_add(e, comp_smart);
}

Ecore_Exe *
comp_run(Evas_Object *obj, const char *cmd, Ecore_Exe_Flags flags)
{
   char *env, *disp, *gl = NULL;
   Comp *c;
   Ecore_Exe *exe;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   if (!c->exes)
     c->exes = eina_hash_int32_new(NULL);
   if (!c->exe_handler)
     c->exe_handler =
       ecore_event_handler_add(ECORE_EXE_EVENT_DEL, (Ecore_Event_Handler_Cb)exe_event_del, c);
   disp = getenv("DISPLAY");
   if (disp) disp = strdup(disp);
   unsetenv("DISPLAY");
   env = getenv("WAYLAND_DISPLAY");
   if (env) env = strdup(env);
   setenv("WAYLAND_DISPLAY", c->env, 1);
   if (c->gl)
     {
        gl = getenv("ELM_ACCEL");
        if (gl) gl = strdup(gl);
        setenv("ELM_ACCEL", "gl", 1);
     }
   exe = ecore_exe_pipe_run(cmd, flags, c);
   if (disp) setenv("DISPLAY", disp, 1);
   if (env) setenv("WAYLAND_DISPLAY", env, 1);
   else unsetenv("WAYLAND_DISPLAY");
   if (c->gl)
     {
        if (gl) setenv("ELM_ACCEL", gl, 1);
        else unsetenv("ELM_ACCEL");
        free(gl);
     }
   free(env);
   free(disp);
   if (exe)
     {
        int32_t pid = ecore_exe_pid_get(exe);
        eina_hash_add(c->exes, &pid, (void*)1);
     }
   return exe;
}

Ecore_Exe *
efl_wl_run(Evas_Object *obj, const char *cmd)
{
   return comp_run(obj, cmd, ECORE_EXE_TERM_WITH_PARENT);
}

Ecore_Exe *
efl_wl_flags_run(Evas_Object *obj, const char *cmd, Ecore_Exe_Flags flags)
{
   return comp_run(obj, cmd, flags);
}

void
efl_wl_pid_add(Evas_Object *obj, int32_t pid)
{
   Comp *c;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   if (!c->exes)
     c->exes = eina_hash_int32_new(NULL);
   eina_hash_add(c->exes, &pid, (void*)1);
}

void
efl_wl_pid_del(Evas_Object *obj, int32_t pid)
{
   Comp *c;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   if (!c->exes) return;
   eina_hash_del_by_key(c->exes, &pid);
}

Eina_Bool
efl_wl_next(Evas_Object *obj)
{
   Comp *c;
   Comp_Surface *cs;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   if (c->surfaces_count < 2) return EINA_FALSE;
   EINA_INLIST_REVERSE_FOREACH(c->surfaces, cs)
     {
        if (cs->shell.activated) continue;
        if ((!cs->role) || (!cs->shell.surface) || cs->dead) continue;
        cs->shell.activated = 1;
        shell_surface_send_configure(cs);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

Eina_Bool
efl_wl_prev(Evas_Object *obj)
{
   Comp *c;
   Comp_Surface *cs;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   if (c->surfaces_count < 2) return EINA_FALSE;
   EINA_INLIST_FOREACH(c->surfaces, cs)
     {
        if (cs->shell.activated) continue;
        if ((!cs->role) || (!cs->shell.surface) || cs->dead) continue;
        cs->shell.activated = 1;
        shell_surface_send_configure(cs);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

void
efl_wl_rotate(Evas_Object *obj, Efl_Wl_Rotation rot, Eina_Bool rtl)
{
   Comp *c;
   Eina_List *l;
   struct wl_resource *res;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   c->rtl = !!rtl;
   c->rotation = rot;
   EINA_LIST_FOREACH(c->output_resources, l, res)
     output_resize(c, res);
}

void
efl_wl_scale_set(Evas_Object *obj, double scale)
{
   Comp *c;
   Eina_List *l;
   struct wl_resource *res;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   c->scale = scale;

   EINA_LIST_FOREACH(c->output_resources, l, res)
     if (wl_resource_get_version(res) >= WL_OUTPUT_SCALE_SINCE_VERSION)
       wl_output_send_scale(res, lround(c->scale));
}

void
efl_wl_aspect_set(Evas_Object *obj, Eina_Bool set)
{
   Comp *c;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   if (c->aspect == (!!set)) return;
   c->aspect = !!set;
   if (c->aspect)
     shell_surface_aspect_update(c->active_surface);
   else
     evas_object_size_hint_aspect_set(obj, EVAS_ASPECT_CONTROL_NONE, 0, 0);
}

void
efl_wl_minmax_set(Evas_Object *obj, Eina_Bool set)
{
   Comp *c;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   if (c->minmax == (!!set)) return;
   c->minmax = !!set;
   if (c->minmax)
     shell_surface_minmax_update(c->active_surface);
   else
     {
        evas_object_size_hint_min_set(obj, 0, 0);
        evas_object_size_hint_max_set(obj, -1, -1);
     }
}

void *
efl_wl_global_add(Evas_Object *obj, const void *interface, uint32_t version, void *data, void *bind_cb)
{
   Comp *c;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   EINA_SAFETY_ON_NULL_RETURN_VAL(interface, NULL);
   c = evas_object_smart_data_get(obj);
   return wl_global_create(c->display, interface, version, data, bind_cb);
}

static void
extracted_focus(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Comp_Surface *cs = data;

   if (cs->dead) return;
   if (!cs->shell.popup)
     {
        cs->shell.activated = 1;
        shell_surface_send_configure(data);
     }
   evas_object_focus_set(cs->c->obj, 1);
}

static void
extracted_unfocus(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Comp_Surface *cs = data;
   Evas_Object *focus;

   if (cs->dead) return;
   focus = evas_focus_get(cs->c->evas);
   if ((!focus) || (focus == cs->c->obj)) return;
   cs->shell.activated = 0;
   shell_surface_send_configure(data);
}

static void
extracted_changed(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Comp_Surface *cs = data;

   if (cs->dead) return;
   shell_surface_send_configure(data);
}

Eina_Bool
efl_wl_surface_extract(Evas_Object *surface)
{
   Comp_Surface *cs;
   if (!eina_streq(evas_object_type_get(surface), "comp_surface")) abort();
   cs = evas_object_smart_data_get(surface);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(cs->extracted, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(cs->dead, EINA_FALSE);
   cs->extracted = 1;
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_RESIZE, extracted_changed, cs);
   if (!cs->shell.popup)
     evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_FOCUS_OUT, extracted_unfocus, cs);
   evas_object_event_callback_add(cs->obj, EVAS_CALLBACK_FOCUS_IN, extracted_focus, cs);
   evas_object_smart_member_del(surface);
   return EINA_TRUE;
}

Evas_Object *
efl_wl_extracted_surface_object_find(void *surface_resource)
{
   Comp_Surface *cs = wl_resource_get_user_data(surface_resource);

   EINA_SAFETY_ON_NULL_RETURN_VAL(cs, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!cs->extracted, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(cs->dead, NULL);

   return cs->obj;
}

Evas_Object *
efl_wl_extracted_surface_extracted_parent_get(Evas_Object *surface)
{
   Comp_Surface *cs;

   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, NULL);
   if (!eina_streq(evas_object_type_get(surface), "comp_surface")) abort();

   cs = evas_object_smart_data_get(surface);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!cs->extracted, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(cs->dead, NULL);

   if (cs->parent)
     {
        if (!cs->parent->extracted) return NULL;
        return cs->parent->obj;
     }
   return NULL;
}

void
efl_wl_seat_keymap_set(Evas_Object *obj, Eo *seat, void *state, int fd, size_t size, void *key_array)
{
   Comp *c;
   Comp_Seat *s;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   EINA_INLIST_FOREACH(c->seats, s)
     {
        if (!seat) efl_wl_seat_keymap_set(obj, s->dev, state, fd, size, key_array);
        else if (s->dev == seat) break;
     }
   if (!seat) return;
   EINA_SAFETY_ON_NULL_RETURN(s);
   seat_kbd_destroy(s);
   s->kbd.external = 1;
   s->kbd.keys_external = key_array;
   s->kbd.state = state;
   s->kbd.keymap_fd = fd;
   s->kbd.keymap_mem_size = size;
   s->kbd.context = NULL;
   s->kbd.keymap = NULL;
   s->kbd.keymap_mem = NULL;
   if (s->keyboard)
     seat_kbd_external_init(s);
}

void
efl_wl_seat_key_repeat_set(Evas_Object *obj, Eo *seat, int repeat_rate, int repeat_delay)
{
   Comp *c;
   Comp_Seat *s;

   if (!eina_streq(evas_object_type_get(obj), "comp")) abort();
   c = evas_object_smart_data_get(obj);
   EINA_INLIST_FOREACH(c->seats, s)
     {
        if (!seat) efl_wl_seat_key_repeat_set(obj, s->dev, repeat_rate, repeat_delay);
        else if (s->dev == seat) break;
     }
   if (!seat) return;
   EINA_SAFETY_ON_NULL_RETURN(s);
   s->kbd.repeat_rate = repeat_rate;
   s->kbd.repeat_delay = repeat_delay;
   seat_kbd_repeat_rate_send(s);
}
