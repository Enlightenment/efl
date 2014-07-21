#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Elementary_Cursor.h>

#include "elm_priv.h"
#include "elm_widget_menu.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_interface_atspi_window.eo.h"
#include "elm_interface_atspi_widget.eo.h"

#define MY_CLASS ELM_WIN_CLASS

#define MY_CLASS_NAME "Elm_Win"
#define MY_CLASS_NAME_LEGACY "elm_win"

static const Elm_Win_Trap *trap = NULL;

#define TRAP(sd, name, ...)                                             \
  do                                                                    \
    {                                                                   \
       if ((!trap) || (!trap->name) ||                                  \
           ((trap->name) &&                                             \
            (trap->name(sd->trap_data, sd->obj, ## __VA_ARGS__)))) \
         ecore_evas_##name(sd->ee, ##__VA_ARGS__);                      \
    }                                                                   \
  while (0)

#define ELM_WIN_DATA_GET(o, sd) \
  Elm_Win_Data * sd = eo_data_scope_get(o, MY_CLASS)

#define ELM_WIN_DATA_GET_OR_RETURN(o, ptr)           \
  ELM_WIN_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRI("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_WIN_DATA_GET_OR_RETURN_VAL(o, ptr, val)  \
  ELM_WIN_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRI("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_WIN_CHECK(obj)                                             \
  if (!obj || !eo_isa(obj, MY_CLASS)) \
    return

#define DECREMENT_MODALITY()                                    \
  EINA_LIST_FOREACH(_elm_win_list, l, current)                  \
    {                                                           \
       ELM_WIN_DATA_GET_OR_RETURN(current, cursd);              \
       if ((obj != current) && (cursd->modal_count > 0))        \
         {                                                      \
            cursd->modal_count--;                               \
         }                                                      \
       if (cursd->modal_count == 0)                             \
         {                                                      \
            edje_object_signal_emit(cursd->layout, \
                        "elm,action,hide_blocker", "elm");      \
            evas_object_smart_callback_call(cursd->main_menu, \
                        "elm,action,unblock_menu", NULL);       \
         }                                                      \
    }

#define INCREMENT_MODALITY()                                    \
  EINA_LIST_FOREACH(_elm_win_list, l, current)                  \
    {                                                           \
       ELM_WIN_DATA_GET_OR_RETURN(current, cursd);              \
       if (obj != current)                                      \
         {                                                      \
            cursd->modal_count++;                               \
         }                                                      \
       if (cursd->modal_count > 0)                              \
         {                                                      \
            edje_object_signal_emit(cursd->layout, \
                             "elm,action,show_blocker", "elm"); \
            evas_object_smart_callback_call(cursd->main_menu, \
                             "elm,action,block_menu", NULL);    \
         }                                                      \
    }

#define ENGINE_GET() (_elm_preferred_engine ? _elm_preferred_engine : (_elm_config->engine ? _elm_config->engine : ""))
#define ENGINE_COMPARE(name) (!strcmp(ENGINE_GET(), name))

typedef struct _Elm_Win_Data Elm_Win_Data;

struct _Elm_Win_Data
{
   Ecore_Evas           *ee;
   Evas                 *evas;
   Evas_Object          *parent; /* parent *window* object*/
   Evas_Object          *img_obj, *frame_obj;
   Evas_Object          *client_obj; /* rect representing the client */
   Evas_Object          *spacer_obj;
   Eo                   *layout;
   Eo                   *box;
   Evas_Object          *obj; /* The object itself */
#ifdef HAVE_ELEMENTARY_X
   struct
   {
      Ecore_X_Window       xwin;
      Ecore_Event_Handler *client_message_handler;
      Ecore_Event_Handler *property_handler;
   } x;
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   struct
   {
      Ecore_Wl_Window *win;
   } wl;
#endif

   Ecore_Job                     *deferred_resize_job;
   Ecore_Job                     *deferred_child_eval_job;

   Elm_Win_Type                   type;
   Elm_Win_Keyboard_Mode          kbdmode;
   Elm_Win_Indicator_Mode         indmode;
   Elm_Win_Indicator_Opacity_Mode ind_o_mode;
   struct
   {
      const char  *info;
      Ecore_Timer *timer;
      int          repeat_count;
      int          shot_counter;
   } shot;
   int                            resize_location;
   int                           *autodel_clear, rot;
   struct
   {
      int x, y;
   } screen;
   struct
   {
      Ecore_Evas  *ee;
      Evas        *evas;
      Evas_Object *obj, *hot_obj;
      int          hot_x, hot_y;
   } pointer;
   struct
   {
      Evas_Object *fobj; /* focus highlight edje object */

      struct
      {
         Evas_Object *target;
         Eina_Bool    visible : 1;
         Eina_Bool    in_theme: 1; /**< focus highlight is handled by theme.
                                     this is set true if edc data item "focus_highlight" is set to "on" during focus in callback. */
      } cur, prev;

      const char  *style;
      Ecore_Job   *reconf_job;

      Eina_Bool    enabled : 1;
      Eina_Bool    theme_changed : 1; /* set true when the focus theme is changed */
      Eina_Bool    animate : 1; /* set true when the focus highlight animate is enabled */
      Eina_Bool    animate_supported : 1; /* set true when the focus highlight animate is supported by theme */
      Eina_Bool    geometry_changed : 1;
   } focus_highlight;

   Evas_Object *icon;
   const char  *title;
   const char  *icon_name;
   const char  *role;

   Evas_Object *main_menu;

   struct
   {
      const char  *name;
      const char **available_list;
      unsigned int count;
   } profile;
   struct
   {
      int          preferred_rot; /* indicates preferred rotation value, -1 means invalid. */
      int         *rots; /* indicates available rotations */
      unsigned int count; /* number of elements in available rotations */
      Eina_Bool    wm_supported : 1; /* set true when the window manager support window rotation */
      Eina_Bool    use : 1; /* set ture when application use window manager rotation. */
   } wm_rot;

   void *trap_data;

   double       aspect;
   int          size_base_w, size_base_h;
   int          size_step_w, size_step_h;
   int          norender;
   int          modal_count;
   Eina_Bool    urgent : 1;
   Eina_Bool    modal : 1;
   Eina_Bool    demand_attention : 1;
   Eina_Bool    autodel : 1;
   Eina_Bool    constrain : 1;
   Eina_Bool    resizing : 1;
   Eina_Bool    iconified : 1;
   Eina_Bool    withdrawn : 1;
   Eina_Bool    sticky : 1;
   Eina_Bool    fullscreen : 1;
   Eina_Bool    maximized : 1;
   Eina_Bool    skip_focus : 1;
   Eina_Bool    floating : 1;
   Eina_Bool    noblank : 1;
};

static const char SIG_DELETE_REQUEST[] = "delete,request";
static const char SIG_FOCUS_OUT[] = "focus,out"; // deprecated. use "focused" instead.
static const char SIG_FOCUS_IN[] = "focus,in"; // deprecated. use "unfocused" instead.
static const char SIG_MOVED[] = "moved";
static const char SIG_WITHDRAWN[] = "withdrawn";
static const char SIG_ICONIFIED[] = "iconified";
static const char SIG_NORMAL[] = "normal";
static const char SIG_STICK[] = "stick";
static const char SIG_UNSTICK[] = "unstick";
static const char SIG_FULLSCREEN[] = "fullscreen";
static const char SIG_UNFULLSCREEN[] = "unfullscreen";
static const char SIG_MAXIMIZED[] = "maximized";
static const char SIG_UNMAXIMIZED[] = "unmaximized";
static const char SIG_IOERR[] = "ioerr";
static const char SIG_INDICATOR_PROP_CHANGED[] = "indicator,prop,changed";
static const char SIG_ROTATION_CHANGED[] = "rotation,changed";
static const char SIG_PROFILE_CHANGED[] = "profile,changed";
static const char SIG_WM_ROTATION_CHANGED[] = "wm,rotation,changed";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DELETE_REQUEST, ""},
   {SIG_FOCUS_OUT, ""},
   {SIG_FOCUS_IN, ""},
   {SIG_MOVED, ""},
   {SIG_WITHDRAWN, ""},
   {SIG_ICONIFIED, ""},
   {SIG_NORMAL, ""},
   {SIG_STICK, ""},
   {SIG_UNSTICK, ""},
   {SIG_FULLSCREEN, ""},
   {SIG_UNFULLSCREEN, ""},
   {SIG_MAXIMIZED, ""},
   {SIG_UNMAXIMIZED, ""},
   {SIG_IOERR, ""},
   {SIG_INDICATOR_PROP_CHANGED, ""},
   {SIG_ROTATION_CHANGED, ""},
   {SIG_PROFILE_CHANGED, ""},
   {SIG_WM_ROTATION_CHANGED, ""},
   {SIG_WIDGET_FOCUSED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_UNFOCUSED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static Eina_Bool _key_action_return(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"return", _key_action_return},
   {"move", _key_action_move},
   {NULL, NULL}
};

Eina_List *_elm_win_list = NULL;
int _elm_win_deferred_free = 0;

static int _elm_win_count = 0;

static Eina_Bool _elm_win_auto_throttled = EINA_FALSE;

static Ecore_Job *_elm_win_state_eval_job = NULL;

static void
_elm_win_on_resize_obj_changed_size_hints(void *data,
                                          Evas *e,
                                          Evas_Object *obj,
                                          void *event_info);
static void
_elm_win_img_callbacks_del(Evas_Object *obj, Evas_Object *imgobj);

#ifdef HAVE_ELEMENTARY_X
static void _elm_win_xwin_update(Elm_Win_Data *sd);
#endif

EAPI double _elm_startup_time = 0;

static void
_elm_win_first_frame_do(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   double end = ecore_time_unix_get();
   char *first = data;

   switch (*first)
     {
      case 'A': abort();
      case 'E':
      case 'D': exit(-1);
      case 'T': fprintf(stderr, "Startup time: '%f' - '%f' = '%f' sec\n", end, _elm_startup_time, end - _elm_startup_time);
                break;
     }

   evas_event_callback_del_full(e, EVAS_CALLBACK_RENDER_POST, _elm_win_first_frame_do, data);
}

static void
_win_noblank_eval(void)
{
   Eina_List *l;
   Evas_Object *obj;
   int noblanks = 0;

#ifdef HAVE_ELEMENTARY_X
   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     {
        ELM_WIN_DATA_GET(obj, sd);

        if (sd->x.xwin)
          {
             if ((sd->noblank) && (!sd->iconified) && (!sd->withdrawn) &&
                 evas_object_visible_get(obj))
             noblanks++;
          }
     }
   if (noblanks > 0) ecore_x_screensaver_supend();
   else ecore_x_screensaver_resume();
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   // XXX: no wl implementation of this yet - maybe higher up at prop level
#endif
}

static void
_elm_win_state_eval(void *data EINA_UNUSED)
{
   Eina_List *l;
   Evas_Object *obj;
   int _elm_win_count_shown = 0;
   int _elm_win_count_iconified = 0;
   int _elm_win_count_withdrawn = 0;

   _elm_win_state_eval_job = NULL;

   if (_elm_config->auto_norender_withdrawn)
     {
        EINA_LIST_FOREACH(_elm_win_list, l, obj)
          {
             if ((elm_win_withdrawn_get(obj)) ||
                 ((elm_win_iconified_get(obj) &&
                   (_elm_config->auto_norender_iconified_same_as_withdrawn))))
               {
                  if (!evas_object_data_get(obj, "__win_auto_norender"))
                    {
                       Evas *evas = evas_object_evas_get(obj);

                       elm_win_norender_push(obj);
                       evas_object_data_set(obj, "__win_auto_norender", obj);

                       if (_elm_config->auto_flush_withdrawn)
                         {
                            edje_file_cache_flush();
                            edje_collection_cache_flush();
                            evas_image_cache_flush(evas);
                            evas_font_cache_flush(evas);
                         }
                       if (_elm_config->auto_dump_withdrawn)
                         {
                            evas_render_dump(evas);
                         }
                    }
               }
             else
               {
                  if (evas_object_data_get(obj, "__win_auto_norender"))
                    {
                       elm_win_norender_pop(obj);
                       evas_object_data_del(obj, "__win_auto_norender");
                    }
               }
          }
     }
   if (((_elm_config->auto_throttle) &&
        (elm_policy_get(ELM_POLICY_THROTTLE) != ELM_POLICY_THROTTLE_NEVER)) ||
        (elm_policy_get(ELM_POLICY_THROTTLE) == ELM_POLICY_THROTTLE_HIDDEN_ALWAYS))
     {
        if (_elm_win_count == 0)
          {
             if (_elm_win_auto_throttled)
               {
                  ecore_throttle_adjust(-_elm_config->auto_throttle_amount);
                  _elm_win_auto_throttled = EINA_FALSE;
               }
          }
        else
          {
             EINA_LIST_FOREACH(_elm_win_list, l, obj)
               {
                  if (elm_win_withdrawn_get(obj))
                    _elm_win_count_withdrawn++;
                  else if (elm_win_iconified_get(obj))
                    _elm_win_count_iconified++;
                  else if (evas_object_visible_get(obj))
                    _elm_win_count_shown++;
               }
             if (_elm_win_count_shown <= 0)
               {
                  if (!_elm_win_auto_throttled)
                    {
                       ecore_throttle_adjust(_elm_config->auto_throttle_amount);
                       _elm_win_auto_throttled = EINA_TRUE;
                    }
               }
             else
               {
                  if (_elm_win_auto_throttled)
                    {
                       ecore_throttle_adjust(-_elm_config->auto_throttle_amount);
                       _elm_win_auto_throttled = EINA_FALSE;
                    }
               }
          }
     }
   _win_noblank_eval();
}

static void
_elm_win_state_eval_queue(void)
{
   ecore_job_del(_elm_win_state_eval_job);
   _elm_win_state_eval_job = ecore_job_add(_elm_win_state_eval, NULL);
}

// example shot spec (wait 0.1 sec then save as my-window.png):
// ELM_ENGINE="shot:delay=0.1:file=my-window.png"

static double
_shot_delay_get(Elm_Win_Data *sd)
{
   char *p, *pd;
   char *d = strdup(sd->shot.info);

   if (!d) return 0.5;
   for (p = (char *)sd->shot.info; *p; p++)
     {
        if (!strncmp(p, "delay=", 6))
          {
             double v;

             for (pd = d, p += 6; (*p) && (*p != ':'); p++, pd++)
               {
                  *pd = *p;
               }
             *pd = 0;
             v = _elm_atof(d);
             free(d);
             return v;
          }
     }
   free(d);

   return 0.5;
}

static char *
_shot_file_get(Elm_Win_Data *sd)
{
   char *p;
   char *tmp = strdup(sd->shot.info);
   char *repname = NULL;

   if (!tmp) return NULL;

   for (p = (char *)sd->shot.info; *p; p++)
     {
        if (!strncmp(p, "file=", 5))
          {
             strcpy(tmp, p + 5);
             if (!sd->shot.repeat_count) return tmp;
             else
               {
                  char *dotptr = strrchr(tmp, '.');
                  if (dotptr)
                    {
                       size_t size = sizeof(char) * (strlen(tmp) + 16);
                       repname = malloc(size);
                       strncpy(repname, tmp, dotptr - tmp);
                       snprintf(repname + (dotptr - tmp), size -
                                (dotptr - tmp), "%03i",
                                sd->shot.shot_counter + 1);
                       strcat(repname, dotptr);
                       free(tmp);
                       return repname;
                    }
               }
          }
     }
   free(tmp);
   if (!sd->shot.repeat_count) return strdup("out.png");

   repname = malloc(sizeof(char) * 24);
   snprintf(repname, sizeof(char) * 24, "out%03i.png",
            sd->shot.shot_counter + 1);

   return repname;
}

static int
_shot_repeat_count_get(Elm_Win_Data *sd)
{
   char *p, *pd;
   char *d = strdup(sd->shot.info);

   if (!d) return 0;
   for (p = (char *)sd->shot.info; *p; p++)
     {
        if (!strncmp(p, "repeat=", 7))
          {
             int v;

             for (pd = d, p += 7; (*p) && (*p != ':'); p++, pd++)
               {
                  *pd = *p;
               }
             *pd = 0;
             v = atoi(d);
             if (v < 0) v = 0;
             if (v > 1000) v = 999;
             free(d);
             return v;
          }
     }
   free(d);

   return 0;
}

static char *
_shot_key_get(Elm_Win_Data *sd EINA_UNUSED)
{
   return NULL;
}

static char *
_shot_flags_get(Elm_Win_Data *sd EINA_UNUSED)
{
   return NULL;
}

static void
_shot_do(Elm_Win_Data *sd)
{
   Ecore_Evas *ee;
   Evas_Object *o;
   unsigned int *pixels;
   int w, h;
   char *file, *key, *flags;

   ecore_evas_manual_render(sd->ee);
   pixels = (void *)ecore_evas_buffer_pixels_get(sd->ee);
   if (!pixels) return;

   ecore_evas_geometry_get(sd->ee, NULL, NULL, &w, &h);
   if ((w < 1) || (h < 1)) return;

   file = _shot_file_get(sd);
   if (!file) return;

   key = _shot_key_get(sd);
   flags = _shot_flags_get(sd);
   ee = ecore_evas_buffer_new(1, 1);
   o = evas_object_image_add(ecore_evas_get(ee));
   evas_object_image_alpha_set(o, ecore_evas_alpha_get(sd->ee));
   evas_object_image_size_set(o, w, h);
   evas_object_image_data_set(o, pixels);
   if (!evas_object_image_save(o, file, key, flags))
     {
        ERR("Cannot save window to '%s' (key '%s', flags '%s')",
            file, key, flags);
     }
   free(file);
   free(key);
   free(flags);
   ecore_evas_free(ee);
   if (sd->shot.repeat_count) sd->shot.shot_counter++;
}

static Eina_Bool
_shot_delay(void *data)
{
   ELM_WIN_DATA_GET(data, sd);

   _shot_do(sd);
   if (sd->shot.repeat_count)
     {
        int remainshot = (sd->shot.repeat_count - sd->shot.shot_counter);
        if (remainshot > 0) return EINA_TRUE;
     }
   sd->shot.timer = NULL;
   elm_exit();

   return EINA_FALSE;
}

static void
_shot_init(Elm_Win_Data *sd)
{
   if (!sd->shot.info) return;

   sd->shot.repeat_count = _shot_repeat_count_get(sd);
   sd->shot.shot_counter = 0;
}

static void
_shot_handle(Elm_Win_Data *sd)
{
   if (!sd->shot.info) return;

   if (!sd->shot.timer)
     sd->shot.timer = ecore_timer_add(_shot_delay_get(sd), _shot_delay,
                                      sd->obj);
}

/* elm-win specific associate, does the trap while ecore_evas_object_associate()
 * does not.
 */
static Elm_Win_Data *
_elm_win_associate_get(const Ecore_Evas *ee)
{
   Evas_Object *obj = ecore_evas_data_get(ee, "elm_win");
   if (!obj) return NULL;
   ELM_WIN_DATA_GET(obj, sd);
   return sd;
}

/* Interceptors Callbacks */
static void
_elm_win_obj_intercept_raise(void *data, Evas_Object *obj EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);
   TRAP(sd, raise);
}

static void
_elm_win_obj_intercept_lower(void *data, Evas_Object *obj EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);
   TRAP(sd, lower);
}

static void
_elm_win_obj_intercept_stack_above(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, Evas_Object *above EINA_UNUSED)
{
   INF("TODO: %s", __FUNCTION__);
}

static void
_elm_win_obj_intercept_stack_below(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, Evas_Object *below EINA_UNUSED)
{
   INF("TODO: %s", __FUNCTION__);
}

static void
_elm_win_obj_intercept_layer_set(void *data, Evas_Object *obj EINA_UNUSED, int l)
{
   ELM_WIN_DATA_GET(data, sd);
   TRAP(sd, layer_set, l);
}

/* Event Callbacks */

static void
_elm_win_obj_callback_changed_size_hints(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);
   Evas_Coord w, h;

   evas_object_size_hint_min_get(obj, &w, &h);
   TRAP(sd, size_min_set, w, h);

   evas_object_size_hint_max_get(obj, &w, &h);
   if (w < 1) w = -1;
   if (h < 1) h = -1;
   TRAP(sd, size_max_set, w, h);
}
/* end of elm-win specific associate */

static void
_elm_win_move(Ecore_Evas *ee)
{
   Elm_Win_Data *sd = _elm_win_associate_get(ee);
   int x, y;

   if (!sd) return;

   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   sd->screen.x = x;
   sd->screen.y = y;
   evas_object_smart_callback_call(sd->obj, SIG_MOVED, NULL);
   evas_nochange_push(evas_object_evas_get(sd->obj));
   evas_object_move(sd->obj, x, y);
   evas_nochange_pop(evas_object_evas_get(sd->obj));
}

static void
_elm_win_resize_job(void *data)
{
   ELM_WIN_DATA_GET(data, sd);
   int w, h;

   sd->deferred_resize_job = NULL;
   ecore_evas_request_geometry_get(sd->ee, NULL, NULL, &w, &h);
   if (sd->constrain)
     {
        int sw, sh;
        ecore_evas_screen_geometry_get(sd->ee, NULL, NULL, &sw, &sh);
        w = MIN(w, sw);
        h = MIN(h, sh);
     }

   if (sd->frame_obj)
     {
        int fx, fy, fw, fh;

        evas_output_framespace_get(sd->evas, &fx, &fy, &fw, &fh);
        evas_object_move(sd->frame_obj, -fx, -fy);
        evas_object_resize(sd->frame_obj, w + fw, h + fh);
     }

   evas_object_resize(sd->obj, w, h);
   evas_object_resize(sd->layout, w, h);
}

static void
_elm_win_resize(Ecore_Evas *ee)
{
   Elm_Win_Data *sd = _elm_win_associate_get(ee);
   if (!sd) return;

   ecore_job_del(sd->deferred_resize_job);
   sd->deferred_resize_job = ecore_job_add(_elm_win_resize_job, sd->obj);
}

static void
_elm_win_mouse_in(Ecore_Evas *ee)
{
   Elm_Win_Data *sd = _elm_win_associate_get(ee);
   if (!sd) return;

   if (sd->resizing) sd->resizing = EINA_FALSE;
}

static void
_elm_win_focus_highlight_reconfigure_job_stop(Elm_Win_Data *sd)
{
   ELM_SAFE_FREE(sd->focus_highlight.reconf_job, ecore_job_del);
}

static void
_elm_win_focus_highlight_visible_set(Elm_Win_Data *sd,
                                     Eina_Bool visible)
{
   Evas_Object *fobj = sd->focus_highlight.fobj;
   if (!fobj) return;

   if (visible)
     {
        evas_object_show(fobj);
        edje_object_signal_emit(fobj, "elm,action,focus,show", "elm");
     }
   else
     {
        edje_object_signal_emit(fobj, "elm,action,focus,hide", "elm");
     }
}

static void
_elm_win_focus_highlight_anim_setup(Elm_Win_Data *sd,
                                    Evas_Object *obj)
{
   Evas_Coord tx, ty, tw, th;
   Evas_Coord px, py, pw, ph;
   Edje_Message_Int_Set *m;
   Evas_Object *target = sd->focus_highlight.cur.target;

   evas_object_geometry_get(obj, &px, &py, &pw, &ph);
   elm_widget_focus_highlight_geometry_get(target, &tx, &ty, &tw, &th);
   evas_object_move(obj, tx, ty);
   evas_object_resize(obj, tw, th);

   if ((px == tx) && (py == ty) && (pw == tw) && (ph == th)) return;

   if (!_elm_config->focus_highlight_clip_disable)
     evas_object_clip_unset(obj);

   m = alloca(sizeof(*m) + (sizeof(int) * 8));
   m->count = 8;
   m->val[0] = px - tx;
   m->val[1] = py - ty;
   m->val[2] = pw;
   m->val[3] = ph;
   m->val[4] = 0;
   m->val[5] = 0;
   m->val[6] = tw;
   m->val[7] = th;
   edje_object_message_send(obj, EDJE_MESSAGE_INT_SET, 1, m);
}

static void
_elm_win_focus_highlight_simple_setup(Elm_Win_Data *sd,
                                      Evas_Object *obj)
{
   Evas_Object *clip, *target = sd->focus_highlight.cur.target;
   Evas_Coord x, y, w, h;

   elm_widget_focus_highlight_geometry_get(target, &x, &y, &w, &h);

   evas_object_move(obj, x, y);
   evas_object_resize(obj, w, h);

   if (!_elm_config->focus_highlight_clip_disable)
     {
        clip = evas_object_clip_get(target);
        if (clip) evas_object_clip_set(obj, clip);
     }

   edje_object_signal_emit(obj, "elm,state,anim,stop", "elm");
}

static void
_elm_win_focus_prev_target_del(void *data,
                               Evas *e EINA_UNUSED,
                               Evas_Object *obj EINA_UNUSED,
                               void *event_info EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);
   sd->focus_highlight.prev.target = NULL;
}

