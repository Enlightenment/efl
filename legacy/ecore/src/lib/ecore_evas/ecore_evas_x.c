#include <string.h>
#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_X11
static int _ecore_evas_init_count = 0;

static Ecore_Event_Handler *ecore_evas_event_handlers[13];

static int leader_ref = 0;
static Ecore_X_Window leader_win = 0;

static void
_ecore_evas_x_group_leader_set(Ecore_Evas *ee)
{
   leader_ref++;
   if (leader_ref == 1)
     {
        char *id = NULL;

        leader_win =
          ecore_x_window_override_new(ee->engine.x.win_root, 1234, 5678, 1, 2);
        ecore_x_window_defaults_set(leader_win);
        if ((id = getenv("DESKTOP_STARTUP_ID")))
          ecore_x_netwm_startup_id_set(leader_win,id);
        ecore_x_icccm_client_leader_set(leader_win, leader_win);
     }
   ee->engine.x.leader = leader_win;
   ecore_x_icccm_client_leader_set(ee->prop.window, leader_win);
}

static void
_ecore_evas_x_group_leader_unset(Ecore_Evas *ee)
{
   ecore_x_window_prop_property_del(ee->prop.window,
                                    ECORE_X_ATOM_WM_CLIENT_LEADER);
   if (ee->engine.x.leader == leader_win)
     {
        leader_ref--;
        if (leader_ref <= 0)
          {
             ecore_x_window_free(leader_win);
             leader_win = 0;
          }
        ee->engine.x.leader = 0;
     }
}

static void
_ecore_evas_x_group_leader_update(Ecore_Evas *ee)
{
   if (ee->engine.x.leader)
      ecore_x_icccm_client_leader_set(ee->prop.window, ee->engine.x.leader);
}

static void
_ecore_evas_x_protocols_set(Ecore_Evas *ee)
{
   Ecore_X_Atom protos[3];
   unsigned int num = 0, tmp = 0;

   if (ee->func.fn_delete_request)
     protos[num++] = ECORE_X_ATOM_WM_DELETE_WINDOW;
   protos[num++] = ECORE_X_ATOM_NET_WM_PING;
   protos[num++] = ECORE_X_ATOM_NET_WM_SYNC_REQUEST;
   ecore_x_icccm_protocol_atoms_set(ee->prop.window, protos, num);

   if (!ee->engine.x.netwm_sync_counter)
     ee->engine.x.netwm_sync_counter = ecore_x_sync_counter_new(0);

   tmp = ee->engine.x.netwm_sync_counter;
   ecore_x_window_prop_card32_set(ee->prop.window,
                                  ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER,
                                  &tmp, 1);
}

static void
_ecore_evas_x_sync_set(Ecore_Evas *ee)
{
   if (((ee->should_be_visible) || (ee->visible)) &&
       ((ecore_x_e_comp_sync_supported_get(ee->engine.x.win_root)) &&
           (!ee->no_comp_sync) && (_ecore_evas_app_comp_sync)))
     {
        if (!ee->engine.x.sync_counter)
           ee->engine.x.sync_counter = ecore_x_sync_counter_new(0);
     }
   else
     {
        if (ee->engine.x.sync_counter)
           ecore_x_sync_counter_free(ee->engine.x.sync_counter);
        ee->engine.x.sync_counter = 0;
     }
   ecore_x_e_comp_sync_counter_set(ee->prop.window, ee->engine.x.sync_counter);
}

static void
_ecore_evas_x_sync_clear(Ecore_Evas *ee)
{
   if (!ee->engine.x.sync_counter) return;
   ecore_x_sync_counter_free(ee->engine.x.sync_counter);
   ee->engine.x.sync_counter = 0;
}

# ifdef BUILD_ECORE_EVAS_OPENGL_X11
static Ecore_X_Window
_ecore_evas_x_gl_window_new(Ecore_Evas *ee, Ecore_X_Window parent, int x, int y, int w, int h, int override, int argb, const int *opt)
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
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
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
_ecore_evas_x_render(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *updates = NULL;
   Eina_List *ll;
   Ecore_Evas *ee2;

   if ((!ee->no_comp_sync) && (_ecore_evas_app_comp_sync) &&
       (ee->engine.x.sync_counter) && (!ee->engine.x.sync_began) &&
       (!ee->engine.x.sync_cancel))
     return 0;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);
   updates = evas_render_updates(ee->evas);
   if (ee->prop.avoid_damage)
     {
        if (ee->engine.x.using_bg_pixmap)
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
#ifdef EVAS_FRAME_QUEUING
                       evas_sync(ee->evas);
#endif
                       ecore_x_window_shape_mask_set(ee->prop.window,
                                                     ee->engine.x.mask);
                    }
                  if (ee->alpha)
                    {
#ifdef EVAS_FRAME_QUEUING
                       /* wait until ee->engine.x.mask being updated */
//                     evas_sync(ee->evas);
#endif
//                     ecore_x_window_shape_input_mask_set(ee->prop.window, ee->engine.x.mask);
                    }
                  evas_render_updates_free(updates);
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

                       if (!ee->engine.x.damages)
                         ee->engine.x.damages = ecore_x_xregion_new();
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
                       ecore_x_xregion_union_rect(tmpr, ee->engine.x.damages,
                                                  &rect);
                       ecore_x_xregion_free(ee->engine.x.damages);
                       ee->engine.x.damages = tmpr;
                    }
                  if (ee->engine.x.damages)
                    {
                       /* if we have a damage pixmap - we can avoid exposures by
                        * disabling them just for setting the mask */
                       ecore_x_event_mask_set(ee->prop.window,
                                              ECORE_X_EVENT_MASK_KEY_DOWN |
                                              ECORE_X_EVENT_MASK_KEY_UP |
                                              ECORE_X_EVENT_MASK_MOUSE_DOWN |
                                              ECORE_X_EVENT_MASK_MOUSE_UP |
                                              ECORE_X_EVENT_MASK_MOUSE_IN |
                                              ECORE_X_EVENT_MASK_MOUSE_OUT |
                                              ECORE_X_EVENT_MASK_MOUSE_MOVE |
                                              //                                         ECORE_X_EVENT_MASK_WINDOW_DAMAGE |
                                              ECORE_X_EVENT_MASK_WINDOW_VISIBILITY |
                                              ECORE_X_EVENT_MASK_WINDOW_CONFIGURE |
                                              ECORE_X_EVENT_MASK_WINDOW_FOCUS_CHANGE |
                                              ECORE_X_EVENT_MASK_WINDOW_PROPERTY |
                                              ECORE_X_EVENT_MASK_WINDOW_COLORMAP
                                             );
                       if (ee->shaped)
                         ecore_x_window_shape_mask_set(ee->prop.window,
                                                       ee->engine.x.mask);
                       /* and re-enable them again */
                       ecore_x_event_mask_set(ee->prop.window,
                                              ECORE_X_EVENT_MASK_KEY_DOWN |
                                              ECORE_X_EVENT_MASK_KEY_UP |
                                              ECORE_X_EVENT_MASK_MOUSE_DOWN |
                                              ECORE_X_EVENT_MASK_MOUSE_UP |
                                              ECORE_X_EVENT_MASK_MOUSE_IN |
                                              ECORE_X_EVENT_MASK_MOUSE_OUT |
                                              ECORE_X_EVENT_MASK_MOUSE_MOVE |
                                              ECORE_X_EVENT_MASK_WINDOW_DAMAGE |
                                              ECORE_X_EVENT_MASK_WINDOW_VISIBILITY |
                                              ECORE_X_EVENT_MASK_WINDOW_CONFIGURE |
                                              ECORE_X_EVENT_MASK_WINDOW_FOCUS_CHANGE |
                                              ECORE_X_EVENT_MASK_WINDOW_PROPERTY |
                                              ECORE_X_EVENT_MASK_WINDOW_COLORMAP
                                             );
                       ecore_x_xregion_set(ee->engine.x.damages, ee->engine.x.gc);
                       ecore_x_pixmap_paste(ee->engine.x.pmap, ee->prop.window,
                                            ee->engine.x.gc, 0, 0, ee->w, ee->h,
                                            0, 0);
                       ecore_x_xregion_free(ee->engine.x.damages);
                       ee->engine.x.damages = NULL;
                    }
                  evas_render_updates_free(updates);
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
#ifdef EVAS_FRAME_QUEUING
                  evas_sync(ee->evas);
#endif
                  ecore_x_window_shape_mask_set(ee->prop.window,
                                                ee->engine.x.mask);
               }
             if (ee->alpha)
               {
#ifdef EVAS_FRAME_QUEUING
                  /* wait until ee->engine.x.mask being updated */
//                evas_sync(ee->evas);
#endif
//                ecore_x_window_shape_input_mask_set(ee->prop.window, ee->engine.x.mask);
               }
             evas_render_updates_free(updates);
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
_ecore_evas_x_resize_shape(Ecore_Evas *ee)
{
   if (!strcmp(ee->driver, "software_x11"))
     {
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
        Evas_Engine_Info_Software_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             unsigned int foreground;
             Ecore_X_GC gc;

             if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
             ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, 1);
             foreground = 0;
             gc = ecore_x_gc_new(ee->engine.x.mask,
                                 ECORE_X_GC_VALUE_MASK_FOREGROUND,
                                 &foreground);
             ecore_x_drawable_rectangle_fill(ee->engine.x.mask, gc,
                                             0, 0, ee->w, ee->h);
             ecore_x_gc_free(gc);
             einfo->info.mask = ee->engine.x.mask;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
             evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_X11 */
     }
   else if (!strcmp(ee->driver, "software_16_x11"))
     {
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
# if 0 /* XXX no shaped window support for software_16_x11 */
        Evas_Engine_Info_Software_16_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_16_X11 *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
             ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, 1);
             einfo->info.mask = ee->engine.x.mask;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
             evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
# endif /* XXX no shaped window support for software_16_x11 */
#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_X11 */
     }
   if (!strcmp(ee->driver, "software_8_x11"))
     {
#if defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
        Evas_Engine_Info_Software_8_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_8_X11 *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             unsigned int foreground;
             Ecore_X_GC gc;

             if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
             ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, 1);
             foreground = 0;
             gc = ecore_x_gc_new(ee->engine.x.mask,
                                 ECORE_X_GC_VALUE_MASK_FOREGROUND,
                                 &foreground);
             ecore_x_drawable_rectangle_fill(ee->engine.x.mask, gc,
                                             0, 0, ee->w, ee->h);
             ecore_x_gc_free(gc);
             einfo->info.mask = ee->engine.x.mask;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
             evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_8_X11 */
     }
}

