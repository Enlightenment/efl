#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <Eina.h>
#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>

#include <Ecore_X.h>
#include <Ecore_X_Atoms.h>

#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
# include <Evas_Engine_Software_X11.h>
#endif

#ifdef BUILD_ECORE_EVAS_OPENGL_X11
# include <Evas_Engine_GL_X11.h>
#endif

#include <Ecore_Evas.h>
#include "ecore_evas_private.h"
#include "ecore_evas_x11.h"

#define EDBG(...)                                                       \
  EINA_LOG(_ecore_evas_log_dom, EINA_LOG_LEVEL_DBG + 1, __VA_ARGS__);

static int _ecore_evas_init_count = 0;

static Ecore_Event_Handler *ecore_evas_event_handlers[13];

static int leader_ref = 0;
static Ecore_X_Window leader_win = 0;

static const char *interface_x11_name = "x11";
static const int   interface_x11_version = 1;

static const char *interface_software_x11_name = "software_x11";
static const int   interface_software_x11_version = 1;

#ifdef BUILD_ECORE_EVAS_OPENGL_X11
static const char *interface_gl_x11_name = "gl_x11";
#endif
static const int   interface_gl_x11_version = 1;

typedef struct _Ecore_Evas_Engine_Data_X11 Ecore_Evas_Engine_Data_X11;

struct _Ecore_Evas_Engine_Data_X11 {
   Ecore_X_Window win_root;
   Eina_List     *win_extra;
   Ecore_X_Pixmap pmap;
   Ecore_X_Pixmap mask;
   Ecore_X_GC     gc;
   Ecore_X_XRegion *damages;
   Ecore_Timer     *outdelay;
   Ecore_X_Event_Mouse_Out out_ev;
   Ecore_X_Sync_Counter sync_counter;
   Ecore_X_Window leader;
   Ecore_X_Sync_Counter netwm_sync_counter;
   int            netwm_sync_val_hi;
   unsigned int   netwm_sync_val_lo;
   int            sync_val; // bigger! this will screw up at 2 billion fram~
   int            screen_num;
   int            px, py, pw, ph;
   unsigned char  direct_resize : 1;
   unsigned char  using_bg_pixmap : 1;
   unsigned char  managed : 1;
   unsigned char  sync_began : 1;
   unsigned char  sync_cancel : 1;
   unsigned char  netwm_sync_set : 1;
   unsigned char  configure_coming : 1;
   struct {
     unsigned char modal : 1;
     unsigned char sticky : 1;
     unsigned char maximized_v : 1;
     unsigned char maximized_h : 1;
     unsigned char shaded : 1;
     unsigned char skip_taskbar : 1;
     unsigned char skip_pager : 1;
     unsigned char fullscreen : 1;
     unsigned char above : 1;
     unsigned char below : 1;
   } state;
   struct {
     unsigned char available : 1; // need to setup available profiles in a w
     unsigned char change : 1; // need to send change event to the WM
     unsigned char done : 1; // need to send change done event to the WM
   } profile;
   struct {
     unsigned char supported: 1;
     unsigned char prepare : 1;
     unsigned char request : 1;
     unsigned char done : 1;
     unsigned char configure_coming : 1;
     Ecore_Job    *manual_mode_job;
   } wm_rot;
   Ecore_X_Window win_shaped_input;
   struct
     {
        unsigned int front, back; // front and back pixmaps (double-buffer)
        Evas_Coord w, h; // store pixmap size (saves X roundtrips)
        int depth; // store depth to save us from fetching engine info pre_render
        void *visual; // store visual used to create pixmap
        unsigned long colormap; // store colormap used to create pixmap
     } pixmap;
   Eina_Bool destroyed : 1; // X window has been deleted and cannot be used
};

static Ecore_Evas_Interface_X11 * _ecore_evas_x_interface_x11_new(void);
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
static Ecore_Evas_Interface_Software_X11 *_ecore_evas_x_interface_software_x11_new(void);
#endif
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
static Ecore_Evas_Interface_Gl_X11 *_ecore_evas_x_interface_gl_x11_new(void);
#endif
static void      _ecore_evas_x_rotation_set(Ecore_Evas *ee, int rotation, int resize);
static Eina_Bool _ecore_evas_x_wm_rot_manual_rotation_done_timeout(void *data);
static void      _ecore_evas_x_wm_rot_manual_rotation_done_timeout_update(Ecore_Evas *ee);
static void      _ecore_evas_x_aux_hints_set(Ecore_Evas *ee, const char *hints);

static void _resize_shape_do(Ecore_Evas *);
static void _shaped_do(Ecore_Evas *, int);
static void _alpha_do(Ecore_Evas *, int);
static void _transparent_do(Ecore_Evas *, int);
static void _avoid_damage_do(Ecore_Evas *, int);
static void _rotation_do(Ecore_Evas *, int, int);

static void
_ecore_evas_x_hints_update(Ecore_Evas *ee)
{
   ecore_x_icccm_hints_set
     (ee->prop.window,
         !ee->prop.focus_skip /* accepts_focus */,
         ee->prop.iconified ? ECORE_X_WINDOW_STATE_HINT_ICONIC :
         ee->prop.withdrawn ? ECORE_X_WINDOW_STATE_HINT_WITHDRAWN :
         ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
         0 /* icon_pixmap */,
         0 /* icon_mask */,
         0 /* icon_window */,
         ee->prop.group_ee_win /* window_group */,
         ee->prop.urgent /* is_urgent */);
}

static void
_ecore_evas_x_group_leader_set(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   leader_ref++;
   if (leader_ref == 1)
     {
        char *id = NULL;

        leader_win =
          ecore_x_window_override_new(edata->win_root, 1234, 5678, 1, 2);
        ecore_x_window_defaults_set(leader_win);
        if ((id = getenv("DESKTOP_STARTUP_ID")))
          ecore_x_netwm_startup_id_set(leader_win,id);
        ecore_x_icccm_client_leader_set(leader_win, leader_win);
     }
   edata->leader = leader_win;
   ecore_x_icccm_client_leader_set(ee->prop.window, leader_win);
}

static void
_ecore_evas_x_group_leader_unset(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   if (!edata->destroyed)
     ecore_x_window_prop_property_del(ee->prop.window,
                                      ECORE_X_ATOM_WM_CLIENT_LEADER);
   if (edata->leader == leader_win)
     {
        leader_ref--;
        if (leader_ref <= 0)
          {
             ecore_x_window_free(leader_win);
             leader_win = 0;
          }
        edata->leader = 0;
     }
}

static void
_ecore_evas_x_group_leader_update(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   if (edata->leader)
      ecore_x_icccm_client_leader_set(ee->prop.window, edata->leader);
}

static void
_ecore_evas_x_protocols_set(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   Ecore_X_Atom protos[3];
   unsigned int num = 0, tmp = 0;

   if (ee->func.fn_delete_request)
     protos[num++] = ECORE_X_ATOM_WM_DELETE_WINDOW;
   protos[num++] = ECORE_X_ATOM_NET_WM_PING;
   protos[num++] = ECORE_X_ATOM_NET_WM_SYNC_REQUEST;
   ecore_x_icccm_protocol_atoms_set(ee->prop.window, protos, num);

   if (!edata->netwm_sync_counter)
     edata->netwm_sync_counter = ecore_x_sync_counter_new(0);

   tmp = edata->netwm_sync_counter;
   ecore_x_window_prop_card32_set(ee->prop.window,
                                  ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER,
                                  &tmp, 1);

   // set property on window to say "I talk the deiconify approve protcol"
   tmp = 1;
   ecore_x_window_prop_card32_set(ee->prop.window,
                                  ECORE_X_ATOM_E_DEICONIFY_APPROVE,
                                  &tmp, 1);
}

static void
_ecore_evas_x_sync_set(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   Ecore_X_Sync_Counter sync_counter = edata->sync_counter;

   if (((ee->should_be_visible) || (ee->visible)) &&
       ((ecore_x_e_comp_sync_supported_get(edata->win_root)) &&
           (!ee->no_comp_sync) && (_ecore_evas_app_comp_sync)))
     {
        if (!edata->sync_counter)
           edata->sync_counter = ecore_x_sync_counter_new(0);
     }
   else
     {
        if (edata->sync_counter)
          {
           ecore_x_sync_counter_free(edata->sync_counter);
             edata->sync_val = 0;
          }
        edata->sync_counter = 0;
     }
   if ((!edata->destroyed) && (sync_counter != edata->sync_counter))
     ecore_x_e_comp_sync_counter_set(ee->prop.window, edata->sync_counter);
}

static void
_ecore_evas_x_sync_clear(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (!edata->sync_counter) return;
   ecore_x_sync_counter_free(edata->sync_counter);
   edata->sync_val = 0;
   edata->sync_counter = 0;
}

static void
_ecore_evas_x_wm_rotation_protocol_set(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ecore_x_e_window_rotation_supported_get(edata->win_root))
     ee->prop.wm_rot.supported = 1;
   else
     ee->prop.wm_rot.supported = 0;
}

static void
_ecore_evas_x_window_profile_protocol_set(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   /* check and set profile protocol hint */
   if (ecore_x_e_window_profile_supported_get(edata->win_root))
     {
        unsigned int v = 1;
        ecore_x_window_prop_card32_set
          (ee->prop.window,
          ECORE_X_ATOM_E_WINDOW_PROFILE_SUPPORTED,
          &v, 1);

        ee->profile_supported = 1;
     }
   else
     ee->profile_supported = 0;
}

static void
_ecore_evas_x_window_profile_set(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (((ee->should_be_visible) || (ee->visible)) &&
       (ee->profile_supported))
     {
        if (edata->profile.available)
          {
             ecore_x_e_window_available_profiles_set
               (ee->prop.window,
               (const char **)ee->prop.profile.available_list,
               (const int)ee->prop.profile.count);

             edata->profile.available = 0;
          }

        if (edata->profile.change)
          {
             if (ee->prop.profile.name)
               {
                  /* We need to keep the profile as an x property to let the WM.
                   * Then the WM reads it when creating the border window.
                   */
                  Ecore_X_Atom a = ecore_x_atom_get(ee->prop.profile.name);
                  ecore_x_window_prop_atom_set
                    (ee->prop.window,
                    ECORE_X_ATOM_E_WINDOW_PROFILE_CHANGE,
                    &a, 1);

                  ecore_x_e_window_profile_change_send
                    (edata->win_root,
                    ee->prop.window,
                    ee->prop.profile.name);
               }
             edata->profile.change = 0;
          }
     }
}

static void
_ecore_evas_x_wm_rot_manual_rotation_done_job(void *data)
{
   Ecore_Evas *ee = (Ecore_Evas *)data;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   edata->wm_rot.manual_mode_job = NULL;
   ee->prop.wm_rot.manual_mode.wait_for_done = EINA_FALSE;

   ecore_x_e_window_rotation_change_done_send
     (edata->win_root, ee->prop.window, ee->rotation, ee->w, ee->h);

   edata->wm_rot.done = 0;
}

static void
_ecore_evas_x_aux_hints_supprted_update(Ecore_Evas *ee)
{
   Ecore_X_Window root = ecore_x_window_root_first_get();
   unsigned char *data = NULL;
   unsigned int num = 0, i = 0;
   int res = 0, n = 0;
   char **str;
   const char *hint;

   EINA_LIST_FREE(ee->prop.aux_hint.supported_list, hint)
     {
        eina_stringshare_del(hint);
     }

   res = ecore_x_window_prop_property_get
     (root, ECORE_X_ATOM_E_WINDOW_AUX_HINT_SUPPORTED_LIST,
      ECORE_X_ATOM_STRING, 0, &data, &n);

   if ((res == 8) && (n > 0))
     {
        str = eina_str_split_full((char *)data, ",", -1, &num);
        for (i = 0; i < num; i++)
          {
             hint = eina_stringshare_add(str[i]);
             ee->prop.aux_hint.supported_list = 
               eina_list_append(ee->prop.aux_hint.supported_list, hint);
          }

        if (str)
          {
             free(str[0]);
             free(str);
          }
     }

   free(data);
}

static void
_ecore_evas_x_aux_hints_update(Ecore_Evas *ee)
{
   Eina_Strbuf *buf = _ecore_evas_aux_hints_string_get(ee);
   if (buf)
     {
        _ecore_evas_x_aux_hints_set(ee, eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
     }
}

# ifdef BUILD_ECORE_EVAS_OPENGL_X11
static Ecore_X_Window
_ecore_evas_x_gl_window_new(Ecore_Evas *ee, Ecore_X_Window parent, int x, int y, int w, int h, Eina_Bool override, int argb, const int *opt)
{
   Evas_Engine_Info_GL_X11 *einfo;
   Ecore_X_Window win;

   einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        int screen;

        if (opt)
          {
             int op;

             for (op = 0; opt[op]; op++)
               {
                  if (opt[op] == ECORE_EVAS_GL_X11_OPT_INDIRECT)
                    {
                       op++;
                       einfo->indirect = opt[op];
                    }
                  else if (opt[op] == ECORE_EVAS_GL_X11_OPT_VSYNC)
                    {
                       op++;
                       einfo->vsync = opt[op];
                    }
#ifdef EVAS_ENGINE_GL_X11_SWAP_MODE_EXISTS                  
                  else if (opt[op] == ECORE_EVAS_GL_X11_OPT_SWAP_MODE)
                    {
                       op++;
                       if ((evas_version->major >= 1) &&
                           (evas_version->minor >= 7) &&
                           (evas_version->micro >= 99))
                       einfo->swap_mode = opt[op];
                    }
#endif
               }
          }

        /* FIXME: this is inefficient as its 1 or more round trips */
        screen = ecore_x_screen_index_get(ecore_x_default_screen_get());
        if (ecore_x_screen_count_get() > 1)
          {
             Ecore_X_Window *roots;
             int num, i;

             num = 0;
             roots = ecore_x_window_root_list(&num);
             if (roots)
               {
                  Ecore_X_Window root;

                  root = ecore_x_window_root_get(parent);
                  for (i = 0; i < num; i++)
                    {
                       if (root == roots[i])
                         {
                            screen = i;
                            break;
                         }
                    }
                  free(roots);
               }
          }

        einfo->info.display = ecore_x_display_get();
        einfo->info.screen = screen;

        einfo->info.destination_alpha = argb;

        einfo->info.visual = einfo->func.best_visual_get(einfo);
        einfo->info.colormap = einfo->func.best_colormap_get(einfo);
        einfo->info.depth = einfo->func.best_depth_get(einfo);

        if ((!einfo->info.visual) ||
            (!einfo->info.colormap) || (!einfo->info.depth))
          {
             WRN("OpenGL X11 init engine '%s' failed - no visual, colormap or depth.", ee->driver);
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                  return 0;
               }
          }

        if (argb)
          {
             if (override)
               win = ecore_x_window_override_argb_new(parent, x, y, w, h);
             else
               win = ecore_x_window_argb_new(parent, x, y, w, h);
          }
        else
          {
             if (override)
               win = ecore_x_window_override_new(parent, x, y, w, h);
             else
               win = ecore_x_window_new(parent, x, y, w, h);
          }

        ecore_x_window_pixel_gravity_set(win, ECORE_X_GRAVITY_FORGET);
        ecore_x_vsync_animator_tick_source_set(win);

        /* attr.backing_store = NotUseful; */
        /* attr.override_redirect = override; */
        /* attr.colormap = einfo->info.colormap; */
        /* attr.border_pixel = 0; */
        /* attr.background_pixmap = None; */
        /* attr.event_mask = */
        /*   KeyPressMask | KeyReleaseMask | */
        /*   ExposureMask | ButtonPressMask | ButtonReleaseMask | */
        /*   EnterWindowMask | LeaveWindowMask | */
        /*   PointerMotionMask | StructureNotifyMask | VisibilityChangeMask | */
        /*   FocusChangeMask | PropertyChangeMask | ColormapChangeMask; */
        /* attr.bit_gravity = ForgetGravity; */

        /* win = */
        /*   XCreateWindow(einfo->info.display, parent, x, y, w, h, 0, */
        /*                 einfo->info.depth, InputOutput, einfo->info.visual, */
        /*                 CWBackingStore | CWColormap | CWBackPixmap | */
        /*                 CWBorderPixel | CWBitGravity | CWEventMask | */
        /*                 CWOverrideRedirect, &attr); */

        einfo->info.drawable = win;

        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             WRN("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             ecore_x_window_free(win);
             return 0;
          }
     }
   else
     win = 0;

   return win;
}
#endif