static void
_elm_win_focus_highlight_reconfigure_job(void *data)
{
   ELM_WIN_DATA_GET(data, sd);
   Evas_Object *target = sd->focus_highlight.cur.target;
   Evas_Object *previous = sd->focus_highlight.prev.target;
   Evas_Object *fobj = sd->focus_highlight.fobj;
   Eina_Bool visible_changed;
   Eina_Bool common_visible;
   const char *sig = NULL;
   const char *focus_style_target = NULL;
   const char *focus_style_previous = NULL;

   _elm_win_focus_highlight_reconfigure_job_stop(sd);

   visible_changed = (sd->focus_highlight.cur.visible !=
                      sd->focus_highlight.prev.visible);

   if ((target == previous) && (!visible_changed) &&
       (!sd->focus_highlight.geometry_changed) &&
       (!sd->focus_highlight.theme_changed))
     return;

   if ((previous) && (sd->focus_highlight.prev.in_theme))
     {
        evas_object_event_callback_del_full
           (previous, EVAS_CALLBACK_DEL, _elm_win_focus_prev_target_del, data);
        elm_widget_signal_emit
           (previous, "elm,action,focus_highlight,hide", "elm");
     }

   if (!target)
     common_visible = EINA_FALSE;
   else if (sd->focus_highlight.cur.in_theme)
     {
        common_visible = EINA_FALSE;
        if (sd->focus_highlight.cur.visible)
          sig = "elm,action,focus_highlight,show";
        else
          sig = "elm,action,focus_highlight,hide";
     }
   else
     common_visible = sd->focus_highlight.cur.visible;

   if (sig)
     elm_widget_signal_emit(target, sig, "elm");

   if ((!target) || (!common_visible) || (sd->focus_highlight.cur.in_theme))
     goto the_end;

   if (previous)
     focus_style_previous = elm_widget_focus_highlight_style_get(previous);
   focus_style_target = elm_widget_focus_highlight_style_get(target);

   if (sd->focus_highlight.theme_changed ||
       (focus_style_target != focus_style_previous))
     {
        const char *str;

        if (focus_style_target)
          str = focus_style_target;
        else if (sd->focus_highlight.style)
          str = sd->focus_highlight.style;
        else
          str = "default";

        elm_widget_theme_object_set
          (sd->obj, fobj, "focus_highlight", "top", str);
        sd->focus_highlight.theme_changed = EINA_FALSE;

        if (sd->focus_highlight.animate)
          {
             str = edje_object_data_get(sd->focus_highlight.fobj, "animate");
             sd->focus_highlight.animate_supported = ((str) && (!strcmp(str, "on")));
          }
        else
          sd->focus_highlight.animate_supported = EINA_FALSE;
     }

   if ((sd->focus_highlight.animate_supported) && (previous) &&
       (!sd->focus_highlight.prev.in_theme))
     _elm_win_focus_highlight_anim_setup(sd, fobj);
   else
     _elm_win_focus_highlight_simple_setup(sd, fobj);
   evas_object_raise(fobj);

the_end:
   _elm_win_focus_highlight_visible_set(sd, common_visible);
   sd->focus_highlight.geometry_changed = EINA_FALSE;
   sd->focus_highlight.prev = sd->focus_highlight.cur;
   evas_object_event_callback_add
     (sd->focus_highlight.prev.target,
      EVAS_CALLBACK_DEL, _elm_win_focus_prev_target_del, data);
}

static void
_elm_win_focus_highlight_reconfigure_job_start(Elm_Win_Data *sd)
{
   ecore_job_del(sd->focus_highlight.reconf_job);

   sd->focus_highlight.reconf_job = ecore_job_add(
       _elm_win_focus_highlight_reconfigure_job, sd->obj);
}

static void
_elm_win_focus_in(Ecore_Evas *ee)
{
   Elm_Win_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;
   unsigned int order = 0;

   if ((!sd) || (sd->modal_count)) return;

   obj = sd->obj;

   _elm_widget_top_win_focused_set(obj, EINA_TRUE);
   if (!elm_widget_focus_order_get(obj)
       || (obj == elm_widget_newest_focus_order_get(obj, &order, EINA_TRUE)))
     {
        elm_widget_focus_steal(obj);
     }
   else
     elm_widget_focus_restore(obj);
   evas_object_smart_callback_call(obj, SIG_FOCUS_IN, NULL);
   sd->focus_highlight.cur.visible = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
   if (sd->frame_obj)
     {
        edje_object_signal_emit(sd->frame_obj, "elm,action,focus", "elm");
     }

   /* do nothing */
   /* else if (sd->img_obj) */
   /*   { */
   /*   } */
}

static void
_elm_win_focus_out(Ecore_Evas *ee)
{
   Elm_Win_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;

   if (!sd) return;

   obj = sd->obj;

   elm_object_focus_set(obj, EINA_FALSE);
   _elm_widget_top_win_focused_set(obj, EINA_FALSE);
   evas_object_smart_callback_call(obj, SIG_FOCUS_OUT, NULL);
   sd->focus_highlight.cur.visible = EINA_FALSE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
   if (sd->frame_obj)
     {
        edje_object_signal_emit(sd->frame_obj, "elm,action,unfocus", "elm");
     }

   /* access */
   _elm_access_object_highlight_disable(evas_object_evas_get(obj));

   /* do nothing */
   /* if (sd->img_obj) */
   /*   { */
   /*   } */
}

static void
_elm_win_available_profiles_del(Elm_Win_Data *sd)
{
   if (!sd->profile.available_list) return;

   unsigned int i;
   for (i = 0; i < sd->profile.count; i++)
     ELM_SAFE_FREE(sd->profile.available_list[i], eina_stringshare_del);
   sd->profile.count = 0;
   ELM_SAFE_FREE(sd->profile.available_list, free);
}

static void
_elm_win_profile_del(Elm_Win_Data *sd)
{
   ELM_SAFE_FREE(sd->profile.name, eina_stringshare_del);
}

static Eina_Bool
_internal_elm_win_profile_set(Elm_Win_Data *sd, const char *profile)
{
   Eina_Bool changed = EINA_FALSE;
   if (profile)
     {
        if (sd->profile.name)
          {
             if (strcmp(sd->profile.name, profile) != 0)
               {
                  eina_stringshare_replace(&(sd->profile.name), profile);
                  changed = EINA_TRUE;
               }
          }
        else
          {
             sd->profile.name = eina_stringshare_add(profile);
             changed = EINA_TRUE;
          }
     }
   else
     _elm_win_profile_del(sd);

   return changed;
}

static void
_elm_win_profile_update(Elm_Win_Data *sd)
{
   if (sd->profile.available_list)
     {
        Eina_Bool found = EINA_FALSE;
        if (sd->profile.name)
          {
             unsigned int i;
             for (i = 0; i < sd->profile.count; i++)
               {
                  if (!strcmp(sd->profile.name,
                              sd->profile.available_list[i]))
                    {
                       found = EINA_TRUE;
                       break;
                    }
               }
          }

        /* If current profile is not present in an available profiles,
         * change current profile to the 1st element of an array.
         */
        if (!found)
          _internal_elm_win_profile_set(sd, sd->profile.available_list[0]);
     }

   _elm_config_profile_set(sd->profile.name);

   /* update sub ee */
   Ecore_Evas *ee2;
   Eina_List *sub, *l = NULL;

   sub = ecore_evas_sub_ecore_evas_list_get(sd->ee);
   EINA_LIST_FOREACH(sub, l, ee2)
     ecore_evas_window_profile_set(ee2, sd->profile.name);

   evas_object_smart_callback_call(sd->obj, SIG_PROFILE_CHANGED, NULL);
}