/* TODO: we need to make this work for all the states, not just sticky */
static Eina_Bool
_ecore_evas_x_event_property_change(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Property *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (e->atom == ECORE_X_ATOM_NET_WM_STATE)
     {
        unsigned int i, num;
        Ecore_X_Window_State *state;
        int sticky = 0;

        /* TODO: we need to move those to the end, with if statements */
        ee->engine.x.state.modal = 0;
        ee->engine.x.state.maximized_v = 0;
        ee->engine.x.state.maximized_h = 0;
        ee->engine.x.state.shaded = 0;
        ee->engine.x.state.skip_taskbar = 0;
        ee->engine.x.state.skip_pager = 0;
        ee->prop.fullscreen = 0;
        ee->engine.x.state.fullscreen = 0;
        ee->engine.x.state.above = 0;
        ee->engine.x.state.below = 0;

        ecore_x_netwm_window_state_get(e->win, &state, &num);
        if (state)
          {
             for (i = 0; i < num; i++)
               {
                  switch (state[i])
                    {
                     case ECORE_X_WINDOW_STATE_MODAL:
                        ee->engine.x.state.modal = 1;
                        break;
                     case ECORE_X_WINDOW_STATE_STICKY:
                        if (ee->prop.sticky && ee->engine.x.state.sticky)
                          break;

                        sticky = 1;
                        ee->prop.sticky = 1;
                        ee->engine.x.state.sticky = 1;
                        if (ee->func.fn_sticky) ee->func.fn_sticky(ee);
                        break;
                     case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
                        ee->engine.x.state.maximized_v = 1;
                        break;
                     case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
                        ee->engine.x.state.maximized_h = 1;
                        break;
                     case ECORE_X_WINDOW_STATE_SHADED:
                        ee->engine.x.state.shaded = 1;
                        break;
                     case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
                        ee->engine.x.state.skip_taskbar = 1;
                        break;
                     case ECORE_X_WINDOW_STATE_SKIP_PAGER:
                        ee->engine.x.state.skip_pager = 1;
                        break;
                     case ECORE_X_WINDOW_STATE_FULLSCREEN:
                        ee->prop.fullscreen = 1;
                        ee->engine.x.state.fullscreen = 1;
                        break;
                     case ECORE_X_WINDOW_STATE_ABOVE:
                        ee->engine.x.state.above = 1;
                        break;
                     case ECORE_X_WINDOW_STATE_BELOW:
                        ee->engine.x.state.below = 1;
                        break;
                     default:
                        break;
                    }
               }
             free(state);
          }

        if ((ee->prop.sticky) && (!sticky))
          {
             ee->prop.sticky = 0;
             ee->engine.x.state.sticky = 0;
             if (ee->func.fn_unsticky) ee->func.fn_unsticky(ee);
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_visibility_change(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Visibility_Change *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
//   printf("VIS CHANGE OBSCURED: %p %i\n", ee, e->fully_obscured);
   if (e->fully_obscured)
     {
        /* FIXME: round trip */
        if (!ecore_x_screen_is_composited(ee->engine.x.screen_num))
          ee->draw_ok = 0;
     }
   else
     ee->draw_ok = 1;
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_client_message(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Client_Message *e;

   e = event;
   if (e->format != 32) return ECORE_CALLBACK_PASS_ON;
   if (e->message_type == ECORE_X_ATOM_E_COMP_SYNC_BEGIN)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        if (e->data.l[0] != (long)ee->prop.window)
          return ECORE_CALLBACK_PASS_ON;
        if (!ee->engine.x.sync_began)
          {
             // qeue a damage + draw. work around an event re-ordering thing.
             evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
        ee->engine.x.sync_began = 1;
        ee->engine.x.sync_cancel = 0;
     }
   else if (e->message_type == ECORE_X_ATOM_E_COMP_SYNC_END)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        if (e->data.l[0] != (long)ee->prop.window)
          return ECORE_CALLBACK_PASS_ON;
        ee->engine.x.sync_began = 0;
        ee->engine.x.sync_cancel = 0;
     }
   else if (e->message_type == ECORE_X_ATOM_E_COMP_SYNC_CANCEL)
     {
        ee = ecore_event_window_match(e->data.l[0]);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        if (e->data.l[0] != (long)ee->prop.window)
          return ECORE_CALLBACK_PASS_ON;
        ee->engine.x.sync_began = 0;
        ee->engine.x.sync_cancel = 1;
     }
   else if ((e->message_type == ECORE_X_ATOM_WM_PROTOCOLS) &&
            (e->data.l[0] == (int)ECORE_X_ATOM_NET_WM_SYNC_REQUEST))
     {
        ee = ecore_event_window_match(e->win);
        if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
        ee->engine.x.netwm_sync_val_lo = (unsigned int)e->data.l[2];
        ee->engine.x.netwm_sync_val_hi = (int)e->data.l[3];
        ee->engine.x.netwm_sync_set = 1;
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_In *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
/*    { */
/*       time_t t; */
/*       char *ct; */

/*       const char *modes[] = { */
/*        "MODE_NORMAL", */
/*        "MODE_WHILE_GRABBED", */
/*        "MODE_GRAB", */
/*        "MODE_UNGRAB" */
/*       }; */
/*       const char *details[] = { */
/*        "DETAIL_ANCESTOR", */
/*        "DETAIL_VIRTUAL", */
/*        "DETAIL_INFERIOR", */
/*        "DETAIL_NON_LINEAR", */
/*        "DETAIL_NON_LINEAR_VIRTUAL", */
/*        "DETAIL_POINTER", */
/*        "DETAIL_POINTER_ROOT", */
/*        "DETAIL_DETAIL_NONE" */
/*       }; */
/*       t = time(NULL); */
/*       ct = ctime(&t); */
/*       ct[strlen(ct) - 1] = 0; */
/*       printf("@@ ->IN 0x%x 0x%x %s md=%s dt=%s\n", */
/*             e->win, e->event_win, */
/*             ct, */
/*             modes[e->mode], */
/*             details[e->detail]); */
/*    } */
   // disable. causes more problems than it fixes
   //   if ((e->mode == ECORE_X_EVENT_MODE_GRAB) ||
   //       (e->mode == ECORE_X_EVENT_MODE_UNGRAB))
   //     return 0;
   /* if (e->mode != ECORE_X_EVENT_MODE_NORMAL) return 0; */
   if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
   ecore_event_evas_modifier_lock_update(ee->evas, e->modifiers);
   evas_event_feed_mouse_in(ee->evas, e->time, NULL);
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Mouse_Out *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
/*    { */
/*       time_t t; */
/*       char *ct; */

/*       const char *modes[] = { */
/*        "MODE_NORMAL", */
/*        "MODE_WHILE_GRABBED", */
/*        "MODE_GRAB", */
/*        "MODE_UNGRAB" */
/*       }; */
/*       const char *details[] = { */
/*        "DETAIL_ANCESTOR", */
/*        "DETAIL_VIRTUAL", */
/*        "DETAIL_INFERIOR", */
/*        "DETAIL_NON_LINEAR", */
/*        "DETAIL_NON_LINEAR_VIRTUAL", */
/*        "DETAIL_POINTER", */
/*        "DETAIL_POINTER_ROOT", */
/*        "DETAIL_DETAIL_NONE" */
/*       }; */
/*       t = time(NULL); */
/*       ct = ctime(&t); */
/*       ct[strlen(ct) - 1] = 0; */
/*       printf("@@ ->OUT 0x%x 0x%x %s md=%s dt=%s\n", */
/*             e->win, e->event_win, */
/*             ct, */
/*             modes[e->mode], */
/*             details[e->detail]); */
/*    } */
   // disable. causes more problems than it fixes
   //   if ((e->mode == ECORE_X_EVENT_MODE_GRAB) ||
   //       (e->mode == ECORE_X_EVENT_MODE_UNGRAB))
   //     return 0;
   /* if (e->mode != ECORE_X_EVENT_MODE_NORMAL) return 0; */
   ecore_event_evas_modifier_lock_update(ee->evas, e->modifiers);
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);
   if (e->mode == ECORE_X_EVENT_MODE_GRAB)
     evas_event_feed_mouse_cancel(ee->evas, e->time, NULL);
   evas_event_feed_mouse_out(ee->evas, e->time, NULL);
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_focus_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Focus_In *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (e->mode == ECORE_X_EVENT_MODE_UNGRAB) return ECORE_CALLBACK_PASS_ON;
   ee->prop.focused = 1;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_focus_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Focus_Out *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (e->mode == ECORE_X_EVENT_MODE_GRAB) return ECORE_CALLBACK_PASS_ON;
//   if (ee->prop.fullscreen)
//     ecore_x_window_focus(ee->prop.window);
   evas_focus_out(ee->evas);
   ee->prop.focused = 0;
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_damage(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Damage *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->engine.x.using_bg_pixmap) return ECORE_CALLBACK_PASS_ON;
//   printf("EXPOSE %p [%i] %i %i %ix%i\n", ee, ee->prop.avoid_damage, e->x, e->y, e->w, e->h);
   if (ee->prop.avoid_damage)
     {
        Ecore_X_Rectangle rect;
        Ecore_X_XRegion *tmpr;

        if (!ee->engine.x.damages)
          ee->engine.x.damages = ecore_x_xregion_new();
        tmpr = ecore_x_xregion_new();
        rect.x = e->x;
        rect.y = e->y;
        rect.width = e->w;
        rect.height = e->h;
        ecore_x_xregion_union_rect(tmpr, ee->engine.x.damages, &rect);
        ecore_x_xregion_free(ee->engine.x.damages);
        ee->engine.x.damages = tmpr;
/* no - this breaks things badly. disable. Ecore_X_Rectangle != XRectangle - see
 *  the typedefs in x's headers and ecore_x's. also same with Region - it's a pointer in x - not an X ID
        Ecore_X_Rectangle rect;
        Ecore_X_XRegion  *tmpr;

        if (!ee->engine.x.damages) ee->engine.x.damages = ecore_x_xregion_new();
        tmpr = ecore_x_xregion_new();
        rect.x = e->x;
        rect.y = e->y;
        rect.width = e->w;
        rect.height = e->h;
        ecore_x_xregion_union_rect(tmpr, ee->engine.x.damages, &rect);
        ecore_x_xregion_free(ee->engine.x.damages);
        ee->engine.x.damages = tmpr;
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
_ecore_evas_x_event_window_destroy(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Destroy *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->func.fn_destroy) ee->func.fn_destroy(ee);
   _ecore_evas_x_sync_clear(ee);
   ecore_evas_free(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_configure(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Configure *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->engine.x.direct_resize) return ECORE_CALLBACK_PASS_ON;

   ee->engine.x.configure_coming = 0;
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
   if ((ee->w != e->w) || (ee->h != e->h))
     {
        ee->w = e->w;
        ee->h = e->h;
        ee->req.w = ee->w;
        ee->req.h = ee->h;
        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
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
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_delete_request(void *data __UNUSED__, int type __UNUSED__, void *event)
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
_ecore_evas_x_event_window_show(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Show *e;
   static int first_map_bug = -1;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
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
   if (ee->visible) return ECORE_CALLBACK_DONE;
//   printf("SHOW EVENT %p\n", ee);
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_x_event_window_hide(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas *ee;
   Ecore_X_Event_Window_Hide *e;

   e = event;
   ee = ecore_event_window_match(e->win);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if (e->win != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (!ee->visible) return ECORE_CALLBACK_DONE;
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
                                    0 /* min_aspect */,
                                    0 /* max_aspect */);
}

/* FIXME, should be in idler */
static void
_ecore_evas_x_state_update(Ecore_Evas *ee)
{
   Ecore_X_Window_State state[10];
   int num = 0;

   /*
   if (bd->client.netwm.state.modal)
     state[num++] = ECORE_X_WINDOW_STATE_MODAL;
   */
   if (ee->engine.x.state.sticky)
     state[num++] = ECORE_X_WINDOW_STATE_STICKY;
   /*
   if (bd->client.netwm.state.maximized_v)
     state[num++] = ECORE_X_WINDOW_STATE_MAXIMIZED_VERT;
   if (bd->client.netwm.state.maximized_h)
     state[num++] = ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ;
   if (bd->client.netwm.state.shaded)
     state[num++] = ECORE_X_WINDOW_STATE_SHADED;
   if (bd->client.netwm.state.skip_taskbar)
     state[num++] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
   if (bd->client.netwm.state.skip_pager)
     state[num++] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
   if (bd->client.netwm.state.hidden)
     state[num++] = ECORE_X_WINDOW_STATE_HIDDEN;
   */
   if (ee->engine.x.state.fullscreen)
     state[num++] = ECORE_X_WINDOW_STATE_FULLSCREEN;
   if (ee->engine.x.state.above)
     state[num++] = ECORE_X_WINDOW_STATE_ABOVE;
   if (ee->engine.x.state.below)
     state[num++] = ECORE_X_WINDOW_STATE_BELOW;

   ecore_x_netwm_window_state_set(ee->prop.window, state, num);
}

static void
_ecore_evas_x_layer_update(Ecore_Evas *ee)
{
   if (ee->should_be_visible)
     {
        /* We need to send a netwm request to the wm */
        /* FIXME: Do we have to remove old state before adding new? */
        if (ee->prop.layer < 3)
          {
             if (ee->engine.x.state.above)
               {
                  ee->engine.x.state.above = 0;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   ee->engine.x.win_root,
                                                   ECORE_X_WINDOW_STATE_ABOVE, -1, 0);
               }
             if (!ee->engine.x.state.below)
               {
                  ee->engine.x.state.below = 1;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   ee->engine.x.win_root,
                                                   ECORE_X_WINDOW_STATE_BELOW, -1, 1);
               }
          }
        else if (ee->prop.layer > 5)
          {
             if (ee->engine.x.state.below)
               {
                  ee->engine.x.state.below = 0;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   ee->engine.x.win_root,
                                                   ECORE_X_WINDOW_STATE_BELOW, -1, 0);
               }
             if (!ee->engine.x.state.above)
               {
                  ee->engine.x.state.above = 1;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   ee->engine.x.win_root,
                                                   ECORE_X_WINDOW_STATE_ABOVE, -1, 1);
               }
          }
        else
          {
             if (ee->engine.x.state.below)
               {
                  ee->engine.x.state.below = 0;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   ee->engine.x.win_root,
                                                   ECORE_X_WINDOW_STATE_BELOW, -1, 0);
               }
             if (ee->engine.x.state.above)
               {
                  ee->engine.x.state.above = 0;
                  ecore_x_netwm_state_request_send(ee->prop.window,
                                                   ee->engine.x.win_root,
                                                   ECORE_X_WINDOW_STATE_ABOVE, -1, 0);
               }
          }
     }
   else
     {
        /* Just set the state */
        if (ee->prop.layer < 3)
          {
             if ((ee->engine.x.state.above) || (!ee->engine.x.state.below))
               {
                  ee->engine.x.state.above = 0;
                  ee->engine.x.state.below = 1;
                  _ecore_evas_x_state_update(ee);
               }
          }
        else if (ee->prop.layer > 5)
          {
             if ((!ee->engine.x.state.above) || (ee->engine.x.state.below))
               {
                  ee->engine.x.state.above = 1;
                  ee->engine.x.state.below = 0;
                  _ecore_evas_x_state_update(ee);
               }
          }
        else
          {
             if ((ee->engine.x.state.above) || (ee->engine.x.state.below))
               {
                  ee->engine.x.state.above = 0;
                  ee->engine.x.state.below = 0;
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

static void
_ecore_evas_x_free(Ecore_Evas *ee)
{
   _ecore_evas_x_group_leader_unset(ee);
   _ecore_evas_x_sync_set(ee);
   if (ee->engine.x.win_shaped_input)
     ecore_x_window_free(ee->engine.x.win_shaped_input);
   ecore_x_window_free(ee->prop.window);
   if (ee->engine.x.pmap) ecore_x_pixmap_free(ee->engine.x.pmap);
   if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
   if (ee->engine.x.gc) ecore_x_gc_free(ee->engine.x.gc);
   if (ee->engine.x.damages) ecore_x_xregion_free(ee->engine.x.damages);
   ee->engine.x.pmap = 0;
   ee->engine.x.mask = 0;
   ee->engine.x.gc = 0;
   ee->engine.x.damages = NULL;
   ecore_event_window_unregister(ee->prop.window);
   while (ee->engine.x.win_extra)
     {
        Ecore_X_Window *winp;

        winp = ee->engine.x.win_extra->data;
        ee->engine.x.win_extra =
          eina_list_remove_list(ee->engine.x.win_extra, ee->engine.x.win_extra);
        ecore_event_window_unregister(*winp);
        free(winp);
     }
   _ecore_evas_x_shutdown();
   ecore_x_shutdown();
}

static void
_ecore_evas_x_callback_delete_request_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   ee->func.fn_delete_request = func;
   _ecore_evas_x_protocols_set(ee);
   _ecore_evas_x_sync_set(ee);
}

static void
_ecore_evas_x_move(Ecore_Evas *ee, int x, int y)
{
   ee->req.x = x;
   ee->req.y = y;
   if (ee->engine.x.direct_resize)
     {
        if (!ee->engine.x.managed)
          {
             if ((x != ee->x) || (y != ee->y))
               {
                  ee->x = x;
                  ee->y = y;
                  ecore_x_window_move(ee->prop.window, x, y);
                  if (!ee->should_be_visible)
                    {
                       /* We need to request pos */
                       ee->prop.request_pos = 1;
                       _ecore_evas_x_size_pos_hints_update(ee);
                    }
                  if (ee->func.fn_move) ee->func.fn_move(ee);
               }
          }
     }
   else
     {
        if (((ee->x != x) || (ee->y != y)) ||
            (ee->engine.x.configure_coming))
          {
             ee->engine.x.configure_coming = 1;
             if (!ee->engine.x.managed)
               {
                  ee->x = x;
                  ee->y = y;
               }
             ecore_x_window_move(ee->prop.window, x, y);
          }
        if (!ee->should_be_visible)
          {
             /* We need to request pos */
             ee->prop.request_pos = 1;
             _ecore_evas_x_size_pos_hints_update(ee);
          }
     }
}

static void
_ecore_evas_x_managed_move(Ecore_Evas *ee, int x, int y)
{
   ee->req.x = x;
   ee->req.y = y;
   if (ee->engine.x.direct_resize)
     {
        ee->engine.x.managed = 1;
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
   ee->req.w = w;
   ee->req.h = h;
   if (ee->engine.x.direct_resize)
     {
        if ((ee->w != w) || (ee->h != h))
          {
             ee->w = w;
             ee->h = h;
             ecore_x_window_resize(ee->prop.window, w, h);
             if ((ee->rotation == 90) || (ee->rotation == 270))
               {
                  evas_output_size_set(ee->evas, ee->h, ee->w);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
               }
             else
               {
                  evas_output_size_set(ee->evas, ee->w, ee->h);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
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
            (ee->engine.x.configure_coming))
     {
        ee->engine.x.configure_coming = 1;
        ecore_x_window_resize(ee->prop.window, w, h);
     }
}

static void
_ecore_evas_x_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   ee->req.x = x;
   ee->req.y = y;
   ee->req.w = w;
   ee->req.h = h;
   if (ee->engine.x.direct_resize)
     {
        if ((ee->w != w) || (ee->h != h) || (x != ee->x) || (y != ee->y))
          {
             int change_size = 0, change_pos = 0;

             if ((ee->w != w) || (ee->h != h)) change_size = 1;
             if (!ee->engine.x.managed)
               {
                  if ((x != ee->x) || (y != ee->y)) change_pos = 1;
               }
             ecore_x_window_move_resize(ee->prop.window, x, y, w, h);
             if (!ee->engine.x.managed)
               {
                  ee->x = x;
                  ee->y = y;
               }
             ee->w = w;
             ee->h = h;
             if ((ee->rotation == 90) || (ee->rotation == 270))
               {
                  evas_output_size_set(ee->evas, ee->h, ee->w);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
               }
             else
               {
                  evas_output_size_set(ee->evas, ee->w, ee->h);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
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
            (ee->engine.x.configure_coming))
     {
        ee->engine.x.configure_coming = 1;
        ecore_x_window_move_resize(ee->prop.window, x, y, w, h);
        if (!ee->engine.x.managed)
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
             ee->engine.x.configure_coming = 1;
             if (!ee->prop.fullscreen)
               {
                  ecore_x_window_resize(ee->prop.window, ee->req.h, ee->req.w);
                  ee->expecting_resize.w = ee->h;
                  ee->expecting_resize.h = ee->w;
               }
             else
               {
                  int w, h;

                  ecore_x_window_size_get(ee->prop.window, &w, &h);
                  ecore_x_window_resize(ee->prop.window, h, w);
                  if ((rotation == 0) || (rotation == 180))
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
             if ((ee->rotation == 90) || (ee->rotation == 270))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.h, ee->req.w);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.w, ee->req.h);
          }
        else
          {
             /* int w, h; */

             /* ecore_x_window_size_get(ee->prop.window, &w, &h); */
             if ((rotation == 0) || (rotation == 180))
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
             if ((ee->rotation == 90) || (ee->rotation == 270))
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
             else
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
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

        if ((ee->rotation == 90) || (ee->rotation == 270))
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
        else
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
}

static void
_ecore_evas_x_rotation_set(Ecore_Evas *ee, int rotation, int resize)
{
   if (ee->rotation == rotation) return;
   if (!strcmp(ee->driver, "opengl_x11"))
     {
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
        Evas_Engine_Info_GL_X11 *einfo;

        einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;
        einfo->info.rotation = rotation;
        _ecore_evas_x_rotation_set_internal(ee, rotation, resize,
                                            (Evas_Engine_Info *)einfo);
#endif /* BUILD_ECORE_EVAS_OPENGL_X11 */
     }
   else if (!strcmp(ee->driver, "software_x11"))
     {
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
        Evas_Engine_Info_Software_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;
        einfo->info.rotation = rotation;
        _ecore_evas_x_rotation_set_internal(ee, rotation, resize,
                                            (Evas_Engine_Info *)einfo);
#endif /* BUILD_ECORE_EVAS_SOFTWARE_X11 */
     }
   else if (!strcmp(ee->driver,  "software_16_x11"))
     {
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
        Evas_Engine_Info_Software_16_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_16_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;
        einfo->info.rotation = rotation;
        _ecore_evas_x_rotation_set_internal(ee, rotation, resize,
                                            (Evas_Engine_Info *)einfo);
#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_X11 */
     }
   else if (!strcmp(ee->driver,  "software_8_x11"))
     {
#if BUILD_ECORE_EVAS_SOFTWARE_8_X11
        Evas_Engine_Info_Software_8_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_8_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;
        einfo->info.rotation = rotation;
        _ecore_evas_x_rotation_set_internal(ee, rotation, resize,
                                            (Evas_Engine_Info *)einfo);
#endif /* BUILD_ECORE_EVAS_SOFTWARE_8_X11 */
     }
}

static void
_ecore_evas_x_shaped_set(Ecore_Evas *ee, int shaped)
{
   if ((ee->shaped == shaped)) return;
   if (!strcmp(ee->driver, "opengl_x11")) return;
   if (!strcmp(ee->driver, "software_x11"))
     {
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
        Evas_Engine_Info_Software_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
        ee->shaped = shaped;
        if (einfo)
          {
             if (ee->shaped)
               {
                  unsigned int foreground;
                  Ecore_X_GC gc;

                  if (!ee->engine.x.mask)
                    ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, 1);
                  foreground = 0;
                  gc = ecore_x_gc_new(ee->engine.x.mask,
                                      ECORE_X_GC_VALUE_MASK_FOREGROUND,
                                      &foreground);
                  ecore_x_drawable_rectangle_fill(ee->engine.x.mask, gc,
                                                  0, 0, ee->w, ee->h);
                  ecore_x_gc_free(gc);
                  einfo->info.mask = ee->engine.x.mask;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
                  evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
                  ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
               }
             else
               {
                  if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
                  ee->engine.x.mask = 0;
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
   else if (!strcmp(ee->driver, "software_16_x11"))
     {
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
# if 0 /* XXX no shaped window support for software_16_x11 */
        Evas_Engine_Info_Software_16_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_16_X11 *)evas_engine_info_get(ee->evas);
        ee->shaped = shaped;
        if (einfo)
          {
             if (ee->shaped)
               {
                  ee->engine.x.mask =
                    ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, 1);
                  einfo->info.mask = ee->engine.x.mask;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
                  evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
               }
             else
               {
                  if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
                  ee->engine.x.mask = 0;
                  einfo->info.mask = 0;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
                  ecore_x_window_shape_mask_set(ee->prop.window, 0);
               }
          }
# endif /* XXX no shaped window support for software_16_x11 */
#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_X11 */
     }
   if (!strcmp(ee->driver, "software_8_x11"))
     {
#if defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
        Evas_Engine_Info_Software_8_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_8_X11 *)evas_engine_info_get(ee->evas);
        ee->shaped = shaped;
        if (einfo)
          {
             if (ee->shaped)
               {
                  unsigned int foreground;
                  Ecore_X_GC gc;

                  if (!ee->engine.x.mask)
                    ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, 1);
                  foreground = 0;
                  gc = ecore_x_gc_new(ee->engine.x.mask,
                                      ECORE_X_GC_VALUE_MASK_FOREGROUND,
                                      &foreground);
                  ecore_x_drawable_rectangle_fill(ee->engine.x.mask, gc,
                                                  0, 0, ee->w, ee->h);
                  ecore_x_gc_free(gc);
                  einfo->info.mask = ee->engine.x.mask;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
                  evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
                  ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
               }
             else
               {
                  if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
                  ee->engine.x.mask = 0;
                  einfo->info.mask = 0;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
                  ecore_x_window_shape_mask_set(ee->prop.window, 0);
                  ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
               }
          }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_8_X11 */
     }
}

/* FIXME, round trip */
static void
_ecore_evas_x_alpha_set(Ecore_Evas *ee, int alpha)
{
   Ecore_X_Window_Attributes att;
   char *id = NULL;

   if ((ee->alpha == alpha)) return;

   if (!strcmp(ee->driver, "software_x11"))
     {
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
        Evas_Engine_Info_Software_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;

        if (!ecore_x_composite_query()) return;

        ee->shaped = 0;
        ee->alpha = alpha;
        ecore_x_window_free(ee->prop.window);
        ecore_event_window_unregister(ee->prop.window);
        if (ee->alpha)
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (!ee->engine.x.mask)
               ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->req.w, ee->req.h, 1);
          }
        else
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
             ee->engine.x.mask = 0;
             ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
          }

        einfo->info.destination_alpha = alpha;

        ecore_x_window_attributes_get(ee->prop.window, &att);
        einfo->info.visual = att.visual;
        einfo->info.colormap = att.colormap;
        einfo->info.depth = att.depth;

//        if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
//        ee->engine.x.mask = 0;
        einfo->info.mask = ee->engine.x.mask;
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
        if (ee->visible) ecore_x_window_show(ee->prop.window);
        if (ee->prop.focused) ecore_x_window_focus(ee->prop.window);
        if (ee->prop.title)
          {
             ecore_x_icccm_title_set(ee->prop.window, ee->prop.title);
             ecore_x_netwm_name_set(ee->prop.window, ee->prop.title);
          }
        ecore_x_icccm_hints_set(ee->prop.window,
                                1 /* accepts_focus */,
                                ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
                                0 /* icon_pixmap */,
                                0 /* icon_mask */,
                                0 /* icon_window */,
                                0 /* window_group */,
                                0 /* is_urgent */);
        _ecore_evas_x_group_leader_update(ee);
        ecore_x_window_defaults_set(ee->prop.window);
        _ecore_evas_x_protocols_set(ee);
        _ecore_evas_x_sync_set(ee);
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
   else if (!strcmp(ee->driver, "opengl_x11"))
     {
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
        Evas_Engine_Info_GL_X11 *einfo;

        einfo = (Evas_Engine_Info_GL_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;

        if (!ecore_x_composite_query()) return;

        ee->shaped = 0;
        ee->alpha = alpha;
        ecore_x_window_free(ee->prop.window);
        ecore_event_window_unregister(ee->prop.window);
        ee->prop.window = 0;

        einfo->info.destination_alpha = alpha;

        if (ee->engine.x.win_root != 0)
          {
             /* FIXME: round trip in ecore_x_window_argb_get */
             if (ecore_x_window_argb_get(ee->engine.x.win_root))
               {
                  ee->prop.window =
                    _ecore_evas_x_gl_window_new(ee, ee->engine.x.win_root,
                                                ee->req.x, ee->req.y,
                                                ee->req.w, ee->req.h,
                                                ee->prop.override, 1, NULL);
               }
             else
               {
                  ee->prop.window =
                    _ecore_evas_x_gl_window_new(ee, ee->engine.x.win_root,
                                                ee->req.x, ee->req.y,
                                                ee->req.w, ee->req.h,
                                                ee->prop.override, ee->alpha,
                                                NULL);
               }
          }
        else
          {
             ee->prop.window =
               _ecore_evas_x_gl_window_new(ee, ee->engine.x.win_root,
                                           ee->req.x, ee->req.y,
                                           ee->req.w, ee->req.h,
                                           ee->prop.override, ee->alpha, NULL);
          }

        if (!ee->prop.window) return;
/*
        if (ee->alpha)
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (!ee->engine.x.mask)
               ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->req.w, ee->req.h, 1);
          }
        else
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
             ee->engine.x.mask = 0;
             ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
          }
 */

        ecore_x_window_attributes_get(ee->prop.window, &att);
        einfo->info.visual = att.visual;
        einfo->info.colormap = att.colormap;
        einfo->info.depth = att.depth;

//        if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
//        ee->engine.x.mask = 0;
//        einfo->info.mask = ee->engine.x.mask;
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
        if (ee->visible) ecore_x_window_show(ee->prop.window);
        if (ee->prop.focused) ecore_x_window_focus(ee->prop.window);
        if (ee->prop.title)
          {
             ecore_x_icccm_title_set(ee->prop.window, ee->prop.title);
             ecore_x_netwm_name_set(ee->prop.window, ee->prop.title);
          }
        ecore_x_icccm_hints_set(ee->prop.window,
                                1 /* accepts_focus */,
                                ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
                                0 /* icon_pixmap */,
                                0 /* icon_mask */,
                                0 /* icon_window */,
                                0 /* window_group */,
                                0 /* is_urgent */);
        _ecore_evas_x_group_leader_update(ee);
        ecore_x_window_defaults_set(ee->prop.window);
        _ecore_evas_x_protocols_set(ee);
        _ecore_evas_x_sync_set(ee);
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
   else if (!strcmp(ee->driver, "software_16_x11"))
     {
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
        Evas_Engine_Info_Software_16_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_16_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;

        ee->shaped = 0;
        ee->alpha = alpha;
        ecore_x_window_free(ee->prop.window);
        ecore_event_window_unregister(ee->prop.window);
        if (ee->alpha)
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (!ee->engine.x.mask)
               ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->req.w, ee->req.h, 1);
          }
        else
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
             ee->engine.x.mask = 0;
             ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
          }

# if 0 /* XXX no alpha window support for software_16_x11 */
        einfo->info.destination_alpha = alpha;
# endif /* XXX no alpha window support for software_16_x11 */

# if 0 /* XXX no shaped window support for software_16_x11 */
//        if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
//        ee->engine.x.mask = 0;
        einfo->info.mask = ee->engine.x.mask;
# endif /* XXX no shaped window support for software_16_x11 */

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
        if (ee->visible) ecore_x_window_show(ee->prop.window);
        if (ee->prop.focused) ecore_x_window_focus(ee->prop.window);
        if (ee->prop.title)
          {
             ecore_x_icccm_title_set(ee->prop.window, ee->prop.title);
             ecore_x_netwm_name_set(ee->prop.window, ee->prop.title);
          }
        ecore_x_icccm_hints_set(ee->prop.window,
                                1 /* accepts_focus */,
                                ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
                                0 /* icon_pixmap */,
                                0 /* icon_mask */,
                                0 /* icon_window */,
                                0 /* window_group */,
                                0 /* is_urgent */);
        _ecore_evas_x_group_leader_update(ee);
        ecore_x_window_defaults_set(ee->prop.window);
        _ecore_evas_x_protocols_set(ee);
        _ecore_evas_x_sync_set(ee);
#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_X11 */
        if ((id = getenv("DESKTOP_STARTUP_ID")))
          {
             ecore_x_netwm_startup_id_set(ee->prop.window, id);
             /* NB: on linux this may simply empty the env as opposed to completely
              * unset it to being empty - unsure as solartis libc crashes looking
              * for the '=' char */
             //        putenv((char*)"DESKTOP_STARTUP_ID=");
          }
     }
   else if (!strcmp(ee->driver, "software_8_x11"))
     {
#if defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
        Evas_Engine_Info_Software_8_X11 *einfo;

        einfo = (Evas_Engine_Info_Software_8_X11 *)evas_engine_info_get(ee->evas);
        if (!einfo) return;

        ee->shaped = 0;
        ee->alpha = alpha;
        ecore_x_window_free(ee->prop.window);
        ecore_event_window_unregister(ee->prop.window);
        if (ee->alpha)
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (!ee->engine.x.mask)
               ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->req.w, ee->req.h, 1);
          }
        else
          {
             if (ee->prop.override)
               ee->prop.window = ecore_x_window_override_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             else
               ee->prop.window = ecore_x_window_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h);
             if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
             ee->engine.x.mask = 0;
             ecore_x_window_shape_input_mask_set(ee->prop.window, 0);
          }

        einfo->info.destination_alpha = alpha;

        ecore_x_window_attributes_get(ee->prop.window, &att);
        einfo->info.visual = att.visual;
        einfo->info.colormap = att.colormap;
        einfo->info.depth = att.depth;