static int
_render_updates_process(Ecore_Evas *ee, Eina_List *updates)
{
   int rend = 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->prop.avoid_damage)
     {
        if (edata->using_bg_pixmap)
          {
             if (updates)
               {
                  Eina_List *l = NULL;
                  Eina_Rectangle *r;

                  EINA_LIST_FOREACH(updates, l, r)
                    ecore_x_window_area_clear(ee->prop.window,
                                              r->x, r->y, r->w, r->h);
                  if (ee->shaped)
                    {
                       ecore_x_window_shape_mask_set(ee->prop.window,
                                                     edata->mask);
                    }
                  if (ee->alpha)
                    {
//                     ecore_x_window_shape_input_mask_set(ee->prop.window, edata->mask);
                    }
                  _ecore_evas_idle_timeout_update(ee);
                  rend = 1;
               }
          }
        else
          {
             if (updates)
               {
                  Eina_List *l = NULL;
                  Eina_Rectangle *r;

                  EINA_LIST_FOREACH(updates, l, r)
                    {
                       Ecore_X_Rectangle rect;
                       Ecore_X_XRegion *tmpr;

                       if (!edata->damages)
                         edata->damages = ecore_x_xregion_new();
                       tmpr = ecore_x_xregion_new();
                       if (ee->rotation == 0)
                         {
                            rect.x = r->x;
                            rect.y = r->y;
                            rect.width = r->w;
                            rect.height = r->h;
                         }
                       else if (ee->rotation == 90)
                         {
                            rect.x = r->y;
                            rect.y = ee->h - r->x - r->w;
                            rect.width = r->h;
                            rect.height = r->w;
                         }
                       else if (ee->rotation == 180)
                         {
                            rect.x = ee->w - r->x - r->w;
                            rect.y = ee->h - r->y - r->h;
                            rect.width = r->w;
                            rect.height = r->h;
                         }
                       else if (ee->rotation == 270)
                         {
                            rect.x = ee->w - r->y - r->h;
                            rect.y = r->x;
                            rect.width = r->h;
                            rect.height = r->w;
                         }
                       ecore_x_xregion_union_rect(tmpr, edata->damages,
                                                  &rect);
                       ecore_x_xregion_free(edata->damages);
                       edata->damages = tmpr;
                    }
                  if (edata->damages)
                    {
                       if (ee->shaped)
                         {
                            /* if we have a damage pixmap - we can avoid exposures by
                             * disabling them just for setting the mask */
                            ecore_x_event_mask_unset(ee->prop.window, ECORE_X_EVENT_MASK_WINDOW_DAMAGE);
                            ecore_x_window_shape_mask_set(ee->prop.window,
                                                          edata->mask);
                            /* and re-enable them again */
                            ecore_x_event_mask_set(ee->prop.window, ECORE_X_EVENT_MASK_WINDOW_DAMAGE);
                         }
                       ecore_x_xregion_set(edata->damages, edata->gc);
                       ecore_x_pixmap_paste(edata->pmap, ee->prop.window,
                                            edata->gc, 0, 0, ee->w, ee->h,
                                            0, 0);
                       ecore_x_xregion_free(edata->damages);
                       edata->damages = NULL;
                    }
                  _ecore_evas_idle_timeout_update(ee);
                  rend = 1;
               }
          }
     }
   else if (((ee->visible) && (ee->draw_ok)) ||
            ((ee->should_be_visible) && (ee->prop.fullscreen)) ||
            ((ee->should_be_visible) && (ee->prop.override)))
     {
        if (updates)
          {
             if (ee->shaped)
               {
                  ecore_x_window_shape_mask_set(ee->prop.window,
                                                edata->mask);
               }
             if (ee->alpha)
               {
//                ecore_x_window_shape_input_mask_set(ee->prop.window, edata->mask);
               }
             _ecore_evas_idle_timeout_update(ee);
             rend = 1;
          }
     }
   else
     evas_norender(ee->evas);

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);

/*
   if (rend)
     {
        static int frames = 0;
        static double t0 = 0.0;
        double t, td;

        t = ecore_time_get();
        frames++;
        if ((t - t0) > 1.0)
          {
             td = t - t0;
             printf("FPS: %3.3f\n", (double)frames / td);
             frames = 0;
             t0 = t;
          }
     }
 */
   return rend;
}

static void
_ecore_evas_x_render_updates(void *data, Evas *e EINA_UNUSED, void *event_info)
{
   Evas_Event_Render_Post *ev = event_info;
   Ecore_Evas *ee = data;

   if (!ev) return;

   EDBG("ee=%p finished asynchronous render.", ee);

   ee->in_async_render = EINA_FALSE;

   _render_updates_process(ee, ev->updated_area);

   if (ee->delayed.resize_shape)
     {
        _resize_shape_do(ee);
        ee->delayed.resize_shape = EINA_FALSE;
     }
   if (ee->delayed.shaped_changed)
     {
        _shaped_do(ee, ee->delayed.shaped);
        ee->delayed.shaped_changed = EINA_FALSE;
     }
   if (ee->delayed.alpha_changed)
     {
        _alpha_do(ee, ee->delayed.alpha);
        ee->delayed.alpha_changed = EINA_FALSE;
     }
   if (ee->delayed.transparent_changed)
     {
        _transparent_do(ee, ee->delayed.transparent);
        ee->delayed.transparent_changed = EINA_FALSE;
     }
   if (ee->delayed.avoid_damage != ee->prop.avoid_damage)
     _avoid_damage_do(ee, ee->delayed.avoid_damage);
   if (ee->delayed.rotation_changed)
     {
        _rotation_do(ee, ee->delayed.rotation, ee->delayed.rotation_resize);
        ee->delayed.rotation_changed = EINA_FALSE;
     }
}

static int
_ecore_evas_x_render(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *ll;
   Ecore_Evas *ee2;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if ((!ee->no_comp_sync) && (_ecore_evas_app_comp_sync) &&
       (edata->sync_counter) && (!edata->sync_began) &&
       (!edata->sync_cancel))
     return 0;

   if (ee->in_async_render)
     {
        EDBG("ee=%p is rendering asynchronously, skip.", ee);
        return 0;
     }

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);
   if (!ee->can_async_render)
     {
        Eina_List *updates = evas_render_updates(ee->evas);
        rend = _render_updates_process(ee, updates);
        evas_render_updates_free(updates);
     }
   else if (evas_render_async(ee->evas))
     {
        EDBG("ee=%p started asynchronous render.", ee);
        ee->in_async_render = EINA_TRUE;
        rend = 1;
     }
   else if (ee->func.fn_post_render) ee->func.fn_post_render(ee);

   return rend;
}

static void
_resize_shape_do(Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
   Evas_Engine_Info_Software_X11 *einfo;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        unsigned int foreground;
        Ecore_X_GC gc;

        if (edata->mask) ecore_x_pixmap_free(edata->mask);
        edata->mask = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, 1);
        foreground = 0;
        gc = ecore_x_gc_new(edata->mask,
                            ECORE_X_GC_VALUE_MASK_FOREGROUND,
                            &foreground);
        ecore_x_drawable_rectangle_fill(edata->mask, gc,
                                        0, 0, ee->w, ee->h);
        ecore_x_gc_free(gc);
        einfo->info.mask = edata->mask;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_X11 */
}

static void
_ecore_evas_x_resize_shape(Ecore_Evas *ee)
{
   if (!strcmp(ee->driver, "software_x11"))
     {
        if (ee->in_async_render)
          {
             ee->delayed.resize_shape = EINA_TRUE;
             return;
          }
        _resize_shape_do(ee);
     }
}