static void
_elm_win_state_change(Ecore_Evas *ee)
{
   Elm_Win_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;
   Eina_Bool ch_withdrawn = EINA_FALSE;
   Eina_Bool ch_sticky = EINA_FALSE;
   Eina_Bool ch_iconified = EINA_FALSE;
   Eina_Bool ch_fullscreen = EINA_FALSE;
   Eina_Bool ch_maximized = EINA_FALSE;
   Eina_Bool ch_profile = EINA_FALSE;
   Eina_Bool ch_wm_rotation = EINA_FALSE;
   const char *profile;

   if (!sd) return;

   obj = sd->obj;

   if (sd->withdrawn != ecore_evas_withdrawn_get(sd->ee))
     {
        sd->withdrawn = ecore_evas_withdrawn_get(sd->ee);
        ch_withdrawn = EINA_TRUE;
     }
   if (sd->sticky != ecore_evas_sticky_get(sd->ee))
     {
        sd->sticky = ecore_evas_sticky_get(sd->ee);
        ch_sticky = EINA_TRUE;
     }
   if (sd->iconified != ecore_evas_iconified_get(sd->ee))
     {
        sd->iconified = ecore_evas_iconified_get(sd->ee);
        ch_iconified = EINA_TRUE;
     }
   if (sd->fullscreen != ecore_evas_fullscreen_get(sd->ee))
     {
        sd->fullscreen = ecore_evas_fullscreen_get(sd->ee);
        ch_fullscreen = EINA_TRUE;
     }
   if (sd->maximized != ecore_evas_maximized_get(sd->ee))
     {
        sd->maximized = ecore_evas_maximized_get(sd->ee);
        ch_maximized = EINA_TRUE;
     }

   profile = ecore_evas_window_profile_get(sd->ee);
   ch_profile = _internal_elm_win_profile_set(sd, profile);

   if (sd->wm_rot.use)
     {
        if (sd->rot != ecore_evas_rotation_get(sd->ee))
          {
             sd->rot = ecore_evas_rotation_get(sd->ee);
             ch_wm_rotation = EINA_TRUE;
          }
     }

   _elm_win_state_eval_queue();

   if ((ch_withdrawn) || (ch_iconified))
     {
        if (sd->withdrawn)
          evas_object_smart_callback_call(obj, SIG_WITHDRAWN, NULL);
        else if (sd->iconified)
          evas_object_smart_callback_call(obj, SIG_ICONIFIED, NULL);
        else
          evas_object_smart_callback_call(obj, SIG_NORMAL, NULL);
     }
   if (ch_sticky)
     {
        if (sd->sticky)
          evas_object_smart_callback_call(obj, SIG_STICK, NULL);
        else
          evas_object_smart_callback_call(obj, SIG_UNSTICK, NULL);
     }
   if (ch_fullscreen)
     {
        if (sd->fullscreen)
          evas_object_smart_callback_call(obj, SIG_FULLSCREEN, NULL);
        else
          evas_object_smart_callback_call(obj, SIG_UNFULLSCREEN, NULL);
     }
   if (ch_maximized)
     {
        if (sd->maximized)
          evas_object_smart_callback_call(obj, SIG_MAXIMIZED, NULL);
        else
          evas_object_smart_callback_call(obj, SIG_UNMAXIMIZED, NULL);
     }
   if (ch_profile)
     {
        _elm_win_profile_update(sd);
     }
   if (ch_wm_rotation)
     {
        evas_object_size_hint_min_set(obj, -1, -1);
        evas_object_size_hint_max_set(obj, -1, -1);
#ifdef HAVE_ELEMENTARY_X
        _elm_win_xwin_update(sd);
#endif
        elm_widget_orientation_set(obj, sd->rot);
        evas_object_smart_callback_call(obj, SIG_ROTATION_CHANGED, NULL);
        evas_object_smart_callback_call(obj, SIG_WM_ROTATION_CHANGED, NULL);
     }
}

EOLIAN static Eina_Bool
_elm_win_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Win_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_win_elm_widget_focus_next(Eo *obj, Elm_Win_Data *_pd EINA_UNUSED, Elm_Focus_Direction dir, Evas_Object **next)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   /* Focus chain */
   if (wd->subobjs)
     {
        if (!(items = elm_widget_focus_custom_chain_get(obj)))
          {
             items = wd->subobjs;
             if (!items)
               return EINA_FALSE;
          }
        list_data_get = eina_list_data_get;

        elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);

        if (*next) return EINA_TRUE;
     }
   *next = (Evas_Object *)obj;
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_win_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Win_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_win_elm_widget_focus_direction(Eo *obj, Elm_Win_Data *_pd EINA_UNUSED, const Evas_Object *base, double degree, Evas_Object **direction, double *weight)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   /* Focus chain */
   if (wd->subobjs)
     {
        if (!(items = elm_widget_focus_custom_chain_get(obj)))
          items = wd->subobjs;

        list_data_get = eina_list_data_get;

        return elm_widget_focus_list_direction_get
                 (obj, base, items, list_data_get, degree, direction, weight);
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_win_elm_widget_on_focus(Eo *obj, Elm_Win_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_on_focus());
   if (!int_ret) return EINA_TRUE;

   if (sd->img_obj)
     evas_object_focus_set(sd->img_obj, elm_widget_focus_get(obj));
   else
     evas_object_focus_set(obj, elm_widget_focus_get(obj));

   return EINA_TRUE;
}

static Eina_Bool
_key_action_return(Evas_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   const char *dir = params;

   if (!strcmp(dir, "previous"))
     elm_widget_focus_cycle(obj, ELM_FOCUS_PREVIOUS);
   else if (!strcmp(dir, "next"))
     elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
   else if (!strcmp(dir, "left"))
     elm_widget_focus_cycle(obj, ELM_FOCUS_LEFT);
   else if (!strcmp(dir, "right"))
     elm_widget_focus_cycle(obj, ELM_FOCUS_RIGHT);
   else if (!strcmp(dir, "up"))
     elm_widget_focus_cycle(obj, ELM_FOCUS_UP);
   else if (!strcmp(dir, "down"))
     elm_widget_focus_cycle(obj, ELM_FOCUS_DOWN);
   else return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_win_elm_widget_event(Eo *obj, Elm_Win_Data *_pd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;
   Evas_Event_Key_Down *ev = event_info;

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_deferred_ecore_evas_free(void *data)
{
   ecore_evas_free(data);
   _elm_win_deferred_free--;
}

EOLIAN static void
_elm_win_evas_object_smart_show(Eo *obj, Elm_Win_Data *sd)
{
   if (sd->modal_count) return;
   const Eina_List *l;
   Evas_Object *current;

   if (!evas_object_visible_get(obj))
     _elm_win_state_eval_queue();
   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());

   if ((sd->modal) && (!evas_object_visible_get(obj)))
     {
        INCREMENT_MODALITY()
     }

   TRAP(sd, show);

   if (sd->shot.info) _shot_handle(sd);
}

EOLIAN static void
_elm_win_evas_object_smart_hide(Eo *obj, Elm_Win_Data *sd)
{
   if (sd->modal_count) return;
   const Eina_List *l;
   Evas_Object *current;

   if (evas_object_visible_get(obj))
     _elm_win_state_eval_queue();
   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());

   if ((sd->modal) && (evas_object_visible_get(obj)))
     {
        DECREMENT_MODALITY()
     }

   TRAP(sd, hide);

   if (sd->frame_obj)
     {
        evas_object_hide(sd->frame_obj);
     }
   if (sd->img_obj)
     {
        evas_object_hide(sd->img_obj);
     }
   if (sd->pointer.obj)
     {
        evas_object_hide(sd->pointer.obj);
        ecore_evas_hide(sd->pointer.ee);
     }
}

static void
_elm_win_on_parent_del(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   if (obj == sd->parent) sd->parent = NULL;
}

static void
_elm_win_focus_target_move(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   sd->focus_highlight.geometry_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_focus_target_resize(void *data,
                             Evas *e EINA_UNUSED,
                             Evas_Object *obj EINA_UNUSED,
                             void *event_info EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   sd->focus_highlight.geometry_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_focus_target_del(void *data,
                          Evas *e EINA_UNUSED,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   sd->focus_highlight.cur.target = NULL;

   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static Evas_Object *
_elm_win_focus_target_get(Evas_Object *obj)
{
   Evas_Object *o = obj;

   do
     {
        if (elm_widget_is(o))
          {
             if (!elm_widget_highlight_ignore_get(o))
               break;
             o = elm_widget_parent_get(o);
             if (!o)
               o = evas_object_smart_parent_get(o);
          }
        else
          {
             o = elm_widget_parent_widget_get(o);
             if (!o)
               o = evas_object_smart_parent_get(o);
          }
     }
   while (o);

   return o;
}

static void
_elm_win_focus_target_callbacks_add(Elm_Win_Data *sd)
{
   Evas_Object *obj = sd->focus_highlight.cur.target;
   if (!obj) return;

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOVE, _elm_win_focus_target_move, sd->obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _elm_win_focus_target_resize, sd->obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_DEL, _elm_win_focus_target_del, sd->obj);
}

static void
_elm_win_focus_target_callbacks_del(Elm_Win_Data *sd)
{
   Evas_Object *obj = sd->focus_highlight.cur.target;

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_MOVE, _elm_win_focus_target_move, sd->obj);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_win_focus_target_resize, sd->obj);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_DEL, _elm_win_focus_target_del, sd->obj);
}

static void
_elm_win_object_focus_in(void *data,
                         Evas *e EINA_UNUSED,
                         void *event_info)
{
   Evas_Object *obj = event_info, *target;
   ELM_WIN_DATA_GET(data, sd);

   if (sd->focus_highlight.cur.target == obj)
     return;

   target = _elm_win_focus_target_get(obj);
   sd->focus_highlight.cur.target = target;
   if (target && elm_widget_highlight_in_theme_get(target))
     sd->focus_highlight.cur.in_theme = EINA_TRUE;
   else
     _elm_win_focus_target_callbacks_add(sd);

   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_object_focus_out(void *data,
                          Evas *e EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   if (!sd->focus_highlight.cur.target)
     return;

   if (!sd->focus_highlight.cur.in_theme)
     _elm_win_focus_target_callbacks_del(sd);

   sd->focus_highlight.cur.target = NULL;
   sd->focus_highlight.cur.in_theme = EINA_FALSE;

   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_focus_highlight_shutdown(Elm_Win_Data *sd)
{
   _elm_win_focus_highlight_reconfigure_job_stop(sd);
   if (sd->focus_highlight.cur.target)
     {
        elm_widget_signal_emit(sd->focus_highlight.cur.target,
                               "elm,action,focus_highlight,hide", "elm");
        _elm_win_focus_target_callbacks_del(sd);
        sd->focus_highlight.cur.target = NULL;
     }
   ELM_SAFE_FREE(sd->focus_highlight.fobj, evas_object_del);

   evas_event_callback_del_full
     (sd->evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
     _elm_win_object_focus_in, sd->obj);
   evas_event_callback_del_full
     (sd->evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
     _elm_win_object_focus_out, sd->obj);
}

static void
_win_img_hide(void *data,
              Evas *e EINA_UNUSED,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   elm_widget_focus_hide_handle(data);
}

static void
_win_img_mouse_up(void *data,
                  Evas *e EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   if (!(ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD))
     elm_widget_focus_mouse_up_handle(data);
}

static void
_win_img_focus_in(void *data,
                  Evas *e EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   elm_widget_focus_steal(data);
}

static void
_win_img_focus_out(void *data,
                   Evas *e EINA_UNUSED,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   elm_widget_focused_object_clear(data);
}

static void
_elm_win_on_img_obj_del(void *data,
                        Evas *e EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);
   _elm_win_img_callbacks_del(sd->obj, sd->img_obj);
   sd->img_obj = NULL;
}

static void
_elm_win_img_callbacks_del(Evas_Object *obj, Evas_Object *imgobj)
{
   evas_object_event_callback_del_full
     (imgobj, EVAS_CALLBACK_DEL, _elm_win_on_img_obj_del, obj);
   evas_object_event_callback_del_full
     (imgobj, EVAS_CALLBACK_HIDE, _win_img_hide, obj);
   evas_object_event_callback_del_full
     (imgobj, EVAS_CALLBACK_MOUSE_UP, _win_img_mouse_up, obj);
   evas_object_event_callback_del_full
     (imgobj, EVAS_CALLBACK_FOCUS_IN, _win_img_focus_in, obj);
   evas_object_event_callback_del_full
     (imgobj, EVAS_CALLBACK_FOCUS_OUT, _win_img_focus_out, obj);
}

EOLIAN static void
_elm_win_evas_object_smart_del(Eo *obj, Elm_Win_Data *sd)
{
   const Eina_List *l;
   Evas_Object *current;

   if ((sd->modal) && (evas_object_visible_get(obj)))
     {
       DECREMENT_MODALITY()
     }

   if ((sd->modal) && (sd->modal_count > 0)) 
     ERR("Deleted modal win was blocked by another modal win which was created after creation of that win.");

   evas_object_event_callback_del_full(sd->layout,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _elm_win_on_resize_obj_changed_size_hints,
                                       obj);
   evas_object_del(sd->box);
   evas_object_del(sd->layout);

   /* NB: child deletion handled by parent's smart del */

   if ((trap) && (trap->del))
     trap->del(sd->trap_data, obj);

   if (sd->parent)
     {
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_DEL, _elm_win_on_parent_del, obj);
        sd->parent = NULL;
     }

   if (sd->autodel_clear) *(sd->autodel_clear) = -1;

   _elm_win_list = eina_list_remove(_elm_win_list, obj);
   _elm_win_count--;
   _elm_win_state_eval_queue();

   if (sd->ee)
     {
        ecore_evas_callback_delete_request_set(sd->ee, NULL);
        ecore_evas_callback_resize_set(sd->ee, NULL);
     }

   ecore_job_del(sd->deferred_resize_job);
   ecore_job_del(sd->deferred_child_eval_job);
   eina_stringshare_del(sd->shot.info);
   ecore_timer_del(sd->shot.timer);

#ifdef HAVE_ELEMENTARY_X
   ecore_event_handler_del(sd->x.client_message_handler);
   ecore_event_handler_del(sd->x.property_handler);
#endif

   if (sd->img_obj)
     {
        _elm_win_img_callbacks_del(obj, sd->img_obj);
        sd->img_obj = NULL;
     }
   else
     {
        if (sd->ee)
          {
             ecore_job_add(_deferred_ecore_evas_free, sd->ee);
             _elm_win_deferred_free++;
          }
     }

   _elm_win_focus_highlight_shutdown(sd);
   eina_stringshare_del(sd->focus_highlight.style);

   eina_stringshare_del(sd->title);
   eina_stringshare_del(sd->icon_name);
   eina_stringshare_del(sd->role);
   evas_object_del(sd->icon);
   evas_object_del(sd->main_menu);

   _elm_win_profile_del(sd);
   _elm_win_available_profiles_del(sd);

   /* Don't let callback in the air that point to sd */
   ecore_evas_callback_delete_request_set(sd->ee, NULL);
   ecore_evas_callback_resize_set(sd->ee, NULL);
   ecore_evas_callback_mouse_in_set(sd->ee, NULL);
   ecore_evas_callback_focus_in_set(sd->ee, NULL);
   ecore_evas_callback_focus_out_set(sd->ee, NULL);
   ecore_evas_callback_move_set(sd->ee, NULL);
   ecore_evas_callback_state_change_set(sd->ee, NULL);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());

   if ((!_elm_win_list) &&
       (elm_policy_get(ELM_POLICY_QUIT) == ELM_POLICY_QUIT_LAST_WINDOW_CLOSED))
     {
        edje_file_cache_flush();
        edje_collection_cache_flush();
        evas_image_cache_flush(evas_object_evas_get(obj));
        evas_font_cache_flush(evas_object_evas_get(obj));
        elm_exit();
     }
}

static void
_elm_win_obj_intercept_show(void *data,
                            Evas_Object *obj)
{
   ELM_WIN_DATA_GET(data, sd);

   // this is called to make sure all smart containers have calculated their
   // sizes BEFORE we show the window to make sure it initially appears at
   // our desired size (ie min size is known first)
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   if (sd->frame_obj)
     {
        evas_object_show(sd->frame_obj);
     }
   if (sd->img_obj)
     {
        evas_object_show(sd->img_obj);
     }
   if (sd->pointer.obj)
     {
        ecore_evas_show(sd->pointer.ee);
        evas_object_show(sd->pointer.obj);
     }
   evas_object_show(obj);
}

EOLIAN static void
_elm_win_evas_object_smart_move(Eo *obj, Elm_Win_Data *sd, Evas_Coord x, Evas_Coord y)
{
   if (sd->img_obj)
     {
        if ((x != sd->screen.x) || (y != sd->screen.y))
          {
             sd->screen.x = x;
             sd->screen.y = y;
             evas_object_smart_callback_call(obj, SIG_MOVED, NULL);
          }
        return;
     }
   else
     {
        TRAP(sd, move, x, y);
        if (!ecore_evas_override_get(sd->ee))  return;
     }

   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   if (ecore_evas_override_get(sd->ee))
     {
        sd->screen.x = x;
        sd->screen.y = y;
        evas_object_smart_callback_call(obj, SIG_MOVED, NULL);
     }
   if (sd->frame_obj)
     {
        /* FIXME: We should update ecore_wl_window_location here !! */
        sd->screen.x = x;
        sd->screen.y = y;
     }
   if (sd->img_obj)
     {
        sd->screen.x = x;
        sd->screen.y = y;
     }
}

EOLIAN static void
_elm_win_evas_object_smart_resize(Eo *obj, Elm_Win_Data *sd, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   if (sd->img_obj)
     {
        if (sd->constrain)
          {
             int sw, sh;

             ecore_evas_screen_geometry_get(sd->ee, NULL, NULL, &sw, &sh);
             w = MIN(w, sw);
             h = MIN(h, sh);
          }
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        evas_object_image_size_set(sd->img_obj, w, h);
     }

   TRAP(sd, resize, w, h);
}