//        if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
//        ee->engine.x.mask = 0;
        einfo->info.mask = ee->engine.x.mask;
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
        if (ee->visible) ecore_x_window_show(ee->prop.window);
        if (ee->prop.focused) ecore_x_window_focus(ee->prop.window);
        if (ee->prop.title)
          {
             ecore_x_icccm_title_set(ee->prop.window, ee->prop.title);
             ecore_x_netwm_name_set(ee->prop.window, ee->prop.title);
          }
        ecore_x_icccm_hints_set(ee->prop.window,
                                1 /* accepts_focus */,
                                ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
                                0 /* icon_pixmap */,
                                0 /* icon_mask */,
                                0 /* icon_window */,
                                0 /* window_group */,
                                0 /* is_urgent */);
        _ecore_evas_x_group_leader_update(ee);
        ecore_x_window_defaults_set(ee->prop.window);
        _ecore_evas_x_protocols_set(ee);
        _ecore_evas_x_sync_set(ee);

        if ((id = getenv("DESKTOP_STARTUP_ID")))
          {
             ecore_x_netwm_startup_id_set(ee->prop.window, id);
             /* NB: on linux this may simply empty the env as opposed to completely
              * unset it to being empty - unsure as solartis libc crashes looking
              * for the '=' char */
             //        putenv((char*)"DESKTOP_STARTUP_ID=");
          }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_8_X11 */
     }
}