/* TODO: we need to make this work for all the states, not just sticky */
static Eina_Bool
_ecore_evas_x_event_property_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Property *e;
   Ecore_Evas_Engine_Data_X11 *edata;
   int state_change = 0;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   edata = ee->engine.data;
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (e->atom == ECORE_X_ATOM_NET_WM_STATE)
     {
        unsigned int i, num;
        Ecore_X_Window_State *state;
        struct {
           struct {
              unsigned char modal : 1;
              unsigned char sticky : 1;
              unsigned char maximized_v : 1;
              unsigned char maximized_h : 1;
              unsigned char shaded : 1;
              unsigned char skip_taskbar : 1;
              unsigned char skip_pager : 1;
              unsigned char fullscreen : 1;
              unsigned char above : 1;
              unsigned char below : 1;
           } x;
           struct {
              char modal : 1;
              char maximized : 1;
              char sticky : 1;
              char fullscreen : 1;
              char focus_skip : 1;
           } prop;
        } prev;
        
        prev.x.modal = edata->state.modal;
        prev.x.sticky = edata->state.sticky;
        prev.x.maximized_v = edata->state.maximized_v;
        prev.x.maximized_h = edata->state.maximized_h;
        prev.x.shaded = edata->state.shaded;
        prev.x.skip_taskbar = edata->state.skip_taskbar;
        prev.x.skip_pager = edata->state.skip_pager;
        prev.x.fullscreen = edata->state.fullscreen;
        prev.x.above = edata->state.above;
        prev.x.below = edata->state.below;
        
        prev.prop.modal = ee->prop.modal;
        prev.prop.maximized = ee->prop.maximized;
        prev.prop.sticky = ee->prop.sticky;
        prev.prop.fullscreen = ee->prop.fullscreen;
        prev.prop.focus_skip = ee->prop.focus_skip;

        edata->state.modal        = 0;
        edata->state.sticky       = 0;
        edata->state.maximized_v  = 0;
        edata->state.maximized_h  = 0;
        edata->state.shaded       = 0;
        edata->state.skip_taskbar = 0;
        edata->state.skip_pager   = 0;
        edata->state.fullscreen   = 0;
        edata->state.above        = 0;
        edata->state.below        = 0;

        ee->prop.modal      = EINA_FALSE;
        ee->prop.maximized  = EINA_FALSE;
        ee->prop.sticky     = EINA_FALSE;
        ee->prop.fullscreen = EINA_FALSE;
        ee->prop.focus_skip = EINA_FALSE;
        
        ecore_x_netwm_window_state_get(e->win, &state, &num);
        if (state)
          {
             for (i = 0; i < num; i++)
               {
                  switch (state[i])
                    {
                     case ECORE_X_WINDOW_STATE_MODAL:
                       edata->state.modal = 1;
                       ee->prop.modal = EINA_TRUE;
                       break;
                     case ECORE_X_WINDOW_STATE_STICKY:
                       ee->prop.sticky = EINA_TRUE;
                       edata->state.sticky = 1;
                       break;
                     case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
                       edata->state.maximized_v = 1;
                       ee->prop.maximized = EINA_TRUE;
                       break;
                     case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
                       edata->state.maximized_h = 1;
                       ee->prop.maximized = EINA_TRUE;
                       break;
                     case ECORE_X_WINDOW_STATE_SHADED:
                       edata->state.shaded = 1;
                       break;
                     case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
                       edata->state.skip_taskbar = 1;
                       ee->prop.focus_skip = EINA_TRUE;
                       break;
                     case ECORE_X_WINDOW_STATE_SKIP_PAGER:
                       edata->state.skip_pager = 1;
                       ee->prop.focus_skip = EINA_TRUE;
                       break;
                     case ECORE_X_WINDOW_STATE_FULLSCREEN:
                       ee->prop.fullscreen = 1;
                       edata->state.fullscreen = 1;
                       break;
                     case ECORE_X_WINDOW_STATE_ABOVE:
                       edata->state.above = 1;
                       break;
                     case ECORE_X_WINDOW_STATE_BELOW:
                       edata->state.below = 1;
                       break;
                     default:
                        break;
                    }
               }
             free(state);
          }
        if (
//                 (prev.x.modal != edata->state.modal) ||
            (prev.x.sticky != edata->state.sticky) ||
            (prev.x.maximized_v != edata->state.maximized_v) ||
            (prev.x.maximized_h != edata->state.maximized_h) ||
//                 (prev.x.shaded != edata->state.shaded) ||
//                 (prev.x.skip_taskbar != edata->state.skip_taskbar) ||
//                 (prev.x.skip_pager != edata->state.skip_pager) ||
            (prev.x.fullscreen != edata->state.fullscreen) ||
//                 (prev.x.above != edata->state.above) ||
//                 (prev.x.below != edata->state.below) ||
//                 (prev.prop.modal != ee->prop.modal) ||
            (prev.prop.maximized != ee->prop.maximized) ||
            (prev.prop.sticky != ee->prop.sticky) ||
            (prev.prop.fullscreen != ee->prop.fullscreen) ||
            (prev.prop.focus_skip != ee->prop.focus_skip))
          state_change = 1;
     }
   else if (e->atom == ECORE_X_ATOM_WM_STATE)
     {
        Ecore_X_Window_State_Hint state;

        // handle WM_STATE changes
        state = ecore_x_icccm_state_get(e->win);
        switch (state)
          {
           case ECORE_X_WINDOW_STATE_HINT_WITHDRAWN:
             if ((!ee->prop.withdrawn) || (ee->prop.iconified))
               {
                  state_change = 1;
                  ee->prop.withdrawn = EINA_TRUE;
                  ee->prop.iconified = EINA_FALSE;
               }
             break;
           case ECORE_X_WINDOW_STATE_HINT_ICONIC:
             if ((!ee->prop.iconified) || (ee->prop.withdrawn))
               {
                  state_change = 1;
                  ee->prop.iconified = EINA_TRUE;
                  ee->prop.withdrawn = EINA_FALSE;
               }
             break;
           case ECORE_X_WINDOW_STATE_HINT_NORMAL:
             if ((ee->prop.iconified) || (ee->prop.withdrawn))
               {
                  state_change = 1;
                  ee->prop.iconified = EINA_FALSE;
                  ee->prop.withdrawn = EINA_FALSE;
               }
             break;
           default:
             break;
          }
     }
   if (state_change)
     {
        if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_visibility_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Visibility_Change *e;
   Ecore_Evas_Engine_Data_X11 *edata;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   edata = ee->engine.data;
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
//   printf("VIS CHANGE OBSCURED: %p %i\n", ee, e->fully_obscured);
   if (e->fully_obscured)
     {
        /* FIXME: round trip */
        if (!ecore_x_screen_is_composited(edata->screen_num))
          ee->draw_ok = 0;
     }
   else
     ee->draw_ok = 1;
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_client_message(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Client_Message *e;
   Ecore_Evas_Engine_Data_X11 *edata;

   e = event;
   if (e->format != 32) return ECORE_CALLBACK_PASS_ON;
   if (e->message_type == ECORE_X_ATOM_E_COMP_SYNC_BEGIN)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        edata = ee->engine.data;
        if (e->data.l[0] != (long)ee->prop.window)
          return ECORE_CALLBACK_PASS_ON;
        if (!edata->sync_began)
          {
             // qeue a damage + draw. work around an event re-ordering thing.
             evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
        edata->sync_began = 1;
        edata->sync_cancel = 0;
     }
   else if (e->message_type == ECORE_X_ATOM_E_COMP_SYNC_END)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        edata = ee->engine.data;
        if (e->data.l[0] != (long)ee->prop.window)
          return ECORE_CALLBACK_PASS_ON;
        edata->sync_began = 0;
        edata->sync_cancel = 0;
     }
   else if (e->message_type == ECORE_X_ATOM_E_COMP_SYNC_CANCEL)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        edata = ee->engine.data;
        if (e->data.l[0] != (long)ee->prop.window)
          return ECORE_CALLBACK_PASS_ON;
        edata->sync_began = 0;
        edata->sync_cancel = 1;
     }
   else if ((e->message_type == ECORE_X_ATOM_WM_PROTOCOLS) &&
            (e->data.l[0] == (int)ECORE_X_ATOM_NET_WM_SYNC_REQUEST))
     {
        ee = ecore_event_window_match(e->win);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        edata = ee->engine.data;
        edata->netwm_sync_val_lo = (unsigned int)e->data.l[2];
        edata->netwm_sync_val_hi = (int)e->data.l[3];
        edata->netwm_sync_set = 1;
     }
   else if (e->message_type == ECORE_X_ATOM_E_WINDOW_PROFILE_CHANGE_REQUEST)
     {
        ee = ecore_event_window_match(e->win);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        edata = ee->engine.data;
        if (ee->profile_supported)
          {
             char *p = ecore_x_atom_name_get(e->data.l[1]);
             if (p)
               {
                  _ecore_evas_window_profile_free(ee);
                  ee->prop.profile.name = (char *)eina_stringshare_add(p);

                  /* window profiles of each sub_ecore_evas will be changed
                   * in fn_state_change callback.
                   */
                  if (ee->func.fn_state_change)
                    ee->func.fn_state_change(ee);

                  edata->profile.done = 1;
                  free(p);
               }
          }
     }
   else if (e->message_type == ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL)
     {
        ///TODO after access structure determined
        // if (ee->func.fn_msg_handle)
        // ee->func.fn_msg_handle(ee, msg_domain, msg_id, data, size);
     }
   else if (e->message_type == ECORE_X_ATOM_E_DEICONIFY_APPROVE)
     {
        ee = ecore_event_window_match(e->win);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        if (e->data.l[1] != 0) //wm sends request message using value 0
          return ECORE_CALLBACK_PASS_ON;

        if (ecore_evas_manual_render_get(ee))
          ecore_evas_manual_render(ee);
        //client sends reply message using value 1
        ecore_x_client_message32_send(e->win, ECORE_X_ATOM_E_DEICONIFY_APPROVE,
                                      ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                      e->win, 1,
                                      0, 0, 0);
     }
   else if (e->message_type == ECORE_X_ATOM_E_WINDOW_ROTATION_CHANGE_PREPARE)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        edata = ee->engine.data;
        if (e->data.l[0] != (long)ee->prop.window)
          return ECORE_CALLBACK_PASS_ON;
        if (ee->prop.wm_rot.supported)
          {
             if (ee->prop.wm_rot.app_set)
               {
                  ee->prop.wm_rot.angle = (int)e->data.l[1];
                  ee->prop.wm_rot.win_resize = (int)e->data.l[2];
                  ee->prop.wm_rot.w = (int)e->data.l[3];
                  ee->prop.wm_rot.h = (int)e->data.l[4];
                  if (ee->prop.wm_rot.win_resize)
                    edata->wm_rot.configure_coming = 1;
                  edata->wm_rot.prepare = 1;
                  edata->wm_rot.request = 0;
                  edata->wm_rot.done = 0;
               }
          }
     }
   else if (e->message_type == ECORE_X_ATOM_E_WINDOW_ROTATION_CHANGE_REQUEST)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        edata = ee->engine.data;
        if (e->data.l[0] != (long)ee->prop.window)
          return ECORE_CALLBACK_PASS_ON;
        if (ee->prop.wm_rot.supported)
          {
             if (ee->prop.wm_rot.app_set)
               {
                  edata->wm_rot.prepare = 0;
                  edata->wm_rot.request = 1;
                  edata->wm_rot.done = 0;
                  if (ee->prop.wm_rot.win_resize)
                    {
                       if ((ee->w == ee->prop.wm_rot.w) &&
                           (ee->h == ee->prop.wm_rot.h))
                         {
                            edata->wm_rot.configure_coming = 0;
                         }
                    }
                  if (!edata->wm_rot.configure_coming)
                    {
                       if (ee->prop.wm_rot.manual_mode.set)
                         {
                            ee->prop.wm_rot.manual_mode.wait_for_done = EINA_TRUE;
                            _ecore_evas_x_wm_rot_manual_rotation_done_timeout_update(ee);
                         }
                       _ecore_evas_x_rotation_set(ee, ee->prop.wm_rot.angle, 1);
                    }
               }
          }
     }
   else if (e->message_type == ECORE_X_ATOM_E_WINDOW_AUX_HINT_ALLOWED)
     {
        ee = ecore_event_window_match(e->win);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */

        int id = e->data.l[1]; /* id of aux hint */
        Eina_List *l;
        Ecore_Evas_Aux_Hint *aux;
        EINA_LIST_FOREACH(ee->prop.aux_hint.hints, l, aux)
          {
             if (id == aux->id)
               {
                  aux->allowed = 1;
                  if (!aux->notified)
                    {
                       if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
                       aux->notified = 1;
                    }
                  break;
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_fake_out(void *data)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   Ecore_X_Event_Mouse_Out *e = &(edata->out_ev);

   edata->outdelay = NULL;

   ecore_event_evas_modifier_lock_update(ee->evas, e->modifiers);
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
   if (e->mode == ECORE_X_EVENT_MODE_GRAB)
     evas_event_feed_mouse_cancel(ee->evas, e->time, NULL);
   evas_event_feed_mouse_out(ee->evas, e->time, NULL);
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
   ee->in = EINA_FALSE;
   return EINA_FALSE;
}

static Eina_Bool
_ecore_evas_x_event_mouse_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_In *e;
   Ecore_Evas_Engine_Data_X11 *edata;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   edata = ee->engine.data;
/*   
    {
       time_t t;
       char *ct;

       const char *modes[] = {
        "MODE_NORMAL",
        "MODE_WHILE_GRABBED",
        "MODE_GRAB",
        "MODE_UNGRAB"
       };
       const char *details[] = {
        "DETAIL_ANCESTOR",
        "DETAIL_VIRTUAL",
        "DETAIL_INFERIOR",
        "DETAIL_NON_LINEAR",
        "DETAIL_NON_LINEAR_VIRTUAL",
        "DETAIL_POINTER",
        "DETAIL_POINTER_ROOT",
        "DETAIL_DETAIL_NONE"
       };
       t = time(NULL);
       ct = ctime(&t);
       ct[strlen(ct) - 1] = 0;
       printf("@@ ->IN 0x%x 0x%x %s md=%s dt=%s\n",
             e->win, e->event_win,
             ct,
             modes[e->mode],
             details[e->detail]);
    }
 */
   // disable. causes more problems than it fixes
   //   if ((e->mode == ECORE_X_EVENT_MODE_GRAB) ||
   //       (e->mode == ECORE_X_EVENT_MODE_UNGRAB))
   //     return 0;

   // handle click to focus passive buton grab side-effects
   if ((e->mode == ECORE_X_EVENT_MODE_UNGRAB) &&
       (e->detail == ECORE_X_EVENT_DETAIL_ANCESTOR) &&
       (edata->outdelay))
     {
        ecore_timer_del(edata->outdelay);
        edata->outdelay = NULL;
        return 0;
     }
   if (edata->outdelay)
     {
        ecore_timer_del(edata->outdelay);
        edata->outdelay = NULL;
        _fake_out(ee);
     }
   
   /* if (e->mode != ECORE_X_EVENT_MODE_NORMAL) return 0; */
   if (!ee->in)
     {
        if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
        ecore_event_evas_modifier_lock_update(ee->evas, e->modifiers);
        evas_event_feed_mouse_in(ee->evas, e->time, NULL);
        _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
        ee->in = EINA_TRUE;
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_mouse_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_Out *e;
   Ecore_Evas_Engine_Data_X11 *edata;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   edata = ee->engine.data;
/*
    {
       time_t t;
       char *ct;

       const char *modes[] = {
        "MODE_NORMAL",
        "MODE_WHILE_GRABBED",
        "MODE_GRAB",
        "MODE_UNGRAB"
       };
       const char *details[] = {
        "DETAIL_ANCESTOR",
        "DETAIL_VIRTUAL",
        "DETAIL_INFERIOR",
        "DETAIL_NON_LINEAR",
        "DETAIL_NON_LINEAR_VIRTUAL",
        "DETAIL_POINTER",
        "DETAIL_POINTER_ROOT",
        "DETAIL_DETAIL_NONE"
       };
       t = time(NULL);
       ct = ctime(&t);
       ct[strlen(ct) - 1] = 0;
       printf("@@ ->OUT 0x%x 0x%x %s md=%s dt=%s\n",
             e->win, e->event_win,
             ct,
             modes[e->mode],
             details[e->detail]);
    }
 */
   // disable. causes more problems than it fixes
   //   if ((e->mode == ECORE_X_EVENT_MODE_GRAB) ||
   //       (e->mode == ECORE_X_EVENT_MODE_UNGRAB))
   //     return 0;

   // click to focus mouse out+in work-around
   if ((e->mode == ECORE_X_EVENT_MODE_GRAB) &&
       (e->detail == ECORE_X_EVENT_DETAIL_ANCESTOR))
     {
        // defer out handling in case its a "fake" out thanks to click
        // to focus (which gets us another out soon after
        if (edata->outdelay) ecore_timer_del(edata->outdelay);
        edata->out_ev = *e;
        edata->outdelay = ecore_timer_add(0.05, _fake_out, ee);
        return 0;
     }
   if (edata->outdelay)
     {
        ecore_timer_del(edata->outdelay);
        edata->outdelay = NULL;
     }
   
//   if (e->mode != ECORE_X_EVENT_MODE_NORMAL) return 0;
//   printf("OUT: ee->in=%i, e->mode=%i, e->detail=%i, dount_count=%i\n",
//          ee->in, e->mode, e->detail, evas_event_down_count_get(ee->evas));
   if (ee->in)
     {
        if ((evas_event_down_count_get(ee->evas) > 0) &&
            (!((e->mode == ECORE_X_EVENT_MODE_GRAB) &&
               (e->detail == ECORE_X_EVENT_DETAIL_NON_LINEAR))))
          return ECORE_CALLBACK_PASS_ON;
        ecore_event_evas_modifier_lock_update(ee->evas, e->modifiers);
        _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
        if (e->mode == ECORE_X_EVENT_MODE_GRAB)
          evas_event_feed_mouse_cancel(ee->evas, e->time, NULL);
        evas_event_feed_mouse_out(ee->evas, e->time, NULL);
        if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
        if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
        ee->in = EINA_FALSE;
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_focus_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Focus_In *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
//xx// filtering with these doesnt help   
//xx//   if (e->mode == ECORE_X_EVENT_MODE_UNGRAB) return ECORE_CALLBACK_PASS_ON;
   ee->prop.focused = EINA_TRUE;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_focus_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Focus_Out *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
//xx// filtering with these doesnt help   
//xx//   if (e->mode == ECORE_X_EVENT_MODE_GRAB) return ECORE_CALLBACK_PASS_ON;

//   if (ee->prop.fullscreen)
//     ecore_x_window_focus(ee->prop.window);
   evas_focus_out(ee->evas);
   ee->prop.focused = EINA_FALSE;
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_damage(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Damage *e;
   Ecore_Evas_Engine_Data_X11 *edata;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   edata = ee->engine.data;
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (edata->using_bg_pixmap) return ECORE_CALLBACK_PASS_ON;
//   printf("EXPOSE %p [%i] %i %i %ix%i\n", ee, ee->prop.avoid_damage, e->x, e->y, e->w, e->h);
   if (ee->prop.avoid_damage)
     {
        Ecore_X_Rectangle rect;
        Ecore_X_XRegion *tmpr;

        if (!edata->damages)
          edata->damages = ecore_x_xregion_new();
        tmpr = ecore_x_xregion_new();
        rect.x = e->x;
        rect.y = e->y;
        rect.width = e->w;
        rect.height = e->h;
        ecore_x_xregion_union_rect(tmpr, edata->damages, &rect);
        ecore_x_xregion_free(edata->damages);
        edata->damages = tmpr;
/* no - this breaks things badly. disable. Ecore_X_Rectangle != XRectangle - see
 *  the typedefs in x's headers and ecore_x's. also same with Region - it's a pointer in x - not an X ID
        Ecore_X_Rectangle rect;
        Ecore_X_XRegion  *tmpr;

        if (!edata->damages) edata->damages = ecore_x_xregion_new();
        tmpr = ecore_x_xregion_new();
        rect.x = e->x;
        rect.y = e->y;
        rect.width = e->w;
        rect.height = e->h;
        ecore_x_xregion_union_rect(tmpr, edata->damages, &rect);
        ecore_x_xregion_free(edata->damages);
        edata->damages = tmpr;
 */
     }
   else
     {
        if (ee->rotation == 0)
          evas_damage_rectangle_add(ee->evas, e->x, e->y, e->w, e->h);
        else if (ee->rotation == 90)
          evas_damage_rectangle_add(ee->evas,
                                    ee->h - e->y - e->h, e->x, e->h, e->w);
        else if (ee->rotation == 180)
          evas_damage_rectangle_add(ee->evas, ee->w - e->x - e->w,
                                    ee->h - e->y - e->h, e->w, e->h);
        else if (ee->rotation == 270)
          evas_damage_rectangle_add(ee->evas, e->y, ee->w - e->x - e->w,
                                    e->h, e->w);
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_destroy(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Destroy *e;
   Ecore_Evas_Engine_Data_X11 *edata;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   edata = ee->engine.data;
   if (ee->func.fn_destroy) ee->func.fn_destroy(ee);
   edata->destroyed = 1;
   _ecore_evas_x_sync_clear(ee);
   ecore_evas_free(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_configure(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Configure *e;
   Ecore_Evas_Engine_Data_X11 *edata;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   edata = ee->engine.data;
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (edata->direct_resize) return ECORE_CALLBACK_PASS_ON;

   edata->configure_coming = 0;
   if ((e->from_wm) || (ee->prop.override))
     {
        if ((ee->x != e->x) || (ee->y != e->y))
          {
             ee->x = e->x;
             ee->y = e->y;
             ee->req.x = ee->x;
             ee->req.y = ee->y;
             if (ee->func.fn_move) ee->func.fn_move(ee);
          }
     }
   if ((ee->w != e->w) || (ee->h != e->h) ||
       (ee->req.w != e->w) || (ee->req.h != e->h))
     {
        ee->w = e->w;
        ee->h = e->h;
        ee->req.w = ee->w;
        ee->req.h = ee->h;
        if (ECORE_EVAS_PORTRAIT(ee))
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        if (ee->prop.avoid_damage)
          {
             int pdam;

             pdam = ecore_evas_avoid_damage_get(ee);
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, pdam);
          }
        if ((ee->shaped) || (ee->alpha))
          _ecore_evas_x_resize_shape(ee);
        if ((ee->expecting_resize.w > 0) && (ee->expecting_resize.h > 0))
          {
             if ((ee->expecting_resize.w == ee->w) &&
                 (ee->expecting_resize.h == ee->h))
               _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                              ecore_x_current_time_get());
             ee->expecting_resize.w = 0;
             ee->expecting_resize.h = 0;
          }
        if (ee->func.fn_resize) ee->func.fn_resize(ee);

        if (ee->prop.wm_rot.supported)
          {
             if (edata->wm_rot.prepare)
               {
                  if ((ee->prop.wm_rot.w == e->w) &&
                      (ee->prop.wm_rot.h == e->h))
                    {
                       edata->wm_rot.configure_coming = 0;
                    }
               }
             else if (edata->wm_rot.request)
               {
                  if ((edata->wm_rot.configure_coming) &&
                      (ee->prop.wm_rot.w == e->w) &&
                      (ee->prop.wm_rot.h == e->h))
                    {
                       edata->wm_rot.configure_coming = 0;
                       if (ee->prop.wm_rot.manual_mode.set)
                         {
                            ee->prop.wm_rot.manual_mode.wait_for_done = EINA_TRUE;
                            _ecore_evas_x_wm_rot_manual_rotation_done_timeout_update(ee);
                         }
                       _ecore_evas_x_rotation_set(ee,
                                                  ee->prop.wm_rot.angle,
                                                  EINA_TRUE);
                    }
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_delete_request(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Delete_Request *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_show(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Show *e;
   static int first_map_bug = -1;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   if (ee->gl_sync_draw_done < 0)
     {
        char *sync = getenv("ECORE_EVAS_GL_SYNC_DRAW_DONE");
        if (sync && (atoi(sync) == 1))
          ee->gl_sync_draw_done = 1;
        else
          ee->gl_sync_draw_done = 0;
     }

   if (first_map_bug < 0)
     {
        char *bug = NULL;

        if ((bug = getenv("ECORE_EVAS_GL_FIRST_MAP_BUG")))
          first_map_bug = atoi(bug);
        else
          first_map_bug = 0;
     }
   if ((first_map_bug) && (!strcmp(ee->driver, "opengl_x11")))
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   if (ee->visible) return ECORE_CALLBACK_PASS_ON;
//   if (ee->visible) return ECORE_CALLBACK_DONE;
//   printf("SHOW EVENT %p\n", ee);
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_hide(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Hide *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->in)
     {
        evas_event_feed_mouse_cancel(ee->evas, e->time, NULL);
        evas_event_feed_mouse_out(ee->evas, e->time, NULL);
        if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
        if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
        ee->in = EINA_FALSE;
     }
   if (!ee->visible) return ECORE_CALLBACK_PASS_ON;
//   if (!ee->visible) return ECORE_CALLBACK_DONE;
//   printf("HIDE EVENT %p\n", ee);
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);
   return ECORE_CALLBACK_PASS_ON;
}

/* FIXME, should be in idler */
/* FIXME, round trip */
static void
_ecore_evas_x_size_pos_hints_update(Ecore_Evas *ee)
{
   ecore_x_icccm_size_pos_hints_set(ee->prop.window,
                                    ee->prop.request_pos /*request_pos */,
                                    ECORE_X_GRAVITY_NW /* gravity */,
                                    ee->prop.min.w /* min_w */,
                                    ee->prop.min.h /* min_h */,
                                    ee->prop.max.w /* max_w */,
                                    ee->prop.max.h /* max_h */,
                                    ee->prop.base.w /* base_w */,
                                    ee->prop.base.h /* base_h */,
                                    ee->prop.step.w /* step_x */,
                                    ee->prop.step.h /* step_y */,
                                    ee->prop.aspect /* min_aspect */,
                                    ee->prop.aspect /* max_aspect */);
}

/* FIXME, should be in idler */
static void
_ecore_evas_x_state_update(Ecore_Evas *ee)
{
   Ecore_X_Window_State state[10];
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   int num = 0;

   if (ee->prop.modal)
     state[num++] = ECORE_X_WINDOW_STATE_MODAL;
   if (ee->prop.sticky)
     state[num++] = ECORE_X_WINDOW_STATE_STICKY;
   if (ee->prop.maximized)
     state[num++] = ECORE_X_WINDOW_STATE_MAXIMIZED_VERT;
   if (ee->prop.maximized)
     state[num++] = ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ;
//   if (bd->client.netwm.state.shaded)
//     state[num++] = ECORE_X_WINDOW_STATE_SHADED;
   if (ee->prop.focus_skip)
     state[num++] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
   if (ee->prop.focus_skip)
     state[num++] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
//   if (bd->client.netwm.state.hidden)
//     state[num++] = ECORE_X_WINDOW_STATE_HIDDEN;
   if (edata->state.fullscreen)
     state[num++] = ECORE_X_WINDOW_STATE_FULLSCREEN;
   if (edata->state.above)
     state[num++] = ECORE_X_WINDOW_STATE_ABOVE;
   if (edata->state.below)
     state[num++] = ECORE_X_WINDOW_STATE_BELOW;
   if (ee->prop.demand_attention)
     state[num++] = ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION;

   ecore_x_netwm_window_state_set(ee->prop.window, state, num);
}

static void
_ecore_evas_x_layer_update(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->should_be_visible)
     {
        /* We need to send a netwm request to the wm */
        /* FIXME: Do we have to remove old state before adding new? */
        if (ee->prop.layer < 3)
          {
             if (edata->state.above)
               {
                  edata->state.above = 0;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   edata->win_root,
                                                   ECORE_X_WINDOW_STATE_ABOVE, -1, 0);
               }
             if (!edata->state.below)
               {
                  edata->state.below = 1;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   edata->win_root,
                                                   ECORE_X_WINDOW_STATE_BELOW, -1, 1);
               }
          }
        else if (ee->prop.layer > 5)
          {
             if (edata->state.below)
               {
                  edata->state.below = 0;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   edata->win_root,
                                                   ECORE_X_WINDOW_STATE_BELOW, -1, 0);
               }
             if (!edata->state.above)
               {
                  edata->state.above = 1;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   edata->win_root,
                                                   ECORE_X_WINDOW_STATE_ABOVE, -1, 1);
               }
          }
        else
          {
             if (edata->state.below)
               {
                  edata->state.below = 0;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   edata->win_root,
                                                   ECORE_X_WINDOW_STATE_BELOW, -1, 0);
               }
             if (edata->state.above)
               {
                  edata->state.above = 0;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   edata->win_root,
                                                   ECORE_X_WINDOW_STATE_ABOVE, -1, 0);
               }
          }
     }
   else
     {
        /* Just set the state */
        if (ee->prop.layer < 3)
          {
             if ((edata->state.above) || (!edata->state.below))
               {
                  edata->state.above = 0;
                  edata->state.below = 1;
                  _ecore_evas_x_state_update(ee);
               }
          }
        else if (ee->prop.layer > 5)
          {
             if ((!edata->state.above) || (edata->state.below))
               {
                  edata->state.above = 1;
                  edata->state.below = 0;
                  _ecore_evas_x_state_update(ee);
               }
          }
        else
          {
             if ((edata->state.above) || (edata->state.below))
               {
                  edata->state.above = 0;
                  edata->state.below = 0;
                  _ecore_evas_x_state_update(ee);
               }
          }
     }
   /* FIXME: Set gnome layer */
}

static int
_ecore_evas_x_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;
   ecore_evas_event_handlers[0] =
     ecore_event_handler_add(ECORE_X_EVENT_MOUSE_IN,
                             _ecore_evas_x_event_mouse_in, NULL);
   ecore_evas_event_handlers[1] =
     ecore_event_handler_add(ECORE_X_EVENT_MOUSE_OUT,
                             _ecore_evas_x_event_mouse_out, NULL);
   ecore_evas_event_handlers[2] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_IN,
                             _ecore_evas_x_event_window_focus_in, NULL);
   ecore_evas_event_handlers[3] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT,
                             _ecore_evas_x_event_window_focus_out, NULL);
   ecore_evas_event_handlers[4] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DAMAGE,
                             _ecore_evas_x_event_window_damage, NULL);
   ecore_evas_event_handlers[5] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY,
                             _ecore_evas_x_event_window_destroy, NULL);
   ecore_evas_event_handlers[6] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CONFIGURE,
                             _ecore_evas_x_event_window_configure, NULL);
   ecore_evas_event_handlers[7] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DELETE_REQUEST,
                             _ecore_evas_x_event_window_delete_request, NULL);
   ecore_evas_event_handlers[8] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_SHOW,
                             _ecore_evas_x_event_window_show, NULL);
   ecore_evas_event_handlers[9] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_HIDE,
                             _ecore_evas_x_event_window_hide, NULL);
   ecore_evas_event_handlers[10] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROPERTY,
                             _ecore_evas_x_event_property_change, NULL);
   ecore_evas_event_handlers[11] =
     ecore_event_handler_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE,
                             _ecore_evas_x_event_visibility_change, NULL);
   ecore_evas_event_handlers[12] =
     ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE,
                             _ecore_evas_x_event_client_message, NULL);
   ecore_event_evas_init();
   return _ecore_evas_init_count;
}