static void
_elm_win_delete_request(Ecore_Evas *ee)
{
   Elm_Win_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;

   if (!sd) return;

   obj = sd->obj;

   int autodel = sd->autodel;
   sd->autodel_clear = &autodel;
   evas_object_ref(obj);
   evas_object_smart_callback_call(obj, SIG_DELETE_REQUEST, NULL);
   // FIXME: if above callback deletes - then the below will be invalid
   if (autodel) evas_object_del(obj);
   else sd->autodel_clear = NULL;
   evas_object_unref(obj);
}

Ecore_X_Window
_elm_ee_xwin_get(const Ecore_Evas *ee)
{
#ifdef HAVE_ELEMENTARY_X
   const char *engine_name;
   if (!ee) return 0;

   engine_name = ecore_evas_engine_name_get(ee);
   if (EINA_UNLIKELY(!engine_name)) return 0;

   if (!strcmp(engine_name, ELM_SOFTWARE_X11))
     {
        return ecore_evas_software_x11_window_get(ee);
     }
   else if (!strcmp(engine_name, ELM_OPENGL_X11))
     {
        return ecore_evas_gl_x11_window_get(ee);
     }
#else
   (void)ee;
#endif
   return 0;
}

#ifdef HAVE_ELEMENTARY_X
static void
_internal_elm_win_xwindow_get(Elm_Win_Data *sd)
{
   sd->x.xwin = _elm_ee_xwin_get(sd->ee);
}
#endif

Ecore_Wl_Window *
_elm_ee_wlwin_get(const Ecore_Evas *ee)
{
#ifdef HAVE_ELEMENTARY_WAYLAND
   const char *engine_name;

   if (!ee) return NULL;

   engine_name = ecore_evas_engine_name_get(ee);
   if (EINA_UNLIKELY(!engine_name)) return NULL;

   if ((!strcmp(engine_name, ELM_WAYLAND_SHM)) ||
       (!strcmp(engine_name, ELM_WAYLAND_EGL)))
     {
        return ecore_evas_wayland_window_get(ee);
     }
#else
   (void)ee;
#endif
   return NULL;
}

#ifdef HAVE_ELEMENTARY_WAYLAND
static void
_elm_win_wlwindow_get(Elm_Win_Data *sd)
{
   sd->wl.win = _elm_ee_wlwin_get(sd->ee);
}
#endif

#ifdef HAVE_ELEMENTARY_X
static void
_elm_win_xwin_update(Elm_Win_Data *sd)
{
   const char *s;

   _internal_elm_win_xwindow_get(sd);
   if (sd->parent)
     {
        ELM_WIN_DATA_GET(sd->parent, sdp);
        if (sdp)
          {
             if (sd->x.xwin)
               ecore_x_icccm_transient_for_set(sd->x.xwin, sdp->x.xwin);
          }
     }

   if (!sd->x.xwin) return;  /* nothing more to do */

   s = sd->title;
   if (!s) s = _elm_appname;
   if (!s) s = "";
   if (sd->icon_name) s = sd->icon_name;
   ecore_x_icccm_icon_name_set(sd->x.xwin, s);
   ecore_x_netwm_icon_name_set(sd->x.xwin, s);

   s = sd->role;
   if (s) ecore_x_icccm_window_role_set(sd->x.xwin, s);

   // set window icon
   if (sd->icon)
     {
        void *data;

        data = evas_object_image_data_get(sd->icon, EINA_FALSE);
        if (data)
          {
             Ecore_X_Icon ic;
             int w = 0, h = 0, stride, x, y;
             unsigned char *p;
             unsigned int *p2;

             evas_object_image_size_get(sd->icon, &w, &h);
             stride = evas_object_image_stride_get(sd->icon);
             if ((w > 0) && (h > 0) &&
                 (stride >= (int)(w * sizeof(unsigned int))))
               {
                  ic.width = w;
                  ic.height = h;
                  ic.data = malloc(w * h * sizeof(unsigned int));

                  if (ic.data)
                    {
                       p = (unsigned char *)data;
                       p2 = (unsigned int *)ic.data;
                       for (y = 0; y < h; y++)
                         {
                            for (x = 0; x < w; x++)
                              {
                                 *p2 = *((unsigned int *)p);
                                 p += sizeof(unsigned int);
                                 p2++;
                              }
                            p += (stride - (w * sizeof(unsigned int)));
                         }
                       ecore_x_netwm_icons_set(sd->x.xwin, &ic, 1);
                       free(ic.data);
                    }
               }
             evas_object_image_data_set(sd->icon, data);
          }
     }

   switch (sd->type)
     {
      case ELM_WIN_BASIC:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_NORMAL);
        break;

      case ELM_WIN_DIALOG_BASIC:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_DIALOG);
        break;

      case ELM_WIN_DESKTOP:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_DESKTOP);
        break;

      case ELM_WIN_DOCK:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_DOCK);
        break;

      case ELM_WIN_TOOLBAR:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_TOOLBAR);
        break;

      case ELM_WIN_MENU:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_MENU);
        break;

      case ELM_WIN_UTILITY:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_UTILITY);
        break;

      case ELM_WIN_SPLASH:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_SPLASH);
        break;

      case ELM_WIN_DROPDOWN_MENU:
        ecore_x_netwm_window_type_set
          (sd->x.xwin, ECORE_X_WINDOW_TYPE_DROPDOWN_MENU);
        break;

      case ELM_WIN_POPUP_MENU:
        ecore_x_netwm_window_type_set
          (sd->x.xwin, ECORE_X_WINDOW_TYPE_POPUP_MENU);
        break;

      case ELM_WIN_TOOLTIP:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_TOOLTIP);
        break;

      case ELM_WIN_NOTIFICATION:
        ecore_x_netwm_window_type_set
          (sd->x.xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);
        break;

      case ELM_WIN_COMBO:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_COMBO);
        break;

      case ELM_WIN_DND:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_DND);
        break;

      default:
        break;
     }
   ecore_x_e_virtual_keyboard_state_set
     (sd->x.xwin, (Ecore_X_Virtual_Keyboard_State)sd->kbdmode);
   if (sd->indmode == ELM_WIN_INDICATOR_SHOW)
     ecore_x_e_illume_indicator_state_set
       (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_STATE_ON);
   else if (sd->indmode == ELM_WIN_INDICATOR_HIDE)
     ecore_x_e_illume_indicator_state_set
       (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_STATE_OFF);
}

#endif

/**
  * @internal
  *
  * Resize the window according to window layout's min and weight.
  * If the window layout's weight is 0.0, the window max is limited to layout's
  * min size.
  *
  * This is called when the window layout's weight hint is changed or when the
  * window is rotated.
  *
  * @param obj window object
  */
static void
_elm_win_resize_objects_eval(Evas_Object *obj)
{
   ELM_WIN_DATA_GET(obj, sd);
   Evas_Coord w, h, minw, minh, maxw, maxh;
   double wx, wy;

   evas_object_size_hint_min_get(sd->layout, &minw, &minh);
   if (minw < 1) minw = 1;
   if (minh < 1) minh = 1;

   evas_object_size_hint_weight_get(sd->layout, &wx, &wy);
   if (!wx) maxw = minw;
   else maxw = 32767;
   if (!wy) maxh = minh;
   else maxh = 32767;

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if (w > maxw) w = maxw;
   if (h > maxh) h = maxh;
   evas_object_resize(obj, w, h);
}

static void
_elm_win_on_resize_obj_changed_size_hints(void *data,
                                          Evas *e EINA_UNUSED,
                                          Evas_Object *obj EINA_UNUSED,
                                          void *event_info EINA_UNUSED)
{
   _elm_win_resize_objects_eval(data);
}

void
_elm_win_shutdown(void)
{
   while (_elm_win_list) evas_object_del(_elm_win_list->data);
   ELM_SAFE_FREE(_elm_win_state_eval_job, ecore_job_del);
}

void
_elm_win_rescale(Elm_Theme *th,
                 Eina_Bool use_theme)
{
   const Eina_List *l;
   Evas_Object *obj;

   if (!use_theme)
     {
        EINA_LIST_FOREACH(_elm_win_list, l, obj)
          elm_widget_theme(obj);
     }
   else
     {
        EINA_LIST_FOREACH(_elm_win_list, l, obj)
          elm_widget_theme_specific(obj, th, EINA_FALSE);
     }
}

void
_elm_win_access(Eina_Bool is_access)
{
   Evas *evas;
   const Eina_List *l;
   Evas_Object *obj;
   Evas_Object *fobj;

   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     {
        elm_widget_access(obj, is_access);

         /* floating orphan object. if there are A, B, C objects and user does
            as below, then there would be floating orphan objects.

              1. elm_object_content_set(layout, A);
              2. elm_object_content_set(layout, B);
              3. elm_object_content_set(layout, C);

            now, the object A and B are floating orphan objects */

        fobj = obj;
        for (;;)
          {
             fobj = evas_object_below_get(fobj);
             if (!fobj) break;

             if (elm_widget_is(fobj) && !elm_widget_parent_get(fobj))
               {
                  elm_widget_access(fobj, is_access);
               }
          }

        if (!is_access)
          {
             evas = evas_object_evas_get(obj);
            if (evas) _elm_access_object_highlight_disable(evas);
          }
     }
}

void
_elm_win_translate(void)
{
   const Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     elm_widget_translate(obj);
}