static void
_ecore_evas_x_transparent_set(Ecore_Evas *ee, int transparent)
{
   if ((ee->transparent == transparent)) return;

   if (!strcmp(ee->driver, "software_x11"))
     {
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
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
}
#endif /* BUILD_ECORE_EVAS_X11 */

#ifdef BUILD_ECORE_EVAS_X11
static void
_ecore_evas_x_show(Ecore_Evas *ee)
{
   ee->should_be_visible = 1;
   if (ee->prop.avoid_damage)
     _ecore_evas_x_render(ee);
   _ecore_evas_x_sync_set(ee);
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
   ecore_x_netwm_client_active_request(ee->engine.x.win_root,
                                       ee->prop.window, 2, 0);
}

static void
_ecore_evas_x_title_set(Ecore_Evas *ee, const char *t)
{
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (t) ee->prop.title = strdup(t);
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
_ecore_evas_object_cursor_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee) ee->prop.cursor.object = NULL;
}

static void
_ecore_evas_x_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   int x, y;

   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);

   if (!obj)
     {
        ee->prop.cursor.object = NULL;
        ee->prop.cursor.layer = 0;
        ee->prop.cursor.hot.x = 0;
        ee->prop.cursor.hot.y = 0;
        ecore_x_window_cursor_show(ee->prop.window, 1);
        return;
     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   ecore_x_window_cursor_show(ee->prop.window, 0);

   evas_pointer_output_xy_get(ee->evas, &x, &y);
   evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
   evas_object_move(ee->prop.cursor.object,
                    x - ee->prop.cursor.hot.x,
                    y - ee->prop.cursor.hot.y);
   evas_object_pass_events_set(ee->prop.cursor.object, 1);
   if (evas_pointer_inside_get(ee->evas))
     evas_object_show(ee->prop.cursor.object);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, ee);
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
_ecore_evas_x_focus_set(Ecore_Evas *ee, int on __UNUSED__)
{
   ecore_x_window_focus(ee->prop.window);
}