int
_ecore_evas_x_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
        unsigned int i;

        for (i = 0; i < sizeof(ecore_evas_event_handlers) / sizeof(Ecore_Event_Handler*); i++)
          {
             if (ecore_evas_event_handlers[i])
               ecore_event_handler_del(ecore_evas_event_handlers[i]);
          }
        ecore_event_evas_shutdown();
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static void
_ecore_evas_x_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (edata->pixmap.back)
     ecore_x_pixmap_free(edata->pixmap.back);
   if (edata->pixmap.front)
     ecore_x_pixmap_free(edata->pixmap.front);
   if (edata->wm_rot.manual_mode_job)
     {
        ecore_job_del(edata->wm_rot.manual_mode_job);
        edata->wm_rot.manual_mode_job = NULL;
     }

   _ecore_evas_x_group_leader_unset(ee);
   if (edata->sync_counter)
     ecore_x_sync_counter_free(edata->sync_counter);
   if (edata->win_shaped_input)
     ecore_x_window_free(edata->win_shaped_input);
   ecore_event_window_unregister(ee->prop.window);
   if (ee->prop.window && (!edata->destroyed)) ecore_x_window_free(ee->prop.window);
   if (edata->pmap) ecore_x_pixmap_free(edata->pmap);
   if (edata->mask) ecore_x_pixmap_free(edata->mask);
   if (edata->gc) ecore_x_gc_free(edata->gc);
   if (edata->damages) ecore_x_xregion_free(edata->damages);
   edata->pmap = 0;
   edata->mask = 0;
   edata->gc = 0;
   edata->damages = NULL;
   while (edata->win_extra)
     {
        Ecore_X_Window *winp;

        winp = edata->win_extra->data;
        edata->win_extra =
          eina_list_remove_list(edata->win_extra, edata->win_extra);
        ecore_event_window_unregister(*winp);
        free(winp);
     }
   if (edata->outdelay)
     {
        ecore_timer_del(edata->outdelay);
        edata->outdelay = NULL;
     }
   free(edata);
   _ecore_evas_x_shutdown();
   ecore_x_shutdown();
}

static void
_ecore_evas_x_callback_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ee->func.fn_delete_request = func;
   _ecore_evas_x_protocols_set(ee);
   _ecore_evas_x_sync_set(ee);
}

static void
_ecore_evas_x_move(Ecore_Evas *ee, int x, int y)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   ee->req.x = x;
   ee->req.y = y;
   if (edata->direct_resize)
     {
        if (!edata->managed)
          {
             if ((x != ee->x) || (y != ee->y))
               {
                  ee->x = x;
                  ee->y = y;
                  ecore_x_window_move(ee->prop.window, x, y);
                  if (!ee->should_be_visible)
                    {
                       /* We need to request pos */
                       ee->prop.request_pos = EINA_TRUE;
                       _ecore_evas_x_size_pos_hints_update(ee);
                    }
                  if (ee->func.fn_move) ee->func.fn_move(ee);
               }
          }
     }
   else
     {
        if (((ee->x != x) || (ee->y != y)) ||
            (edata->configure_coming))
          {
             edata->configure_coming = 1;
             if (!edata->managed)
               {
                  ee->x = x;
                  ee->y = y;
               }
             ecore_x_window_move(ee->prop.window, x, y);
          }
        if (!ee->should_be_visible)
          {
             /* We need to request pos */
             ee->prop.request_pos = EINA_TRUE;
             _ecore_evas_x_size_pos_hints_update(ee);
          }
     }
}

static void
_ecore_evas_x_managed_move(Ecore_Evas *ee, int x, int y)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   ee->req.x = x;
   ee->req.y = y;
   if (edata->direct_resize)
     {
        edata->managed = 1;
        if ((x != ee->x) || (y != ee->y))
          {
             ee->x = x;
             ee->y = y;
             if (ee->func.fn_move) ee->func.fn_move(ee);
          }
     }
}

static void
_ecore_evas_x_resize(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   ee->req.w = w;
   ee->req.h = h;

   /* check for valid property window
    * 
    * NB: If we do not have one, check for valid pixmap rendering */
   if (!ee->prop.window)
     {
        /* the ecore_evas was resized. we need to free the back pixmap */
        if ((edata->pixmap.w != w) || (edata->pixmap.h != h))
          {
             /* free the backing pixmap */
             if (edata->pixmap.back) 
               ecore_x_pixmap_free(edata->pixmap.back);
          }
     }

   if (edata->direct_resize)
     {
        if ((ee->w != w) || (ee->h != h))
          {
             ee->w = w;
             ee->h = h;
             if (ee->prop.window) ecore_x_window_resize(ee->prop.window, w, h);
             if (ECORE_EVAS_PORTRAIT(ee))
               {
                  evas_output_size_set(ee->evas, ee->w, ee->h);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
               }
             else
               {
                  evas_output_size_set(ee->evas, ee->h, ee->w);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
               }
             if (ee->prop.avoid_damage)
               {
                  int pdam;

                  pdam = ecore_evas_avoid_damage_get(ee);
                  ecore_evas_avoid_damage_set(ee, 0);
                  ecore_evas_avoid_damage_set(ee, pdam);
               }
             if ((ee->shaped) || (ee->alpha))
               _ecore_evas_x_resize_shape(ee);
             if (ee->func.fn_resize) ee->func.fn_resize(ee);
          }
     }
   else if (((ee->w != w) || (ee->h != h)) || 
            (edata->configure_coming))
     {
        edata->configure_coming = 1;
        if (ee->prop.window) ecore_x_window_resize(ee->prop.window, w, h);
     }
}

static void
_ecore_evas_x_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   ee->req.x = x;
   ee->req.y = y;
   ee->req.w = w;
   ee->req.h = h;
   if (edata->direct_resize)
     {
        if ((ee->w != w) || (ee->h != h) || (x != ee->x) || (y != ee->y))
          {
             int change_size = 0, change_pos = 0;

             if ((ee->w != w) || (ee->h != h)) change_size = 1;
             if (!edata->managed)
               {
                  if ((x != ee->x) || (y != ee->y)) change_pos = 1;
               }
             ecore_x_window_move_resize(ee->prop.window, x, y, w, h);
             if (!edata->managed)
               {
                  ee->x = x;
                  ee->y = y;
               }
             ee->w = w;
             ee->h = h;
             if (ECORE_EVAS_PORTRAIT(ee))
               {
                  evas_output_size_set(ee->evas, ee->w, ee->h);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
               }
             else
               {
                  evas_output_size_set(ee->evas, ee->h, ee->w);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
               }
             if (ee->prop.avoid_damage)
               {
                  int pdam;

                  pdam = ecore_evas_avoid_damage_get(ee);
                  ecore_evas_avoid_damage_set(ee, 0);
                  ecore_evas_avoid_damage_set(ee, pdam);
               }
             if ((ee->shaped) || (ee->alpha))
               _ecore_evas_x_resize_shape(ee);
             if (change_pos)
               {
                  if (ee->func.fn_move) ee->func.fn_move(ee);
               }
             if (change_size)
               {
                  if (ee->func.fn_resize) ee->func.fn_resize(ee);
               }
          }
     }
   else if (((ee->w != w) || (ee->h != h) || (ee->x != x) || (ee->y != y)) || 
            (edata->configure_coming))
     {
        edata->configure_coming = 1;
        ecore_x_window_move_resize(ee->prop.window, x, y, w, h);
        if (!edata->managed)
          {
             ee->x = x;
             ee->y = y;
          }
     }
}

static void
_ecore_evas_x_rotation_set_internal(Ecore_Evas *ee, int rotation, int resize,
                                    Evas_Engine_Info *einfo)
{
   int rot_dif;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   rot_dif = ee->rotation - rotation;
   if (rot_dif < 0) rot_dif = -rot_dif;

   if (rot_dif != 180)
     {
        int minw, minh, maxw, maxh, basew, baseh, stepw, steph;

        if (!evas_engine_info_set(ee->evas, einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }

        if (!resize)
          {
             edata->configure_coming = 1;
             if (!ee->prop.fullscreen)
               {
                  ecore_x_window_resize(ee->prop.window, ee->req.h, ee->req.w);
                  ee->expecting_resize.w = ee->h;
                  ee->expecting_resize.h = ee->w;
                  evas_output_size_set(ee->evas, ee->req.h, ee->req.w);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->req.h, ee->req.w);
               }
             else
               {
                  int w, h;

                  ecore_x_window_size_get(ee->prop.window, &w, &h);
                  ecore_x_window_resize(ee->prop.window, h, w);
                  if (PORTRAIT_CHECK(rotation))
                    {
                       evas_output_size_set(ee->evas, ee->req.w, ee->req.h);
                       evas_output_viewport_set(ee->evas, 0, 0, ee->req.w, ee->req.h);
                    }
                  else
                    {
                       evas_output_size_set(ee->evas, ee->req.h, ee->req.w);
                       evas_output_viewport_set(ee->evas, 0, 0, ee->req.h, ee->req.w);
                    }
                  if (ee->func.fn_resize) ee->func.fn_resize(ee);
               }
             if (PORTRAIT_CHECK(rotation))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.w, ee->req.h);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.h, ee->req.w);
          }
        else
          {
             /* int w, h; */

             /* ecore_x_window_size_get(ee->prop.window, &w, &h); */
             if (PORTRAIT_CHECK(rotation))
               {
                  evas_output_size_set(ee->evas, ee->w, ee->h);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
               }
             else
               {
                  evas_output_size_set(ee->evas, ee->h, ee->w);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
               }
             if (ee->func.fn_resize) ee->func.fn_resize(ee);
             if (PORTRAIT_CHECK(rotation))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
          }
        ecore_evas_size_min_get(ee, &minw, &minh);
        ecore_evas_size_max_get(ee, &maxw, &maxh);
        ecore_evas_size_base_get(ee, &basew, &baseh);
        ecore_evas_size_step_get(ee, &stepw, &steph);
        ee->rotation = rotation;
        ecore_evas_size_min_set(ee, minh, minw);
        ecore_evas_size_max_set(ee, maxh, maxw);
        ecore_evas_size_base_set(ee, baseh, basew);
        ecore_evas_size_step_set(ee, steph, stepw);
        _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                       ecore_x_current_time_get());
     }
   else
     {
        if (!evas_engine_info_set(ee->evas, einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        ee->rotation = rotation;
        _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                       ecore_x_current_time_get());
        if (ee->func.fn_resize) ee->func.fn_resize(ee);

        if (ECORE_EVAS_PORTRAIT(ee))
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
        else
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
     }
}

static Eina_Bool
_ecore_evas_x_wm_rotation_check(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   if (ee->prop.wm_rot.supported)
     {
        if (ee->prop.wm_rot.app_set)
          {
             if (edata->wm_rot.request)
               {
                  if (ee->prop.wm_rot.win_resize)
                    {
                       if (!((ee->w == ee->prop.wm_rot.w) &&
                             (ee->h == ee->prop.wm_rot.h)))
                         {
                            return EINA_FALSE;
                         }
                       else
                         edata->wm_rot.configure_coming = 0;
                    }
               }
          }
     }
   return EINA_TRUE;
}

static void
_rotation_do(Ecore_Evas *ee, int rotation, int resize)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
   if (ee->prop.wm_rot.supported)
     {
        if (!_ecore_evas_x_wm_rotation_check(ee)) return;
        if (!resize) resize = 1;
     }
   Evas_Engine_Info_Software_X11 *einfo;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (!einfo) return;
   einfo->info.rotation = rotation;
   _ecore_evas_x_rotation_set_internal(ee, rotation, resize,
                                       (Evas_Engine_Info *)einfo);
   if (ee->prop.wm_rot.supported)
     {
        if (ee->prop.wm_rot.app_set)
          {
             if (edata->wm_rot.request)
               {
                  if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
                  edata->wm_rot.request = 0;
                  edata->wm_rot.done = 1;
               }
          }
     }

   int angles[2] = { rotation, rotation };
   ecore_x_window_prop_property_set(ee->prop.window,
                                    ECORE_X_ATOM_E_ILLUME_ROTATE_WINDOW_ANGLE,
                                    ECORE_X_ATOM_CARDINAL, 32, &angles, 2);
#endif /* BUILD_ECORE_EVAS_SOFTWARE_X11 */
}

static void
_ecore_evas_x_rotation_set(Ecore_Evas *ee, int rotation, int resize)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->rotation == rotation)
     {
        if (ee->prop.wm_rot.supported)
          {
             if (edata->wm_rot.request)
               {
                  if (ee->prop.wm_rot.manual_mode.set)
                    {
                       ee->prop.wm_rot.manual_mode.wait_for_done = EINA_FALSE;
                       if (ee->prop.wm_rot.manual_mode.timer)
                         ecore_timer_del(ee->prop.wm_rot.manual_mode.timer);
                       ee->prop.wm_rot.manual_mode.timer = NULL;
                    }
                  /* send rotation done message to wm, even if window is already rotated.
                   * that's why wm must be wait for comming rotation done message
                   * after sending rotation request.
                   */
                  ecore_x_e_window_rotation_change_done_send
                    (edata->win_root, ee->prop.window, ee->rotation, ee->w, ee->h);
                  edata->wm_rot.request = 0;
               }
          }
        return;
     }

   if (ee->prop.wm_rot.supported)
     {
        if (!_ecore_evas_x_wm_rotation_check(ee)) return;
        if (!resize) resize = 1;
     }

   if (!strcmp(ee->driver, "opengl_x11"))
     {
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
        Evas_Engine_Info_GL_X11 *einfo;

        einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;
        einfo->info.rotation = rotation;
        _ecore_evas_x_rotation_set_internal(ee, rotation, resize,
                                            (Evas_Engine_Info *)einfo);
        if (ee->prop.wm_rot.supported)
          {
             if (ee->prop.wm_rot.app_set)
               {
                  if (edata->wm_rot.request)
                    {
                       if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
                       edata->wm_rot.request = 0;
                       edata->wm_rot.done = 1;
                    }
               }
          }

        int angles[2] = { rotation, rotation };
        ecore_x_window_prop_property_set(ee->prop.window,
                                         ECORE_X_ATOM_E_ILLUME_ROTATE_WINDOW_ANGLE,
                                         ECORE_X_ATOM_CARDINAL, 32, &angles, 2);
#endif /* BUILD_ECORE_EVAS_OPENGL_X11 */
     }
   else if (!strcmp(ee->driver, "software_x11"))
     {
        if (ee->in_async_render)
          {
             ee->delayed.rotation = rotation;
             ee->delayed.rotation_resize = resize;
             ee->delayed.rotation_changed = EINA_TRUE;
             return;
          }
        _rotation_do(ee, rotation, resize);
        return;
     }
}

static void
_shaped_do(Ecore_Evas *ee, int shaped)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   Evas_Engine_Info_Software_X11 *einfo;

   if (ee->shaped == shaped) return;
   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   ee->shaped = shaped;
   if (einfo)
     {
        if (ee->shaped)
          {
             unsigned int foreground;
             Ecore_X_GC gc;

             if (!edata->mask)
               edata->mask = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, 1);
             foreground = 0;
             gc = ecore_x_gc_new(edata->mask,
                                 ECORE_X_GC_VALUE_MASK_FOREGROUND,
                                 &foreground);
             ecore_x_drawable_rectangle_fill(edata->mask, gc,
                                             0, 0, ee->w, ee->h);
             ecore_x_gc_free(gc);
             einfo->info.mask = edata->mask;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
             evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
             ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
          }
        else
          {
             if (edata->mask) ecore_x_pixmap_free(edata->mask);
             edata->mask = 0;
             einfo->info.mask = 0;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
             ecore_x_window_shape_mask_set(ee->prop.window, 0);
             ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
          }
     }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_X11 */
}