#ifdef HAVE_ELEMENTARY_X
static Eina_Bool
_elm_win_client_message(void *data,
                        int type EINA_UNUSED,
                        void *event)
{
   ELM_WIN_DATA_GET(data, sd);
   Ecore_X_Event_Client_Message *e = event;

   if (e->format != 32) return ECORE_CALLBACK_PASS_ON;
   if (e->message_type == ECORE_X_ATOM_E_COMP_FLUSH)
     {
        if ((unsigned int)e->data.l[0] == sd->x.xwin)
          {
             Evas *evas = evas_object_evas_get(sd->obj);
             if (evas)
               {
                  edje_file_cache_flush();
                  edje_collection_cache_flush();
                  evas_image_cache_flush(evas);
                  evas_font_cache_flush(evas);
               }
          }
     }
   else if (e->message_type == ECORE_X_ATOM_E_COMP_DUMP)
     {
        if ((unsigned int)e->data.l[0] == sd->x.xwin)
          {
             Evas *evas = evas_object_evas_get(sd->obj);
             if (evas)
               {
                  edje_file_cache_flush();
                  edje_collection_cache_flush();
                  evas_image_cache_flush(evas);
                  evas_font_cache_flush(evas);
                  evas_render_dump(evas);
               }
          }
     }
   else if (e->message_type == ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL)
     {
        if ((unsigned int)e->data.l[0] == sd->x.xwin)
          {
             if ((unsigned int)e->data.l[1] ==
                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_NEXT)
               {
                  // XXX: call right access func
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_PREV)
               {
                  // XXX: call right access func
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_ACTIVATE)
               {
                  _elm_access_highlight_object_activate
                    (sd->obj, ELM_ACTIVATE_DEFAULT);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ)
               {
                  /* there would be better way to read highlight object */
                  Evas *evas;
                  evas = evas_object_evas_get(sd->obj);
                  if (!evas) return ECORE_CALLBACK_PASS_ON;

                  _elm_access_mouse_event_enabled_set(EINA_TRUE);

                  evas_event_feed_mouse_in(evas, 0, NULL);
                  evas_event_feed_mouse_move
                    (evas, e->data.l[2], e->data.l[3], 0, NULL);

                  _elm_access_mouse_event_enabled_set(EINA_FALSE);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ_NEXT)
               {
                  _elm_access_highlight_cycle(sd->obj, ELM_FOCUS_NEXT);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ_PREV)
               {
                  _elm_access_highlight_cycle(sd->obj, ELM_FOCUS_PREVIOUS);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_UP)
               {
                  _elm_access_highlight_object_activate
                    (sd->obj, ELM_ACTIVATE_UP);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_DOWN)
               {
                  _elm_access_highlight_object_activate
                    (sd->obj, ELM_ACTIVATE_DOWN);
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_elm_win_property_change(void *data,
                         int type EINA_UNUSED,
                         void *event)
{
   ELM_WIN_DATA_GET(data, sd);
   Ecore_X_Event_Window_Property *e = event;

   if (e->atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_STATE)
     {
        if (e->win == sd->x.xwin)
          {
             sd->indmode = (Elm_Win_Indicator_Mode)ecore_x_e_illume_indicator_state_get(e->win);
             evas_object_smart_callback_call(sd->obj, SIG_INDICATOR_PROP_CHANGED, NULL);
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}
#endif

static void
_elm_win_focus_highlight_hide(void *data EINA_UNUSED,
                              Evas_Object *obj,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   evas_object_hide(obj);
}

static void
_elm_win_focus_highlight_anim_end(void *data,
                                  Evas_Object *obj,
                                  const char *emission EINA_UNUSED,
                                  const char *source EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   _elm_win_focus_highlight_simple_setup(sd, obj);
}

static void
_elm_win_focus_highlight_init(Elm_Win_Data *sd)
{
   evas_event_callback_add(sd->evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
                           _elm_win_object_focus_in, sd->obj);
   evas_event_callback_add(sd->evas,
                           EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
                           _elm_win_object_focus_out, sd->obj);

   sd->focus_highlight.cur.target = _elm_win_focus_target_get(evas_focus_get(sd->evas));
   if (sd->focus_highlight.cur.target)
     {
        if (elm_widget_highlight_in_theme_get(sd->focus_highlight.cur.target))
          sd->focus_highlight.cur.in_theme = EINA_TRUE;
        else
          _elm_win_focus_target_callbacks_add(sd);
     }

   sd->focus_highlight.prev.target = NULL;
   sd->focus_highlight.fobj = edje_object_add(sd->evas);
   sd->focus_highlight.theme_changed = EINA_TRUE;

   edje_object_signal_callback_add(sd->focus_highlight.fobj,
                                   "elm,action,focus,hide,end", "*",
                                   _elm_win_focus_highlight_hide, NULL);
   edje_object_signal_callback_add(sd->focus_highlight.fobj,
                                   "elm,action,focus,anim,end", "*",
                                   _elm_win_focus_highlight_anim_end, sd->obj);
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_frame_cb_move_start(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *sig EINA_UNUSED,
                             const char *source)
{
   ELM_WIN_DATA_GET(data, sd);

   if (!sd) return;

#ifdef HAVE_ELEMENTARY_WAYLAND
   if (!strcmp(source, "elm"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, ELM_CURSOR_HAND1);
   else
     ecore_wl_window_cursor_default_restore(sd->wl.win);
#else
   (void)source;
#endif

   /* NB: Wayland handles moving surfaces by itself so we cannot
    * specify a specific x/y we want. Instead, we will pass in the
    * existing x/y values so they can be recorded as 'previous'
    * position. The new position will get updated automatically when
    * the move is finished */

   ecore_evas_wayland_move(sd->ee, sd->screen.x, sd->screen.y);
}

static void
_elm_win_frame_cb_move_stop(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *sig EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   if (!sd) return;

#ifdef HAVE_ELEMENTARY_WAYLAND
   ecore_wl_window_cursor_default_restore(sd->wl.win);
#endif
}

#ifdef HAVE_ELEMENTARY_WAYLAND
struct _resize_info
{
   const char *name;
   int location;
};

static struct _resize_info _border_side[4] =
{
     { ELM_CURSOR_TOP_SIDE, 1 },
     { ELM_CURSOR_LEFT_SIDE, 4 },
     { ELM_CURSOR_BOTTOM_SIDE, 2 },
     { ELM_CURSOR_RIGHT_SIDE, 8 },
};

static struct _resize_info _border_corner[4] =
{
     { ELM_CURSOR_TOP_LEFT_CORNER, 5 },
     { ELM_CURSOR_BOTTOM_LEFT_CORNER, 6 },
     { ELM_CURSOR_BOTTOM_RIGHT_CORNER, 10 },
     { ELM_CURSOR_TOP_RIGHT_CORNER, 9 },
};
#endif

static void
_elm_win_frame_obj_update(Elm_Win_Data *sd)
{
   int fx, fy, fw, fh;
   int ox, oy, ow, oh;
   int sx, sy, sw, sh;
   int x, y, w, h;
   evas_object_geometry_get(sd->frame_obj, &fx, &fy, &fw, &fh);
   evas_object_geometry_get(sd->client_obj, &ox, &oy, &ow, &oh);
   evas_object_geometry_get(sd->spacer_obj, &sx, &sy, &sw, &sh);

   evas_output_framespace_get(sd->evas, &x, &y, &w, &h);

   if ((x != (ox - fx)) || (y != (oy - fy)) ||
       (w != (fw - ow)) || (h != (fh - oh)))
     {
        evas_output_framespace_set(sd->evas, (ox - fx), (oy - fy),
                                   (fw - ow), (fh - oh));
     }

#ifdef HAVE_ELEMENTARY_WAYLAND
   ecore_wl_window_opaque_region_set(sd->wl.win, -fx, -(fy - sy), sw, sh);
#endif
}

static void
_elm_win_frame_obj_move(void *data,
                        Evas *e EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Elm_Win_Data *sd;

   if (!(sd = data)) return;
   if (!sd->client_obj) return;

   _elm_win_frame_obj_update(sd);
}

static void
_elm_win_frame_obj_resize(void *data,
                          Evas *e EINA_UNUSED,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   Elm_Win_Data *sd;

   if (!(sd = data)) return;
   if (!sd->client_obj) return;

   _elm_win_frame_obj_update(sd);
}

static void
_elm_win_frame_cb_resize_show(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *sig EINA_UNUSED,
                              const char *source)
{
   ELM_WIN_DATA_GET(data, sd);

   if (!sd) return;
   if (sd->resizing) return;

#ifdef HAVE_ELEMENTARY_WAYLAND
   int i;
   i = sd->rot / 90;
   if (!strcmp(source, "elm.event.resize.t"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          _border_side[(0 + i) % 4].name);
   else if (!strcmp(source, "elm.event.resize.b"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          _border_side[(2 + i) % 4].name);
   else if (!strcmp(source, "elm.event.resize.l"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          _border_side[(1 + i) % 4].name);
   else if (!strcmp(source, "elm.event.resize.r"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          _border_side[(3 + i) % 4].name);
   else if (!strcmp(source, "elm.event.resize.tl"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          _border_corner[(0 + i) % 4].name);
   else if (!strcmp(source, "elm.event.resize.tr"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          _border_corner[(3 + i) % 4].name);
   else if (!strcmp(source, "elm.event.resize.bl"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          _border_corner[(1 + i) % 4].name);
   else if (!strcmp(source, "elm.event.resize.br"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          _border_corner[(2 + i) % 4].name);
   else
     ecore_wl_window_cursor_default_restore(sd->wl.win);
#else
   (void)source;
#endif
}

static void
_elm_win_frame_cb_resize_hide(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *sig EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   if (!sd) return;
   if (sd->resizing) return;

#ifdef HAVE_ELEMENTARY_WAYLAND
   ecore_wl_window_cursor_default_restore(sd->wl.win);
#endif
}

static void
_elm_win_frame_cb_resize_start(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *sig EINA_UNUSED,
                               const char *source)
{
#ifdef HAVE_ELEMENTARY_WAYLAND
   ELM_WIN_DATA_GET(data, sd);
   int i;

   if (!sd) return;
   if (sd->resizing) return;

   sd->resizing = EINA_TRUE;
   i = sd->rot / 90;
   if (!strcmp(source, "elm.event.resize.t"))
     sd->resize_location = _border_side[(0 + i) % 4].location;
   else if (!strcmp(source, "elm.event.resize.b"))
     sd->resize_location = _border_side[(2 + i) % 4].location;
   else if (!strcmp(source, "elm.event.resize.l"))
     sd->resize_location = _border_side[(1 + i) % 4].location;
   else if (!strcmp(source, "elm.event.resize.r"))
     sd->resize_location = _border_side[(3 + i) % 4].location;
   else if (!strcmp(source, "elm.event.resize.tl"))
     sd->resize_location = _border_corner[(0 + i) % 4].location;
   else if (!strcmp(source, "elm.event.resize.tr"))
     sd->resize_location = _border_corner[(3 + i) % 4].location;
   else if (!strcmp(source, "elm.event.resize.bl"))
     sd->resize_location = _border_corner[(1 + i) % 4].location;
   else if (!strcmp(source, "elm.event.resize.br"))
     sd->resize_location = _border_corner[(2 + i) % 4].location;
   else
     sd->resize_location = 0;

   if (sd->resize_location > 0)
     ecore_evas_wayland_resize(sd->ee, sd->resize_location);
#else
   (void)data;
   (void)source;
#endif
}

static void
_elm_win_frame_cb_minimize(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *sig EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   if (!sd) return;
//   sd->iconified = EINA_TRUE;
   TRAP(sd, iconified_set, EINA_TRUE);
}

static void
_elm_win_frame_maximized_state_update(Elm_Win_Data *sd, Eina_Bool maximized)
{
   const char *emission;

   if (maximized)
     emission = "elm,state,maximized";
   else
     emission = "elm,state,unmaximized";

   edje_object_signal_emit(sd->frame_obj, emission, "elm");
   edje_object_message_signal_process(sd->frame_obj);
   evas_object_smart_calculate(sd->frame_obj);

   _elm_win_frame_obj_update(sd);
}

static void
_elm_win_frame_cb_maximize(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *sig EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Eina_Bool value;
   ELM_WIN_DATA_GET(data, sd);

   if (!sd) return;
   if (sd->maximized) value = EINA_FALSE;
   else value = EINA_TRUE;

   _elm_win_frame_maximized_state_update(sd, value);

   TRAP(sd, maximized_set, value);
}

static void
_elm_win_frame_cb_close(void *data,
                        Evas_Object *obj EINA_UNUSED,
                        const char *sig EINA_UNUSED,
                        const char *source EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);
   Evas_Object *win;

   /* FIXME: After the current freeze, this should be handled differently.
    *
    * Ideally, we would want to mimic the X11 backend and use something
    * like ECORE_WL_EVENT_WINDOW_DELETE and handle the delete_request
    * inside of ecore_evas. That would be the 'proper' way, but since we are
    * in a freeze right now, I cannot add a new event value, or a new
    * event structure to ecore_wayland.
    *
    * So yes, this is a temporary 'stop-gap' solution which will be fixed
    * when the freeze is over, but it does fix a trac bug for now, and in a
    * way which does not break API or the freeze. - dh
    */

   if (!sd) return;

   win = sd->obj;

   int autodel = sd->autodel;
   sd->autodel_clear = &autodel;
   evas_object_ref(win);
   evas_object_smart_callback_call(win, SIG_DELETE_REQUEST, NULL);
   // FIXME: if above callback deletes - then the below will be invalid
   if (autodel) evas_object_del(win);
   else sd->autodel_clear = NULL;
   evas_object_unref(win);
}

static void
_elm_win_frame_add(Elm_Win_Data *sd,
                   const char *style)
{
   Evas_Object *obj = sd->obj;
   int w, h, mw, mh;
   short layer;

   sd->frame_obj = edje_object_add(sd->evas);
   layer = evas_object_layer_get(obj);
   evas_object_layer_set(sd->frame_obj, layer + 1);
   if (!elm_widget_theme_object_set
       (sd->obj, sd->frame_obj, "border", "base", style))
     {
        ELM_SAFE_FREE(sd->frame_obj, evas_object_del);
        return;
     }

   sd->spacer_obj = evas_object_rectangle_add(sd->evas);
   evas_object_color_set(sd->spacer_obj, 0, 0, 0, 0);
   evas_object_repeat_events_set(sd->spacer_obj, EINA_TRUE);
   edje_object_part_swallow(sd->frame_obj, "elm.swallow.frame_spacer",
                            sd->spacer_obj);

   sd->client_obj = evas_object_rectangle_add(sd->evas);
   evas_object_color_set(sd->client_obj, 0, 0, 0, 0);
   /* NB: Tried pass_events here, but that fails to send events */
   evas_object_repeat_events_set(sd->client_obj, EINA_TRUE);
   edje_object_part_swallow(sd->frame_obj, "elm.swallow.client",
                            sd->client_obj);

   evas_object_is_frame_object_set(sd->frame_obj, EINA_TRUE);

   evas_object_event_callback_add
     (sd->frame_obj, EVAS_CALLBACK_MOVE, _elm_win_frame_obj_move, sd);
   evas_object_event_callback_add
     (sd->frame_obj, EVAS_CALLBACK_RESIZE, _elm_win_frame_obj_resize, sd);

   /* NB: Do NOT remove these calls !! Needed to calculate proper
    * framespace on inital show of the window */
   edje_object_size_min_calc(sd->frame_obj, &mw, &mh);
   evas_object_move(sd->frame_obj, 0, 0);
   evas_object_resize(sd->frame_obj, mw, mh);
   evas_object_smart_calculate(sd->frame_obj);

   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,move,start", "elm",
     _elm_win_frame_cb_move_start, obj);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,move,stop", "elm",
     _elm_win_frame_cb_move_stop, obj);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,resize,show", "*",
     _elm_win_frame_cb_resize_show, obj);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,resize,hide", "*",
     _elm_win_frame_cb_resize_hide, obj);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,resize,start", "*",
     _elm_win_frame_cb_resize_start, obj);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,minimize", "elm",
     _elm_win_frame_cb_minimize, obj);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,maximize", "elm",
     _elm_win_frame_cb_maximize, obj);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,close", "elm", _elm_win_frame_cb_close, obj);

   if (sd->title)
     {
        edje_object_part_text_escaped_set
          (sd->frame_obj, "elm.text.title", sd->title);
     }

   ecore_evas_geometry_get(sd->ee, NULL, NULL, &w, &h);
   ecore_evas_resize(sd->ee, w, h);
}

static void
_elm_win_frame_del(Elm_Win_Data *sd)
{
   int w, h;

   ELM_SAFE_FREE(sd->client_obj, evas_object_del);

   if (sd->frame_obj)
     {
        evas_object_event_callback_del_full
          (sd->frame_obj, EVAS_CALLBACK_MOVE, _elm_win_frame_obj_move, sd);
        evas_object_event_callback_del_full
          (sd->frame_obj, EVAS_CALLBACK_RESIZE, _elm_win_frame_obj_resize, sd);

        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,move,start", "elm",
              _elm_win_frame_cb_move_start);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,move,stop", "elm",
              _elm_win_frame_cb_move_stop);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,resize,show", "*",
              _elm_win_frame_cb_resize_show);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,resize,hide", "*",
              _elm_win_frame_cb_resize_hide);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,resize,start", "*",
              _elm_win_frame_cb_resize_start);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,minimize", "elm",
              _elm_win_frame_cb_minimize);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,maximize", "elm",
              _elm_win_frame_cb_maximize);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,close", "elm",
              _elm_win_frame_cb_close);

        ELM_SAFE_FREE(sd->frame_obj, evas_object_del);
     }

   evas_output_framespace_set(sd->evas, 0, 0, 0, 0);
   ecore_evas_geometry_get(sd->ee, NULL, NULL, &w, &h);
   ecore_evas_resize(sd->ee, w, h);
}

#ifdef ELM_DEBUG
static void
_debug_key_down(void *data EINA_UNUSED,
                Evas *e EINA_UNUSED,
                Evas_Object *obj,
                void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return;

   if ((strcmp(ev->key, "F12")) ||
       (!evas_key_modifier_is_set(ev->modifiers, "Control")))
     return;

   INF("Tree graph generated.");
   elm_object_tree_dot_dump(obj, "./dump.dot");
}

#endif

static void
_win_inlined_image_set(Elm_Win_Data *sd)
{
   evas_object_image_alpha_set(sd->img_obj, EINA_FALSE);
   evas_object_image_filled_set(sd->img_obj, EINA_TRUE);

   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_DEL, _elm_win_on_img_obj_del, sd->obj);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_HIDE, _win_img_hide, sd->obj);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_MOUSE_UP, _win_img_mouse_up, sd->obj);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_FOCUS_IN, _win_img_focus_in, sd->obj);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_FOCUS_OUT, _win_img_focus_out, sd->obj);
}

static void
_elm_win_on_icon_del(void *data,
                     Evas *e EINA_UNUSED,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   ELM_WIN_DATA_GET(data, sd);

   if (sd->icon == obj) sd->icon = NULL;
}

EOLIAN static void
_elm_win_evas_object_smart_add(Eo *obj, Elm_Win_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
}

#ifdef HAVE_ELEMENTARY_X
static void
_elm_x_io_err(void *data EINA_UNUSED)
{
   Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     evas_object_smart_callback_call(obj, SIG_IOERR, NULL);
   elm_exit();
}
#endif

EAPI Evas_Object *
elm_win_add(Evas_Object *parent,
            const char *name,
            Elm_Win_Type type)
{
   Evas_Object *obj = eo_add_custom(MY_CLASS, parent, elm_obj_win_constructor(name, type));
   eo_unref(obj);
   return obj;
}

static void
_elm_win_cb_hide(void *data EINA_UNUSED,
                 Evas *e EINA_UNUSED,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   _elm_win_state_eval_queue();
}

static void
_elm_win_cb_show(void *data EINA_UNUSED,
                 Evas *e EINA_UNUSED,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   _elm_win_state_eval_queue();
}

/**
  * @internal
  *
  * Recalculate the size of window considering its resize objects' weight and
  * min size. If any of its resize objects' weight equals to 0.0, window
  * layout's weight will be set to 0.0.
  *
  * @param o box object
  * @param p box's private data
  * @param data window object
  */