static void
_ecore_evas_x_iconified_set(Ecore_Evas *ee, int on)
{
   if (ee->prop.iconified == on) return;
   ee->prop.iconified = on;
   if (on)
     {
        ecore_x_icccm_hints_set(ee->prop.window,
                                1 /* accepts_focus */,
                                ECORE_X_WINDOW_STATE_HINT_ICONIC /* initial_state */,
                                0 /* icon_pixmap */,
                                0 /* icon_mask */,
                                0 /* icon_window */,
                                0 /* window_group */,
                                0 /* is_urgent */);
        ecore_x_icccm_iconic_request_send(ee->prop.window, ee->engine.x.win_root);
     }
   else
     {
        ecore_x_icccm_hints_set(ee->prop.window,
                                1 /* accepts_focus */,
                                ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
                                0 /* icon_pixmap */,
                                0 /* icon_mask */,
                                0 /* icon_window */,
                                0 /* window_group */,
                                0 /* is_urgent */);
        ecore_evas_show(ee);
     }
}

static void
_ecore_evas_x_borderless_set(Ecore_Evas *ee, int on)
{
   if (ee->prop.borderless == on) return;
   ee->prop.borderless = on;
   ecore_x_mwm_borderless_set(ee->prop.window, ee->prop.borderless);
}

/* FIXME: This function changes the initial state of the ee
 * whilest the iconic function changes the current state! */
static void
_ecore_evas_x_withdrawn_set(Ecore_Evas *ee, int withdrawn)
{
   Ecore_X_Window_State_Hint hint;

   if (ee->prop.withdrawn == withdrawn) return;

   ee->prop.withdrawn = withdrawn;
   if (withdrawn)
     hint = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
   else
     hint = ECORE_X_WINDOW_STATE_HINT_NORMAL;

   ecore_x_icccm_hints_set(ee->prop.window,
                           1 /* accepts_focus */,
                           hint /* initial_state */,
                           0 /* icon_pixmap */,
                           0 /* icon_mask */,
                           0 /* icon_window */,
                           0 /* window_group */,
                           0 /* is_urgent */);
}

static void
_ecore_evas_x_sticky_set(Ecore_Evas *ee, int sticky)
{
   if (ee->prop.sticky == sticky) return;

   /* We dont want to set prop.sticky here as it will cause
    * the sticky callback not to get called. Its set on the
    * property change event.
    * ee->prop.sticky = sticky;
    */
   ee->engine.x.state.sticky = sticky;
   if (ee->should_be_visible)
     ecore_x_netwm_state_request_send(ee->prop.window, ee->engine.x.win_root,
                                      ECORE_X_WINDOW_STATE_STICKY, -1, sticky);
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
             evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
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
             evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
          }
#endif
     }
}
*/

static void
_ecore_evas_x_override_set(Ecore_Evas *ee, int on)
{
   if (ee->prop.override == on) return;
   ecore_x_window_hide(ee->prop.window);
   ecore_x_window_override_set(ee->prop.window, on);
   if (ee->visible) ecore_x_window_show(ee->prop.window);
   if (ee->prop.focused) ecore_x_window_focus(ee->prop.window);
   ee->prop.override = on;
}

static void
_ecore_evas_x_fullscreen_set(Ecore_Evas *ee, int on)
{
   if (ee->prop.fullscreen == on) return;

   /* FIXME: Detect if WM is EWMH compliant and handle properly if not,
    * i.e. reposition, resize, and change borderless hint */
   ee->engine.x.state.fullscreen = on;
   if (ee->should_be_visible)
     ecore_x_netwm_state_request_send(ee->prop.window, ee->engine.x.win_root,
                                      ECORE_X_WINDOW_STATE_FULLSCREEN, -1, on);
   else
     _ecore_evas_x_state_update(ee);
}

static void
_ecore_evas_x_avoid_damage_set(Ecore_Evas *ee, int on)
{
   if (ee->prop.avoid_damage == on) return;
   if (!strcmp(ee->driver, "opengl_x11")) return;

   if (!strcmp(ee->driver, "software_x11"))
     {
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
        Evas_Engine_Info_Software_X11 *einfo;

        ee->prop.avoid_damage = on;
        einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->prop.avoid_damage)
               {
                  ee->engine.x.pmap = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, einfo->info.depth);
                  ee->engine.x.gc = ecore_x_gc_new(ee->engine.x.pmap, 0, NULL);
                  einfo->info.drawable = ee->engine.x.pmap;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
                  if ((ee->rotation == 90) || (ee->rotation == 270))
                    evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
                  else
                    evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
                  if (ee->prop.avoid_damage == ECORE_EVAS_AVOID_DAMAGE_BUILT_IN)
                    {
                       ee->engine.x.using_bg_pixmap = 1;
                       ecore_x_window_pixmap_set(ee->prop.window, ee->engine.x.pmap);
                       ecore_x_window_area_expose(ee->prop.window, 0, 0, ee->w, ee->h);
                    }
                  if (ee->engine.x.direct_resize)
                    {
                       /* Turn this off for now
                          ee->engine.x.using_bg_pixmap = 1;
                          ecore_x_window_pixmap_set(ee->prop.window, ee->engine.x.pmap);
                       */
                    }
               }
             else
               {
                  if (ee->engine.x.pmap) ecore_x_pixmap_free(ee->engine.x.pmap);
                  if (ee->engine.x.gc) ecore_x_gc_free(ee->engine.x.gc);
                  if (ee->engine.x.using_bg_pixmap)
                    {
                       ecore_x_window_pixmap_set(ee->prop.window, 0);
                       ee->engine.x.using_bg_pixmap = 0;
                       ecore_x_window_area_expose(ee->prop.window, 0, 0, ee->w, ee->h);
                    }
                  ee->engine.x.pmap = 0;
                  ee->engine.x.gc = 0;
                  einfo->info.drawable = ee->prop.window;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
               }
          }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_X11 */
     }
   else if (!strcmp(ee->driver, "software_16_x11"))
     {
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
        Evas_Engine_Info_Software_16_X11 *einfo;

        ee->prop.avoid_damage = on;
        einfo = (Evas_Engine_Info_Software_16_X11 *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->prop.avoid_damage)
               {
                  ee->engine.x.pmap = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, 16);
                  ee->engine.x.gc = ecore_x_gc_new(ee->engine.x.pmap, 0, NULL);
                  einfo->info.drawable = ee->engine.x.pmap;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
                  if ((ee->rotation == 90) || (ee->rotation == 270))
                    evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
                  else
                    evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
                  if (ee->engine.x.direct_resize)
                    {
                       /* Turn this off for now
                          ee->engine.x.using_bg_pixmap = 1;
                          ecore_x_window_pixmap_set(ee->prop.window, ee->engine.x.pmap);
                       */
                    }
               }
             else
               {
                  if (ee->engine.x.pmap) ecore_x_pixmap_free(ee->engine.x.pmap);
                  if (ee->engine.x.gc) ecore_x_gc_free(ee->engine.x.gc);
                  if (ee->engine.x.using_bg_pixmap)
                    {
                       ecore_x_window_pixmap_set(ee->prop.window, 0);
                       ee->engine.x.using_bg_pixmap = 0;
                    }
                  ee->engine.x.pmap = 0;
                  ee->engine.x.gc = 0;
                  einfo->info.drawable = ee->prop.window;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
               }
          }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_X11 */
     }
   else if (!strcmp(ee->driver, "software_8_x11"))
     {
#if BUILD_ECORE_EVAS_SOFTWARE_8_X11
        Evas_Engine_Info_Software_8_X11 *einfo;

        ee->prop.avoid_damage = on;
        einfo = (Evas_Engine_Info_Software_8_X11 *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->prop.avoid_damage)
               {
                  ee->engine.x.pmap = ecore_x_pixmap_new(ee->prop.window, ee->w, ee->h, einfo->info.depth);
                  ee->engine.x.gc = ecore_x_gc_new(ee->engine.x.pmap, 0, NULL);
                  einfo->info.drawable = ee->engine.x.pmap;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
                  if ((ee->rotation == 90) || (ee->rotation == 270))
                    evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
                  else
                    evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
                  if (ee->prop.avoid_damage == ECORE_EVAS_AVOID_DAMAGE_BUILT_IN)
                    {
                       ee->engine.x.using_bg_pixmap = 1;
                       ecore_x_window_pixmap_set(ee->prop.window, ee->engine.x.pmap);
                       ecore_x_window_area_expose(ee->prop.window, 0, 0, ee->w, ee->h);
                    }
                  if (ee->engine.x.direct_resize)
                    {
                       /* Turn this off for now
                          ee->engine.x.using_bg_pixmap = 1;
                          ecore_x_window_pixmap_set(ee->prop.window, ee->engine.x.pmap);
                       */
                    }
               }
             else
               {
                  if (ee->engine.x.pmap) ecore_x_pixmap_free(ee->engine.x.pmap);
                  if (ee->engine.x.gc) ecore_x_gc_free(ee->engine.x.gc);
                  if (ee->engine.x.using_bg_pixmap)
                    {
                       ecore_x_window_pixmap_set(ee->prop.window, 0);
                       ee->engine.x.using_bg_pixmap = 0;
                       ecore_x_window_area_expose(ee->prop.window, 0, 0, ee->w, ee->h);
                    }
                  ee->engine.x.pmap = 0;
                  ee->engine.x.gc = 0;
                  einfo->info.drawable = ee->prop.window;
                  if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
                    {
                       ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
                    }
               }
          }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_8_X11 */
     }
}