static void
_ecore_evas_x_shaped_set(Ecore_Evas *ee, int shaped)
{
   if (!strcmp(ee->driver, "opengl_x11")) return;
   if (!strcmp(ee->driver, "software_x11"))
     {
        if (ee->in_async_render)
          {
             ee->delayed.shaped = shaped;
             ee->delayed.shaped_changed = EINA_TRUE;
             return;
          }
        _shaped_do(ee, shaped);
     }
}

static void
_alpha_do(Ecore_Evas *ee, int alpha)
{
   char *id = NULL;
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
   Ecore_X_Window_Attributes att;
   Evas_Engine_Info_Software_X11 *einfo;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->alpha == alpha) return;

   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (!einfo) return;

   if (!ecore_x_composite_query()) return;

   ee->shaped = 0;
   ee->alpha = alpha;
   _ecore_evas_x_sync_clear(ee);
   ecore_x_window_free(ee->prop.window);
   ecore_event_window_unregister(ee->prop.window);
   if (ee->alpha)
     {
        if (ee->prop.override)
          ee->prop.window = ecore_x_window_override_argb_new(edata->win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
        else
          ee->prop.window = ecore_x_window_argb_new(edata->win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
        //             if (!edata->mask)
        //               edata->mask = ecore_x_pixmap_new(ee->prop.window, ee->req.w, ee->req.h, 1);
     }
   else
     {
        if (ee->prop.override)
          ee->prop.window = ecore_x_window_override_new(edata->win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
        else
          ee->prop.window = ecore_x_window_new(edata->win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
        if (edata->mask) ecore_x_pixmap_free(edata->mask);
        edata->mask = 0;
        ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
        ecore_x_vsync_animator_tick_source_set(ee->prop.window);
     }

   einfo->info.destination_alpha = alpha;

   ecore_x_window_attributes_get(ee->prop.window, &att);
   einfo->info.visual = att.visual;
   einfo->info.colormap = att.colormap;
   einfo->info.depth = att.depth;

   //        if (edata->mask) ecore_x_pixmap_free(edata->mask);
   //        edata->mask = 0;
   einfo->info.mask = edata->mask;
   einfo->info.drawable = ee->prop.window;
   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     {
        ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
     }
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.w, ee->req.h);
   ecore_x_window_shape_mask_set(ee->prop.window, 0);
   ecore_x_input_multi_select(ee->prop.window);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
   if (ee->prop.borderless)
     ecore_x_mwm_borderless_set(ee->prop.window, ee->prop.borderless);
   if (ee->visible || ee->should_be_visible)
     ecore_x_window_show(ee->prop.window);
   if (ee->prop.focused) ecore_x_window_focus(ee->prop.window);
   if (ee->prop.title)
     {
        ecore_x_icccm_title_set(ee->prop.window, ee->prop.title);
        ecore_x_netwm_name_set(ee->prop.window, ee->prop.title);
     }
   if (ee->prop.name)
     ecore_x_icccm_name_class_set(ee->prop.window,
                                  ee->prop.name, ee->prop.clas);
   _ecore_evas_x_hints_update(ee);
   _ecore_evas_x_group_leader_update(ee);
   ecore_x_window_defaults_set(ee->prop.window);
   _ecore_evas_x_protocols_set(ee);
   _ecore_evas_x_window_profile_protocol_set(ee);
   _ecore_evas_x_wm_rotation_protocol_set(ee);
   _ecore_evas_x_aux_hints_supprted_update(ee);
   _ecore_evas_x_aux_hints_update(ee);
   _ecore_evas_x_sync_set(ee);
   _ecore_evas_x_size_pos_hints_update(ee);
#endif /* BUILD_ECORE_EVAS_SOFTWARE_X11 */
   if ((id = getenv("DESKTOP_STARTUP_ID")))
     {
        ecore_x_netwm_startup_id_set(ee->prop.window, id);
        /* NB: on linux this may simply empty the env as opposed to completely
         * unset it to being empty - unsure as solartis libc crashes looking
         * for the '=' char */
        //        putenv((char*)"DESKTOP_STARTUP_ID=");
     }
}

/* FIXME, round trip */
static void
_ecore_evas_x_alpha_set(Ecore_Evas *ee, int alpha)
{
   char *id = NULL;

   if (!strcmp(ee->driver, "software_x11"))
     {
        if (ee->in_async_render)
          {
             if (ee->visible)
               {
                  ee->delayed.alpha = alpha;
                  ee->delayed.alpha_changed = EINA_TRUE;
                  return;
               }
             evas_sync(ee->evas);
          }
        _alpha_do(ee, alpha);
     }
   else if (!strcmp(ee->driver, "opengl_x11"))
     {
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
        Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
        Ecore_X_Window_Attributes att;
        if (ee->alpha == alpha) return;

        Evas_Engine_Info_GL_X11 *einfo;
        Ecore_Window prev_win;

        einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;

        if (!ecore_x_composite_query()) return;

        ee->shaped = 0;
        ee->alpha = alpha;
        _ecore_evas_x_sync_clear(ee);
        prev_win = ee->prop.window;
        ee->prop.window = 0;

        einfo->info.destination_alpha = alpha;

        if (edata->win_root != 0)
          {
             /* FIXME: round trip in ecore_x_window_argb_get */
             if (ecore_x_window_argb_get(edata->win_root))
               {
                  ee->prop.window =
                    _ecore_evas_x_gl_window_new(ee, edata->win_root,
                                                ee->req.x, ee->req.y,
                                                ee->req.w, ee->req.h,
                                                ee->prop.override, 1, NULL);
               }
             else
               {
                  ee->prop.window =
                    _ecore_evas_x_gl_window_new(ee, edata->win_root,
                                                ee->req.x, ee->req.y,
                                                ee->req.w, ee->req.h,
                                                ee->prop.override, ee->alpha,
                                                NULL);
               }
          }
        else
          {
             ee->prop.window =
               _ecore_evas_x_gl_window_new(ee, edata->win_root,
                                           ee->req.x, ee->req.y,
                                           ee->req.w, ee->req.h,
                                           ee->prop.override, ee->alpha, NULL);
          }

        ecore_x_window_free(prev_win);
        ecore_event_window_unregister(prev_win);

        if (!ee->prop.window) return;
/*
        if (ee->alpha)
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_argb_new(edata->win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_argb_new(edata->win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (!edata->mask)
               edata->mask = ecore_x_pixmap_new(ee->prop.window, ee->req.w, ee->req.h, 1);
          }
        else
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_new(edata->win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_new(edata->win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (edata->mask) ecore_x_pixmap_free(edata->mask);
             edata->mask = 0;
             ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
          }
 */

        ecore_x_window_attributes_get(ee->prop.window, &att);
        einfo->info.visual = att.visual;
        einfo->info.colormap = att.colormap;
        einfo->info.depth = att.depth;

//        if (edata->mask) ecore_x_pixmap_free(edata->mask);
//        edata->mask = 0;
//        einfo->info.mask = edata->mask;
        einfo->info.drawable = ee->prop.window;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.w, ee->req.h);
//        ecore_x_window_shape_mask_set(ee->prop.window, 0);
        ecore_x_input_multi_select(ee->prop.window);
        ecore_event_window_register(ee->prop.window, ee, ee->evas,
                                    (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                                    (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                                    (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                                    (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
        if (ee->prop.borderless)
          ecore_x_mwm_borderless_set(ee->prop.window, ee->prop.borderless);
        if (ee->visible || ee->should_be_visible)
          ecore_x_window_show(ee->prop.window);
        if (ee->prop.focused) ecore_x_window_focus(ee->prop.window);
        if (ee->prop.title)
          {
             ecore_x_icccm_title_set(ee->prop.window, ee->prop.title);
             ecore_x_netwm_name_set(ee->prop.window, ee->prop.title);
          }
        if (ee->prop.name)
          ecore_x_icccm_name_class_set(ee->prop.window,
                                       ee->prop.name, ee->prop.clas);
        _ecore_evas_x_hints_update(ee);
        _ecore_evas_x_group_leader_update(ee);
        ecore_x_window_defaults_set(ee->prop.window);
        _ecore_evas_x_protocols_set(ee);
        _ecore_evas_x_window_profile_protocol_set(ee);
        _ecore_evas_x_wm_rotation_protocol_set(ee);
        _ecore_evas_x_aux_hints_supprted_update(ee);
        _ecore_evas_x_aux_hints_update(ee);
        _ecore_evas_x_sync_set(ee);
        _ecore_evas_x_size_pos_hints_update(ee);
#endif /* BUILD_ECORE_EVAS_OPENGL_X11 */
        if ((id = getenv("DESKTOP_STARTUP_ID")))
          {
             ecore_x_netwm_startup_id_set(ee->prop.window, id);
             /* NB: on linux this may simply empty the env as opposed to completely
              * unset it to being empty - unsure as solartis libc crashes looking
              * for the '=' char */
             //        putenv((char*)"DESKTOP_STARTUP_ID=");
          }
     }
}

static void
_transparent_do(Ecore_Evas *ee, int transparent)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
   if (ee->transparent == transparent) return;

   Evas_Engine_Info_Software_X11 *einfo;

   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (!einfo) return;

   ee->transparent = transparent;
   einfo->info.destination_alpha = transparent;
   if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
     {
        ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
     }
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
#endif
}

static void
_ecore_evas_x_transparent_set(Ecore_Evas *ee, int transparent)
{
   if (!strcmp(ee->driver, "software_x11"))
     {
        if (ee->in_async_render)
          {
             ee->delayed.transparent = transparent;
             ee->delayed.transparent_changed = EINA_TRUE;
             return;
          }
        _transparent_do(ee, transparent);
     }
}

static void
_ecore_evas_x_window_group_set(Ecore_Evas *ee, const Ecore_Evas *group_ee)
{
   if (ee->prop.group_ee == group_ee) return;

   ee->prop.group_ee = (Ecore_Evas *)group_ee;
   if (ee->prop.group_ee)
     ee->prop.group_ee_win = group_ee->prop.window;
   else
     ee->prop.group_ee_win = 0;
   _ecore_evas_x_hints_update(ee);
}

static void
_ecore_evas_x_aspect_set(Ecore_Evas *ee, double aspect)
{
   if (ee->prop.aspect == aspect) return;

   ee->prop.aspect = aspect;
   _ecore_evas_x_size_pos_hints_update(ee);
// netwm state  
//   if (ee->should_be_visible)
//     ecore_x_netwm_state_request_send(ee->prop.window, ee->engine.x.win_root,
//                                      ECORE_X_WINDOW_STATE_STICKY, -1, sticky);
//   else
//     _ecore_evas_x_state_update(ee);
}

static void
_ecore_evas_x_urgent_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.urgent == on) return;

   ee->prop.urgent = on;
   _ecore_evas_x_hints_update(ee);
}

static void
_ecore_evas_x_modal_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->prop.modal == on) return;

   ee->prop.modal = on;
   if (ee->should_be_visible)
     ecore_x_netwm_state_request_send(ee->prop.window, edata->win_root,
                                      ECORE_X_WINDOW_STATE_MODAL, -1, on);
   else
     _ecore_evas_x_state_update(ee);
}

static void
_ecore_evas_x_demand_attention_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->prop.demand_attention == on) return;

   ee->prop.demand_attention = on;
   if (ee->should_be_visible)
     ecore_x_netwm_state_request_send(ee->prop.window, edata->win_root,
                                      ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION,
                                      -1, on);
   else
     _ecore_evas_x_state_update(ee);
}

static void
_ecore_evas_x_focus_skip_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->prop.focus_skip == on) return;

   ee->prop.focus_skip = on;
   if (ee->should_be_visible)
     {
        ecore_x_netwm_state_request_send(ee->prop.window, edata->win_root,
                                         ECORE_X_WINDOW_STATE_SKIP_TASKBAR, -1,
                                         on);
        ecore_x_netwm_state_request_send(ee->prop.window, edata->win_root,
                                         ECORE_X_WINDOW_STATE_SKIP_PAGER, -1,
                                         on);
     }
   else
     _ecore_evas_x_state_update(ee);
   _ecore_evas_x_hints_update(ee);
}

static void
_ecore_evas_x_show(Ecore_Evas *ee)
{
   ee->should_be_visible = 1;
   if (ee->prop.avoid_damage)
     _ecore_evas_x_render(ee);
   _ecore_evas_x_sync_set(ee);
   _ecore_evas_x_window_profile_set(ee);
   ecore_x_window_show(ee->prop.window);
   if (ee->prop.fullscreen)
     ecore_x_window_focus(ee->prop.window);
}

static void
_ecore_evas_x_hide(Ecore_Evas *ee)
{
   ecore_x_window_hide(ee->prop.window);
   ee->should_be_visible = 0;
   _ecore_evas_x_sync_set(ee);
}

static void
_ecore_evas_x_raise(Ecore_Evas *ee)
{
   ecore_x_window_raise(ee->prop.window);
}

static void
_ecore_evas_x_lower(Ecore_Evas *ee)
{
   ecore_x_window_lower(ee->prop.window);
}

static void
_ecore_evas_x_activate(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   ecore_evas_show(ee);
   ecore_x_netwm_client_active_request(edata->win_root,
                                       ee->prop.window, 2, 0);
}

static void
_ecore_evas_x_title_set(Ecore_Evas *ee, const char *t)
{
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (!t) return;
   ee->prop.title = strdup(t);
   ecore_x_icccm_title_set(ee->prop.window, ee->prop.title);
   ecore_x_netwm_name_set(ee->prop.window, ee->prop.title);
}

static void
_ecore_evas_x_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   if (ee->prop.name) free(ee->prop.name);
   if (ee->prop.clas) free(ee->prop.clas);
   ee->prop.name = NULL;
   ee->prop.clas = NULL;
   if (n) ee->prop.name = strdup(n);
   if (c) ee->prop.clas = strdup(c);
   ecore_x_icccm_name_class_set(ee->prop.window, ee->prop.name, ee->prop.clas);
}

static void
_ecore_evas_x_size_min_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.min.w == w) && (ee->prop.min.h == h)) return;
   ee->prop.min.w = w;
   ee->prop.min.h = h;
   _ecore_evas_x_size_pos_hints_update(ee);
}

static void
_ecore_evas_x_size_max_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.max.w == w) && (ee->prop.max.h == h)) return;
   ee->prop.max.w = w;
   ee->prop.max.h = h;
   _ecore_evas_x_size_pos_hints_update(ee);
}

static void
_ecore_evas_x_size_base_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->prop.base.w == w) && (ee->prop.base.h == h)) return;
   ee->prop.base.w = w;
   ee->prop.base.h = h;
   _ecore_evas_x_size_pos_hints_update(ee);
}

static void
_ecore_evas_x_size_step_set(Ecore_Evas *ee, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((ee->prop.step.w == w) && (ee->prop.step.h == h)) return;
   ee->prop.step.w = w;
   ee->prop.step.h = h;
   _ecore_evas_x_size_pos_hints_update(ee);
}

static void
_ecore_evas_object_cursor_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee) ee->prop.cursor.object = NULL;
}

static void
_ecore_evas_x_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   int x = 0, y = 0;
   Evas_Object *old;

   old = ee->prop.cursor.object;
   if (!obj)
     {
        ee->prop.cursor.object = NULL;
        ee->prop.cursor.layer = 0;
        ee->prop.cursor.hot.x = 0;
        ee->prop.cursor.hot.y = 0;
        ecore_x_window_cursor_show(ee->prop.window, 1);
        goto end;
     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   evas_pointer_output_xy_get(ee->evas, &x, &y);

   if (obj != old)
     {
        ecore_x_window_cursor_show(ee->prop.window, 0);
        evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
        evas_object_pass_events_set(ee->prop.cursor.object, 1);
        if (evas_pointer_inside_get(ee->evas))
          evas_object_show(ee->prop.cursor.object);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                       _ecore_evas_object_cursor_del, ee);
     }

   evas_object_move(ee->prop.cursor.object, x - ee->prop.cursor.hot.x,
                    y - ee->prop.cursor.hot.y);

end:
   if ((old) && (obj != old))
     {
        evas_object_event_callback_del_full
          (old, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, ee);
        evas_object_del(old);
     }
}

/*
 * @param ee
 * @param layer If < 3, @a ee will be put below all other windows.
 *              If > 5, @a ee will be "always-on-top"
 *              If = 4, @a ee will be put in the default layer.
 *              Acceptable values range from 1 to 255 (0 reserved for
 *              desktop windows)
 */
static void
_ecore_evas_x_layer_set(Ecore_Evas *ee, int layer)
{
   if (ee->prop.layer == layer) return;

   /* FIXME: Should this logic be here? */
   if (layer < 1)
     layer = 1;
   else if (layer > 255)
     layer = 255;

   ee->prop.layer = layer;
   _ecore_evas_x_layer_update(ee);
}

static void
_ecore_evas_x_focus_set(Ecore_Evas *ee, Eina_Bool on EINA_UNUSED)
{
   ecore_x_window_focus(ee->prop.window);
}

static void
_ecore_evas_x_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->prop.iconified == on) return;
   if (((ee->should_be_visible) && (!ee->visible)) || (!ee->visible))
     ee->prop.iconified = on;
   _ecore_evas_x_hints_update(ee);
   if (on)
     ecore_x_icccm_iconic_request_send(ee->prop.window, edata->win_root);
   else
     ecore_evas_show(ee);
}

static void
_ecore_evas_x_borderless_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.borderless == on) return;
   ee->prop.borderless = on;
   ecore_x_mwm_borderless_set(ee->prop.window, ee->prop.borderless);
}

/* FIXME: This function changes the initial state of the ee
 * whilest the iconic function changes the current state! */