static void
_window_layout_stack(Evas_Object *o, Evas_Object_Box_Data *p, void *data)
{
   const Eina_List *l;
   Evas_Object *child;
   Evas_Object_Box_Option *opt;
   Evas_Coord x, y, w, h;
   double wx, wy;
   Evas_Coord minw = -1, minh = -1;
   double weight_x = EVAS_HINT_EXPAND;
   double weight_y = EVAS_HINT_EXPAND;

   EINA_LIST_FOREACH(p->children, l, opt)
     {
        child = opt->obj;
        evas_object_size_hint_weight_get(child, &wx, &wy);
        if (wx == 0.0) weight_x = 0;
        if (wy == 0.0) weight_y = 0;

        evas_object_size_hint_min_get(child, &w, &h);
        if (w > minw) minw = w;
        if (h > minh) minh = h;
     }

   evas_object_size_hint_min_set(o, minw, minh);
   evas_object_geometry_get(o, &x, &y, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   evas_object_resize(o, w, h);

   EINA_LIST_FOREACH(p->children, l, opt)
     {
        child = opt->obj;
        evas_object_move(child, x, y);
        evas_object_resize(child, w, h);
     }

   ELM_WIN_DATA_GET(data, sd);
   evas_object_size_hint_weight_set(sd->layout, weight_x, weight_y);
   evas_object_smart_changed(sd->layout);
}

static Eina_Bool
_accel_is_gl(void)
{
   const char *env = NULL;
   const char *str;
   
   str = _elm_accel_preference;
   env = getenv("ELM_ACCEL");
   if (env) str = env;
   if ((str) &&
       ((!strcasecmp(str, "gl")) ||
        (!strcasecmp(str, "opengl")) ||
        (!strcasecmp(str, "3d")) ||
        (!strcasecmp(str, "hw")) ||
        (!strcasecmp(str, "accel")) ||
        (!strcasecmp(str, "hardware"))
       ))
     return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static void
_elm_win_constructor(Eo *obj, Elm_Win_Data *sd, const char *name, Elm_Win_Type type)
{
   sd->obj = obj; // in ctor

   Evas_Object *parent = NULL;
   Evas *e;
   const Eina_List *l;
   const char *fontpath, *engine = NULL, *enginelist[32], *disp;
   int i;

   Elm_Win_Data tmp_sd;

   eo_do(obj, parent = eo_parent_get());

   /* just to store some data while trying out to create a canvas */
   memset(&tmp_sd, 0, sizeof(Elm_Win_Data));

   switch (type)
     {
      case ELM_WIN_INLINED_IMAGE:
        if (!parent) break;
          {
             e = evas_object_evas_get(parent);
             Ecore_Evas *ee;

             if (!e) break;

             ee = ecore_evas_ecore_evas_get(e);
             if (!ee) break;

             tmp_sd.img_obj = ecore_evas_object_image_new(ee);
             if (!tmp_sd.img_obj) break;

             tmp_sd.ee = ecore_evas_object_ecore_evas_get(tmp_sd.img_obj);
             if (!tmp_sd.ee)
               {
                  ELM_SAFE_FREE(tmp_sd.img_obj, evas_object_del);
               }
          }
        break;

      case ELM_WIN_SOCKET_IMAGE:
        tmp_sd.ee = ecore_evas_extn_socket_new(1, 1);
        break;

      default:
        disp = getenv("ELM_DISPLAY");
        if ((disp) && (!strcmp(disp, "x11")))
          {
             if (_accel_is_gl())
               {
                  enginelist[0] = ELM_OPENGL_X11;
                  enginelist[1] = ELM_SOFTWARE_X11;
                  enginelist[2] = NULL;
               }
             else
               {
                  enginelist[0] = ELM_SOFTWARE_X11;
                  enginelist[1] = ELM_OPENGL_X11;
                  enginelist[2] = NULL;
               }
          }
        else if ((disp) && (!strcmp(disp, "wl")))
          {
             if (_accel_is_gl())
               {
                  enginelist[0] = ELM_WAYLAND_EGL;
                  enginelist[1] = ELM_WAYLAND_SHM;
                  enginelist[2] = NULL;
               }
             else
               {
                  enginelist[0] = ELM_WAYLAND_SHM;
                  enginelist[1] = ELM_WAYLAND_EGL;
                  enginelist[2] = NULL;
               }
          }
        else if ((disp) && (!strcmp(disp, "win")))
          {
             enginelist[1] = ELM_SOFTWARE_WIN32;
             enginelist[2] = NULL;
          }
        else if ((disp) && (!strcmp(disp, "sdl")))
          {
             if (_accel_is_gl())
               {
                  enginelist[0] = ELM_OPENGL_SDL;
                  enginelist[1] = ELM_SOFTWARE_SDL;
                  enginelist[2] = NULL;
               }
             else
               {
                  enginelist[0] = ELM_SOFTWARE_SDL;
                  enginelist[1] = ELM_OPENGL_SDL;
                  enginelist[2] = NULL;
               }
          }
        else if ((disp) && (!strcmp(disp, "mac")))
          {
             enginelist[0] = ELM_OPENGL_COCOA;
             enginelist[1] = NULL;
          }
        else if ((disp) && (!strcmp(disp, "ews")))
          {
             enginelist[0] = ELM_EWS;
             enginelist[1] = NULL;
          }
        else if ((disp) && (!strcmp(disp, "fb")))
          {
             enginelist[0] = ELM_DRM;
             enginelist[1] = ELM_SOFTWARE_FB;
             enginelist[2] = NULL;
          }
        else if ((disp) && (!strcmp(disp, "buffer")))
          {
             enginelist[0] = ELM_BUFFER;
             enginelist[1] = NULL;
          }
        else if ((disp) && (!strcmp(disp, "ps3")))
          {
             enginelist[0] = ELM_SOFTWARE_PSL1GHT;
             enginelist[1] = NULL;
          }
        else if ((disp) && (!strcmp(disp, "shot")))
          {
             enginelist[0] = ENGINE_GET();
             enginelist[1] = NULL;
          }
        else
          {
             if (_accel_is_gl())
               {
                  enginelist[ 0] = ELM_OPENGL_X11;
                  enginelist[ 1] = ELM_WAYLAND_EGL;
                  enginelist[ 2] = ELM_DRM;
                  enginelist[ 3] = ELM_SOFTWARE_FB;
                  enginelist[ 4] = ELM_OPENGL_COCOA;
                  enginelist[ 5] = ELM_OPENGL_SDL;
                  enginelist[ 6] = ELM_SOFTWARE_X11;
                  enginelist[ 7] = ELM_WAYLAND_SHM;
                  enginelist[ 8] = ELM_SOFTWARE_SDL;
                  enginelist[ 9] = ELM_SOFTWARE_PSL1GHT;
                  enginelist[10] = NULL;
               }
             else
               {
                  enginelist[ 0] = ENGINE_GET();
                  enginelist[ 1] = ELM_SOFTWARE_X11;
                  enginelist[ 2] = ELM_WAYLAND_SHM;
                  enginelist[ 3] = ELM_DRM;
                  enginelist[ 4] = ELM_SOFTWARE_FB;
                  enginelist[ 5] = ELM_OPENGL_COCOA;
                  enginelist[ 6] = ELM_SOFTWARE_SDL;
                  enginelist[ 7] = ELM_OPENGL_X11;
                  enginelist[ 8] = ELM_WAYLAND_EGL;
                  enginelist[ 9] = ELM_DRM;
                  enginelist[10] = ELM_OPENGL_SDL;
                  enginelist[11] = ELM_SOFTWARE_WIN32;
                  enginelist[12] = NULL;
               }
          }
        for (i = 0; i < 30; i++)
          {
             if ((i > 0) && (!enginelist[i])) break;
             if (!strcmp(enginelist[i], ELM_SOFTWARE_X11))
               tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
             else if (!strcmp(enginelist[i], ELM_OPENGL_X11))
               {
                  int opt[10], opt_i = 0;

                  if (_elm_config->vsync)
                    {
                       opt[opt_i++] = ECORE_EVAS_GL_X11_OPT_VSYNC;
                       opt[opt_i++] = 1;
                       opt[opt_i++] = 0;
                    }
                  if (opt_i > 0)
                    tmp_sd.ee = ecore_evas_gl_x11_options_new(NULL, 0, 0, 0, 1, 1, opt);
                  else
                    tmp_sd.ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 1, 1);
               }
             else if (!strcmp(enginelist[i], ELM_WAYLAND_SHM))
               tmp_sd.ee = ecore_evas_wayland_shm_new(NULL, 0, 0, 0, 1, 1, 0);
             else if (!strcmp(enginelist[i], ELM_WAYLAND_EGL))
               tmp_sd.ee = ecore_evas_wayland_egl_new(NULL, 0, 0, 0, 1, 1, 0);
             else if (!strcmp(enginelist[i], ELM_SOFTWARE_WIN32))
               tmp_sd.ee = ecore_evas_software_gdi_new(NULL, 0, 0, 1, 1);
             else if (!strcmp(enginelist[i], ELM_SOFTWARE_SDL))
               tmp_sd.ee = ecore_evas_sdl_new(NULL, 0, 0, 0, 0, 0, 1);
             else if (!strcmp(enginelist[i], ELM_OPENGL_SDL))
               tmp_sd.ee = ecore_evas_gl_sdl_new(NULL, 1, 1, 0, 0);
             else if (!strcmp(enginelist[i], ELM_OPENGL_COCOA))
               tmp_sd.ee = ecore_evas_cocoa_new(NULL, 1, 1, 0, 0);
             else if (!strcmp(enginelist[i], ELM_EWS))
               tmp_sd.ee = ecore_evas_ews_new(0, 0, 1, 1);
             else if (!strcmp(enginelist[i], ELM_SOFTWARE_FB))
               tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
             else if (!strcmp(enginelist[i], ELM_BUFFER))
               tmp_sd.ee = ecore_evas_buffer_new(1, 1);
             else if (!strcmp(enginelist[i], ELM_SOFTWARE_PSL1GHT))
               tmp_sd.ee = ecore_evas_psl1ght_new(NULL, 1, 1);
             else if (!strcmp(enginelist[i], ELM_DRM))
               tmp_sd.ee = ecore_evas_drm_new(NULL, 0, 0, 0, 1, 1);
             else if (!strncmp(enginelist[i], "shot:", 5))
               {
                  tmp_sd.ee = ecore_evas_buffer_new(1, 1);
                  ecore_evas_manual_render_set(tmp_sd.ee, EINA_TRUE);
                  tmp_sd.shot.info = eina_stringshare_add(ENGINE_GET() + 5);
               }
             engine = enginelist[i];
             if (tmp_sd.ee) break;
          }
        break;
     }

   if (!tmp_sd.ee)
     {
        ERR("Cannot create window.");
        eo_error_set(obj);
        return;
     }

   eo_do(obj, eo_parent_set(ecore_evas_get(tmp_sd.ee)));
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));

   if (getenv("ELM_FIRST_FRAME"))
     evas_event_callback_add(ecore_evas_get(tmp_sd.ee), EVAS_CALLBACK_RENDER_POST,
                             _elm_win_first_frame_do, getenv("ELM_FIRST_FRAME"));

   /* copying possibly altered fields back */
#define SD_CPY(_field)             \
  do                               \
    {                              \
       sd->_field = tmp_sd._field; \
    } while (0)

   SD_CPY(ee);
   SD_CPY(img_obj);
   SD_CPY(shot.info);
#undef SD_CPY

   if ((trap) && (trap->add))
     sd->trap_data = trap->add(obj);

   /* complementary actions, which depend on final smart data
    * pointer */
   if (type == ELM_WIN_INLINED_IMAGE)
     _win_inlined_image_set(sd);
#ifdef HAVE_ELEMENTARY_X
   else if ((engine) &&
            ((!strcmp(engine, ELM_SOFTWARE_X11)) ||
             (!strcmp(engine, ELM_OPENGL_X11))))
     {
        sd->x.client_message_handler = ecore_event_handler_add
            (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, obj);
        sd->x.property_handler = ecore_event_handler_add
            (ECORE_X_EVENT_WINDOW_PROPERTY, _elm_win_property_change, obj);
     }
#endif
   else if ((engine) && (!strncmp(engine, "shot:", 5)))
     _shot_init(sd);

   sd->kbdmode = ELM_WIN_KEYBOARD_UNKNOWN;
   sd->indmode = ELM_WIN_INDICATOR_UNKNOWN;

#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        ecore_x_io_error_handler_set(_elm_x_io_err, NULL);
     }
#endif

#ifdef HAVE_ELEMENTARY_WAYLAND
   if ((ENGINE_COMPARE(ELM_WAYLAND_SHM)) || (ENGINE_COMPARE(ELM_WAYLAND_EGL)))
     _elm_win_wlwindow_get(sd);
#endif

   if ((_elm_config->bgpixmap)
#ifdef HAVE_ELEMENTARY_X
       &&
       (((sd->x.xwin) && (!ecore_x_screen_is_composited(0))) ||
           (!sd->x.xwin)))
#else
     )
#endif
     TRAP(sd, avoid_damage_set, ECORE_EVAS_AVOID_DAMAGE_EXPOSE);
   // bg pixmap done by x - has other issues like can be redrawn by x before it
   // is filled/ready by app
   //     TRAP(sd, avoid_damage_set, ECORE_EVAS_AVOID_DAMAGE_BUILT_IN);

   sd->type = type;
   sd->parent = parent;
   sd->modal_count = 0;

   if (sd->parent)
     evas_object_event_callback_add
       (sd->parent, EVAS_CALLBACK_DEL, _elm_win_on_parent_del, obj);

   sd->evas = ecore_evas_get(sd->ee);

   evas_object_color_set(obj, 0, 0, 0, 0);
   evas_object_move(obj, 0, 0);
   evas_object_resize(obj, 1, 1);
   evas_object_layer_set(obj, 50);
   evas_object_pass_events_set(obj, EINA_TRUE);

   if (type == ELM_WIN_INLINED_IMAGE)
     elm_widget_parent2_set(obj, parent);

   /* use own version of ecore_evas_object_associate() that does TRAP() */
   ecore_evas_data_set(sd->ee, "elm_win", obj);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
      _elm_win_obj_callback_changed_size_hints, obj);

   evas_object_intercept_raise_callback_add
     (obj, _elm_win_obj_intercept_raise, obj);
   evas_object_intercept_lower_callback_add
     (obj, _elm_win_obj_intercept_lower, obj);
   evas_object_intercept_stack_above_callback_add
     (obj, _elm_win_obj_intercept_stack_above, obj);
   evas_object_intercept_stack_below_callback_add
     (obj, _elm_win_obj_intercept_stack_below, obj);
   evas_object_intercept_layer_set_callback_add
     (obj, _elm_win_obj_intercept_layer_set, obj);
   evas_object_intercept_show_callback_add
     (obj, _elm_win_obj_intercept_show, obj);

   TRAP(sd, name_class_set, name, _elm_appname);
   ecore_evas_callback_delete_request_set(sd->ee, _elm_win_delete_request);
   ecore_evas_callback_resize_set(sd->ee, _elm_win_resize);
   ecore_evas_callback_mouse_in_set(sd->ee, _elm_win_mouse_in);
   ecore_evas_callback_focus_in_set(sd->ee, _elm_win_focus_in);
   ecore_evas_callback_focus_out_set(sd->ee, _elm_win_focus_out);
   ecore_evas_callback_move_set(sd->ee, _elm_win_move);
   ecore_evas_callback_state_change_set(sd->ee, _elm_win_state_change);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _elm_win_cb_hide, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _elm_win_cb_show, NULL);

   evas_image_cache_set(sd->evas, (_elm_config->image_cache * 1024));
   evas_font_cache_set(sd->evas, (_elm_config->font_cache * 1024));

   EINA_LIST_FOREACH(_elm_config->font_dirs, l, fontpath)
     evas_font_path_append(sd->evas, fontpath);

   if (!_elm_config->font_hinting)
     evas_font_hinting_set(sd->evas, EVAS_FONT_HINTING_NONE);
   else if (_elm_config->font_hinting == 1)
     evas_font_hinting_set(sd->evas, EVAS_FONT_HINTING_AUTO);
   else if (_elm_config->font_hinting == 2)
     evas_font_hinting_set(sd->evas, EVAS_FONT_HINTING_BYTECODE);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif

   _elm_win_list = eina_list_append(_elm_win_list, obj);
   _elm_win_count++;

   if ((engine) && ((!strcmp(engine, ELM_SOFTWARE_FB)) || (!strcmp(engine, ELM_DRM))))
     {
        TRAP(sd, fullscreen_set, 1);
     }
   else if ((type != ELM_WIN_INLINED_IMAGE) &&
            ((engine) &&
             ((!strcmp(engine, ELM_WAYLAND_SHM) ||
              (!strcmp(engine, ELM_WAYLAND_EGL))))))
     _elm_win_frame_add(sd, "default");

   if (_elm_config->focus_highlight_enable)
     elm_win_focus_highlight_enabled_set(obj, EINA_TRUE);
   if (_elm_config->focus_highlight_animate)
     elm_win_focus_highlight_animate_set(obj, EINA_TRUE);

#ifdef ELM_DEBUG
   Evas_Modifier_Mask mask = evas_key_modifier_mask_get(sd->evas, "Control");
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_KEY_DOWN, _debug_key_down, NULL);

   if (evas_object_key_grab(obj, "F12", mask, 0, EINA_TRUE))
     INF("Ctrl+F12 key combination exclusive for dot tree generation\n");
   else
     ERR("failed to grab F12 key to elm widgets (dot) tree generation");
#endif

   if ((_elm_config->softcursor_mode == ELM_SOFTCURSOR_MODE_ON) ||
       ((_elm_config->softcursor_mode == ELM_SOFTCURSOR_MODE_AUTO) &&
        ((engine) && 
         ((!strcmp(engine, ELM_SOFTWARE_FB)) || (!strcmp(engine, ELM_DRM))))))
     {
        Evas_Object *o;
        Evas_Coord mw = 1, mh = 1, hx = 0, hy = 0;

        sd->pointer.obj = o = edje_object_add(ecore_evas_get(tmp_sd.ee));
        _elm_theme_object_set(obj, o, "pointer", "base", "default");
        edje_object_size_min_calc(o, &mw, &mh);
        evas_object_resize(o, mw, mh);
        edje_object_part_geometry_get(o, "elm.swallow.hotspot",
                                      &hx, &hy, NULL, NULL);
        sd->pointer.hot_x = hx;
        sd->pointer.hot_y = hy;
        evas_object_show(o);
        ecore_evas_object_cursor_set(tmp_sd.ee, o, EVAS_LAYER_MAX, hx, hy);
     }
   else if (_elm_config->softcursor_mode == ELM_SOFTCURSOR_MODE_OFF)
     {
        // do nothing
     }

   sd->wm_rot.wm_supported = ecore_evas_wm_rotation_supported_get(sd->ee);
   sd->wm_rot.preferred_rot = -1; // it means that elm_win doesn't use preferred rotation.

   sd->layout = edje_object_add(sd->evas);
   _elm_theme_object_set(obj, sd->layout, "win", "base", "default");
   sd->box = evas_object_box_add(sd->evas);
   evas_object_box_layout_set(sd->box, _window_layout_stack, obj, NULL);
   edje_object_part_swallow(sd->layout, "elm.swallow.contents", sd->box);
   evas_object_move(sd->layout, 0, 0);
   evas_object_resize(sd->layout, 1, 1);
   edje_object_update_hints_set(sd->layout, EINA_TRUE);
   evas_object_event_callback_add(sd->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _elm_win_on_resize_obj_changed_size_hints, obj);

   if (_elm_config->atspi_mode == ELM_ATSPI_MODE_ON)
     {
        eo_do(obj, elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_WINDOW));
        elm_interface_atspi_accessible_children_changed_added_signal_emit(_elm_atspi_bridge_root_get(), obj);
        eo_do(obj, eo_event_callback_call(ELM_INTERFACE_ATSPI_WINDOW_EVENT_WINDOW_CREATED, NULL));
     }

   evas_object_show(sd->layout);
}

EOLIAN static void
_elm_win_eo_base_constructor(Eo *obj, Elm_Win_Data *_pd EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

EOLIAN static Elm_Win_Type
_elm_win_type_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->type;
}

EOLIAN static void
_elm_win_noblank_set(Eo *obj EINA_UNUSED, Elm_Win_Data *pd, Eina_Bool noblank)
{
   noblank = !!noblank;
   if (pd->noblank == noblank) return;
   pd->noblank = noblank;
   _win_noblank_eval();
}

EOLIAN static Eina_Bool
_elm_win_noblank_get(Eo *obj EINA_UNUSED, Elm_Win_Data *pd)
{
   return pd->noblank;
}