static void
_ecore_evas_x_screen_geometry_get(const Ecore_Evas *ee __UNUSED__, int *x, int *y, int *w, int *h)
{
   if (x) *x = 0;
   if (y) *y = 0;
   ecore_x_screen_size_get(ecore_x_default_screen_get(), w, h);
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
     NULL,
     _ecore_evas_x_fullscreen_set,
     _ecore_evas_x_avoid_damage_set,
     _ecore_evas_x_withdrawn_set,
     _ecore_evas_x_sticky_set,
     _ecore_evas_x_ignore_events_set,
     _ecore_evas_x_alpha_set,
     _ecore_evas_x_transparent_set,

     NULL, // render
     _ecore_evas_x_screen_geometry_get
};
#endif /* BUILD_ECORE_EVAS_X11 */

/*
 * FIXME: there are some round trips. Especially, we can split
 * ecore_x_init in 2 functions and suppress some round trips.
 */

#if defined (BUILD_ECORE_EVAS_SOFTWARE_X11) || defined (BUILD_ECORE_EVAS_OPENGL_X11) || defined (BUILD_ECORE_EVAS_SOFTWARE_16_X11) || defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
static void
_ecore_evas_x_flush_pre(void *data, Evas *e __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;

   if (ee->no_comp_sync) return;
   if (!_ecore_evas_app_comp_sync) return;
   if (ee->engine.x.sync_counter)
     {
        if (ee->engine.x.sync_began)
          {
             ee->engine.x.sync_val++;
             if (!ee->engine.x.sync_cancel)
               {
                  if (!ee->semi_sync)
                     ecore_x_sync_counter_val_wait(ee->engine.x.sync_counter,
                                                   ee->engine.x.sync_val);
               }
          }
     }
}