static void
_ecore_evas_x_withdrawn_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.withdrawn == on) return;
//   ee->prop.withdrawn = on;
   _ecore_evas_x_hints_update(ee);
   if (on)
     ecore_evas_hide(ee);
   else
     ecore_evas_show(ee);
}

static void
_ecore_evas_x_sticky_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->prop.sticky == on) return;

   /* We dont want to set prop.sticky here as it will cause
    * the sticky callback not to get called. Its set on the
    * property change event.
    * ee->prop.sticky = on;
    */
//   edata->state.sticky = on;
   if (ee->should_be_visible)
     ecore_x_netwm_state_request_send(ee->prop.window, edata->win_root,
                                      ECORE_X_WINDOW_STATE_STICKY, -1, on);
   else
     _ecore_evas_x_state_update(ee);
}

static void
_ecore_evas_x_ignore_events_set(Ecore_Evas *ee, int ignore)
{
   if (ee->ignore_events == ignore) return;

   ee->ignore_events = ignore;
   if (ee->prop.window)
     ecore_x_window_ignore_set(ee->prop.window, ignore);
}

/*
static void
_ecore_evas_x_reinit_win(Ecore_Evas *ee)
{
   if (!strcmp(ee->driver, "software_x11"))
     {
#ifdef BUILD_ECORE_EVAS_X11
        Evas_Engine_Info_Software_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             einfo->info.drawable = ee->prop.window;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
          }
#endif
     }
   else if (!strcmp(ee->driver, "opengl_x11"))
     {
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
        Evas_Engine_Info_GL_X11 *einfo;

        einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             einfo->info.drawable = ee->prop.window;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
          }
#endif
     }
}
*/

static void
_ecore_evas_x_override_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (ee->prop.override == on) return;
   if (ee->should_be_visible) ecore_x_window_hide(ee->prop.window);
   ecore_x_window_override_set(ee->prop.window, on);
   if (ee->should_be_visible) ecore_x_window_show(ee->prop.window);
   if (ee->prop.focused) ecore_x_window_focus(ee->prop.window);
   ee->prop.override = on;
}

static void
_ecore_evas_x_maximized_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->prop.maximized == on) return;
   edata->state.maximized_h = 1;
   edata->state.maximized_v = 1;
//   ee->prop.maximized = on;
   if (ee->should_be_visible)
     {
        ecore_x_netwm_state_request_send(ee->prop.window, edata->win_root,
                                         ECORE_X_WINDOW_STATE_MAXIMIZED_VERT, -1, on);
        ecore_x_netwm_state_request_send(ee->prop.window, edata->win_root,
                                         ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ, -1, on);
     }
   else
     _ecore_evas_x_state_update(ee);
}

static void
_ecore_evas_x_fullscreen_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->prop.fullscreen == on) return;

   /* FIXME: Detect if WM is EWMH compliant and handle properly if not,
    * i.e. reposition, resize, and change borderless hint */
   edata->state.fullscreen = on;
   if (ee->should_be_visible)
     ecore_x_netwm_state_request_send(ee->prop.window, edata->win_root,
                                      ECORE_X_WINDOW_STATE_FULLSCREEN, -1, on);
   else
     _ecore_evas_x_state_update(ee);
}

static void
_ecore_evas_x_profile_set(Ecore_Evas *ee, const char *profile)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   _ecore_evas_window_profile_free(ee);
   ee->prop.profile.name = NULL;

   if (profile)
     ee->prop.profile.name = (char *)eina_stringshare_add(profile);
   edata->profile.change = 1;
   _ecore_evas_x_window_profile_set(ee);
}

static void
_ecore_evas_x_profiles_set(Ecore_Evas *ee, const char **plist, int n)
{
   int i;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   _ecore_evas_window_available_profiles_free(ee);
   ee->prop.profile.available_list = NULL;

   if ((plist) && (n >= 1))
     {
        ee->prop.profile.available_list = calloc(n, sizeof(char *));
        if (ee->prop.profile.available_list)
          {
             for (i = 0; i < n; i++)
               ee->prop.profile.available_list[i] = (char *)eina_stringshare_add(plist[i]);
             ee->prop.profile.count = n;
          }
     }
   edata->profile.available = 1;
   _ecore_evas_x_window_profile_set(ee);
}

static void
_avoid_damage_do(Ecore_Evas *ee, int on)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
   Evas_Engine_Info_Software_X11 *einfo;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   ee->prop.avoid_damage = on;
   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        if (ee->prop.avoid_damage)
          {
             edata->pmap = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, einfo->info.depth);
             edata->gc = ecore_x_gc_new(edata->pmap, 0, NULL);
             einfo->info.drawable = edata->pmap;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
             if (ECORE_EVAS_PORTRAIT(ee))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
             if (ee->prop.avoid_damage == ECORE_EVAS_AVOID_DAMAGE_BUILT_IN)
               {
                  edata->using_bg_pixmap = 1;
                  ecore_x_window_pixmap_set(ee->prop.window, edata->pmap);
                  ecore_x_window_area_expose(ee->prop.window, 0, 0, ee->w, ee->h);
               }
             if (edata->direct_resize)
               {
                  /* Turn this off for now
                     edata->using_bg_pixmap = 1;
                     ecore_x_window_pixmap_set(ee->prop.window, edata->pmap);
                     */
               }
          }
        else
          {
             if (edata->pmap) ecore_x_pixmap_free(edata->pmap);
             if (edata->gc) ecore_x_gc_free(edata->gc);
             if (edata->using_bg_pixmap)
               {
                  ecore_x_window_pixmap_set(ee->prop.window, 0);
                  edata->using_bg_pixmap = 0;
                  ecore_x_window_area_expose(ee->prop.window, 0, 0, ee->w, ee->h);
               }
             edata->pmap = 0;
             edata->gc = 0;
             einfo->info.drawable = ee->prop.window;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
          }
     }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_X11 */
}

static void
_ecore_evas_x_avoid_damage_set(Ecore_Evas *ee, int on)
{
   if (ee->prop.avoid_damage == on) return;
   if (!strcmp(ee->driver, "opengl_x11")) return;

   if (!strcmp(ee->driver, "software_x11"))
     {
        if (ee->in_async_render)
          {
             ee->delayed.avoid_damage = on;
             return;
          }
        _avoid_damage_do(ee, on);
     }
}

static void
_ecore_evas_x_screen_geometry_get(const Ecore_Evas *ee EINA_UNUSED, int *x, int *y, int *w, int *h)
{
   int outnum = 0;
   int px = 0, py = 0, pw = 0, ph = 0;
   Ecore_X_Window root;
   Ecore_X_Randr_Output *out = NULL;
   Ecore_X_Randr_Crtc crtc;
   unsigned int val[4] = { 0 };
   
   if (ecore_x_window_prop_card32_get
       (ee->prop.window, ecore_x_atom_get("E_ZONE_GEOMETRY"), val, 4) == 4)
     {
        if (x) *x = (int)val[0];
        if (y) *y = (int)val[1];
        if (w) *w = (int)val[2];
        if (h) *h = (int)val[3];
        return;
     }
   
   root = ecore_x_window_root_get(ee->prop.window);
   out = ecore_x_randr_window_outputs_get(ee->prop.window, &outnum);
   if (!out)
     {
norandr:
        if (out) free(out);
        if (x) *x = 0;
        if (y) *y = 0;
        ecore_x_window_size_get(root, w, h);
        return;
     }
   crtc = ecore_x_randr_output_crtc_get(root, out[0]);
   if (!crtc) goto norandr;
   ecore_x_randr_crtc_geometry_get(root, crtc, &px, &py, &pw, &ph);
   if ((pw == 0) || (ph == 0)) goto norandr;
   if (x) *x = px;
   if (y) *y = py;
   if (w) *w = pw;
   if (h) *h = ph;
   free(out);
}

static void
_ecore_evas_x_screen_dpi_get(const Ecore_Evas *ee, int *xdpi, int *ydpi)
{
   int scdpi, xmm = 0, ymm = 0, outnum = 0, w = 0, h = 0;
   int px = 0, py = 0;
   Ecore_X_Window root;
   Ecore_X_Randr_Output *out = NULL;
   Ecore_X_Randr_Crtc crtc;

   root = ecore_x_window_root_get(ee->prop.window);
   out = ecore_x_randr_window_outputs_get(ee->prop.window, &outnum);
   if (!out)
     {
norandr:
        if (out) free(out);
        scdpi = ecore_x_dpi_get();
        if (xdpi) *xdpi = scdpi;
        if (ydpi) *ydpi = scdpi;
        return;
     }
   crtc = ecore_x_randr_output_crtc_get(root, out[0]);
   if (!crtc) goto norandr;
   ecore_x_randr_crtc_geometry_get(root, crtc, &px, &py, &w, &h);
   if ((w == 0) || (h == 0)) goto norandr;
   ecore_x_randr_output_size_mm_get(root, out[0], &xmm, &ymm);
   if ((xmm == 0) || (ymm == 0)) goto norandr;
   if (xdpi) *xdpi = (w * 254) / (xmm * 10); // 25.4mm / inch
   if (ydpi) *ydpi = (h * 254) / (ymm * 10); // 25.4mm / inch
   free(out);
}

static void 
_ecore_evas_x_pointer_xy_get(const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   if (ee->prop.window)
     ecore_x_pointer_xy_get(ee->prop.window, x, y);
}

static Eina_Bool 
_ecore_evas_x_pointer_warp(const Ecore_Evas *ee, Evas_Coord x, Evas_Coord y)
{
   return ecore_x_pointer_warp(ee->prop.window, x, y);
}

static void
_ecore_evas_x_wm_rot_preferred_rotation_set(Ecore_Evas *ee, int rot)
{
   if (ee->prop.wm_rot.supported)
     {
        if (!ee->prop.wm_rot.app_set)
          {
             ecore_x_e_window_rotation_app_set(ee->prop.window, EINA_TRUE);
             ee->prop.wm_rot.app_set = EINA_TRUE;
          }
        ecore_x_e_window_rotation_preferred_rotation_set(ee->prop.window, rot);
        ee->prop.wm_rot.preferred_rot = rot;
     }
}

static void
_ecore_evas_x_wm_rot_available_rotations_set(Ecore_Evas *ee, const int *rots, unsigned int count)
{
   if (ee->prop.wm_rot.supported)
     {
        if (!ee->prop.wm_rot.app_set)
          {
             ecore_x_e_window_rotation_app_set(ee->prop.window, EINA_TRUE);
             ee->prop.wm_rot.app_set = EINA_TRUE;
          }

        if (ee->prop.wm_rot.available_rots)
          {
             free(ee->prop.wm_rot.available_rots);
             ee->prop.wm_rot.available_rots = NULL;
          }

        ee->prop.wm_rot.count = 0;

        if (count > 0)
          {
             ee->prop.wm_rot.available_rots = calloc(count, sizeof(int));
             if (!ee->prop.wm_rot.available_rots) return;

             memcpy(ee->prop.wm_rot.available_rots, rots, sizeof(int) * count);
          }

        ee->prop.wm_rot.count = count;

        ecore_x_e_window_rotation_available_rotations_set(ee->prop.window, rots, count);
     }
}

static void
_ecore_evas_x_wm_rot_manual_rotation_done_set(Ecore_Evas *ee, Eina_Bool set)
{
   ee->prop.wm_rot.manual_mode.set = set;
}

static void
_ecore_evas_x_wm_rot_manual_rotation_done(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if ((ee->prop.wm_rot.supported) &&
       (ee->prop.wm_rot.app_set) &&
       (ee->prop.wm_rot.manual_mode.set))
     {
        if (ee->prop.wm_rot.manual_mode.wait_for_done)
          {
             if (ee->prop.wm_rot.manual_mode.timer)
               ecore_timer_del(ee->prop.wm_rot.manual_mode.timer);
             ee->prop.wm_rot.manual_mode.timer = NULL;

             if (edata->wm_rot.manual_mode_job)
               ecore_job_del(edata->wm_rot.manual_mode_job);
             edata->wm_rot.manual_mode_job = ecore_job_add
               (_ecore_evas_x_wm_rot_manual_rotation_done_job, ee);
          }
     }
}

static Eina_Bool
_ecore_evas_x_wm_rot_manual_rotation_done_timeout(void *data)
{
   Ecore_Evas *ee = data;

   ee->prop.wm_rot.manual_mode.timer = NULL;
   _ecore_evas_x_wm_rot_manual_rotation_done(ee);
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_evas_x_wm_rot_manual_rotation_done_timeout_update(Ecore_Evas *ee)
{
   if (ee->prop.wm_rot.manual_mode.timer)
     ecore_timer_del(ee->prop.wm_rot.manual_mode.timer);

   ee->prop.wm_rot.manual_mode.timer = ecore_timer_add
     (4.0f, _ecore_evas_x_wm_rot_manual_rotation_done_timeout, ee);
}

static void
_ecore_evas_x_aux_hints_set(Ecore_Evas *ee, const char *hints)
{
   if (hints)
     ecore_x_window_prop_property_set
       (ee->prop.window, ECORE_X_ATOM_E_WINDOW_AUX_HINT,
        ECORE_X_ATOM_STRING, 8, (void *)hints, strlen(hints) + 1);
   else
     ecore_x_window_prop_property_del
       (ee->prop.window, ECORE_X_ATOM_E_WINDOW_AUX_HINT);
}

static Ecore_Evas_Engine_Func _ecore_x_engine_func =
{
   _ecore_evas_x_free,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_x_callback_delete_request_set,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_x_move,
   _ecore_evas_x_managed_move,
   _ecore_evas_x_resize,
   _ecore_evas_x_move_resize,
   _ecore_evas_x_rotation_set,
   _ecore_evas_x_shaped_set,
   _ecore_evas_x_show,
   _ecore_evas_x_hide,
   _ecore_evas_x_raise,
   _ecore_evas_x_lower,
   _ecore_evas_x_activate,
   _ecore_evas_x_title_set,
   _ecore_evas_x_name_class_set,
   _ecore_evas_x_size_min_set,
   _ecore_evas_x_size_max_set,
   _ecore_evas_x_size_base_set,
   _ecore_evas_x_size_step_set,
   _ecore_evas_x_object_cursor_set,
   _ecore_evas_x_layer_set,
   _ecore_evas_x_focus_set,
   _ecore_evas_x_iconified_set,
   _ecore_evas_x_borderless_set,
   _ecore_evas_x_override_set,
   _ecore_evas_x_maximized_set,
   _ecore_evas_x_fullscreen_set,
   _ecore_evas_x_avoid_damage_set,
   _ecore_evas_x_withdrawn_set,
   _ecore_evas_x_sticky_set,
   _ecore_evas_x_ignore_events_set,
   _ecore_evas_x_alpha_set,
   _ecore_evas_x_transparent_set,
   _ecore_evas_x_profiles_set,
   _ecore_evas_x_profile_set,

   _ecore_evas_x_window_group_set,
   _ecore_evas_x_aspect_set,
   _ecore_evas_x_urgent_set,
   _ecore_evas_x_modal_set,
   _ecore_evas_x_demand_attention_set,
   _ecore_evas_x_focus_skip_set,

   NULL, // render
   _ecore_evas_x_screen_geometry_get,
   _ecore_evas_x_screen_dpi_get,
   NULL,
   NULL, //fn_msg_send

   _ecore_evas_x_pointer_xy_get,
   _ecore_evas_x_pointer_warp,

   _ecore_evas_x_wm_rot_preferred_rotation_set,
   _ecore_evas_x_wm_rot_available_rotations_set,
   _ecore_evas_x_wm_rot_manual_rotation_done_set,
   _ecore_evas_x_wm_rot_manual_rotation_done,

   _ecore_evas_x_aux_hints_set
};

/*
 * FIXME: there are some round trips. Especially, we can split
 * ecore_x_init in 2 functions and suppress some round trips.
 */

#if defined (BUILD_ECORE_EVAS_SOFTWARE_X11) || defined (BUILD_ECORE_EVAS_OPENGL_X11)
static void
_ecore_evas_x_render_pre(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   /* printf("Ecore_Evas Render Pre\n"); */
   /* printf("\tPixman Size: %d %d\n", edata->pixmap.w, edata->pixmap.h); */
   /* printf("\tEE Size: %d %d\n", ee->w, ee->h); */

   /* before rendering to the back buffer pixmap, we should check the 
    * size. If the back buffer is not the proper size, destroy it and 
    * create a new one at the proper size */
   if ((edata->pixmap.w != ee->w) || (edata->pixmap.h != ee->h))
     {
        /* free the backing pixmap */
        if (edata->pixmap.back) 
          ecore_x_pixmap_free(edata->pixmap.back);

        edata->pixmap.back = 
          ecore_x_pixmap_new(edata->win_root, ee->w, ee->h, 
                             edata->pixmap.depth);

        edata->pixmap.w = ee->w;
        edata->pixmap.h = ee->h;

        if (!strcmp(ee->driver, "software_x11"))
          {
# ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
             Evas_Engine_Info_Software_X11 *einfo;

             einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
             if (einfo)
               {
                  einfo->info.drawable = edata->pixmap.back;

                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() init engine '%s' failed.", 
                           ee->driver);
                    }
               }
# endif
          }
        else if (!strcmp(ee->driver, "opengl_x11"))
          {
# ifdef BUILD_ECORE_EVAS_OPENGL_X11
             Evas_Engine_Info_GL_X11 *einfo;

             einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
             if (einfo)
               {
                  einfo->info.drawable = edata->pixmap.back;

                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() init engine '%s' failed.", 
                           ee->driver);
                    }
               }
# endif
          }
     }
}

static void
_ecore_evas_x_flush_pre(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if (ee->no_comp_sync) return;
   if (!_ecore_evas_app_comp_sync) return;
   if (!edata->sync_counter) return;
   if (!edata->sync_began) return;

   edata->sync_val++;
   if (!edata->sync_cancel)
     {
        if (!ee->semi_sync)
          ecore_x_sync_counter_val_wait(edata->sync_counter,
                                        edata->sync_val);
     }
}