EAPI Evas_Object *
elm_win_util_standard_add(const char *name,
                          const char *title)
{
   Evas_Object *win, *bg;

   win = elm_win_add(NULL, name, ELM_WIN_BASIC);
   if (!win) return NULL;

   elm_win_title_set(win, title);
   bg = elm_bg_add(win);
   if (!bg)
     {
        evas_object_del(win);
        return NULL;
     }
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   return win;
}

EOLIAN static void
_elm_win_resize_object_add(Eo *obj, Elm_Win_Data *sd, Evas_Object *subobj)
{
   elm_widget_sub_object_add(obj, subobj);

   if (!evas_object_box_append(sd->box, subobj))
     ERR("could not append %p to box", subobj);
}

EOLIAN static void
_elm_win_resize_object_del(Eo *obj, Elm_Win_Data *sd, Evas_Object *subobj)
{
   if (!elm_widget_sub_object_del(obj, subobj))
     ERR("could not remove sub object %p from %p", subobj, obj);

   evas_object_box_remove(sd->box, subobj);
}

EOLIAN static void
_elm_win_title_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, const char *title)
{
   if (!title) return;
   eina_stringshare_replace(&(sd->title), title);
   TRAP(sd, title_set, sd->title);
   if (sd->frame_obj)
     edje_object_part_text_escaped_set
       (sd->frame_obj, "elm.text.title", sd->title);
}

EOLIAN static const char*
_elm_win_title_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->title;
}

EOLIAN static void
_elm_win_icon_name_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, const char *icon_name)
{
   if (!icon_name) return;
   eina_stringshare_replace(&(sd->icon_name), icon_name);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static const char*
_elm_win_icon_name_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->icon_name;
}

EOLIAN static void
_elm_win_role_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, const char *role)
{
   if (!role) return;
   eina_stringshare_replace(&(sd->role), role);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static const char*
_elm_win_role_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->role;
}

EOLIAN static void
_elm_win_icon_object_set(Eo *obj, Elm_Win_Data *sd, Evas_Object *icon)
{
   if (sd->icon)
     evas_object_event_callback_del_full
       (sd->icon, EVAS_CALLBACK_DEL, _elm_win_on_icon_del, obj);
   sd->icon = icon;
   if (sd->icon)
     evas_object_event_callback_add
       (sd->icon, EVAS_CALLBACK_DEL, _elm_win_on_icon_del, obj);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static const Evas_Object*
_elm_win_icon_object_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->icon;
}

EOLIAN static void
_elm_win_autodel_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool autodel)
{
   sd->autodel = autodel;
}

EOLIAN static Eina_Bool
_elm_win_autodel_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->autodel;
}

EOLIAN static void
_elm_win_activate(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   TRAP(sd, activate);
}

EOLIAN static void
_elm_win_lower(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   TRAP(sd, lower);
}

EOLIAN static void
_elm_win_raise(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   TRAP(sd, raise);
}

EOLIAN static void
_elm_win_center(Eo *obj, Elm_Win_Data *sd, Eina_Bool h, Eina_Bool v)
{
   int win_w, win_h, screen_w, screen_h, nx, ny;

   if ((trap) && (trap->center) && (!trap->center(sd->trap_data, obj)))
     return;

   ecore_evas_screen_geometry_get(sd->ee, NULL, NULL, &screen_w, &screen_h);
   if ((!screen_w) || (!screen_h)) return;

   evas_object_geometry_get(obj, NULL, NULL, &win_w, &win_h);
   if ((!win_w) || (!win_h)) return;

   if (h) nx = win_w >= screen_w ? 0 : (screen_w / 2) - (win_w / 2);
   else nx = sd->screen.x;
   if (v) ny = win_h >= screen_h ? 0 : (screen_h / 2) - (win_h / 2);
   else ny = sd->screen.y;
   if (nx < 0) nx = 0;
   if (ny < 0) ny = 0;

   evas_object_move(obj, nx, ny);
}

EOLIAN static void
_elm_win_borderless_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool borderless)
{
   Eina_Bool need_frame = EINA_FALSE;

   const char *engine_name = ecore_evas_engine_name_get(sd->ee);
   need_frame = engine_name &&
                ((!strcmp(engine_name, ELM_WAYLAND_SHM)) ||
                 (!strcmp(engine_name, ELM_WAYLAND_EGL)));

   if (need_frame)
     need_frame = !sd->fullscreen;

   if (borderless)
     {
        if (need_frame)
          _elm_win_frame_del(sd);
     }
   else
     {
        if (need_frame)
          _elm_win_frame_add(sd, "default");

        if (sd->frame_obj)
          evas_object_show(sd->frame_obj);
     }

   TRAP(sd, borderless_set, borderless);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_borderless_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return ecore_evas_borderless_get(sd->ee);
}

EOLIAN static void
_elm_win_shaped_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool shaped)
{
   TRAP(sd, shaped_set, shaped);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_shaped_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return ecore_evas_shaped_get(sd->ee);
}

EOLIAN static void
_elm_win_alpha_set(Eo *obj, Elm_Win_Data *sd, Eina_Bool enabled)
{
   if (sd->img_obj)
     {
        evas_object_image_alpha_set(sd->img_obj, enabled);
        ecore_evas_alpha_set(sd->ee, enabled);
     }
   else
     {
#ifdef HAVE_ELEMENTARY_X
        if (sd->x.xwin)
          {
             if (enabled)
               {
                  if (!ecore_x_screen_is_composited(0))
                    elm_win_shaped_set(obj, enabled);
                  else
                    TRAP(sd, alpha_set, enabled);
               }
             else
               TRAP(sd, alpha_set, enabled);
             _elm_win_xwin_update(sd);
          }
        else
#endif
          TRAP(sd, alpha_set, enabled);
     }
}

EOLIAN static Eina_Bool
_elm_win_alpha_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   if (sd->img_obj)
     {
        return evas_object_image_alpha_get(sd->img_obj);
     }
   else
     {
        return ecore_evas_alpha_get(sd->ee);
     }

   return EINA_FALSE;
}

EOLIAN static void
_elm_win_override_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool override)
{
   TRAP(sd, override_set, override);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_override_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return ecore_evas_override_get(sd->ee);
}

EOLIAN static void
_elm_win_fullscreen_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool fullscreen)
{
   // YYY: handle if sd->img_obj
   if (ENGINE_COMPARE(ELM_SOFTWARE_FB) ||
       ENGINE_COMPARE(ELM_DRM))
     {
        // these engines... can ONLY be fullscreen
        return;
     }
   else
     {
//        sd->fullscreen = fullscreen;

        Eina_Bool need_frame = EINA_FALSE;

        const char *engine_name = ecore_evas_engine_name_get(sd->ee);
        need_frame = engine_name &&
                     ((!strcmp(engine_name, ELM_WAYLAND_SHM)) ||
                      (!strcmp(engine_name, ELM_WAYLAND_EGL)));

        if (need_frame)
          need_frame = !ecore_evas_borderless_get(sd->ee);

        TRAP(sd, fullscreen_set, fullscreen);

        if (fullscreen)
          {
             if (need_frame)
               _elm_win_frame_del(sd);
          }
        else
          {
             if (need_frame)
               _elm_win_frame_add(sd, "default");

             if (sd->frame_obj)
               evas_object_show(sd->frame_obj);
          }
#ifdef HAVE_ELEMENTARY_X
        _elm_win_xwin_update(sd);
#endif
     }
}

EOLIAN static Eina_Bool
_elm_win_fullscreen_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   const char* engine_name = ecore_evas_engine_name_get(sd->ee);

   if (engine_name &&
       ((!strcmp(engine_name, ELM_SOFTWARE_FB)) ||
        (!strcmp(engine_name, ELM_DRM))))
     {
        // these engines... can ONLY be fullscreen
        return EINA_TRUE;
     }
   else
     {
        return sd->fullscreen;
     }

   return EINA_FALSE;
}

static void
_dbus_menu_set(Eina_Bool dbus_connect, void *data)
{
   ELM_WIN_DATA_GET_OR_RETURN(data, sd);

   if (dbus_connect)
     {
        DBG("Setting menu to D-Bus");
        edje_object_part_unswallow(sd->layout, sd->main_menu);
        edje_object_signal_emit(sd->layout, "elm,action,hide_menu", "elm");
        _elm_menu_menu_bar_hide(sd->main_menu);
     }
   else
     {
        DBG("Setting menu to local mode");
        edje_object_part_swallow(sd->layout, "elm.swallow.menu", sd->main_menu);
        edje_object_signal_emit(sd->layout, "elm,action,show_menu", "elm");
        evas_object_show(sd->main_menu);
     }
}

EOLIAN static Evas_Object *
_elm_win_main_menu_get(Eo *obj, Elm_Win_Data *sd)
{
   Eina_Bool use_dbus = EINA_FALSE;

   if (sd->main_menu) goto end;

   sd->main_menu = elm_menu_add(obj);
   _elm_menu_menu_bar_set(sd->main_menu, EINA_TRUE);

#ifdef HAVE_ELEMENTARY_X
   if (!_elm_config->disable_external_menu && sd->x.xwin) use_dbus = EINA_TRUE;
#endif

#ifdef HAVE_ELEMENTARY_X
   if (use_dbus && _elm_dbus_menu_register(sd->main_menu))
     {
        _elm_dbus_menu_app_menu_register(sd->x.xwin, sd->main_menu,
                                         _dbus_menu_set, obj);
     }
   else
#endif
     _dbus_menu_set(EINA_FALSE, obj);

end:
   return sd->main_menu;
}

EOLIAN static void
_elm_win_maximized_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool maximized)
{
   _elm_win_frame_maximized_state_update(sd, maximized);
   // YYY: handle if sd->img_obj
   TRAP(sd, maximized_set, maximized);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_maximized_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->maximized;
}

EOLIAN static void
_elm_win_iconified_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool iconified)
{
//   sd->iconified = iconified;
   TRAP(sd, iconified_set, iconified);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_iconified_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->iconified;
}

EOLIAN static void
_elm_win_withdrawn_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool withdrawn)
{
//   sd->withdrawn = withdrawn;
   TRAP(sd, withdrawn_set, withdrawn);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_withdrawn_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->withdrawn;
}

EOLIAN static void
_elm_win_available_profiles_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, const char **profiles, unsigned int count)
{
   Eina_Bool found = EINA_FALSE;

   _elm_win_available_profiles_del(sd);
   if ((profiles) && (count >= 1))
     {
        sd->profile.available_list = calloc(count, sizeof(char *));
        if (sd->profile.available_list)
          {
             if (!sd->profile.name) found = EINA_TRUE;

             unsigned int i;
             for (i = 0; i < count; i++)
               {
                  sd->profile.available_list[i] = eina_stringshare_add(profiles[i]);

                  /* check to see if a given array has a current profile of elm_win */
                  if ((sd->profile.name) &&
                      (!strcmp(sd->profile.name, profiles[i])))
                    {
                       found = EINA_TRUE;
                    }
               }
             sd->profile.count = count;
          }
     }

   if (ecore_evas_window_profile_supported_get(sd->ee))
     {
        ecore_evas_window_available_profiles_set(sd->ee,
                                                 sd->profile.available_list,
                                                 sd->profile.count);

        /* current profile of elm_win is wrong, change profile */
        if ((sd->profile.available_list) && (!found))
          {
             eina_stringshare_replace(&(sd->profile.name),
                                      sd->profile.available_list[0]);
             ecore_evas_window_profile_set(sd->ee, sd->profile.name);
          }

     }
   else
     {
        if (sd->profile.available_list)
          _elm_win_profile_update(sd);
     }
}

EOLIAN static Eina_Bool
_elm_win_available_profiles_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, char ***profiles, unsigned int *count)
{
   if (ecore_evas_window_profile_supported_get(sd->ee))
     {
        return ecore_evas_window_available_profiles_get(sd->ee,
                                                       profiles,
                                                       count);
     }
   else
     {
        if (profiles) *profiles = (char **)sd->profile.available_list;
        if (count) *count = sd->profile.count;
        return EINA_TRUE;
     }
}

EOLIAN static void
_elm_win_profile_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, const char *profile)
{
   /* check to see if a given profile is present in an available profiles */
   if ((profile) && (sd->profile.available_list))
     {
        Eina_Bool found = EINA_FALSE;
        unsigned int i;
        for (i = 0; i < sd->profile.count; i++)
          {
             if (!strcmp(profile,
                         sd->profile.available_list[i]))
               {
                  found = EINA_TRUE;
                  break;
               }
          }
        if (!found) return;
     }

   if (ecore_evas_window_profile_supported_get(sd->ee))
     {
        if (!profile) _elm_win_profile_del(sd);
        ecore_evas_window_profile_set(sd->ee, profile);
     }
   else
     {
        if (_internal_elm_win_profile_set(sd, profile))
          _elm_win_profile_update(sd);
     }
}

EOLIAN static const char*
_elm_win_profile_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->profile.name;
}

EOLIAN static void
_elm_win_urgent_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool urgent)
{
   sd->urgent = urgent;
   TRAP(sd, urgent_set, urgent);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_urgent_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->urgent;
}

EOLIAN static void
_elm_win_demand_attention_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool demand_attention)
{
   sd->demand_attention = demand_attention;
   TRAP(sd, demand_attention_set, demand_attention);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_demand_attention_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->demand_attention;
}

EOLIAN static void
_elm_win_modal_set(Eo *obj, Elm_Win_Data *sd, Eina_Bool modal)
{
   if (sd->modal_count) return;

   const Eina_List *l;
   Evas_Object *current;

   if ((modal) && (!sd->modal) && (evas_object_visible_get(obj)))
     {
       INCREMENT_MODALITY()
     }
   else if ((!modal) && (sd->modal) && (evas_object_visible_get(obj)))
     {
       DECREMENT_MODALITY()
     }

   sd->modal = modal;
   TRAP(sd, modal_set, modal);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_modal_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->modal;
}

EOLIAN static void
_elm_win_aspect_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, double aspect)
{
   sd->aspect = aspect;
   TRAP(sd, aspect_set, aspect);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static double
_elm_win_aspect_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->aspect;
}

EOLIAN static void
_elm_win_size_base_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int w, int h)
{
   sd->size_base_w = w;
   sd->size_base_h = h;
   TRAP(sd, size_base_set, w, h);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static void
_elm_win_size_base_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int *w, int *h)
{
   if (w) *w = sd->size_base_w;
   if (w) *h = sd->size_base_h;
}

EOLIAN static void
_elm_win_size_step_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int w, int h)
{
   sd->size_step_w = w;
   sd->size_step_h = h;
   TRAP(sd, size_step_set, w, h);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static void
_elm_win_size_step_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int *w, int *h)
{
   if (w) *w = sd->size_step_w;
   if (w) *h = sd->size_step_h;
}

EOLIAN static void
_elm_win_layer_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int layer)
{
   TRAP(sd, layer_set, layer);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static int
_elm_win_layer_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return ecore_evas_layer_get(sd->ee);
}

EAPI void
elm_win_norender_push(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->norender++;
   if (sd->norender == 1) ecore_evas_manual_render_set(sd->ee, EINA_TRUE);
}

EAPI void
elm_win_norender_pop(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (sd->norender <= 0) return;
   sd->norender--;
   if (sd->norender == 0) ecore_evas_manual_render_set(sd->ee, EINA_FALSE);
}

EAPI int
elm_win_norender_get(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, -1);
   return sd->norender;
}

EAPI void
elm_win_render(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);
   ecore_evas_manual_render(sd->ee);
}

static int
_win_rotation_degree_check(int rotation)
{
   if ((rotation > 360) || (rotation < 0))
     {
        WRN("Rotation degree should be 0 ~ 360 (passed degree: %d)", rotation);
        rotation %= 360;
        if (rotation < 0) rotation += 360;
     }
   return rotation;
}

/*
 * This API resizes the internal window(ex: X window) and evas_output.
 * But this does not resize the elm window object and its contents.
 */
static void
_win_rotate(Evas_Object *obj, Elm_Win_Data *sd, int rotation, Eina_Bool resize)
{
   rotation = _win_rotation_degree_check(rotation);
   if (sd->rot == rotation) return;
   sd->rot = rotation;
   if (resize) TRAP(sd, rotation_with_resize_set, rotation);
   else TRAP(sd, rotation_set, rotation);
   evas_object_size_hint_min_set(obj, -1, -1);
   evas_object_size_hint_max_set(obj, -1, -1);
   _elm_win_resize_objects_eval(obj);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
   _elm_win_frame_obj_update(sd);
   elm_widget_orientation_set(obj, rotation);
   evas_object_smart_callback_call(obj, SIG_ROTATION_CHANGED, NULL);
}

EOLIAN static void
_elm_win_rotation_set(Eo *obj, Elm_Win_Data *sd, int rotation)
{
   _win_rotate(obj, sd, rotation, EINA_FALSE);
}

/*
 * This API does not resize the internal window (ex: X window).
 * But this resizes evas_output, elm window, and its contents.
 */