static void
_ecore_evas_x_flush_post(void *data, Evas *e __UNUSED__, void *event_info __UNUSED__)
{
   Ecore_Evas *ee = data;

   if ((!ee->no_comp_sync) && (_ecore_evas_app_comp_sync))
     {
        if (ee->engine.x.sync_counter)
          {
             if (ee->engine.x.sync_began)
               {
                  if (!ee->engine.x.sync_cancel)
                    {
                       ecore_x_e_comp_sync_draw_size_done_send
                          (ee->engine.x.win_root, ee->prop.window, ee->w, ee->h);
                    }
               }
          }
     }
   if (ee->engine.x.netwm_sync_set)
     {
        ecore_x_sync_counter_2_set(ee->engine.x.netwm_sync_counter,
                                   ee->engine.x.netwm_sync_val_hi,
                                   ee->engine.x.netwm_sync_val_lo);
        ee->engine.x.netwm_sync_set = 0;
     }
}
#endif

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
EAPI Ecore_Evas *
ecore_evas_software_x11_new(const char *disp_name, Ecore_X_Window parent,
                            int x, int y, int w, int h)
{
   Evas_Engine_Info_Software_X11 *einfo;
   Ecore_Evas *ee;
   int argb = 0, rmethod;
   static int redraw_debug = -1;
   char *id = NULL;

   rmethod = evas_render_method_lookup("software_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

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
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->engine.x.state.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE,
                           _ecore_evas_x_flush_pre, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST,
                           _ecore_evas_x_flush_post, ee);
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   ee->engine.x.win_root = parent;
   ee->engine.x.screen_num = 0;

   if (parent != 0)
     {
        ee->engine.x.screen_num = 1; /* FIXME: get real scren # */
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

# ifdef EVAS_FRAME_QUEUING
          {
             char *render_mode;

             render_mode = getenv("EVAS_RENDER_MODE");
             if ((render_mode) && (!strcmp(render_mode, "non-blocking")))
               einfo->render_mode = EVAS_RENDER_MODE_NONBLOCKING;
          }
# endif

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

   ecore_x_icccm_hints_set(ee->prop.window,
                           1 /* accepts_focus */,
                           ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
                           0 /* icon_pixmap */,
                           0 /* icon_mask */,
                           0 /* icon_window */,
                           0 /* window_group */,
                           0 /* is_urgent */);
   _ecore_evas_x_group_leader_set(ee);
   ecore_x_window_defaults_set(ee->prop.window);
   _ecore_evas_x_protocols_set(ee);
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
#else
EAPI Ecore_Evas *
ecore_evas_software_x11_new(const char *disp_name __UNUSED__, Ecore_X_Window parent __UNUSED__,
                            int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   return NULL;
}
#endif

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
EAPI Ecore_X_Window
ecore_evas_software_x11_window_get(const Ecore_Evas *ee)
{
   return (Ecore_X_Window) ecore_evas_window_get(ee);
}
#else
EAPI Ecore_X_Window
ecore_evas_software_x11_window_get(const Ecore_Evas *ee __UNUSED__)
{
   return 0;
}
#endif

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
EAPI void
ecore_evas_software_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
   ee->engine.x.direct_resize = on;
   if (ee->prop.avoid_damage)
     {
        if (ee->engine.x.direct_resize)
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
#else
EAPI void
ecore_evas_software_x11_direct_resize_set(Ecore_Evas *ee __UNUSED__, Eina_Bool on __UNUSED__)
{
}
#endif

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
EAPI Eina_Bool
ecore_evas_software_x11_direct_resize_get(const Ecore_Evas *ee)
{
   return ee->engine.x.direct_resize;
}
#else
EAPI Eina_Bool
ecore_evas_software_x11_direct_resize_get(const Ecore_Evas *ee __UNUSED__)
{
   return 0;
}
#endif

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
EAPI void
ecore_evas_software_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
   Ecore_X_Window *winp;

   winp = malloc(sizeof(Ecore_X_Window));
   if (winp)
     {
        *winp = win;
        ee->engine.x.win_extra = eina_list_append(ee->engine.x.win_extra, winp);
        ecore_x_input_multi_select(win);
        ecore_event_window_register(win, ee, ee->evas,
                                    (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                                    (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                                    (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                                    (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
     }
}
#else
EAPI void
ecore_evas_software_x11_extra_event_window_add(Ecore_Evas *ee __UNUSED__, Ecore_X_Window win __UNUSED__)
{
}
#endif

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
EAPI Ecore_Evas *
ecore_evas_gl_x11_new(const char *disp_name, Ecore_X_Window parent,
                      int x, int y, int w, int h)
{
   return ecore_evas_gl_x11_options_new(disp_name, parent, x, y, w, h, NULL);
}

EAPI Ecore_Evas *
ecore_evas_gl_x11_options_new(const char *disp_name, Ecore_X_Window parent,
                              int x, int y, int w, int h, const int *opt)
{
   Ecore_Evas *ee;
   int rmethod;
   char *id = NULL;

   rmethod = evas_render_method_lookup("gl_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_x_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;

   ee->driver = "opengl_x11";
   if (!getenv("ECORE_EVAS_COMP_NOSEMISYNC"))
      ee->semi_sync = 1; // gl engine doesn't need to sync - its whole swaps
//   ee->no_comp_sync = 1; // gl engine doesn't need to sync - its whole swaps
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
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->engine.x.state.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE, _ecore_evas_x_flush_pre, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST, _ecore_evas_x_flush_post, ee);
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   if (parent == 0) parent = DefaultRootWindow(ecore_x_display_get());
   ee->engine.x.win_root = parent;

   if (ee->engine.x.win_root != 0)
     {
        ee->engine.x.screen_num = 1; /* FIXME: get real scren # */
       /* FIXME: round trip in ecore_x_window_argb_get */
        if (ecore_x_window_argb_get(ee->engine.x.win_root))
          {
             ee->prop.window = _ecore_evas_x_gl_window_new
               (ee, ee->engine.x.win_root, x, y, w, h, 0, 1, opt);
          }
        else
          ee->prop.window = _ecore_evas_x_gl_window_new
          (ee, ee->engine.x.win_root, x, y, w, h, 0, 0, opt);
     }
   else
     ee->prop.window = _ecore_evas_x_gl_window_new
     (ee, ee->engine.x.win_root, x, y, w, h, 0, 0, opt);
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

   ecore_x_icccm_hints_set(ee->prop.window,
                           1 /* accepts_focus */,
                           ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
                           0 /* icon_pixmap */,
                           0 /* icon_mask */,
                           0 /* icon_window */,
                           0 /* window_group */,
                           0 /* is_urgent */);
   _ecore_evas_x_group_leader_set(ee);
   ecore_x_window_defaults_set(ee->prop.window);
   _ecore_evas_x_protocols_set(ee);
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
#else
EAPI Ecore_Evas *
ecore_evas_gl_x11_new(const char *disp_name __UNUSED__, Ecore_X_Window parent __UNUSED__,
                      int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   return NULL;
}
EAPI Ecore_Evas *
ecore_evas_gl_x11_options_new(const char *disp_name __UNUSED__, Ecore_X_Window parent __UNUSED__,
                              int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__, const int *opt __UNUSED__)
{
   return NULL;
}
#endif /* ! BUILD_ECORE_EVAS_OPENGL_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
EAPI Ecore_X_Window
ecore_evas_gl_x11_window_get(const Ecore_Evas *ee)
{
   return (Ecore_X_Window) ecore_evas_window_get(ee);
}
#else
EAPI Ecore_X_Window
ecore_evas_gl_x11_window_get(const Ecore_Evas *ee __UNUSED__)
{
   return 0;
}
#endif /* ! BUILD_ECORE_EVAS_OPENGL_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
EAPI void
ecore_evas_gl_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
   ee->engine.x.direct_resize = on;
}
#else
EAPI void
ecore_evas_gl_x11_direct_resize_set(Ecore_Evas *ee __UNUSED__, Eina_Bool on __UNUSED__)
{
}
#endif /* ! BUILD_ECORE_EVAS_OPENGL_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
EAPI Eina_Bool
ecore_evas_gl_x11_direct_resize_get(const Ecore_Evas *ee)
{
   return ee->engine.x.direct_resize;
}
#else
EAPI Eina_Bool
ecore_evas_gl_x11_direct_resize_get(const Ecore_Evas *ee __UNUSED__)
{
   return 0;
}
#endif /* ! BUILD_ECORE_EVAS_OPENGL_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
EAPI void
ecore_evas_gl_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
   ecore_evas_software_x11_extra_event_window_add(ee, win);
}
#else
EAPI void
ecore_evas_gl_x11_extra_event_window_add(Ecore_Evas *ee __UNUSED__, Ecore_X_Window win __UNUSED__)
{
}
#endif /* ! BUILD_ECORE_EVAS_OPENGL_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
EAPI void
ecore_evas_gl_x11_pre_post_swap_callback_set(const Ecore_Evas *ee, void *data, void (*pre_cb) (void *data, Evas *e), void (*post_cb) (void *data, Evas *e))
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
#else
EAPI void
ecore_evas_gl_x11_pre_post_swap_callback_set(const Ecore_Evas *ee __UNUSED__, void *data __UNUSED__, void (*pre_cb) (void *data, Evas *e) __UNUSED__, void (*post_cb) (void *data, Evas *e) __UNUSED__)
{
   return;
}
#endif /* ! BUILD_ECORE_EVAS_OPENGL_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed or maybe removed rather?
 */
EAPI Ecore_Evas *
ecore_evas_xrender_x11_new(const char *disp_name __UNUSED__, Ecore_X_Window parent __UNUSED__,
                           int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   return NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed or maybe removed rather?
 */
EAPI Ecore_X_Window
ecore_evas_xrender_x11_window_get(const Ecore_Evas *ee __UNUSED__)
{
   return 0;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI void
ecore_evas_xrender_x11_direct_resize_set(Ecore_Evas *ee __UNUSED__, Eina_Bool on __UNUSED__)
{
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI Eina_Bool
ecore_evas_xrender_x11_direct_resize_get(const Ecore_Evas *ee __UNUSED__)
{
   return 0;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI void
ecore_evas_xrender_x11_extra_event_window_add(Ecore_Evas *ee __UNUSED__, Ecore_X_Window win __UNUSED__)
{
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
EAPI Ecore_Evas *
ecore_evas_software_x11_16_new(const char *disp_name, Ecore_X_Window parent,
                               int x, int y, int w, int h)
{
   Evas_Engine_Info_Software_16_X11 *einfo;
   Ecore_Evas *ee;
   int rmethod;
   static int redraw_debug = -1;

   rmethod = evas_render_method_lookup("software_16_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_x_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;

   ee->driver = "software_16_x11";
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
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->engine.x.state.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE, _ecore_evas_x_flush_pre, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST, _ecore_evas_x_flush_post, ee);
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   ee->engine.x.win_root = parent;
   if (parent != 0)
     {
       /* FIXME: round trip in ecore_x_window_argb_get */
        if (ecore_x_window_argb_get(parent))
          {
             ee->prop.window = ecore_x_window_argb_new(parent, x, y, w, h);
          }
        else
          ee->prop.window = ecore_x_window_new(parent, x, y, w, h);
     }
   else
     ee->prop.window = ecore_x_window_new(parent, x, y, w, h);
   if (getenv("DESKTOP_STARTUP_ID"))
     {
        ecore_x_netwm_startup_id_set(ee->prop.window,
                                     getenv("DESKTOP_STARTUP_ID"));
        /* NB: on linux this may simply empty the env as opposed to completely
         * unset it to being empty - unsure as solartis libc crashes looking
         * for the '=' char */
//        putenv((char*)"DESKTOP_STARTUP_ID=");
     }
   einfo = (Evas_Engine_Info_Software_16_X11 *)evas_engine_info_get(ee->evas);

   if (einfo)
     {
        if (ScreenCount(ecore_x_display_get()) > 1)
          {
             Ecore_X_Window *roots;
             int num, i;

             num = 0;
             roots = ecore_x_window_root_list(&num);
             if (roots)
               {
                  XWindowAttributes at;

                  if (XGetWindowAttributes(ecore_x_display_get(),
                                           parent, &at))
                    {
                       for (i = 0; i < num; i++)
                         {
                            if (at.root == roots[i])
                              break;
                         }
                    }
                  free(roots);
               }
          }

        if (redraw_debug < 0)
          {
             if (getenv("REDRAW_DEBUG"))
               redraw_debug = atoi(getenv("REDRAW_DEBUG"));
             else
               redraw_debug = 0;
          }
        einfo->info.display  = ecore_x_display_get();
        einfo->info.drawable = ee->prop.window;

        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }

   ecore_x_icccm_hints_set(ee->prop.window,
                           1 /* accepts_focus */,
                           ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
                           0 /* icon_pixmap */,
                           0 /* icon_mask */,
                           0 /* icon_window */,
                           0 /* window_group */,
                           0 /* is_urgent */);
   _ecore_evas_x_group_leader_set(ee);
   ecore_x_window_defaults_set(ee->prop.window);
   _ecore_evas_x_protocols_set(ee);
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
#else
EAPI Ecore_Evas *
ecore_evas_software_x11_16_new(const char *disp_name __UNUSED__, Ecore_X_Window parent __UNUSED__,
                               int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   return NULL;
}
#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_16_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
EAPI Ecore_X_Window
ecore_evas_software_x11_16_window_get(const Ecore_Evas *ee)
{
   return (Ecore_X_Window) ecore_evas_window_get(ee);
}
#else
EAPI Ecore_X_Window
ecore_evas_software_x11_16_window_get(const Ecore_Evas *ee __UNUSED__)
{
   return 0;
}
#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_16_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
EAPI void
ecore_evas_software_x11_16_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
   ee->engine.x.direct_resize = on;
   if (ee->prop.avoid_damage)
     {
        if (ee->engine.x.direct_resize)
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
#else
EAPI void
ecore_evas_software_x11_16_direct_resize_set(Ecore_Evas *ee __UNUSED__, Eina_Bool on __UNUSED__)
{
}
#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_16_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
EAPI Eina_Bool
ecore_evas_software_x11_16_direct_resize_get(const Ecore_Evas *ee)
{
   return ee->engine.x.direct_resize;
}
#else
EAPI Eina_Bool
ecore_evas_software_x11_16_direct_resize_get(const Ecore_Evas *ee __UNUSED__)
{
   return 0;
}
#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_16_X11 */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
#if BUILD_ECORE_EVAS_SOFTWARE_16_X11
EAPI void
ecore_evas_software_x11_16_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
   Ecore_X_Window *winp;

   winp = malloc(sizeof(Ecore_X_Window));
   if (winp)
     {
        *winp = win;
        ee->engine.x.win_extra = eina_list_append(ee->engine.x.win_extra, winp);
        ecore_x_input_multi_select(win);
        ecore_event_window_register(win, ee, ee->evas,
                                    (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                                    (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                                    (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                                    (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
     }
}
#else
EAPI void
ecore_evas_software_x11_16_extra_event_window_add(Ecore_Evas *ee __UNUSED__, Ecore_X_Window win __UNUSED__)
{
}
#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_16_X11 */


/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI Ecore_Evas *
ecore_evas_software_x11_8_new(const char *disp_name, Ecore_X_Window parent,
                              int x, int y, int w, int h)
{
#if defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
   Evas_Engine_Info_Software_8_X11 *einfo;
   Ecore_Evas *ee;
   int argb = 0;
   int rmethod;
   static int redraw_debug = -1;

   rmethod = evas_render_method_lookup("software_8_x11");
   if (!rmethod) return NULL;
   if (!ecore_x_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_x_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_x_engine_func;

   ee->driver = "software_8_x11";
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
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   ee->engine.x.state.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE, _ecore_evas_x_flush_pre, ee);
   evas_event_callback_add(ee->evas, EVAS_CALLBACK_RENDER_FLUSH_POST, _ecore_evas_x_flush_post, ee);
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   ee->engine.x.win_root = parent;
   //   if (parent != 0)
   //     {
   //       /* FIXME: round trip in ecore_x_window_argb_get */
   //        if (ecore_x_window_argb_get(parent))
   //          {
   //             ee->engine.x.win = ecore_x_window_argb_new(parent, x, y, w, h);
   //             argb = 1;
   //          }
   //        else
   //          ee->engine.x.win = ecore_x_window_new(parent, x, y, w, h);
   //     }
   //   else
   ee->prop.window = ecore_x_window_new(parent, x, y, w, h);
   if (getenv("DESKTOP_STARTUP_ID"))
     {
        ecore_x_netwm_startup_id_set(ee->prop.window,
              getenv("DESKTOP_STARTUP_ID"));
        /* NB: on linux this may simply empty the env as opposed to completely
         * unset it to being empty - unsure as solartis libc crashes looking
         * for the '=' char */
        //        putenv((char*)"DESKTOP_STARTUP_ID=");
     }
   einfo = (Evas_Engine_Info_Software_8_X11 *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        xcb_screen_iterator_t iter;
        xcb_screen_t         *screen;

        /* FIXME: this is inefficient as its a round trip */
        //einfo->info.backend = 1;
        screen = ecore_x_default_screen_get();
        iter = xcb_setup_roots_iterator (xcb_get_setup (ecore_x_connection_get()));
        if (iter.rem > 1)
          {
             xcb_get_geometry_cookie_t cookie;
             xcb_get_geometry_reply_t *reply;
             Ecore_X_Window           *roots;
             int                       num;
             uint8_t                   i;

             num = 0;
             cookie = xcb_get_geometry_unchecked(ecore_x_connection_get(), parent);
             roots = ecore_x_window_root_list(&num);
             if (roots)
               {
                  reply = xcb_get_geometry_reply(ecore_x_connection_get(), cookie, NULL);

                  if (reply)
                    {
                       for (i = 0; i < num; xcb_screen_next (&iter), i++)
                         {
                            if (reply->root == roots[i])
                              {
                                 screen = iter.data;
                                 break;
                              }
                         }
                       free(reply);
                    }
                  free(roots);
               }
             else
               {
                  reply = xcb_get_geometry_reply(ecore_x_connection_get(), cookie, NULL);
                  if (reply) free(reply);
               }
          }

        if (redraw_debug < 0)
          {
             if (getenv("REDRAW_DEBUG"))
               redraw_debug = atoi(getenv("REDRAW_DEBUG"));
             else
               redraw_debug = 0;
          }
        einfo->info.connection = ecore_x_connection_get();
        einfo->info.screen = screen;
        einfo->info.drawable = ee->prop.window;
        if (argb)
          {
             /* FIXME: round trip */
             xcb_get_geometry_cookie_t          cookie_geom;
             xcb_get_window_attributes_cookie_t cookie_attr;
             xcb_get_geometry_reply_t          *reply_geom;
             xcb_get_window_attributes_reply_t *reply_attr;

             cookie_geom = xcb_get_geometry_unchecked(ecore_x_connection_get(), ee->prop.window);
             cookie_attr = xcb_get_window_attributes_unchecked(ecore_x_connection_get(), ee->prop.window);

             reply_geom = xcb_get_geometry_reply(ecore_x_connection_get(), cookie_geom, NULL);
             reply_attr = xcb_get_window_attributes_reply(ecore_x_connection_get(), cookie_attr, NULL);
             if (reply_attr && reply_geom)
               {
                  einfo->info.visual   = xcb_visualtype_get(ecore_x_default_screen_get(), reply_attr->visual);
                  einfo->info.colormap = reply_attr->colormap;
                  einfo->info.depth    = reply_geom->depth;
                  einfo->info.destination_alpha = 1;
                  free(reply_geom);
                  free(reply_attr);
               }
          }
        else
          {
             xcb_screen_t *screen;

             screen = ecore_x_default_screen_get();
             einfo->info.visual   = xcb_visualtype_get(screen, screen->root_visual);
             einfo->info.colormap = screen->default_colormap;
             einfo->info.depth    = screen->root_depth;
             einfo->info.destination_alpha = 0;
          }
        einfo->info.rotation = 0;
        einfo->info.debug    = redraw_debug;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }

   ecore_x_icccm_hints_set(ee->prop.window,
                           1 /* accepts_focus */,
                           ECORE_X_WINDOW_STATE_HINT_NORMAL /* initial_state */,
                           0 /* icon_pixmap */,
                           0 /* icon_mask */,
                           0 /* icon_window */,
                           0 /* window_group */,
                           0 /* is_urgent */);
   _ecore_evas_x_group_leader_set(ee);
   ecore_x_window_defaults_set(ee->prop.window);
   _ecore_evas_x_protocols_set(ee);
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
#else
   return NULL;
   (void)(disp_name);
   (void)(parent);
   (void)(x);
   (void)(y);
   (void)(w);
   (void)(h);
#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_8_X11 */
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI Ecore_X_Window
ecore_evas_software_x11_8_window_get(const Ecore_Evas *ee)
{
#if defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
   return (Ecore_X_Window) ecore_evas_window_get(ee);
#else
   return 0;
   (void)(ee);
#endif
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI Ecore_X_Window
ecore_evas_software_x11_8_subwindow_get(const Ecore_Evas *ee)
{
#if defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
   return (Ecore_X_Window) ecore_evas_window_get(ee);
#else
   return 0;
   (void)(ee);
#endif
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI void
ecore_evas_software_x11_8_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
#if defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
   ee->engine.x.direct_resize = on;
   if (ee->prop.avoid_damage)
     {
        if (ee->engine.x.direct_resize)
          {
             /* turn this off for now
                ee->engine.x.using_bg_pixmap = 1;
                ecore_x_window_pixmap_set(ee->engine.x.win, ee->engine.x.pmap);
                */
          }
        else
          {
             /* turn this off too- bg pixmap is controlled by avoid damage directly
                ee->engine.x.using_bg_pixmap = 0;
                ecore_x_window_pixmap_set(ee->engine.x.win, 0);
                ecore_x_window_area_expose(ee->engine.x.win, 0, 0, ee->w, ee->h);
                */
          }
     }
#else
   return;
   (void)(ee);
   (void)(on);
#endif
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI Eina_Bool
ecore_evas_software_x11_8_direct_resize_get(const Ecore_Evas *ee)
{
#if defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
   return ee->engine.x.direct_resize;
#else
   return 0;
   (void)(ee);
#endif
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI void
ecore_evas_software_x11_8_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
#if defined (BUILD_ECORE_EVAS_SOFTWARE_8_X11)
   Ecore_X_Window *winp;

   winp = malloc(sizeof(Ecore_X_Window));
   if (winp)
     {
        *winp = win;
        ee->engine.x.win_extra = eina_list_append(ee->engine.x.win_extra, winp);
        ecore_x_input_multi_select(win);
        ecore_event_window_register(win, ee, ee->evas,
                                    (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                                    (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                                    (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                                    (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
     }
#else
   return;
   (void)(ee);
   (void)(win);
#endif
}

EAPI void
ecore_evas_x11_leader_set(Ecore_Evas *ee, Ecore_X_Window win)
{
#ifdef BUILD_ECORE_EVAS_X11
   _ecore_evas_x_group_leader_unset(ee);
   ee->engine.x.leader = win;
   _ecore_evas_x_group_leader_update(ee);
#else
   return;
   ee = NULL;
   win = 0;
#endif
}

EAPI Ecore_X_Window
ecore_evas_x11_leader_get(Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_EVAS_X11
   return ee->engine.x.leader;
#else
   return 0;
   ee = NULL;
#endif
}

EAPI void
ecore_evas_x11_leader_default_set(Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_EVAS_X11
   _ecore_evas_x_group_leader_unset(ee);
   _ecore_evas_x_group_leader_set(ee);
#else
   return;
   ee = NULL;
#endif
}

#ifdef BUILD_ECORE_EVAS_X11
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
#endif

EAPI void
ecore_evas_x11_shape_input_rectangle_set(Ecore_Evas *ee, int x, int y, int w, int h)
{
#ifdef BUILD_ECORE_EVAS_X11
   Eina_Bool ret;
   Ecore_X_Rectangle src_rect;
   Ecore_X_Rectangle dst_rect;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_rectangle_set");
        return;
     }

   src_rect.x = x;
   src_rect.y = y;
   src_rect.width = w;
   src_rect.height = h;

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = 0;
   dst_rect.height = 0;

   ret = _ecore_evas_x11_convert_rectangle_with_angle(ee, &dst_rect, &src_rect);

   if (!ee->engine.x.win_shaped_input)
      ee->engine.x.win_shaped_input = ecore_x_window_override_new(ee->engine.x.win_root,
                                                                  0, 0, 1, 1);

   if (ret)
      ecore_x_window_shape_input_rectangle_set(ee->engine.x.win_shaped_input,
                                               dst_rect.x, dst_rect.y,
                                               dst_rect.width, dst_rect.height);
#else
   return;
   ee = NULL;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_evas_x11_shape_input_rectangle_add(Ecore_Evas *ee, int x, int y, int w, int h)
{
#ifdef BUILD_ECORE_EVAS_X11
   Eina_Bool ret;
   Ecore_X_Rectangle src_rect;
   Ecore_X_Rectangle dst_rect;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_rectangle_add");
        return;
     }

   src_rect.x = x;
   src_rect.y = y;
   src_rect.width = w;
   src_rect.height = h;

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = 0;
   dst_rect.height = 0;

   ret = _ecore_evas_x11_convert_rectangle_with_angle(ee, &dst_rect, &src_rect);

   if (!ee->engine.x.win_shaped_input)
      ee->engine.x.win_shaped_input = ecore_x_window_override_new(ee->engine.x.win_root,
                                                                  0, 0, 1, 1);

   if (ret)
      ecore_x_window_shape_input_rectangle_add(ee->engine.x.win_shaped_input,
                                               dst_rect.x, dst_rect.y,
                                               dst_rect.width, dst_rect.height);
#else
   return;
   ee = NULL;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_evas_x11_shape_input_rectangle_subtract(Ecore_Evas *ee, int x, int y, int w, int h)
{
#ifdef BUILD_ECORE_EVAS_X11
   Eina_Bool ret;
   Ecore_X_Rectangle src_rect;
   Ecore_X_Rectangle dst_rect;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_rectangle_subtract");
        return;
     }

   src_rect.x = x;
   src_rect.y = y;
   src_rect.width = w;
   src_rect.height = h;

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = 0;
   dst_rect.height = 0;

   ret = _ecore_evas_x11_convert_rectangle_with_angle(ee, &dst_rect, &src_rect);

   if (!ee->engine.x.win_shaped_input)
      ee->engine.x.win_shaped_input = ecore_x_window_override_new(ee->engine.x.win_root,
                                                                  0, 0, 1, 1);

   if (ret)
      ecore_x_window_shape_input_rectangle_subtract(ee->engine.x.win_shaped_input,
                                                    dst_rect.x, dst_rect.y,
                                                    dst_rect.width, dst_rect.height);
#else
   return;
   ee = NULL;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_evas_x11_shape_input_empty(Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_EVAS_X11
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_empty");
        return;
     }

   if (!ee->engine.x.win_shaped_input)
      ee->engine.x.win_shaped_input = ecore_x_window_override_new(ee->engine.x.win_root, 0, 0, 1, 1);

   ecore_x_window_shape_input_rectangle_set(ee->engine.x.win_shaped_input, 0, 0, 0, 0);
#else
   return;
   ee = NULL;
#endif
}

EAPI void
ecore_evas_x11_shape_input_reset(Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_EVAS_X11
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_reset");
        return;
     }

   if (!ee->engine.x.win_shaped_input)
      ee->engine.x.win_shaped_input = ecore_x_window_override_new(ee->engine.x.win_root, 0, 0, 1, 1);

   ecore_x_window_shape_input_rectangle_set(ee->engine.x.win_shaped_input, 0, 0, 65535, 65535);
#else
   return;
   ee = NULL;
#endif
}

EAPI void
ecore_evas_x11_shape_input_apply(Ecore_Evas *ee)
{
#ifdef BUILD_ECORE_EVAS_X11
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_x11_shape_input_apply");
        return;
     }

   if (!ee->engine.x.win_shaped_input) return;

   ecore_x_window_shape_input_window_set(ee->prop.window, ee->engine.x.win_shaped_input);
#else
   return;
   ee = NULL;
#endif
}