static void
_ecore_evas_x_flush_post(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   if ((!ee->prop.window) && (edata->pixmap.back))
     {
        Ecore_X_Pixmap prev;

        /* printf("Ecore_Evas Flush Post\n"); */
        /* printf("\tBack Pixmap: %d\n", edata->pixmap.back); */
        /* printf("\tFront Pixmap: %d\n", edata->pixmap.front); */

        /* done drawing to the back buffer. flip it to the front so that 
         * any calls to "fetch pixmap" will return the front buffer already 
         * pre-rendered */

        /* record the current front buffer */
        prev = edata->pixmap.front;

        /* flip them */
        edata->pixmap.front = edata->pixmap.back;

        /* reassign back buffer to be the old front one */
        edata->pixmap.back = prev;

        /* update evas drawable */
        if (!strcmp(ee->driver, "software_x11"))
          {
# ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
             Evas_Engine_Info_Software_X11 *einfo;

             einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
             if (einfo)
               {
                  einfo->info.drawable = edata->pixmap.back;

                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() init engine '%s' failed.", 
                           ee->driver);
                    }
               }
# endif
          }
        else if (!strcmp(ee->driver, "opengl_x11"))
          {
# ifdef BUILD_ECORE_EVAS_OPENGL_X11
             Evas_Engine_Info_GL_X11 *einfo;

             einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
             if (einfo)
               {
                  einfo->info.drawable = edata->pixmap.back;

                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() init engine '%s' failed.", 
                           ee->driver);
                    }
               }
# endif
          }
     }

   if ((!ee->no_comp_sync) && (_ecore_evas_app_comp_sync) &&
       (ee->gl_sync_draw_done != 1))
     {
        if (edata->sync_counter)
          {
             if (edata->sync_began)
               {
                  if (!edata->sync_cancel)
                    {
                       if (ee->prop.window)
                         {
                            ecore_x_e_comp_sync_draw_size_done_send
                              (edata->win_root, ee->prop.window, ee->w, ee->h);
                         }
                    }
               }
          }
     }
   if (edata->netwm_sync_set)
     {
        ecore_x_sync_counter_2_set(edata->netwm_sync_counter,
                                   edata->netwm_sync_val_hi,
                                   edata->netwm_sync_val_lo);
        edata->netwm_sync_set = 0;
     }
   if (edata->profile.done)
     {
        if (ee->prop.window)
          {
             ecore_x_e_window_profile_change_done_send
               (edata->win_root, ee->prop.window, ee->prop.profile.name);
          }
        edata->profile.done = 0;
     }
   if ((ee->prop.wm_rot.supported) &&
       (edata->wm_rot.done))
     {
        if (!ee->prop.wm_rot.manual_mode.set)
          {
             ecore_x_e_window_rotation_change_done_send
               (edata->win_root, ee->prop.window, ee->rotation, ee->w, ee->h);
             edata->wm_rot.done = 0;
          }
     }
}
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
EAPI Ecore_Evas *
ecore_evas_software_x11_new_internal(const char *disp_name, Ecore_X_Window parent,
				     int x, int y, int w, int h)
{
   Evas_Engine_Info_Software_X11 *einfo;
   Ecore_Evas_Interface_X11 *iface;
   Ecore_Evas_Interface_Software_X11 *siface;
   Ecore_Evas_Engine_Data_X11 *edata;
   Ecore_Evas *ee;
   int argb = 0, rmethod;
   static int redraw_debug = -1;
   char *id = NULL;

   rmethod = evas_render_method_lookup("software_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   edata = calloc(1, sizeof(Ecore_Evas_Engine_Data_X11));
   if (!edata)
     {
	free(ee);
	return NULL;
     }

   ee->engine.data = edata;
   iface = _ecore_evas_x_interface_x11_new();
   siface = _ecore_evas_x_interface_software_x11_new();

   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, siface);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_x_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;

   ee->driver = "software_x11";
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->x = x;
   ee->y = y;
   ee->w = w;
   ee->h = h;
   ee->req.x = ee->x;
   ee->req.y = ee->y;
   ee->req.w = ee->w;
   ee->req.h = ee->h;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = 0;
   edata->state.sticky = 0;

   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = 0;
   else
     ee->can_async_render = 1;

   /* init evas here */
   ee->evas = evas_new();
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE,
                           _ecore_evas_x_flush_pre, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST,
                           _ecore_evas_x_flush_post, ee);
   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
			     _ecore_evas_x_render_updates, ee);
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   edata->win_root = parent;
   edata->screen_num = 0;

   if (parent != 0)
     {
        edata->screen_num = 1; /* FIXME: get real scren # */
       /* FIXME: round trip in ecore_x_window_argb_get */
        if (ecore_x_window_argb_get(parent))
          {
             ee->prop.window = ecore_x_window_argb_new(parent, x, y, w, h);
             argb = 1;
          }
        else
          ee->prop.window = ecore_x_window_new(parent, x, y, w, h);
     }
   else
     ee->prop.window = ecore_x_window_new(parent, x, y, w, h);
   ecore_x_vsync_animator_tick_source_set(ee->prop.window);
   if ((id = getenv("DESKTOP_STARTUP_ID")))
     {
        ecore_x_netwm_startup_id_set(ee->prop.window, id);
        /* NB: on linux this may simply empty the env as opposed to completely
         * unset it to being empty - unsure as solartis libc crashes looking
         * for the '=' char */
//        putenv((char*)"DESKTOP_STARTUP_ID=");
     }
   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        Ecore_X_Screen *screen;

        /* FIXME: this is inefficient as its 1 or more round trips */
        screen = ecore_x_default_screen_get();
        if (ecore_x_screen_count_get() > 1)
          {
             Ecore_X_Window *roots;
             int num, i;

             num = 0;
             roots = ecore_x_window_root_list(&num);
             if (roots)
               {
                  Ecore_X_Window root;

                  root = ecore_x_window_root_get(parent);
                  for (i = 0; i < num; i++)
                    {
                       if (root == roots[i])
                         {
                            screen = ecore_x_screen_get(i);
                            break;
                         }
                    }
                  free(roots);
               }
          }

        einfo->info.destination_alpha = argb;

        if (redraw_debug < 0)
          {
             if (getenv("REDRAW_DEBUG"))
               redraw_debug = atoi(getenv("REDRAW_DEBUG"));
             else
               redraw_debug = 0;
          }

# ifdef BUILD_ECORE_EVAS_SOFTWARE_XCB
        einfo->info.backend = EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XCB;
        einfo->info.connection = ecore_x_connection_get();
        einfo->info.screen = screen;
# else
        einfo->info.backend = EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB;
        einfo->info.connection = ecore_x_display_get();
        einfo->info.screen = NULL;
# endif
        einfo->info.drawable = ee->prop.window;

        if (argb)
          {
             Ecore_X_Window_Attributes at;

             ecore_x_window_attributes_get(ee->prop.window, &at);
             einfo->info.visual = at.visual;
             einfo->info.colormap = at.colormap;
             einfo->info.depth = at.depth;
             einfo->info.destination_alpha = 1;
          }
        else
          {
             einfo->info.visual =
               ecore_x_default_visual_get(einfo->info.connection, screen);
             einfo->info.colormap =
               ecore_x_default_colormap_get(einfo->info.connection, screen);
             einfo->info.depth =
               ecore_x_default_depth_get(einfo->info.connection, screen);
             einfo->info.destination_alpha = 0;
          }

        einfo->info.rotation = 0;
        einfo->info.debug = redraw_debug;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }

   _ecore_evas_x_hints_update(ee);
   _ecore_evas_x_group_leader_set(ee);
   ecore_x_window_defaults_set(ee->prop.window);
   _ecore_evas_x_protocols_set(ee);
   _ecore_evas_x_window_profile_protocol_set(ee);
   _ecore_evas_x_wm_rotation_protocol_set(ee);
   _ecore_evas_x_aux_hints_supprted_update(ee);
   _ecore_evas_x_aux_hints_update(ee);
   _ecore_evas_x_sync_set(ee);

   ee->engine.func->fn_render = _ecore_evas_x_render;
   _ecore_evas_register(ee);
   ecore_x_input_multi_select(ee->prop.window);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_software_x11_pixmap_new_internal(const char *disp_name, Ecore_X_Window parent,
                                            int x, int y, int w, int h)
{
   Evas_Engine_Info_Software_X11 *einfo;
   Ecore_Evas_Interface_X11 *iface;
   Ecore_Evas_Interface_Software_X11 *siface;
   Ecore_Evas_Engine_Data_X11 *edata;
   Ecore_Evas *ee;
   int argb = 0, rmethod;
   static int redraw_debug = -1;

   rmethod = evas_render_method_lookup("software_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   edata = calloc(1, sizeof(Ecore_Evas_Engine_Data_X11));
   if (!edata)
     {
	free(ee);
	return NULL;
     }

   ee->engine.data = edata;
   iface = _ecore_evas_x_interface_x11_new();
   siface = _ecore_evas_x_interface_software_x11_new();

   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, siface);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_x_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;

   ee->driver = "software_x11";
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->x = x;
   ee->y = y;
   ee->w = w;
   ee->h = h;
   ee->req.x = ee->x;
   ee->req.y = ee->y;
   ee->req.w = ee->w;
   ee->req.h = ee->h;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = 0;
   edata->state.sticky = 0;

   if (getenv("ECORE_EVAS_FORCE_SYNC_RENDER"))
     ee->can_async_render = 0;
   else
     ee->can_async_render = 1;

   /* init evas here */
   ee->evas = evas_new();

   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE,
                           _ecore_evas_x_flush_pre, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST,
                           _ecore_evas_x_flush_post, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_PRE, 
                           _ecore_evas_x_render_pre, ee);

   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
			     _ecore_evas_x_render_updates, ee);
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   edata->win_root = parent;
   edata->screen_num = 0;
   edata->direct_resize = 1;

   if (parent != 0)
     {
        edata->screen_num = 1; /* FIXME: get real scren # */
       /* FIXME: round trip in ecore_x_window_argb_get */
        if (ecore_x_window_argb_get(parent))
          argb = 1;
     }

   /* if ((id = getenv("DESKTOP_STARTUP_ID"))) */
   /*   { */
        /* ecore_x_netwm_startup_id_set(ee->prop.window, id); */
        /* NB: on linux this may simply empty the env as opposed to completely
         * unset it to being empty - unsure as solartis libc crashes looking
         * for the '=' char */
//        putenv((char*)"DESKTOP_STARTUP_ID=");
     /* } */

   einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        Ecore_X_Screen *screen;

        /* FIXME: this is inefficient as its 1 or more round trips */
        screen = ecore_x_default_screen_get();
        if (ecore_x_screen_count_get() > 1)
          {
             Ecore_X_Window *roots;
             int num, i;

             num = 0;
             roots = ecore_x_window_root_list(&num);
             if (roots)
               {
                  Ecore_X_Window root;

                  root = ecore_x_window_root_get(parent);
                  for (i = 0; i < num; i++)
                    {
                       if (root == roots[i])
                         {
                            screen = ecore_x_screen_get(i);
                            break;
                         }
                    }
                  free(roots);
               }
          }

        einfo->info.destination_alpha = argb;

        if (redraw_debug < 0)
          {
             if (getenv("REDRAW_DEBUG"))
               redraw_debug = atoi(getenv("REDRAW_DEBUG"));
             else
               redraw_debug = 0;
          }

# ifdef BUILD_ECORE_EVAS_SOFTWARE_XCB
        einfo->info.backend = EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XCB;
        einfo->info.connection = ecore_x_connection_get();
        einfo->info.screen = screen;
# else
        einfo->info.backend = EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB;
        einfo->info.connection = ecore_x_display_get();
        einfo->info.screen = NULL;
# endif

        if ((argb) && (ee->prop.window))
          {
             Ecore_X_Window_Attributes at;

             ecore_x_window_attributes_get(ee->prop.window, &at);
             einfo->info.visual = at.visual;
             einfo->info.colormap = at.colormap;
             einfo->info.depth = at.depth;
             einfo->info.destination_alpha = 1;
          }
        else
          {
             einfo->info.visual =
               ecore_x_default_visual_get(einfo->info.connection, screen);
             einfo->info.colormap =
               ecore_x_default_colormap_get(einfo->info.connection, screen);
             einfo->info.depth =
               ecore_x_default_depth_get(einfo->info.connection, screen);
             einfo->info.destination_alpha = 0;
          }

        einfo->info.rotation = 0;
        einfo->info.debug = redraw_debug;

        /* record pixmap size to save X roundtrips */
        edata->pixmap.w = w;
        edata->pixmap.h = h;
        edata->pixmap.depth = einfo->info.depth;
        edata->pixmap.visual = einfo->info.visual;
        edata->pixmap.colormap = einfo->info.colormap;

        /* create front and back pixmaps for double-buffer rendering */
        edata->pixmap.front = 
          ecore_x_pixmap_new(parent, w, h, edata->pixmap.depth);
        edata->pixmap.back = 
          ecore_x_pixmap_new(parent, w, h, edata->pixmap.depth);

        einfo->info.drawable = edata->pixmap.back;

        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }

   /* FIXME: Allow of these set properties or do something with the 
    * ee->prop.window (x window), which we do not have in pixmap case */

   /* _ecore_evas_x_hints_update(ee); */
   /* _ecore_evas_x_group_leader_set(ee); */
   /* ecore_x_window_defaults_set(ee->prop.window); */
   /* _ecore_evas_x_protocols_set(ee); */
   /* _ecore_evas_x_window_profile_protocol_set(ee); */
   /* _ecore_evas_x_sync_set(ee); */

   ee->engine.func->fn_render = _ecore_evas_x_render;
   _ecore_evas_register(ee);

   ee->draw_ok = 1;

   /* ecore_x_input_multi_select(ee->prop.window); */
   /* ecore_event_window_register(ee->prop.window, ee, ee->evas, */
   /*                             (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process, */
   /*                             (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process, */
   /*                             (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process, */
   /*                             (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process); */

   return ee;
}

static Ecore_X_Window
_ecore_evas_software_x11_window_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "software_x11"))) return 0;
   return (Ecore_X_Window) ecore_evas_window_get(ee);
}

static Ecore_X_Pixmap
_ecore_evas_software_x11_pixmap_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "software_x11"))) return 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   return (Ecore_X_Pixmap) edata->pixmap.front;
}

static void *
_ecore_evas_software_x11_pixmap_visual_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "software_x11"))) return 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   return edata->pixmap.visual;
}

static unsigned long 
_ecore_evas_software_x11_pixmap_colormap_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "software_x11"))) return 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   return edata->pixmap.colormap;
}

static int 
_ecore_evas_software_x11_pixmap_depth_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "software_x11"))) return 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   return edata->pixmap.depth;
}

static void
_ecore_evas_software_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   edata->direct_resize = on;
   if (ee->prop.avoid_damage)
     {
        if (edata->direct_resize)
          {
/* turn this off for now
             ee->engine.x.using_bg_pixmap = 1;
             ecore_x_window_pixmap_set(ee->prop.window, ee->engine.x.pmap);
 */
          }
        else
          {
/* turn this off too- bg pixmap is controlled by avoid damage directly
             ee->engine.x.using_bg_pixmap = 0;
             ecore_x_window_pixmap_set(ee->prop.window, 0);
             ecore_x_window_area_expose(ee->prop.window, 0, 0, ee->w, ee->h);
 */
          }
     }
}

static Eina_Bool
_ecore_evas_software_x11_direct_resize_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   return edata->direct_resize;
}

static void
_ecore_evas_software_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   Ecore_X_Window *winp;

   winp = malloc(sizeof(Ecore_X_Window));
   if (winp)
     {
        *winp = win;
        edata->win_extra = eina_list_append(edata->win_extra, winp);
        ecore_x_input_multi_select(win);
        ecore_event_window_register(win, ee, ee->evas,
                                    (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                                    (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                                    (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                                    (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
     }
}
#endif

#ifdef BUILD_ECORE_EVAS_OPENGL_X11
EAPI Ecore_Evas *
ecore_evas_gl_x11_options_new_internal(const char *disp_name, Ecore_X_Window parent,
				       int x, int y, int w, int h, const int *opt)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Data_X11 *edata;
   Ecore_Evas_Interface_X11 *iface;
   Ecore_Evas_Interface_Gl_X11 *giface;
   int rmethod;
   char *id = NULL;

   rmethod = evas_render_method_lookup("gl_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   edata = calloc(1, sizeof(Ecore_Evas_Engine_Data_X11));
   if (!edata)
     {
	free(ee);
	return NULL;
     }

   iface = _ecore_evas_x_interface_x11_new();
   giface = _ecore_evas_x_interface_gl_x11_new();

   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, giface);

   ee->engine.data = edata;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->gl_sync_draw_done = -1;

   _ecore_evas_x_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;

   ee->driver = "opengl_x11";
#if 1
   ee->semi_sync = 0; // gl engine doesn't need to sync - its whole swaps
#else
   if (!getenv("ECORE_EVAS_COMP_NOSEMISYNC"))
      ee->semi_sync = 1; // gl engine doesn't need to sync - its whole swaps
//   ee->no_comp_sync = 1; // gl engine doesn't need to sync - its whole swaps
#endif
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->x = x;
   ee->y = y;
   ee->w = w;
   ee->h = h;
   ee->req.x = ee->x;
   ee->req.y = ee->y;
   ee->req.w = ee->w;
   ee->req.h = ee->h;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = 0;
   edata->state.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE, _ecore_evas_x_flush_pre, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST, _ecore_evas_x_flush_post, ee);
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   if (parent == 0) parent = ecore_x_window_root_first_get();
   edata->win_root = parent;

   if (edata->win_root != 0)
     {
        edata->screen_num = 1; /* FIXME: get real scren # */
       /* FIXME: round trip in ecore_x_window_argb_get */
        if (ecore_x_window_argb_get(edata->win_root))
          {
             ee->prop.window = _ecore_evas_x_gl_window_new
               (ee, edata->win_root, x, y, w, h, 0, 1, opt);
          }
        else
          ee->prop.window = _ecore_evas_x_gl_window_new
          (ee, edata->win_root, x, y, w, h, 0, 0, opt);
     }
   else
     ee->prop.window = _ecore_evas_x_gl_window_new
     (ee, edata->win_root, x, y, w, h, 0, 0, opt);
   if (!ee->prop.window)
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }
   if ((id = getenv("DESKTOP_STARTUP_ID")))
     {
        ecore_x_netwm_startup_id_set(ee->prop.window, id);
        /* NB: on linux this may simply empty the env as opposed to completely
         * unset it to being empty - unsure as solartis libc crashes looking
         * for the '=' char */
//        putenv((char*)"DESKTOP_STARTUP_ID=");
     }

   _ecore_evas_x_hints_update(ee);
   _ecore_evas_x_group_leader_set(ee);
   ecore_x_window_defaults_set(ee->prop.window);
   _ecore_evas_x_protocols_set(ee);
   _ecore_evas_x_window_profile_protocol_set(ee);
   _ecore_evas_x_wm_rotation_protocol_set(ee);
   _ecore_evas_x_aux_hints_supprted_update(ee);
   _ecore_evas_x_aux_hints_update(ee);
   _ecore_evas_x_sync_set(ee);

   ee->engine.func->fn_render = _ecore_evas_x_render;
   _ecore_evas_register(ee);
   ecore_x_input_multi_select(ee->prop.window);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);

   return ee;
}