EOLIAN static void
_elm_win_rotation_with_resize_set(Eo *obj, Elm_Win_Data *sd, int rotation)
{
   _win_rotate(obj, sd, rotation, EINA_TRUE);
}

EOLIAN static int
_elm_win_rotation_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->rot;
}

EOLIAN static Eina_Bool
_elm_win_wm_rotation_supported_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->wm_rot.wm_supported;
}

/* This will unset a preferred rotation, if given preferred rotation is '-1'.
 */
EAPI void
elm_win_wm_rotation_preferred_rotation_set(const Evas_Object *obj,
                                           int rotation)
{
   ELM_WIN_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_win_wm_preferred_rotation_set(rotation));
}

EOLIAN static void
_elm_win_wm_preferred_rotation_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int rotation)
{
   int rot;

   if (!sd->wm_rot.use)
     sd->wm_rot.use = EINA_TRUE;

   // '-1' means that elm_win doesn't use preferred rotation.
   if (rotation == -1)
     rot = -1;
   else
     rot = _win_rotation_degree_check(rotation);

   if (sd->wm_rot.preferred_rot == rot) return;
   sd->wm_rot.preferred_rot = rot;

   ecore_evas_wm_rotation_preferred_rotation_set(sd->ee, rot);
}

EOLIAN static int
_elm_win_wm_preferred_rotation_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->wm_rot.preferred_rot;
}

EOLIAN static void
_elm_win_wm_available_rotations_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, const int *rotations, unsigned int count)
{
   unsigned int i;
   int r;

   if (!sd->wm_rot.use)
     sd->wm_rot.use = EINA_TRUE;

   ELM_SAFE_FREE(sd->wm_rot.rots, free);
   sd->wm_rot.count = 0;

   if (count > 0)
     {
        sd->wm_rot.rots = calloc(count, sizeof(int));
        if (!sd->wm_rot.rots) return;
        for (i = 0; i < count; i++)
          {
             r = _win_rotation_degree_check(rotations[i]);
             sd->wm_rot.rots[i] = r;
          }
     }

   sd->wm_rot.count = count;

   ecore_evas_wm_rotation_available_rotations_set(sd->ee,
                                                  sd->wm_rot.rots,
                                                  sd->wm_rot.count);
}

EOLIAN static Eina_Bool
_elm_win_wm_available_rotations_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int **rotations, unsigned int *count)
{
   if (!sd->wm_rot.use) return EINA_FALSE;

   if (sd->wm_rot.count > 0)
     {
        if (rotations)
          {
             *rotations = calloc(sd->wm_rot.count, sizeof(int));
             if (*rotations)
               {
                  memcpy(*rotations,
                         sd->wm_rot.rots,
                         sizeof(int) * sd->wm_rot.count);
               }
          }
     }

   if (count) *count = sd->wm_rot.count;
   return EINA_TRUE;
}

EOLIAN static void
_elm_win_wm_manual_rotation_done_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool set)
{
   if (!sd->wm_rot.use) return;
   ecore_evas_wm_rotation_manual_rotation_done_set(sd->ee, set);
}

EOLIAN static Eina_Bool
_elm_win_wm_manual_rotation_done_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   if (!sd->wm_rot.use) return EINA_FALSE;
   return ecore_evas_wm_rotation_manual_rotation_done_get(sd->ee);
}

EOLIAN static void
_elm_win_wm_manual_rotation_done(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   if (!sd->wm_rot.use) return;
   ecore_evas_wm_rotation_manual_rotation_done(sd->ee);
}

EOLIAN static void
_elm_win_sticky_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool sticky)
{
//   sd->sticky = sticky;
   TRAP(sd, sticky_set, sticky);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EOLIAN static Eina_Bool
_elm_win_sticky_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->sticky;
}

EOLIAN static void
_elm_win_keyboard_mode_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Elm_Win_Keyboard_Mode mode)
{
   if (mode == sd->kbdmode) return;
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
#endif
   sd->kbdmode = mode;
#ifdef HAVE_ELEMENTARY_X
   if (sd->x.xwin)
     ecore_x_e_virtual_keyboard_state_set
       (sd->x.xwin, (Ecore_X_Virtual_Keyboard_State)sd->kbdmode);
#endif
}

EOLIAN static Elm_Win_Keyboard_Mode
_elm_win_keyboard_mode_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->kbdmode;
}

EOLIAN static void
_elm_win_keyboard_win_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool is_keyboard)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_virtual_keyboard_set(sd->x.xwin, is_keyboard);
#else
   (void)is_keyboard;
#endif
}

EOLIAN static Eina_Bool
_elm_win_keyboard_win_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin) return ecore_x_e_virtual_keyboard_get(sd->x.xwin);
#endif
   return EINA_FALSE;
}

EOLIAN static void
_elm_win_indicator_mode_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Elm_Win_Indicator_Mode mode)
{
   if (mode == sd->indmode) return;
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
#endif
   sd->indmode = mode;
#ifdef HAVE_ELEMENTARY_X
   if (sd->x.xwin)
     {
        if (sd->indmode == ELM_WIN_INDICATOR_SHOW)
          ecore_x_e_illume_indicator_state_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_STATE_ON);
        else if (sd->indmode == ELM_WIN_INDICATOR_HIDE)
          ecore_x_e_illume_indicator_state_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_STATE_OFF);
     }
#endif
   evas_object_smart_callback_call(obj, SIG_INDICATOR_PROP_CHANGED, NULL);
}

EOLIAN static Elm_Win_Indicator_Mode
_elm_win_indicator_mode_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->indmode;
}

EOLIAN static void
_elm_win_indicator_opacity_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Elm_Win_Indicator_Opacity_Mode mode)
{
   if (mode == sd->ind_o_mode) return;
   sd->ind_o_mode = mode;
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        if (sd->ind_o_mode == ELM_WIN_INDICATOR_OPAQUE)
          ecore_x_e_illume_indicator_opacity_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_OPAQUE);
        else if (sd->ind_o_mode == ELM_WIN_INDICATOR_TRANSLUCENT)
          ecore_x_e_illume_indicator_opacity_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_TRANSLUCENT);
        else if (sd->ind_o_mode == ELM_WIN_INDICATOR_TRANSPARENT)
          ecore_x_e_illume_indicator_opacity_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_TRANSPARENT);
     }
#endif
   evas_object_smart_callback_call(obj, SIG_INDICATOR_PROP_CHANGED, NULL);
}

EOLIAN static Elm_Win_Indicator_Opacity_Mode
_elm_win_indicator_opacity_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->ind_o_mode;
}

EOLIAN static void
_elm_win_screen_position_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int *x, int *y)
{
   if (x) *x = sd->screen.x;
   if (y) *y = sd->screen.y;
}

EOLIAN static Eina_Bool
_elm_win_focus_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return ecore_evas_focus_get(sd->ee);
}

EOLIAN static void
_elm_win_screen_constrain_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool constrain)
{
   sd->constrain = !!constrain;
}

EOLIAN static Eina_Bool
_elm_win_screen_constrain_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->constrain;
}

EOLIAN static void
_elm_win_screen_size_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int *x, int *y, int *w, int *h)
{
   ecore_evas_screen_geometry_get(sd->ee, x, y, w, h);
}

EOLIAN static void
_elm_win_screen_dpi_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int *xdpi, int *ydpi)
{
   ecore_evas_screen_dpi_get(sd->ee, xdpi, ydpi);
}

EOLIAN static void
_elm_win_conformant_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool conformant)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_conformant_set(sd->x.xwin, conformant);
#else
   (void)conformant;
#endif
}

EOLIAN static Eina_Bool
_elm_win_conformant_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_conformant_get(sd->x.xwin);
#endif

   return EINA_FALSE;
}

EOLIAN static void
_elm_win_quickpanel_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool quickpanel)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        ecore_x_e_illume_quickpanel_set(sd->x.xwin, quickpanel);
        if (quickpanel)
          {
             Ecore_X_Window_State states[2];

             states[0] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
             states[1] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
             ecore_x_netwm_window_state_set(sd->x.xwin, states, 2);
             ecore_x_icccm_hints_set(sd->x.xwin, 0, 0, 0, 0, 0, 0, 0);
          }
     }
#else
   (void)quickpanel;
#endif
}

EOLIAN static Eina_Bool
_elm_win_quickpanel_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_quickpanel_get(sd->x.xwin);
#endif

   return EINA_FALSE;
}

EOLIAN static void
_elm_win_quickpanel_priority_major_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int priority)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_quickpanel_priority_major_set(sd->x.xwin, priority);
#else
   (void)priority;
#endif
}

EOLIAN static int
_elm_win_quickpanel_priority_major_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_quickpanel_priority_major_get(sd->x.xwin);
#endif

   return -1;
}

EOLIAN static void
_elm_win_quickpanel_priority_minor_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int priority)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_quickpanel_priority_minor_set(sd->x.xwin, priority);
#else
   (void)priority;
#endif
}

EOLIAN static int
_elm_win_quickpanel_priority_minor_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_quickpanel_priority_minor_get(sd->x.xwin);
#endif

   return -1;
}

EOLIAN static void
_elm_win_quickpanel_zone_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, int zone)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_quickpanel_zone_set(sd->x.xwin, zone);
#else
   (void)zone;
#endif
}

EOLIAN static int
_elm_win_quickpanel_zone_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_quickpanel_zone_get(sd->x.xwin);
#endif

   return 0;
}

EOLIAN static void
_elm_win_prop_focus_skip_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool skip)
{
   sd->skip_focus = skip;
   TRAP(sd, focus_skip_set, skip);
}

EOLIAN static void
_elm_win_illume_command_send(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Elm_Illume_Command command, void *params)
{
   (void) params;

#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        switch (command)
          {
           case ELM_ILLUME_COMMAND_FOCUS_BACK:
             ecore_x_e_illume_focus_back_send(sd->x.xwin);
             break;

           case ELM_ILLUME_COMMAND_FOCUS_FORWARD:
             ecore_x_e_illume_focus_forward_send(sd->x.xwin);
             break;

           case ELM_ILLUME_COMMAND_FOCUS_HOME:
             ecore_x_e_illume_focus_home_send(sd->x.xwin);
             break;

           case ELM_ILLUME_COMMAND_CLOSE:
             ecore_x_e_illume_close_send(sd->x.xwin);
             break;

           default:
             break;
          }
     }
#else
   (void)command;
#endif
}

EOLIAN static Evas_Object*
_elm_win_inlined_image_object_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->img_obj;
}

EOLIAN static void
_elm_win_focus_highlight_enabled_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool enabled)
{
   enabled = !!enabled;
   if (sd->focus_highlight.enabled == enabled)
     return;

   sd->focus_highlight.enabled = enabled;

   if (sd->focus_highlight.enabled)
     _elm_win_focus_highlight_init(sd);
   else
     _elm_win_focus_highlight_shutdown(sd);
}

EOLIAN static Eina_Bool
_elm_win_focus_highlight_enabled_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->focus_highlight.enabled;
}

EOLIAN static Eina_Bool
_elm_win_elm_widget_theme_apply(Eo *obj, Elm_Win_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_TRUE;

   sd->focus_highlight.theme_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);

   return EINA_TRUE;
}

EOLIAN static void
_elm_win_focus_highlight_style_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, const char *style)
{
   eina_stringshare_replace(&sd->focus_highlight.style, style);
   sd->focus_highlight.theme_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

EOLIAN static const char*
_elm_win_focus_highlight_style_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->focus_highlight.style;
}

EOLIAN static void
_elm_win_focus_highlight_animate_set(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, Eina_Bool animate)
{
   animate = !!animate;
   if (sd->focus_highlight.animate == animate)
     return;

   sd->focus_highlight.animate = animate;
   sd->focus_highlight.theme_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

EOLIAN static Eina_Bool
_elm_win_focus_highlight_animate_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   return sd->focus_highlight.animate;
}

EOLIAN static Eina_Bool
_elm_win_socket_listen(Eo *obj EINA_UNUSED, Elm_Win_Data *sd, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   if (!sd->ee) return EINA_FALSE;

   if (!ecore_evas_extn_socket_listen(sd->ee, svcname, svcnum, svcsys))
     return EINA_FALSE;

   return EINA_TRUE;
}

/* windowing specific calls - shall we do this differently? */

EOLIAN static Ecore_X_Window
_elm_win_xwindow_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
#ifdef HAVE_ELEMENTARY_X
   if (sd->x.xwin) return sd->x.xwin;
   if (sd->parent) return elm_win_xwindow_get(sd->parent);
#endif
   return 0;
}

EAPI Ecore_Wl_Window *
elm_win_wl_window_get(const Evas_Object *obj)
{
   if (!obj) return NULL;

   if ((!ENGINE_COMPARE(ELM_WAYLAND_SHM)) &&
       (!ENGINE_COMPARE(ELM_WAYLAND_EGL)))
     return NULL;

   if (!evas_object_smart_type_check_ptr(obj, MY_CLASS_NAME_LEGACY))
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        return _elm_ee_wlwin_get(ee);
     }

   ELM_WIN_CHECK(obj) NULL;
   Ecore_Wl_Window *ret = NULL;
   eo_do((Eo *) obj, ret = elm_obj_win_wl_window_get());
   return ret;
}

EOLIAN static Ecore_Wl_Window*
_elm_win_wl_window_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
#if HAVE_ELEMENTARY_WAYLAND
   if (sd->wl.win) return sd->wl.win;
   if (sd->parent) return elm_win_wl_window_get(sd->parent);
#else
   (void)sd;
#endif
   return NULL;
}

EAPI Eina_Bool
elm_win_trap_set(const Elm_Win_Trap *t)
{
   DBG("old %p, new %p", trap, t);

   if ((t) && (t->version != ELM_WIN_TRAP_VERSION))
     {
        CRI("trying to set a trap version %lu while %lu was expected!",
                 t->version, ELM_WIN_TRAP_VERSION);
        return EINA_FALSE;
     }

   trap = t;
   return EINA_TRUE;
}

EAPI void
elm_win_floating_mode_set(Evas_Object *obj, Eina_Bool floating)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   floating = !!floating;
   if (floating == sd->floating) return;
   sd->floating = floating;
#ifdef HAVE_ELEMENTARY_X
   _internal_elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        if (sd->floating)
          ecore_x_e_illume_window_state_set
             (sd->x.xwin, ECORE_X_ILLUME_WINDOW_STATE_FLOATING);
        else
          ecore_x_e_illume_window_state_set
             (sd->x.xwin, ECORE_X_ILLUME_WINDOW_STATE_NORMAL);
     }
#endif
}

EAPI Eina_Bool
elm_win_floating_mode_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->floating;
}

EOLIAN static Ecore_Window
_elm_win_window_id_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd)
{
   if ((ENGINE_COMPARE(ELM_WAYLAND_SHM)) || (ENGINE_COMPARE(ELM_WAYLAND_EGL)))
     {
#if HAVE_ELEMENTARY_WAYLAND
        if (sd->wl.win) return (Ecore_Window)ecore_wl_window_surface_id_get(sd->wl.win);
        if (sd->parent)
          {
             Ecore_Wl_Window *parent;

             parent = elm_win_wl_window_get(sd->parent);
             if (parent) return (Ecore_Window)ecore_wl_window_surface_id_get(parent);
             return 0;
          }
#endif
     }
   else
     {
#ifdef HAVE_ELEMENTARY_X
        _internal_elm_win_xwindow_get(sd);
        if (sd->x.xwin) return (Ecore_Window)sd->x.xwin;
        if (sd->parent) return (Ecore_Window)elm_win_xwindow_get(sd->parent);
#endif
     }

   return 0;
}

void
_elm_win_focus_highlight_in_theme_update(Evas_Object *obj, Eina_Bool in_theme)
{
   ELM_WIN_DATA_GET(obj, sd);
   sd->focus_highlight.cur.in_theme = !!in_theme;
}

void
_elm_win_focus_highlight_start(Evas_Object *obj)
{
   ELM_WIN_DATA_GET(obj, sd);

   if (!elm_win_focus_highlight_enabled_get(obj)) return;
   sd->focus_highlight.cur.visible = EINA_TRUE;
   sd->focus_highlight.geometry_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job(obj);
}

EAPI Ecore_Window
elm_win_window_id_get(const Evas_Object *obj)
{
   if (!obj) return 0;

   if (!evas_object_smart_type_check_ptr(obj, MY_CLASS_NAME_LEGACY))
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        return ecore_evas_window_get(ee);
     }

   ELM_WIN_CHECK(obj) 0;
   Ecore_Window ret = 0;
   eo_do((Eo *) obj, ret = elm_obj_win_window_id_get());
   return ret;
}

EOLIAN static void
_elm_win_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo*
_elm_win_elm_interface_atspi_accessible_parent_get(Eo *obj EINA_UNUSED, Elm_Win_Data *sd EINA_UNUSED)
{
   // attach all kinds of windows directly to ATSPI application root object
   return _elm_atspi_bridge_root_get();
}

#include "elm_win.eo.c"