EAPI Ecore_Evas *
ecore_evas_gl_x11_new_internal(const char *disp_name, Ecore_X_Window parent,
                      int x, int y, int w, int h)
{
   return ecore_evas_gl_x11_options_new_internal(disp_name, parent, x, y, w, h, NULL);
}

EAPI Ecore_Evas *
ecore_evas_gl_x11_pixmap_new_internal(const char *disp_name, Ecore_X_Window parent,
                                      int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Data_X11 *edata;
   Ecore_Evas_Interface_X11 *iface;
   Ecore_Evas_Interface_Gl_X11 *giface;
   Evas_Engine_Info_GL_X11 *einfo;
   int rmethod, argb = 0;
   static int redraw_debug = -1;

   rmethod = evas_render_method_lookup("gl_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   edata = calloc(1, sizeof(Ecore_Evas_Engine_Data_X11));
   if (!edata)
     {
	free(ee);
	return NULL;
     }

   iface = _ecore_evas_x_interface_x11_new();
   giface = _ecore_evas_x_interface_gl_x11_new();

   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, giface);

   ee->engine.data = edata;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->gl_sync_draw_done = -1;

   _ecore_evas_x_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;

   ee->driver = "opengl_x11";
#if 1
   ee->semi_sync = 0; // gl engine doesn't need to sync - its whole swaps
#else
   if (!getenv("ECORE_EVAS_COMP_NOSEMISYNC"))
      ee->semi_sync = 1; // gl engine doesn't need to sync - its whole swaps
//   ee->no_comp_sync = 1; // gl engine doesn't need to sync - its whole swaps
#endif
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->x = x;
   ee->y = y;
   ee->w = w;
   ee->h = h;
   ee->req.x = ee->x;
   ee->req.y = ee->y;
   ee->req.w = ee->w;
   ee->req.h = ee->h;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = 0;
   edata->state.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE, 
                           _ecore_evas_x_flush_pre, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST, 
                           _ecore_evas_x_flush_post, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_PRE, 
                           _ecore_evas_x_render_pre, ee);
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   if (ee->can_async_render)
     evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_POST,
			     _ecore_evas_x_render_updates, ee);

   if (parent == 0) parent = ecore_x_window_root_first_get();
   edata->win_root = parent;

   if (parent != 0)
     {
        edata->screen_num = 1; /* FIXME: get real scren # */
       /* FIXME: round trip in ecore_x_window_argb_get */
        if (ecore_x_window_argb_get(parent))
          argb = 1;
     }

   edata->direct_resize = 1;

   einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        int screen = 0;

        /* FIXME: this is inefficient as its 1 or more round trips */
        screen = ecore_x_screen_index_get(ecore_x_default_screen_get());
        if (ecore_x_screen_count_get() > 1)
          {
             Ecore_X_Window *roots;
             int num, i;

             num = 0;
             roots = ecore_x_window_root_list(&num);
             if (roots)
               {
                  Ecore_X_Window root;

                  root = ecore_x_window_root_get(parent);
                  for (i = 0; i < num; i++)
                    {
                       if (root == roots[i])
                         {
                            screen = i;
                            break;
                         }
                    }
                  free(roots);
               }
          }

        einfo->info.display = ecore_x_display_get();
        einfo->info.screen = screen;

        einfo->info.destination_alpha = argb;

        einfo->info.visual = einfo->func.best_visual_get(einfo);
        einfo->info.colormap = einfo->func.best_colormap_get(einfo);
        einfo->info.depth = einfo->func.best_depth_get(einfo);

        if (redraw_debug < 0)
          {
             if (getenv("REDRAW_DEBUG"))
               redraw_debug = atoi(getenv("REDRAW_DEBUG"));
             else
               redraw_debug = 0;
          }

        einfo->info.rotation = 0;

        /* record pixmap size to save X roundtrips */
        edata->pixmap.w = w;
        edata->pixmap.h = h;
        edata->pixmap.depth = einfo->info.depth;
        edata->pixmap.visual = einfo->info.visual;
        edata->pixmap.colormap = einfo->info.colormap;

        /* create front and back pixmaps for double-buffer rendering */
        edata->pixmap.front = 
          ecore_x_pixmap_new(parent, w, h, edata->pixmap.depth);
        edata->pixmap.back = 
          ecore_x_pixmap_new(parent, w, h, edata->pixmap.depth);

        einfo->info.drawable = edata->pixmap.back;

        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }

   /* _ecore_evas_x_hints_update(ee); */
   /* _ecore_evas_x_group_leader_set(ee); */
   /* ecore_x_window_defaults_set(ee->prop.window); */
   /* _ecore_evas_x_protocols_set(ee); */
   /* _ecore_evas_x_window_profile_protocol_set(ee); */
   /* _ecore_evas_x_sync_set(ee); */

   ee->engine.func->fn_render = _ecore_evas_x_render;
   _ecore_evas_register(ee);

   /* ecore_x_input_multi_select(ee->prop.window); */
   /* ecore_event_window_register(ee->prop.window, ee, ee->evas, */
   /*                             (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process, */
   /*                             (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process, */
   /*                             (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process, */
   /*                             (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process); */

   return ee;
}

static Ecore_X_Window
_ecore_evas_gl_x11_window_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "opengl_x11"))) return 0;
   return (Ecore_X_Window) ecore_evas_window_get(ee);
}

static Ecore_X_Pixmap
_ecore_evas_gl_x11_pixmap_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "opengl_x11"))) return 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   return (Ecore_X_Pixmap) edata->pixmap.front;
}

static void *
_ecore_evas_gl_x11_pixmap_visual_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "opengl_x11"))) return 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   return edata->pixmap.visual;
}

static unsigned long 
_ecore_evas_gl_x11_pixmap_colormap_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "opengl_x11"))) return 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   return edata->pixmap.colormap;
}

static int 
_ecore_evas_gl_x11_pixmap_depth_get(const Ecore_Evas *ee)
{
   if (!(!strcmp(ee->driver, "opengl_x11"))) return 0;
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   return edata->pixmap.depth;
}

static void
_ecore_evas_gl_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;
   edata->direct_resize = on;
}

static Eina_Bool
_ecore_evas_gl_x11_direct_resize_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   return edata->direct_resize;
}

static void
_ecore_evas_gl_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
   ecore_evas_software_x11_extra_event_window_add(ee, win);
}

static void
_ecore_evas_gl_x11_pre_post_swap_callback_set(const Ecore_Evas *ee, void *data, void (*pre_cb) (void *data, Evas *e), void (*post_cb) (void *data, Evas *e))
{
   Evas_Engine_Info_GL_X11 *einfo;

   if (!(!strcmp(ee->driver, "opengl_x11"))) return;

   einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->callback.pre_swap = pre_cb;
        einfo->callback.post_swap = post_cb;
        einfo->callback.data = data;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
     }
}
#endif /* ! BUILD_ECORE_EVAS_OPENGL_X11 */

static void
_ecore_evas_x11_leader_set(Ecore_Evas *ee, Ecore_X_Window win)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   _ecore_evas_x_group_leader_unset(ee);
   edata->leader = win;
   _ecore_evas_x_group_leader_update(ee);
}

static Ecore_X_Window
_ecore_evas_x11_leader_get(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata = ee->engine.data;

   return edata->leader;
}

static void
_ecore_evas_x11_leader_default_set(Ecore_Evas *ee)
{
   _ecore_evas_x_group_leader_unset(ee);
   _ecore_evas_x_group_leader_set(ee);
}

static Eina_Bool
_ecore_evas_x11_convert_rectangle_with_angle(Ecore_Evas *ee, Ecore_X_Rectangle *dst_rect, Ecore_X_Rectangle *src_rect)
{
   if ((!src_rect) || (!dst_rect)) return 0;

   if (ee->rotation == 0)
     {
        dst_rect->x = src_rect->x;
        dst_rect->y = src_rect->y;
        dst_rect->width = src_rect->width;
        dst_rect->height = src_rect->height;
     }
   else if (ee->rotation == 90)
     {
        dst_rect->x = src_rect->y;
        dst_rect->y = ee->req.h - src_rect->x - src_rect->width;
        dst_rect->width = src_rect->height;
        dst_rect->height = src_rect->width;
     }
   else if (ee->rotation == 180)
     {
        dst_rect->x = ee->req.w - src_rect->x - src_rect->width;
        dst_rect->y = ee->req.h - src_rect->y - src_rect->height;
        dst_rect->width = src_rect->width;
        dst_rect->height = src_rect->height;
     }
   else if (ee->rotation == 270)
     {
        dst_rect->x = ee->req.w - src_rect->y - src_rect->height;
        dst_rect->y = src_rect->x;
        dst_rect->width = src_rect->height;
        dst_rect->height = src_rect->width;
     }
   else
     {
        return 0;
     }

   return 1;
}

static void
_ecore_evas_x11_shape_input_rectangle_set(Ecore_Evas *ee, int x, int y, int w, int h)
{
   Eina_Bool ret;
   Ecore_X_Rectangle src_rect;
   Ecore_X_Rectangle dst_rect;
   Ecore_Evas_Engine_Data_X11 *edata;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_rectangle_set");
        return;
     }

   edata = ee->engine.data;
   src_rect.x = x;
   src_rect.y = y;
   src_rect.width = w;
   src_rect.height = h;

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = 0;
   dst_rect.height = 0;

   ret = _ecore_evas_x11_convert_rectangle_with_angle(ee, &dst_rect, &src_rect);

   if (!edata->win_shaped_input)
      edata->win_shaped_input = ecore_x_window_override_new(edata->win_root,
                                                                  0, 0, 1, 1);

   if (ret)
      ecore_x_window_shape_input_rectangle_set(edata->win_shaped_input,
                                               dst_rect.x, dst_rect.y,
                                               dst_rect.width, dst_rect.height);
}

static void
_ecore_evas_x11_shape_input_rectangle_add(Ecore_Evas *ee, int x, int y, int w, int h)
{
   Eina_Bool ret;
   Ecore_X_Rectangle src_rect;
   Ecore_X_Rectangle dst_rect;
   Ecore_Evas_Engine_Data_X11 *edata;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_rectangle_add");
        return;
     }

   edata = ee->engine.data;
   src_rect.x = x;
   src_rect.y = y;
   src_rect.width = w;
   src_rect.height = h;

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = 0;
   dst_rect.height = 0;

   ret = _ecore_evas_x11_convert_rectangle_with_angle(ee, &dst_rect, &src_rect);

   if (!edata->win_shaped_input)
      edata->win_shaped_input = ecore_x_window_override_new(edata->win_root,
                                                                  0, 0, 1, 1);

   if (ret)
      ecore_x_window_shape_input_rectangle_add(edata->win_shaped_input,
                                               dst_rect.x, dst_rect.y,
                                               dst_rect.width, dst_rect.height);
}

static void
_ecore_evas_x11_shape_input_rectangle_subtract(Ecore_Evas *ee, int x, int y, int w, int h)
{
   Eina_Bool ret;
   Ecore_X_Rectangle src_rect;
   Ecore_X_Rectangle dst_rect;
   Ecore_Evas_Engine_Data_X11 *edata;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_rectangle_subtract");
        return;
     }

   edata = ee->engine.data;
   src_rect.x = x;
   src_rect.y = y;
   src_rect.width = w;
   src_rect.height = h;

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = 0;
   dst_rect.height = 0;

   ret = _ecore_evas_x11_convert_rectangle_with_angle(ee, &dst_rect, &src_rect);

   if (!edata->win_shaped_input)
      edata->win_shaped_input = ecore_x_window_override_new(edata->win_root,
                                                                  0, 0, 1, 1);

   if (ret)
      ecore_x_window_shape_input_rectangle_subtract(edata->win_shaped_input,
                                                    dst_rect.x, dst_rect.y,
                                                    dst_rect.width, dst_rect.height);
}

static void
_ecore_evas_x11_shape_input_empty(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata;
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_empty");
        return;
     }

   edata = ee->engine.data;
   if (!edata->win_shaped_input)
      edata->win_shaped_input = ecore_x_window_override_new(edata->win_root, 0, 0, 1, 1);

   ecore_x_window_shape_input_rectangle_set(edata->win_shaped_input, 0, 0, 0, 0);
}

static void
_ecore_evas_x11_shape_input_reset(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata;
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_reset");
        return;
     }

   edata = ee->engine.data;
   if (!edata->win_shaped_input)
      edata->win_shaped_input = ecore_x_window_override_new(edata->win_root, 0, 0, 1, 1);

   ecore_x_window_shape_input_rectangle_set(edata->win_shaped_input, 0, 0, 65535, 65535);
}

static void
_ecore_evas_x11_shape_input_apply(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Data_X11 *edata;
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_apply");
        return;
     }

   edata = ee->engine.data;
   if (!edata->win_shaped_input) return;

   ecore_x_window_shape_input_window_set(ee->prop.window, edata->win_shaped_input);
}

static Ecore_Evas_Interface_X11 *
_ecore_evas_x_interface_x11_new(void)
{
   Ecore_Evas_Interface_X11 *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_X11));
   if (!iface) return NULL;

   iface->base.name = interface_x11_name;
   iface->base.version = interface_x11_version;

   iface->leader_set = _ecore_evas_x11_leader_set;
   iface->leader_get = _ecore_evas_x11_leader_get;
   iface->leader_default_set = _ecore_evas_x11_leader_default_set;
   iface->shape_input_rectangle_set = _ecore_evas_x11_shape_input_rectangle_set;
   iface->shape_input_rectangle_add = _ecore_evas_x11_shape_input_rectangle_add;
   iface->shape_input_rectangle_subtract = _ecore_evas_x11_shape_input_rectangle_subtract;
   iface->shape_input_empty = _ecore_evas_x11_shape_input_empty;
   iface->shape_input_reset = _ecore_evas_x11_shape_input_reset;
   iface->shape_input_reset = _ecore_evas_x11_shape_input_apply;

   return iface;
}

#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
static Ecore_Evas_Interface_Software_X11 *
_ecore_evas_x_interface_software_x11_new(void)
{
   Ecore_Evas_Interface_Software_X11 *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Software_X11));
   if (!iface) return NULL;

   iface->base.name = interface_software_x11_name;
   iface->base.version = interface_software_x11_version;

   iface->window_get = _ecore_evas_software_x11_window_get;
   iface->pixmap_get = _ecore_evas_software_x11_pixmap_get;
   iface->resize_set = _ecore_evas_software_x11_direct_resize_set;
   iface->resize_get = _ecore_evas_software_x11_direct_resize_get;
   iface->extra_event_window_add = _ecore_evas_software_x11_extra_event_window_add;
   iface->pixmap_visual_get = _ecore_evas_software_x11_pixmap_visual_get;
   iface->pixmap_colormap_get = _ecore_evas_software_x11_pixmap_colormap_get;
   iface->pixmap_depth_get = _ecore_evas_software_x11_pixmap_depth_get;

   return iface;
}
#endif

#ifdef BUILD_ECORE_EVAS_OPENGL_X11
static Ecore_Evas_Interface_Gl_X11 *
_ecore_evas_x_interface_gl_x11_new(void)
{
   Ecore_Evas_Interface_Gl_X11 *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Gl_X11));
   if (!iface) return NULL;

   iface->base.name = interface_gl_x11_name;
   iface->base.version = interface_gl_x11_version;

   iface->window_get = _ecore_evas_gl_x11_window_get;
   iface->pixmap_get = _ecore_evas_gl_x11_pixmap_get;
   iface->resize_set = _ecore_evas_gl_x11_direct_resize_set;
   iface->resize_get = _ecore_evas_gl_x11_direct_resize_get;
   iface->extra_event_window_add = _ecore_evas_gl_x11_extra_event_window_add;
   iface->pre_post_swap_callback_set = _ecore_evas_gl_x11_pre_post_swap_callback_set;
   iface->pixmap_visual_get = _ecore_evas_gl_x11_pixmap_visual_get;
   iface->pixmap_colormap_get = _ecore_evas_gl_x11_pixmap_colormap_get;
   iface->pixmap_depth_get = _ecore_evas_gl_x11_pixmap_depth_get;

   return iface;
}
#endif
