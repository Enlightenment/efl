#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

static const char WIN_SMART_NAME[] = "elm_win";

static const Elm_Win_Trap *trap = NULL;

#define TRAP(sd, name, ...)                                             \
  do                                                                    \
    {                                                                   \
       if ((!trap) || (!trap->name) ||                                  \
           ((trap->name) &&                                             \
            (trap->name(sd->trap_data, sd->base.obj, ## __VA_ARGS__)))) \
         ecore_evas_##name(sd->ee, ##__VA_ARGS__);                      \
    }                                                                   \
  while (0)

#define ELM_WIN_DATA_GET(o, sd) \
  Elm_Win_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_WIN_DATA_GET_OR_RETURN(o, ptr)           \
  ELM_WIN_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_WIN_DATA_GET_OR_RETURN_VAL(o, ptr, val)  \
  ELM_WIN_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_WIN_CHECK(obj)                                             \
  if (!obj || !elm_widget_type_check((obj), WIN_SMART_NAME, __func__)) \
    return

#define ENGINE_GET() (_elm_preferred_engine ? _elm_preferred_engine : (_elm_config->engine ? _elm_config->engine : ""))
#define ENGINE_COMPARE(name) (!strcmp(ENGINE_GET(), name))

typedef struct _Elm_Win_Smart_Data Elm_Win_Smart_Data;

struct _Elm_Win_Smart_Data
{
   Elm_Widget_Smart_Data base; /* base widget smart data as
                                * first member obligatory, as
                                * we're inheriting from it */

   Ecore_Evas           *ee;
   Evas                 *evas;
   Evas_Object          *parent; /* parent *window* object*/
   Evas_Object          *img_obj, *frame_obj;
   Eina_List            *resize_objs; /* a window may have
                                       * *multiple* resize
                                       * objects */
#ifdef HAVE_ELEMENTARY_X
   struct
   {
      Ecore_X_Window       xwin;
      Ecore_Event_Handler *client_message_handler;
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
   int                            show_count;
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
      Evas_Object *top;

      struct
      {
         Evas_Object *target;
         Eina_Bool    visible : 1;
         Eina_Bool    handled : 1;
      } cur, prev;

      const char  *style;
      Ecore_Job   *reconf_job;

      Eina_Bool    enabled : 1;
      Eina_Bool    changed_theme : 1;
      Eina_Bool    top_animate : 1;
      Eina_Bool    geometry_changed : 1;
   } focus_highlight;

   Evas_Object *icon;
   const char  *title;
   const char  *icon_name;
   const char  *role;

   void *trap_data;

   double       aspect;
   int          size_base_w, size_base_h;
   int          size_step_w, size_step_h;
   int          norender;
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
};

static const char SIG_DELETE_REQUEST[] = "delete,request";
static const char SIG_FOCUS_OUT[] = "focus,out";
static const char SIG_FOCUS_IN[] = "focus,in";
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
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (WIN_SMART_NAME, _elm_win, Elm_Widget_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, _smart_callbacks);

Eina_List *_elm_win_list = NULL;
int _elm_win_deferred_free = 0;

// example shot spec (wait 0.1 sec then save as my-window.png):
// ELM_ENGINE="shot:delay=0.1:file=my-window.png"

static double
_shot_delay_get(Elm_Win_Smart_Data *sd)
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
_shot_file_get(Elm_Win_Smart_Data *sd)
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
_shot_repeat_count_get(Elm_Win_Smart_Data *sd)
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
_shot_key_get(Elm_Win_Smart_Data *sd __UNUSED__)
{
   return NULL;
}

static char *
_shot_flags_get(Elm_Win_Smart_Data *sd __UNUSED__)
{
   return NULL;
}

static void
_shot_do(Elm_Win_Smart_Data *sd)
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
   if (key) free(key);
   if (flags) free(flags);
   ecore_evas_free(ee);
   if (sd->shot.repeat_count) sd->shot.shot_counter++;
}

static Eina_Bool
_shot_delay(void *data)
{
   Elm_Win_Smart_Data *sd = data;

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
_shot_init(Elm_Win_Smart_Data *sd)
{
   if (!sd->shot.info) return;

   sd->shot.repeat_count = _shot_repeat_count_get(sd);
   sd->shot.shot_counter = 0;
}

static void
_shot_handle(Elm_Win_Smart_Data *sd)
{
   if (!sd->shot.info) return;

   sd->shot.timer = ecore_timer_add(_shot_delay_get(sd), _shot_delay, sd);
}

/* elm-win specific associate, does the trap while ecore_evas_object_associate()
 * does not.
 */
static Elm_Win_Smart_Data *
_elm_win_associate_get(const Ecore_Evas *ee)
{
   return ecore_evas_data_get(ee, "elm_win");
}

/* Interceptors Callbacks */
static void
_elm_win_obj_intercept_raise(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   TRAP(sd, raise);
}

static void
_elm_win_obj_intercept_lower(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   TRAP(sd, lower);
}

static void
_elm_win_obj_intercept_stack_above(void *data __UNUSED__, Evas_Object *obj __UNUSED__, Evas_Object *above __UNUSED__)
{
   INF("TODO: %s", __FUNCTION__);
}

static void
_elm_win_obj_intercept_stack_below(void *data __UNUSED__, Evas_Object *obj __UNUSED__, Evas_Object *below __UNUSED__)
{
   INF("TODO: %s", __FUNCTION__);
}

static void
_elm_win_obj_intercept_layer_set(void *data, Evas_Object *obj __UNUSED__, int l)
{
   Elm_Win_Smart_Data *sd = data;
   TRAP(sd, layer_set, l);
}

/* Event Callbacks */

static void
_elm_win_obj_callback_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
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
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   int x, y;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   sd->screen.x = x;
   sd->screen.y = y;
   evas_object_smart_callback_call(ELM_WIDGET_DATA(sd)->obj, SIG_MOVED, NULL);
}

static void
_elm_win_resize_job(void *data)
{
   Elm_Win_Smart_Data *sd = data;
   const Eina_List *l;
   Evas_Object *obj;
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
        int fw, fh;

        evas_output_framespace_get(sd->evas, NULL, NULL, &fw, &fh);
        evas_object_resize(sd->frame_obj, w + fw, h + fh);
     }

   /* if (sd->img_obj) */
   /*   { */
   /*   } */

   evas_object_resize(ELM_WIDGET_DATA(sd)->obj, w, h);
   EINA_LIST_FOREACH(sd->resize_objs, l, obj)
     {
        evas_object_move(obj, 0, 0);
        evas_object_resize(obj, w, h);
     }
}

static void
_elm_win_resize(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   EINA_SAFETY_ON_NULL_RETURN(sd);

   if (sd->deferred_resize_job) ecore_job_del(sd->deferred_resize_job);
   sd->deferred_resize_job = ecore_job_add(_elm_win_resize_job, sd);
}

static void
_elm_win_mouse_in(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   EINA_SAFETY_ON_NULL_RETURN(sd);

   if (sd->resizing) sd->resizing = EINA_FALSE;
}

static void
_elm_win_focus_highlight_reconfigure_job_stop(Elm_Win_Smart_Data *sd)
{
   if (sd->focus_highlight.reconf_job)
     ecore_job_del(sd->focus_highlight.reconf_job);
   sd->focus_highlight.reconf_job = NULL;
}

static void
_elm_win_focus_highlight_visible_set(Elm_Win_Smart_Data *sd,
                                     Eina_Bool visible)
{
   Evas_Object *top;

   top = sd->focus_highlight.top;
   if (visible)
     {
        if (top)
          {
             evas_object_show(top);
             edje_object_signal_emit(top, "elm,action,focus,show", "elm");
          }
     }
   else
     {
        if (top)
          edje_object_signal_emit(top, "elm,action,focus,hide", "elm");
     }
}

static void
_elm_win_focus_highlight_anim_setup(Elm_Win_Smart_Data *sd,
                                    Evas_Object *obj)
{
   Evas_Coord tx, ty, tw, th;
   Evas_Coord w, h, px, py, pw, ph;
   Edje_Message_Int_Set *m;
   Evas_Object *previous = sd->focus_highlight.prev.target;
   Evas_Object *target = sd->focus_highlight.cur.target;

   evas_object_geometry_get(ELM_WIDGET_DATA(sd)->obj, NULL, NULL, &w, &h);
   evas_object_geometry_get(target, &tx, &ty, &tw, &th);
   evas_object_geometry_get(previous, &px, &py, &pw, &ph);
   evas_object_move(obj, 0, 0);
   evas_object_resize(obj, tw, th);
   evas_object_clip_unset(obj);

   m = alloca(sizeof(*m) + (sizeof(int) * 8));
   m->count = 8;
   m->val[0] = px;
   m->val[1] = py;
   m->val[2] = pw;
   m->val[3] = ph;
   m->val[4] = tx;
   m->val[5] = ty;
   m->val[6] = tw;
   m->val[7] = th;
   edje_object_message_send(obj, EDJE_MESSAGE_INT_SET, 1, m);
}

static void
_elm_win_focus_highlight_simple_setup(Elm_Win_Smart_Data *sd,
                                      Evas_Object *obj)
{
   Evas_Object *clip, *target = sd->focus_highlight.cur.target;
   Evas_Coord x, y, w, h;

   clip = evas_object_clip_get(target);
   evas_object_geometry_get(target, &x, &y, &w, &h);

   evas_object_move(obj, x, y);
   evas_object_resize(obj, w, h);
   evas_object_clip_set(obj, clip);
}

static void
_elm_win_focus_highlight_reconfigure(Elm_Win_Smart_Data *sd)
{
   Evas_Object *target = sd->focus_highlight.cur.target;
   Evas_Object *previous = sd->focus_highlight.prev.target;
   Evas_Object *top = sd->focus_highlight.top;
   Eina_Bool visible_changed;
   Eina_Bool common_visible;
   const char *sig = NULL;

   _elm_win_focus_highlight_reconfigure_job_stop(sd);

   visible_changed = (sd->focus_highlight.cur.visible !=
                      sd->focus_highlight.prev.visible);

   if ((target == previous) && (!visible_changed) &&
       (!sd->focus_highlight.geometry_changed))
     return;

   if ((previous) && (sd->focus_highlight.prev.handled))
     elm_widget_signal_emit
       (previous, "elm,action,focus_highlight,hide", "elm");

   if (!target)
     common_visible = EINA_FALSE;
   else if (sd->focus_highlight.cur.handled)
     {
        common_visible = EINA_FALSE;
        if (sd->focus_highlight.cur.visible)
          sig = "elm,action,focus_highlight,show";
        else
          sig = "elm,action,focus_highlight,hide";
     }
   else
     common_visible = sd->focus_highlight.cur.visible;

   _elm_win_focus_highlight_visible_set(sd, common_visible);
   if (sig)
     elm_widget_signal_emit(target, sig, "elm");

   if ((!target) || (!common_visible) || (sd->focus_highlight.cur.handled))
     goto the_end;

   if (sd->focus_highlight.changed_theme)
     {
        const char *str;
        if (sd->focus_highlight.style)
          str = sd->focus_highlight.style;
        else
          str = "default";
        elm_widget_theme_object_set
          (ELM_WIDGET_DATA(sd)->obj, top, "focus_highlight", "top", str);
        sd->focus_highlight.changed_theme = EINA_FALSE;

        if (_elm_config->focus_highlight_animate)
          {
             str = edje_object_data_get(sd->focus_highlight.top, "animate");
             sd->focus_highlight.top_animate = ((str) && (!strcmp(str, "on")));
          }
     }

   if ((sd->focus_highlight.top_animate) && (previous) &&
       (!sd->focus_highlight.prev.handled))
     _elm_win_focus_highlight_anim_setup(sd, top);
   else
     _elm_win_focus_highlight_simple_setup(sd, top);
   evas_object_raise(top);

the_end:
   sd->focus_highlight.geometry_changed = EINA_FALSE;
   sd->focus_highlight.prev = sd->focus_highlight.cur;
}

static void
_elm_win_focus_highlight_reconfigure_job(void *data)
{
   _elm_win_focus_highlight_reconfigure((Elm_Win_Smart_Data *)data);
}

static void
_elm_win_focus_highlight_reconfigure_job_start(Elm_Win_Smart_Data *sd)
{
   if (sd->focus_highlight.reconf_job)
     ecore_job_del(sd->focus_highlight.reconf_job);

   sd->focus_highlight.reconf_job = ecore_job_add(
       _elm_win_focus_highlight_reconfigure_job, sd);
}

static void
_elm_win_focus_in(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   obj = ELM_WIDGET_DATA(sd)->obj;
   
   _elm_widget_top_win_focused_set(obj, EINA_TRUE);
   if (!elm_widget_focus_order_get(obj))
     {
        elm_widget_focus_steal(obj);
        sd->show_count++;
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
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   obj = ELM_WIDGET_DATA(sd)->obj;

   elm_object_focus_set(obj, EINA_FALSE);
   _elm_widget_top_win_focused_set(obj, EINA_FALSE);
   evas_object_smart_callback_call(obj, SIG_FOCUS_OUT, NULL);
   sd->focus_highlight.cur.visible = EINA_FALSE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
   if (sd->frame_obj)
     {
        edje_object_signal_emit(sd->frame_obj, "elm,action,unfocus", "elm");
     }

   /* do nothing */
   /* if (sd->img_obj) */
   /*   { */
   /*   } */
}

static void
_elm_win_state_change(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;
   Eina_Bool ch_withdrawn = EINA_FALSE;
   Eina_Bool ch_sticky = EINA_FALSE;
   Eina_Bool ch_iconified = EINA_FALSE;
   Eina_Bool ch_fullscreen = EINA_FALSE;
   Eina_Bool ch_maximized = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   obj = ELM_WIDGET_DATA(sd)->obj;

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
}

static Eina_Bool
_elm_win_smart_focus_next(const Evas_Object *obj,
                          Elm_Focus_Direction dir,
                          Evas_Object **next)
{
   ELM_WIN_DATA_GET(obj, sd);

   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   /* Focus chain */
   if (ELM_WIDGET_DATA(sd)->subobjs)
     {
        if (!(items = elm_widget_focus_custom_chain_get(obj)))
          {
             items = ELM_WIDGET_DATA(sd)->subobjs;
             if (!items)
               return EINA_FALSE;
          }
        list_data_get = eina_list_data_get;

        elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);

        if (*next)
          return EINA_TRUE;
     }
   *next = (Evas_Object *)obj;
   return EINA_FALSE;
}

static Eina_Bool
_elm_win_smart_focus_direction(const Evas_Object *obj,
                               const Evas_Object *base,
                               double degree,
                               Evas_Object **direction,
                               double *weight)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   ELM_WIN_DATA_GET(obj, sd);

   /* Focus chain */
   if (ELM_WIDGET_DATA(sd)->subobjs)
     {
        if (!(items = elm_widget_focus_custom_chain_get(obj)))
          items = ELM_WIDGET_DATA(sd)->subobjs;

        list_data_get = eina_list_data_get;

        return elm_widget_focus_list_direction_get
                 (obj, base, items, list_data_get, degree, direction, weight);
     }

   return EINA_FALSE;
}

static Eina_Bool
_elm_win_smart_on_focus(Evas_Object *obj)
{
   ELM_WIN_DATA_GET(obj, sd);

   if (sd->img_obj)
     evas_object_focus_set(sd->img_obj, elm_widget_focus_get(obj));
   else
     evas_object_focus_set(obj, elm_widget_focus_get(obj));

   return EINA_TRUE;
}

static Eina_Bool
_elm_win_smart_event(Evas_Object *obj,
                     Evas_Object *src __UNUSED__,
                     Evas_Callback_Type type,
                     void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_Object *current_focused;

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if (type != EVAS_CALLBACK_KEY_DOWN)
     return EINA_FALSE;

   current_focused = elm_widget_focused_object_get(obj);
   if (!strcmp(ev->keyname, "Tab"))
     {
        if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
          elm_widget_focus_cycle(obj, ELM_FOCUS_PREVIOUS);
        else
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);

        goto success;
     }
   else if ((!strcmp(ev->keyname, "Left")) ||
            ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        if (current_focused == obj)
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
        else
          elm_widget_focus_direction_go(obj, 270.0);

        goto success;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        if (current_focused == obj)
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
        else
          elm_widget_focus_direction_go(obj, 90.0);

        goto success;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        if (current_focused == obj)
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
        else
          elm_widget_focus_direction_go(obj, 0.0);

        goto success;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        if (current_focused == obj)
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
        else
          elm_widget_focus_direction_go(obj, 180.0);

        goto success;
     }

   return EINA_FALSE;

success:
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_deferred_ecore_evas_free(void *data)
{
   ecore_evas_free(data);
   _elm_win_deferred_free--;
}

static void
_elm_win_smart_show(Evas_Object *obj)
{
   ELM_WIN_DATA_GET(obj, sd);

   _elm_win_parent_sc->base.show(obj);

   TRAP(sd, show);

   if (!sd->show_count) sd->show_count++;
   if (sd->shot.info) _shot_handle(sd);
}

static void
_elm_win_smart_hide(Evas_Object *obj)
{
   ELM_WIN_DATA_GET(obj, sd);

   _elm_win_parent_sc->base.hide(obj);

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
                       Evas *e __UNUSED__,
                       Evas_Object *obj,
                       void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   if (obj == sd->parent) sd->parent = NULL;
}

static void
_elm_win_focus_target_move(void *data,
                           Evas *e __UNUSED__,
                           Evas_Object *obj __UNUSED__,
                           void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   sd->focus_highlight.geometry_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_focus_target_resize(void *data,
                             Evas *e __UNUSED__,
                             Evas_Object *obj __UNUSED__,
                             void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   sd->focus_highlight.geometry_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_focus_target_del(void *data,
                          Evas *e __UNUSED__,
                          Evas_Object *obj __UNUSED__,
                          void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

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
_elm_win_focus_target_callbacks_add(Elm_Win_Smart_Data *sd)
{
   Evas_Object *obj = sd->focus_highlight.cur.target;

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOVE, _elm_win_focus_target_move, sd);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _elm_win_focus_target_resize, sd);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_DEL, _elm_win_focus_target_del, sd);
}

static void
_elm_win_focus_target_callbacks_del(Elm_Win_Smart_Data *sd)
{
   Evas_Object *obj = sd->focus_highlight.cur.target;

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_MOVE, _elm_win_focus_target_move, sd);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_win_focus_target_resize, sd);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_DEL, _elm_win_focus_target_del, sd);
}

static void
_elm_win_object_focus_in(void *data,
                         Evas *e __UNUSED__,
                         void *event_info)
{
   Evas_Object *obj = event_info, *target;
   Elm_Win_Smart_Data *sd = data;

   if (sd->focus_highlight.cur.target == obj)
     return;

   target = _elm_win_focus_target_get(obj);
   sd->focus_highlight.cur.target = target;
   if (elm_widget_highlight_in_theme_get(target))
     sd->focus_highlight.cur.handled = EINA_TRUE;
   else
     _elm_win_focus_target_callbacks_add(sd);

   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_object_focus_out(void *data,
                          Evas *e __UNUSED__,
                          void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   if (!sd->focus_highlight.cur.target)
     return;

   if (!sd->focus_highlight.cur.handled)
     _elm_win_focus_target_callbacks_del(sd);

   sd->focus_highlight.cur.target = NULL;
   sd->focus_highlight.cur.handled = EINA_FALSE;

   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_focus_highlight_shutdown(Elm_Win_Smart_Data *sd)
{
   _elm_win_focus_highlight_reconfigure_job_stop(sd);
   if (sd->focus_highlight.cur.target)
     {
        _elm_win_focus_target_callbacks_del(sd);
        sd->focus_highlight.cur.target = NULL;
     }
   if (sd->focus_highlight.top)
     {
        evas_object_del(sd->focus_highlight.top);
        sd->focus_highlight.top = NULL;
     }

   evas_event_callback_del_full
     (sd->evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
     _elm_win_object_focus_in, sd);
   evas_event_callback_del_full
     (sd->evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
     _elm_win_object_focus_out, sd);
}

static void
_elm_win_on_img_obj_del(void *data,
                        Evas *e __UNUSED__,
                        Evas_Object *obj __UNUSED__,
                        void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   sd->img_obj = NULL;
}

static void
_elm_win_smart_del(Evas_Object *obj)
{
   ELM_WIN_DATA_GET(obj, sd);

   /* NB: child deletion handled by parent's smart del */

   if ((trap) && (trap->del))
     trap->del(sd->trap_data, obj);

   if (sd->parent)
     {
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_DEL, _elm_win_on_parent_del, sd);
        sd->parent = NULL;
     }

   if (sd->autodel_clear) *(sd->autodel_clear) = -1;

   _elm_win_list = eina_list_remove(_elm_win_list, obj);

   if (sd->ee)
     {
        ecore_evas_callback_delete_request_set(sd->ee, NULL);
        ecore_evas_callback_resize_set(sd->ee, NULL);
     }
   if (sd->deferred_resize_job) ecore_job_del(sd->deferred_resize_job);
   if (sd->deferred_child_eval_job) ecore_job_del(sd->deferred_child_eval_job);
   if (sd->shot.info) eina_stringshare_del(sd->shot.info);
   if (sd->shot.timer) ecore_timer_del(sd->shot.timer);

#ifdef HAVE_ELEMENTARY_X
   if (sd->x.client_message_handler)
     ecore_event_handler_del(sd->x.client_message_handler);
#endif

   if (sd->img_obj)
     {
        evas_object_event_callback_del_full
           (sd->img_obj, EVAS_CALLBACK_DEL, _elm_win_on_img_obj_del, sd);
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

   if (sd->title) eina_stringshare_del(sd->title);
   if (sd->icon_name) eina_stringshare_del(sd->icon_name);
   if (sd->role) eina_stringshare_del(sd->role);
   if (sd->icon) evas_object_del(sd->icon);

   /* Don't let callback in the air that point to sd */
   ecore_evas_callback_delete_request_set(sd->ee, NULL);
   ecore_evas_callback_resize_set(sd->ee, NULL);
   ecore_evas_callback_mouse_in_set(sd->ee, NULL);
   ecore_evas_callback_focus_in_set(sd->ee, NULL);
   ecore_evas_callback_focus_out_set(sd->ee, NULL);
   ecore_evas_callback_move_set(sd->ee, NULL);
   ecore_evas_callback_state_change_set(sd->ee, NULL);

   _elm_win_parent_sc->base.del(obj); /* handles freeing sd */

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
   Elm_Win_Smart_Data *sd = data;

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

static void
_elm_win_smart_move(Evas_Object *obj,
                    Evas_Coord x,
                    Evas_Coord y)
{
   ELM_WIN_DATA_GET(obj, sd);

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

   _elm_win_parent_sc->base.move(obj, x, y);

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

static void
_elm_win_smart_resize(Evas_Object *obj,
                      Evas_Coord w,
                      Evas_Coord h)
{
   ELM_WIN_DATA_GET(obj, sd);

   _elm_win_parent_sc->base.resize(obj, w, h);

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
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   obj = ELM_WIDGET_DATA(sd)->obj;

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
   Ecore_X_Window xwin = 0;

   if (!ee) return 0;
   if (ENGINE_COMPARE(ELM_SOFTWARE_X11))
     {
        if (ee) xwin = ecore_evas_software_x11_window_get(ee);
     }
   else if (ENGINE_COMPARE(ELM_SOFTWARE_FB) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_WINCE) ||
            ENGINE_COMPARE(ELM_SOFTWARE_SDL) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_SDL) ||
            ENGINE_COMPARE(ELM_OPENGL_SDL) ||
            ENGINE_COMPARE(ELM_OPENGL_COCOA))
     {
     }
   else if (ENGINE_COMPARE(ELM_SOFTWARE_16_X11))
     {
        if (ee) xwin = ecore_evas_software_x11_16_window_get(ee);
     }
   else if (ENGINE_COMPARE(ELM_SOFTWARE_8_X11))
     {
        if (ee) xwin = ecore_evas_software_x11_8_window_get(ee);
     }
   else if (ENGINE_COMPARE(ELM_OPENGL_X11))
     {
        if (ee) xwin = ecore_evas_gl_x11_window_get(ee);
     }
   else if (ENGINE_COMPARE(ELM_SOFTWARE_WIN32))
     {
        if (ee) xwin = (long)ecore_evas_win32_window_get(ee);
     }
   return xwin;

#endif
   return 0;
}

#ifdef HAVE_ELEMENTARY_X
static void
_elm_win_xwindow_get(Elm_Win_Smart_Data *sd)
{
   sd->x.xwin = _elm_ee_xwin_get(sd->ee);
}

#endif

#ifdef HAVE_ELEMENTARY_X
static void
_elm_win_xwin_update(Elm_Win_Smart_Data *sd)
{
   const char *s;

   _elm_win_xwindow_get(sd);
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

static void
_elm_win_resize_objects_eval(Evas_Object *obj)
{
   const Eina_List *l;
   const Evas_Object *child;

   ELM_WIN_DATA_GET(obj, sd);
   Evas_Coord w, h, minw = -1, minh = -1, maxw = -1, maxh = -1;
   int xx = 1, xy = 1;
   double wx, wy;

   EINA_LIST_FOREACH(sd->resize_objs, l, child)
     {
        evas_object_size_hint_weight_get(child, &wx, &wy);
        if (wx == 0.0) xx = 0;
        if (wy == 0.0) xy = 0;

        evas_object_size_hint_min_get(child, &w, &h);
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        if (w > minw) minw = w;
        if (h > minh) minh = h;

        evas_object_size_hint_max_get(child, &w, &h);
        if (w < 1) w = -1;
        if (h < 1) h = -1;
        if (maxw == -1) maxw = w;
        else if ((w > 0) && (w < maxw))
          maxw = w;
        if (maxh == -1) maxh = h;
        else if ((h > 0) && (h < maxh))
          maxh = h;
     }
   if (!xx) maxw = minw;
   else maxw = 32767;
   if (!xy) maxh = minh;
   else maxh = 32767;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if ((maxw >= 0) && (w > maxw)) w = maxw;
   if ((maxh >= 0) && (h > maxh)) h = maxh;
   evas_object_resize(obj, w, h);
}

static void
_elm_win_on_resize_obj_del(void *data,
                           Evas *e __UNUSED__,
                           Evas_Object *obj __UNUSED__,
                           void *event_info __UNUSED__)
{
   ELM_WIN_DATA_GET(data, sd);

   sd->resize_objs = eina_list_remove(sd->resize_objs, obj);

   _elm_win_resize_objects_eval(data);
}

static void
_elm_win_on_resize_obj_changed_size_hints(void *data,
                                          Evas *e __UNUSED__,
                                          Evas_Object *obj __UNUSED__,
                                          void *event_info __UNUSED__)
{
   _elm_win_resize_objects_eval(data);
}

void
_elm_win_shutdown(void)
{
   while (_elm_win_list)
     evas_object_del(_elm_win_list->data);
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
   const Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH (_elm_win_list, l, obj)
     elm_widget_access(obj, is_access);
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
                        int type __UNUSED__,
                        void *event)
{
   Elm_Win_Smart_Data *sd = data;
   Ecore_X_Event_Client_Message *e = event;

   if (e->format != 32) return ECORE_CALLBACK_PASS_ON;
   if (e->message_type == ECORE_X_ATOM_E_COMP_FLUSH)
     {
        if ((unsigned int)e->data.l[0] == sd->x.xwin)
          {
             Evas *evas = evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj);
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
             Evas *evas = evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj);
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
                  // XXX: call right access func
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ)
               {
                  // XXX: call right access func
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ_NEXT)
               {
                  // XXX: call right access func
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ_PREV)
               {
                  // XXX: call right access func
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

#endif

static void
_elm_win_focus_highlight_hide(void *data __UNUSED__,
                              Evas_Object *obj,
                              const char *emission __UNUSED__,
                              const char *source __UNUSED__)
{
   evas_object_hide(obj);
}

static void
_elm_win_focus_highlight_anim_end(void *data,
                                  Evas_Object *obj,
                                  const char *emission __UNUSED__,
                                  const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   _elm_win_focus_highlight_simple_setup(sd, obj);
}

static void
_elm_win_focus_highlight_init(Elm_Win_Smart_Data *sd)
{
   evas_event_callback_add(sd->evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
                           _elm_win_object_focus_in, sd);
   evas_event_callback_add(sd->evas,
                           EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
                           _elm_win_object_focus_out, sd);

   sd->focus_highlight.cur.target = evas_focus_get(sd->evas);

   sd->focus_highlight.top = edje_object_add(sd->evas);
   sd->focus_highlight.changed_theme = EINA_TRUE;
   edje_object_signal_callback_add(sd->focus_highlight.top,
                                   "elm,action,focus,hide,end", "",
                                   _elm_win_focus_highlight_hide, NULL);
   edje_object_signal_callback_add(sd->focus_highlight.top,
                                   "elm,action,focus,anim,end", "",
                                   _elm_win_focus_highlight_anim_end, sd);
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_frame_cb_move_start(void *data,
                             Evas_Object *obj __UNUSED__,
                             const char *sig __UNUSED__,
                             const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   /* FIXME: Change mouse pointer */

   /* NB: Wayland handles moving surfaces by itself so we cannot
    * specify a specific x/y we want. Instead, we will pass in the
    * existing x/y values so they can be recorded as 'previous'
    * position. The new position will get updated automatically when
    * the move is finished */

   ecore_evas_wayland_move(sd->ee, sd->screen.x, sd->screen.y);
}

static void
_elm_win_frame_cb_resize_show(void *data,
                              Evas_Object *obj __UNUSED__,
                              const char *sig __UNUSED__,
                              const char *source)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   if (sd->resizing) return;

#ifdef HAVE_ELEMENTARY_WAYLAND
   if (!strcmp(source, "elm.event.resize.t"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, ELM_CURSOR_TOP_SIDE);
   else if (!strcmp(source, "elm.event.resize.b"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, ELM_CURSOR_BOTTOM_SIDE);
   else if (!strcmp(source, "elm.event.resize.l"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, ELM_CURSOR_LEFT_SIDE);
   else if (!strcmp(source, "elm.event.resize.r"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, ELM_CURSOR_RIGHT_SIDE);
   else if (!strcmp(source, "elm.event.resize.tl"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, 
                                          ELM_CURSOR_TOP_LEFT_CORNER);
   else if (!strcmp(source, "elm.event.resize.tr"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, 
                                          ELM_CURSOR_TOP_RIGHT_CORNER);
   else if (!strcmp(source, "elm.event.resize.bl"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, 
                                          ELM_CURSOR_BOTTOM_LEFT_CORNER);
   else if (!strcmp(source, "elm.event.resize.br"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, 
                                          ELM_CURSOR_BOTTOM_RIGHT_CORNER);
   else
     ecore_wl_window_cursor_default_restore(sd->wl.win);
#else
   (void)source;
#endif
}

static void
_elm_win_frame_cb_resize_hide(void *data,
                              Evas_Object *obj __UNUSED__,
                              const char *sig __UNUSED__,
                              const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   if (sd->resizing) return;

#ifdef HAVE_ELEMENTARY_WAYLAND
   ecore_wl_window_cursor_default_restore(sd->wl.win);
#endif
}

static void
_elm_win_frame_cb_resize_start(void *data,
                               Evas_Object *obj __UNUSED__,
                               const char *sig __UNUSED__,
                               const char *source)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   if (sd->resizing) return;

   sd->resizing = EINA_TRUE;

   if (!strcmp(source, "elm.event.resize.t"))
     sd->resize_location = 1;
   else if (!strcmp(source, "elm.event.resize.b"))
     sd->resize_location = 2;
   else if (!strcmp(source, "elm.event.resize.l"))
     sd->resize_location = 4;
   else if (!strcmp(source, "elm.event.resize.r"))
     sd->resize_location = 8;
   else if (!strcmp(source, "elm.event.resize.tl"))
     sd->resize_location = 5;
   else if (!strcmp(source, "elm.event.resize.tr"))
     sd->resize_location = 9;
   else if (!strcmp(source, "elm.event.resize.bl"))
     sd->resize_location = 6;
   else if (!strcmp(source, "elm.event.resize.br"))
     sd->resize_location = 10;
   else
     sd->resize_location = 0;

   if (sd->resize_location > 0)
     ecore_evas_wayland_resize(sd->ee, sd->resize_location);
}

static void
_elm_win_frame_cb_minimize(void *data,
                           Evas_Object *obj __UNUSED__,
                           const char *sig __UNUSED__,
                           const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   sd->iconified = EINA_TRUE;
   TRAP(sd, iconified_set, EINA_TRUE);
}

static void
_elm_win_frame_cb_maximize(void *data,
                           Evas_Object *obj __UNUSED__,
                           const char *sig __UNUSED__,
                           const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   if (sd->maximized) sd->maximized = EINA_FALSE;
   else sd->maximized = EINA_TRUE;
   TRAP(sd, maximized_set, sd->maximized);
}

static void
_elm_win_frame_cb_close(void *data,
                        Evas_Object *obj __UNUSED__,
                        const char *sig __UNUSED__,
                        const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;
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

   if (!(sd = data)) return;

   win = ELM_WIDGET_DATA(sd)->obj;

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
_elm_win_frame_add(Elm_Win_Smart_Data *sd,
                   const char *style)
{
   evas_output_framespace_set(sd->evas, 0, 22, 0, 26);

   sd->frame_obj = edje_object_add(sd->evas);
   elm_widget_theme_object_set
     (ELM_WIDGET_DATA(sd)->obj, sd->frame_obj, "border", "base", style);

   evas_object_is_frame_object_set(sd->frame_obj, EINA_TRUE);
   evas_object_move(sd->frame_obj, 0, 0);
   evas_object_resize(sd->frame_obj, 1, 1);

   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,move,start", "elm",
     _elm_win_frame_cb_move_start, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,resize,show", "*",
     _elm_win_frame_cb_resize_show, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,resize,hide", "*",
     _elm_win_frame_cb_resize_hide, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,resize,start", "*",
     _elm_win_frame_cb_resize_start, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,minimize", "elm",
     _elm_win_frame_cb_minimize, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,maximize", "elm",
     _elm_win_frame_cb_maximize, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,close", "elm", _elm_win_frame_cb_close, sd);

   if (sd->title)
     {
        edje_object_part_text_escaped_set
          (sd->frame_obj, "elm.text.title", sd->title);
     }
}

static void 
_elm_win_frame_del(Elm_Win_Smart_Data *sd)
{
   if (sd->frame_obj)
     {
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,move,start", "elm",
              _elm_win_frame_cb_move_start);
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

        evas_object_del(sd->frame_obj);
        sd->frame_obj = NULL;
     }

   evas_output_framespace_set(sd->evas, 0, 0, 0, 0);
}

#ifdef ELM_DEBUG
static void
_debug_key_down(void *data __UNUSED__,
                Evas *e __UNUSED__,
                Evas_Object *obj,
                void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return;

   if ((strcmp(ev->keyname, "F12")) ||
       (!evas_key_modifier_is_set(ev->modifiers, "Control")))
     return;

   printf("Tree graph generated.\n");
   elm_object_tree_dot_dump(obj, "./dump.dot");
}

#endif

static void
_win_img_hide(void *data,
              Evas *e __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   elm_widget_focus_hide_handle(ELM_WIDGET_DATA(sd)->obj);
}

static void
_win_img_mouse_up(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   Elm_Win_Smart_Data *sd = data;
   Evas_Event_Mouse_Up *ev = event_info;
   if (!(ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD))
     elm_widget_focus_mouse_up_handle(ELM_WIDGET_DATA(sd)->obj);
}

static void
_win_img_focus_in(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   elm_widget_focus_steal(ELM_WIDGET_DATA(sd)->obj);
}

static void
_win_img_focus_out(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   elm_widget_focused_object_clear(ELM_WIDGET_DATA(sd)->obj);
}

static void
_win_inlined_image_set(Elm_Win_Smart_Data *sd)
{
   evas_object_image_alpha_set(sd->img_obj, EINA_FALSE);
   evas_object_image_filled_set(sd->img_obj, EINA_TRUE);

   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_DEL, _elm_win_on_img_obj_del, sd);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_HIDE, _win_img_hide, sd);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_MOUSE_UP, _win_img_mouse_up, sd);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_FOCUS_IN, _win_img_focus_in, sd);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_FOCUS_OUT, _win_img_focus_out, sd);
}

static void
_elm_win_on_icon_del(void *data,
                     Evas *e __UNUSED__,
                     Evas_Object *obj,
                     void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   if (sd->icon == obj) sd->icon = NULL;
}

static void
_elm_win_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Win_Smart_Data);

   _elm_win_parent_sc->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
}

static void
_elm_win_smart_set_user(Elm_Widget_Smart_Class *sc)
{
   sc->base.add = _elm_win_smart_add;
   sc->base.del = _elm_win_smart_del;
   sc->base.show = _elm_win_smart_show;
   sc->base.hide = _elm_win_smart_hide;
   sc->base.move = _elm_win_smart_move;
   sc->base.resize = _elm_win_smart_resize;

   sc->focus_next = _elm_win_smart_focus_next;
   sc->focus_direction = _elm_win_smart_focus_direction;
   sc->on_focus = _elm_win_smart_on_focus;
   sc->event = _elm_win_smart_event;
}

#ifdef HAVE_ELEMENTARY_X
static void
_elm_x_io_err(void *data __UNUSED__)
{
   Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     {
        evas_object_smart_callback_call(obj, SIG_IOERR, NULL);
     }
   elm_exit();
}
#endif

EAPI Evas_Object *
elm_win_add(Evas_Object *parent,
            const char *name,
            Elm_Win_Type type)
{
   Evas *e;
   Evas_Object *obj;
   const Eina_List *l;
   const char *fontpath, *fallback = NULL;

   Elm_Win_Smart_Data tmp_sd;

   /* just to store some data while trying out to create a canvas */
   memset(&tmp_sd, 0, sizeof(Elm_Win_Smart_Data));

#define FALLBACK_TRY(engine)                                      \
  if (!tmp_sd.ee) {                                               \
     CRITICAL(engine " engine creation failed. Trying default."); \
  } while (0)
#define FALLBACK_STORE(engine)                               \
   if (tmp_sd.ee)                                            \
   {                                                         \
      CRITICAL(engine "Fallback to %s successful.", engine); \
      fallback = engine;                                     \
   }

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
                evas_object_del(tmp_sd.img_obj);
                tmp_sd.img_obj = NULL;
             }
        }
        break;

      case ELM_WIN_SOCKET_IMAGE:
        tmp_sd.ee = ecore_evas_extn_socket_new(1, 1);
        break;

      default:
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11))
          {
             tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
             FALLBACK_TRY("Software X11");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                  FALLBACK_STORE("Software FB");
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_FB))
          {
             tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
             FALLBACK_TRY("Software FB");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_DIRECTFB))
          {
             tmp_sd.ee = ecore_evas_directfb_new(NULL, 1, 0, 0, 1, 1);
             FALLBACK_TRY("Software DirectFB");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_16_X11))
          {
             tmp_sd.ee = ecore_evas_software_x11_16_new(NULL, 0, 0, 0, 1, 1);
             FALLBACK_TRY("Software-16");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_8_X11))
          {
             tmp_sd.ee = ecore_evas_software_x11_8_new(NULL, 0, 0, 0, 1, 1);
             FALLBACK_TRY("Software-8");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_OPENGL_X11))
          {
             int opt[10];
             int opt_i = 0;

             if (_elm_config->vsync)
               {
                  opt[opt_i] = ECORE_EVAS_GL_X11_OPT_VSYNC;
                  opt_i++;
                  opt[opt_i] = 1;
                  opt_i++;
               }
             if (opt_i > 0)
               tmp_sd.ee = ecore_evas_gl_x11_options_new
                   (NULL, 0, 0, 0, 1, 1, opt);
             else
               tmp_sd.ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 1, 1);
             FALLBACK_TRY("OpenGL");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_WIN32))
          {
             tmp_sd.ee = ecore_evas_software_gdi_new(NULL, 0, 0, 1, 1);
             FALLBACK_TRY("Software Win32");
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_16_WINCE))
          {
             tmp_sd.ee = ecore_evas_software_wince_gdi_new(NULL, 0, 0, 1, 1);
             FALLBACK_TRY("Software-16-WinCE");
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_PSL1GHT))
          {
             tmp_sd.ee = ecore_evas_psl1ght_new(NULL, 1, 1);
             FALLBACK_TRY("PSL1GHT");
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_SDL))
          {
             tmp_sd.ee = ecore_evas_sdl_new(NULL, 0, 0, 0, 0, 0, 1);
             FALLBACK_TRY("Software SDL");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_16_SDL))
          {
             tmp_sd.ee = ecore_evas_sdl16_new(NULL, 0, 0, 0, 0, 0, 1);
             FALLBACK_TRY("Software-16-SDL");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_OPENGL_SDL))
          {
             tmp_sd.ee = ecore_evas_gl_sdl_new(NULL, 1, 1, 0, 0);
             FALLBACK_TRY("OpenGL SDL");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_OPENGL_COCOA))
          {
             tmp_sd.ee = ecore_evas_cocoa_new(NULL, 1, 1, 0, 0);
             FALLBACK_TRY("OpenGL Cocoa");
          }
        else if (ENGINE_COMPARE(ELM_BUFFER))
          {
             tmp_sd.ee = ecore_evas_buffer_new(1, 1);
          }
        else if (ENGINE_COMPARE(ELM_EWS))
          {
             tmp_sd.ee = ecore_evas_ews_new(0, 0, 1, 1);
          }
        else if (ENGINE_COMPARE(ELM_WAYLAND_SHM))
          {
             tmp_sd.ee = ecore_evas_wayland_shm_new(NULL, 0, 0, 0, 1, 1, 0);
          }
        else if (ENGINE_COMPARE(ELM_WAYLAND_EGL))
          {
             tmp_sd.ee = ecore_evas_wayland_egl_new(NULL, 0, 0, 0, 1, 1, 0);
          }
        else if (!strncmp(ENGINE_GET(), "shot:", 5))
          {
             tmp_sd.ee = ecore_evas_buffer_new(1, 1);
             ecore_evas_manual_render_set(tmp_sd.ee, EINA_TRUE);
             tmp_sd.shot.info = eina_stringshare_add
                 (ENGINE_GET() + 5);
          }
#undef FALLBACK_TRY
        break;
     }

   if (!tmp_sd.ee)
     {
        ERR("Cannot create window.");
        return NULL;
     }

   obj = evas_object_smart_add
       (ecore_evas_get(tmp_sd.ee), _elm_win_smart_class_new());

   ELM_WIN_DATA_GET(obj, sd);

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
   else if (ENGINE_COMPARE(ELM_SOFTWARE_X11))
     sd->x.client_message_handler = ecore_event_handler_add
         (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, sd);
   else if (ENGINE_COMPARE(ELM_SOFTWARE_16_X11))
     sd->x.client_message_handler = ecore_event_handler_add
         (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, sd);
   else if (ENGINE_COMPARE(ELM_SOFTWARE_8_X11))
     sd->x.client_message_handler = ecore_event_handler_add
         (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, sd);
   else if (ENGINE_COMPARE(ELM_OPENGL_X11))
     sd->x.client_message_handler = ecore_event_handler_add
         (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, sd);
#endif

   else if (!strncmp(ENGINE_GET(), "shot:", 5))
     _shot_init(sd);

   sd->kbdmode = ELM_WIN_KEYBOARD_UNKNOWN;
   sd->indmode = ELM_WIN_INDICATOR_UNKNOWN;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        ecore_x_io_error_handler_set(_elm_x_io_err, NULL);
     }
#endif

#ifdef HAVE_ELEMENTARY_WAYLAND
   sd->wl.win = ecore_evas_wayland_window_get(sd->ee);
#endif

   if ((_elm_config->bgpixmap) && (!_elm_config->compositing))
     TRAP(sd, avoid_damage_set, ECORE_EVAS_AVOID_DAMAGE_EXPOSE);
   // bg pixmap done by x - has other issues like can be redrawn by x before it
   // is filled/ready by app
   //     TRAP(sd, avoid_damage_set, ECORE_EVAS_AVOID_DAMAGE_BUILT_IN);

   sd->type = type;
   sd->parent = parent;

   if (sd->parent)
     evas_object_event_callback_add
       (sd->parent, EVAS_CALLBACK_DEL, _elm_win_on_parent_del, sd);

   sd->evas = ecore_evas_get(sd->ee);

   evas_object_color_set(obj, 0, 0, 0, 0);
   evas_object_move(obj, 0, 0);
   evas_object_resize(obj, 1, 1);
   evas_object_layer_set(obj, 50);
   evas_object_pass_events_set(obj, EINA_TRUE);

   if (type == ELM_WIN_INLINED_IMAGE)
     elm_widget_parent2_set(obj, parent);

   /* use own version of ecore_evas_object_associate() that does TRAP() */
   ecore_evas_data_set(sd->ee, "elm_win", sd);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
      _elm_win_obj_callback_changed_size_hints, sd);

   evas_object_intercept_raise_callback_add
     (obj, _elm_win_obj_intercept_raise, sd);
   evas_object_intercept_lower_callback_add
     (obj, _elm_win_obj_intercept_lower, sd);
   evas_object_intercept_stack_above_callback_add
     (obj, _elm_win_obj_intercept_stack_above, sd);
   evas_object_intercept_stack_below_callback_add
     (obj, _elm_win_obj_intercept_stack_below, sd);
   evas_object_intercept_layer_set_callback_add
     (obj, _elm_win_obj_intercept_layer_set, sd);
   evas_object_intercept_show_callback_add
     (obj, _elm_win_obj_intercept_show, sd);

   TRAP(sd, name_class_set, name, _elm_appname);
   ecore_evas_callback_delete_request_set(sd->ee, _elm_win_delete_request);
   ecore_evas_callback_resize_set(sd->ee, _elm_win_resize);
   ecore_evas_callback_mouse_in_set(sd->ee, _elm_win_mouse_in);
   ecore_evas_callback_focus_in_set(sd->ee, _elm_win_focus_in);
   ecore_evas_callback_focus_out_set(sd->ee, _elm_win_focus_out);
   ecore_evas_callback_move_set(sd->ee, _elm_win_move);
   ecore_evas_callback_state_change_set(sd->ee, _elm_win_state_change);

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

   if (((fallback) && (!strcmp(fallback, "Software FB"))) ||
       ((!fallback) && (ENGINE_COMPARE(ELM_SOFTWARE_FB))))
     {
        TRAP(sd, fullscreen_set, 1);
     }
   else if (ENGINE_COMPARE(ELM_WAYLAND_SHM))
     _elm_win_frame_add(sd, "default");
   else if (ENGINE_COMPARE(ELM_WAYLAND_EGL))
     _elm_win_frame_add(sd, "default");

   if (_elm_config->focus_highlight_enable)
     elm_win_focus_highlight_enabled_set(obj, EINA_TRUE);

#ifdef ELM_DEBUG
   Evas_Modifier_Mask mask = evas_key_modifier_mask_get(sd->evas, "Control");
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_KEY_DOWN, _debug_key_down, sd);

   if (evas_object_key_grab(obj, "F12", mask, 0, EINA_TRUE))
     INF("Ctrl+F12 key combination exclusive for dot tree generation\n");
   else
     ERR("failed to grab F12 key to elm widgets (dot) tree generation");
#endif

   if ((_elm_config->softcursor_mode == ELM_SOFTCURSOR_MODE_ON) ||
       ((_elm_config->softcursor_mode == ELM_SOFTCURSOR_MODE_AUTO) &&
           (((fallback) && (!strcmp(fallback, "Software FB"))) ||
               ((!fallback) && (ENGINE_COMPARE(ELM_SOFTWARE_FB))))))
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

   return obj;
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

EAPI void
elm_win_resize_object_add(Evas_Object *obj,
                          Evas_Object *subobj)
{
   Evas_Coord w, h;

   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (eina_list_data_find(sd->resize_objs, subobj)) return;

   if (!ELM_WIDGET_DATA(sd)->api->sub_object_add(obj, subobj))
     ERR("could not add %p as sub object of %p", subobj, obj);

   sd->resize_objs = eina_list_append(sd->resize_objs, subobj);

   evas_object_event_callback_add
     (subobj, EVAS_CALLBACK_DEL, _elm_win_on_resize_obj_del, obj);
   evas_object_event_callback_add
     (subobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _elm_win_on_resize_obj_changed_size_hints, obj);

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_move(subobj, 0, 0);
   evas_object_resize(subobj, w, h);

   _elm_win_resize_objects_eval(obj);
}

EAPI void
elm_win_resize_object_del(Evas_Object *obj,
                          Evas_Object *subobj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (!ELM_WIDGET_DATA(sd)->api->sub_object_del(obj, subobj))
     ERR("could not remove sub object %p from %p", subobj, obj);

   sd->resize_objs = eina_list_remove(sd->resize_objs, subobj);

   evas_object_event_callback_del_full
     (subobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _elm_win_on_resize_obj_changed_size_hints, obj);
   evas_object_event_callback_del_full
     (subobj, EVAS_CALLBACK_DEL, _elm_win_on_resize_obj_del, obj);

   _elm_win_resize_objects_eval(obj);
}

EAPI void
elm_win_title_set(Evas_Object *obj,
                  const char *title)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (!title) return;
   eina_stringshare_replace(&(sd->title), title);
   TRAP(sd, title_set, sd->title);
   if (sd->frame_obj)
     edje_object_part_text_escaped_set
       (sd->frame_obj, "elm.text.title", sd->title);
}

EAPI const char *
elm_win_title_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return sd->title;
}

EAPI void
elm_win_icon_name_set(Evas_Object *obj,
                      const char *icon_name)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (!icon_name) return;
   eina_stringshare_replace(&(sd->icon_name), icon_name);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI const char *
elm_win_icon_name_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return sd->icon_name;
}

EAPI void
elm_win_role_set(Evas_Object *obj, const char *role)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (!role) return;
   eina_stringshare_replace(&(sd->role), role);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI const char *
elm_win_role_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return sd->role;
}

EAPI void
elm_win_icon_object_set(Evas_Object *obj,
                        Evas_Object *icon)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (sd->icon)
     evas_object_event_callback_del_full
       (sd->icon, EVAS_CALLBACK_DEL, _elm_win_on_icon_del, sd);
   sd->icon = icon;
   if (sd->icon)
     evas_object_event_callback_add
       (sd->icon, EVAS_CALLBACK_DEL, _elm_win_on_icon_del, sd);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI const Evas_Object *
elm_win_icon_object_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return sd->icon;
}

EAPI void
elm_win_autodel_set(Evas_Object *obj,
                    Eina_Bool autodel)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->autodel = autodel;
}

EAPI Eina_Bool
elm_win_autodel_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->autodel;
}

EAPI void
elm_win_activate(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   TRAP(sd, activate);
}

EAPI void
elm_win_lower(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   TRAP(sd, lower);
}

EAPI void
elm_win_raise(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   TRAP(sd, raise);
}

EAPI void
elm_win_center(Evas_Object *obj,
               Eina_Bool h,
               Eina_Bool v)
{
   int win_w, win_h, screen_w, screen_h, nx, ny;

   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

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

EAPI void
elm_win_borderless_set(Evas_Object *obj,
                       Eina_Bool borderless)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   TRAP(sd, borderless_set, borderless);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_borderless_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ecore_evas_borderless_get(sd->ee);
}

EAPI void
elm_win_shaped_set(Evas_Object *obj,
                   Eina_Bool shaped)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   TRAP(sd, shaped_set, shaped);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_shaped_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ecore_evas_shaped_get(sd->ee);
}

EAPI void
elm_win_alpha_set(Evas_Object *obj,
                  Eina_Bool alpha)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (sd->img_obj)
     {
        evas_object_image_alpha_set(sd->img_obj, alpha);
        ecore_evas_alpha_set(sd->ee, alpha);
     }
   else
     {
#ifdef HAVE_ELEMENTARY_X
        if (sd->x.xwin)
          {
             if (alpha)
               {
                  if (!_elm_config->compositing)
                    elm_win_shaped_set(obj, alpha);
                  else
                    TRAP(sd, alpha_set, alpha);
               }
             else
               TRAP(sd, alpha_set, alpha);
             _elm_win_xwin_update(sd);
          }
        else
#endif
          TRAP(sd, alpha_set, alpha);
     }
}

EAPI Eina_Bool
elm_win_alpha_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (sd->img_obj)
     {
        return evas_object_image_alpha_get(sd->img_obj);
     }

   return ecore_evas_alpha_get(sd->ee);
}

EAPI void
elm_win_override_set(Evas_Object *obj,
                     Eina_Bool override)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   TRAP(sd, override_set, override);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_override_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ecore_evas_override_get(sd->ee);
}

EAPI void
elm_win_fullscreen_set(Evas_Object *obj,
                       Eina_Bool fullscreen)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);
   // YYY: handle if sd->img_obj
   if (ENGINE_COMPARE(ELM_SOFTWARE_FB) ||
       ENGINE_COMPARE(ELM_SOFTWARE_16_WINCE))
     {
        // these engines... can ONLY be fullscreen
        return;
     }
   else
     {
        sd->fullscreen = fullscreen;

        if (fullscreen)
          {
             if (ENGINE_COMPARE(ELM_WAYLAND_SHM))
               _elm_win_frame_del(sd);
             else if (ENGINE_COMPARE(ELM_WAYLAND_EGL))
               _elm_win_frame_del(sd);
          }
        else
          {
             if (ENGINE_COMPARE(ELM_WAYLAND_SHM))
               _elm_win_frame_add(sd, "default");
             else if (ENGINE_COMPARE(ELM_WAYLAND_EGL))
               _elm_win_frame_add(sd, "default");

             evas_object_show(sd->frame_obj);
          }

        TRAP(sd, fullscreen_set, fullscreen);
#ifdef HAVE_ELEMENTARY_X
        _elm_win_xwin_update(sd);
#endif
     }
}

EAPI Eina_Bool
elm_win_fullscreen_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (ENGINE_COMPARE(ELM_SOFTWARE_FB) ||
       ENGINE_COMPARE(ELM_SOFTWARE_16_WINCE))
     {
        // these engines... can ONLY be fullscreen
        return EINA_TRUE;
     }
   else
     {
        return sd->fullscreen;
     }
}

EAPI void
elm_win_maximized_set(Evas_Object *obj,
                      Eina_Bool maximized)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->maximized = maximized;
   // YYY: handle if sd->img_obj
   TRAP(sd, maximized_set, maximized);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_maximized_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->maximized;
}

EAPI void
elm_win_iconified_set(Evas_Object *obj,
                      Eina_Bool iconified)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->iconified = iconified;
   TRAP(sd, iconified_set, iconified);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_iconified_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->iconified;
}

EAPI void
elm_win_withdrawn_set(Evas_Object *obj,
                      Eina_Bool withdrawn)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->withdrawn = withdrawn;
   TRAP(sd, withdrawn_set, withdrawn);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_withdrawn_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->withdrawn;
}

EAPI void
elm_win_urgent_set(Evas_Object *obj,
                   Eina_Bool urgent)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->urgent = urgent;
   TRAP(sd, urgent_set, urgent);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_urgent_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->urgent;
}

EAPI void
elm_win_demand_attention_set(Evas_Object *obj,
                             Eina_Bool demand_attention)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->demand_attention = demand_attention;
   TRAP(sd, demand_attention_set, demand_attention);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_demand_attention_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->demand_attention;
}

EAPI void
elm_win_modal_set(Evas_Object *obj,
                  Eina_Bool modal)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->modal = modal;
   TRAP(sd, modal_set, modal);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_modal_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->modal;
}

EAPI void
elm_win_aspect_set(Evas_Object *obj,
                   double aspect)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->aspect = aspect;
   TRAP(sd, aspect_set, aspect);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI double
elm_win_aspect_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) 0.0;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, 0.0);

   return sd->aspect;
}

EAPI void
elm_win_size_base_set(Evas_Object *obj, int w, int h)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);
   sd->size_base_w = w;
   sd->size_base_h = h;
   TRAP(sd, size_base_set, w, h);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI void
elm_win_size_base_get(Evas_Object *obj, int *w, int *h)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);
   if (w) *w = sd->size_base_w;
   if (w) *h = sd->size_base_h;
}

EAPI void
elm_win_size_step_set(Evas_Object *obj, int w, int h)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);
   sd->size_step_w = w;
   sd->size_step_h = h;
   TRAP(sd, size_step_set, w, h);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI void
elm_win_size_step_get(Evas_Object *obj, int *w, int *h)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);
   if (w) *w = sd->size_step_w;
   if (w) *h = sd->size_step_h;
}

EAPI void
elm_win_layer_set(Evas_Object *obj,
                  int layer)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   TRAP(sd, layer_set, layer);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI int
elm_win_layer_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, -1);

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

EAPI void
elm_win_rotation_set(Evas_Object *obj,
                     int rotation)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (sd->rot == rotation) return;
   sd->rot = rotation;
   TRAP(sd, rotation_set, rotation);
   evas_object_size_hint_min_set(obj, -1, -1);
   evas_object_size_hint_max_set(obj, -1, -1);
   _elm_win_resize_objects_eval(obj);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI void
elm_win_rotation_with_resize_set(Evas_Object *obj,
                                 int rotation)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (sd->rot == rotation) return;
   sd->rot = rotation;
   TRAP(sd, rotation_with_resize_set, rotation);
   evas_object_size_hint_min_set(obj, -1, -1);
   evas_object_size_hint_max_set(obj, -1, -1);
   _elm_win_resize_objects_eval(obj);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI int
elm_win_rotation_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, -1);

   return sd->rot;
}

EAPI void
elm_win_sticky_set(Evas_Object *obj,
                   Eina_Bool sticky)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->sticky = sticky;
   TRAP(sd, sticky_set, sticky);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_sticky_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->sticky;
}

EAPI void
elm_win_keyboard_mode_set(Evas_Object *obj,
                          Elm_Win_Keyboard_Mode mode)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (mode == sd->kbdmode) return;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
#endif
   sd->kbdmode = mode;
#ifdef HAVE_ELEMENTARY_X
   if (sd->x.xwin)
     ecore_x_e_virtual_keyboard_state_set
       (sd->x.xwin, (Ecore_X_Virtual_Keyboard_State)sd->kbdmode);
#endif
}

EAPI Elm_Win_Keyboard_Mode
elm_win_keyboard_mode_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) ELM_WIN_KEYBOARD_UNKNOWN;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, ELM_WIN_KEYBOARD_UNKNOWN);

   return sd->kbdmode;
}

EAPI void
elm_win_keyboard_win_set(Evas_Object *obj,
                         Eina_Bool is_keyboard)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_virtual_keyboard_set(sd->x.xwin, is_keyboard);
#else
   (void)is_keyboard;
#endif
}

EAPI Eina_Bool
elm_win_keyboard_win_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_virtual_keyboard_get(sd->x.xwin);
#endif
   return EINA_FALSE;
}

EAPI void
elm_win_indicator_mode_set(Evas_Object *obj,
                           Elm_Win_Indicator_Mode mode)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (mode == sd->indmode) return;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
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
}

EAPI Elm_Win_Indicator_Mode
elm_win_indicator_mode_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) ELM_WIN_INDICATOR_UNKNOWN;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, ELM_WIN_INDICATOR_UNKNOWN);

   return sd->indmode;
}

EAPI void
elm_win_indicator_opacity_set(Evas_Object *obj,
                              Elm_Win_Indicator_Opacity_Mode mode)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (mode == sd->ind_o_mode) return;
   sd->ind_o_mode = mode;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
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
}

EAPI Elm_Win_Indicator_Opacity_Mode
elm_win_indicator_opacity_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) ELM_WIN_INDICATOR_OPACITY_UNKNOWN;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, ELM_WIN_INDICATOR_OPACITY_UNKNOWN);

   return sd->ind_o_mode;
}

EAPI void
elm_win_screen_position_get(const Evas_Object *obj,
                            int *x,
                            int *y)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (x) *x = sd->screen.x;
   if (y) *y = sd->screen.y;
}

EAPI Eina_Bool
elm_win_focus_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return ecore_evas_focus_get(sd->ee);
}

EAPI void
elm_win_screen_constrain_set(Evas_Object *obj,
                             Eina_Bool constrain)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->constrain = !!constrain;
}

EAPI Eina_Bool
elm_win_screen_constrain_get(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->constrain;
}

EAPI void
elm_win_screen_size_get(const Evas_Object *obj,
                        int *x,
                        int *y,
                        int *w,
                        int *h)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   ecore_evas_screen_geometry_get(sd->ee, x, y, w, h);
}

EAPI void
elm_win_screen_dpi_get(const Evas_Object *obj,
                       int *xdpi,
                       int *ydpi)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   ecore_evas_screen_dpi_get(sd->ee, xdpi, ydpi);
}

EAPI void
elm_win_conformant_set(Evas_Object *obj,
                       Eina_Bool conformant)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_conformant_set(sd->x.xwin, conformant);
#else
   (void)conformant;
#endif
}

EAPI Eina_Bool
elm_win_conformant_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_conformant_get(sd->x.xwin);
#endif
   return EINA_FALSE;
}

EAPI void
elm_win_quickpanel_set(Evas_Object *obj,
                       Eina_Bool quickpanel)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
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

EAPI Eina_Bool
elm_win_quickpanel_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_quickpanel_get(sd->x.xwin);
#endif
   return EINA_FALSE;
}

EAPI void
elm_win_quickpanel_priority_major_set(Evas_Object *obj,
                                      int priority)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_quickpanel_priority_major_set(sd->x.xwin, priority);
#else
   (void)priority;
#endif
}

EAPI int
elm_win_quickpanel_priority_major_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, -1);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_quickpanel_priority_major_get(sd->x.xwin);
#endif
   return -1;
}

EAPI void
elm_win_quickpanel_priority_minor_set(Evas_Object *obj,
                                      int priority)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_quickpanel_priority_minor_set(sd->x.xwin, priority);
#else
   (void)priority;
#endif
}

EAPI int
elm_win_quickpanel_priority_minor_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, -1);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_quickpanel_priority_minor_get(sd->x.xwin);
#endif
   return -1;
}

EAPI void
elm_win_quickpanel_zone_set(Evas_Object *obj,
                            int zone)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_quickpanel_zone_set(sd->x.xwin, zone);
#else
   (void)zone;
#endif
}

EAPI int
elm_win_quickpanel_zone_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) 0;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, 0);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     return ecore_x_e_illume_quickpanel_zone_get(sd->x.xwin);
#endif
   return 0;
}

EAPI void
elm_win_prop_focus_skip_set(Evas_Object *obj,
                            Eina_Bool skip)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->skip_focus = skip;
   TRAP(sd, focus_skip_set, skip);
}

EAPI void
elm_win_illume_command_send(Evas_Object *obj,
                            Elm_Illume_Command command,
                            void *params __UNUSED__)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
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

EAPI Evas_Object *
elm_win_inlined_image_object_get(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return sd->img_obj;
}

EAPI void
elm_win_focus_highlight_enabled_set(Evas_Object *obj,
                                    Eina_Bool enabled)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   enabled = !!enabled;
   if (sd->focus_highlight.enabled == enabled)
     return;

   sd->focus_highlight.enabled = enabled;

   if (sd->focus_highlight.enabled)
     _elm_win_focus_highlight_init(sd);
   else
     _elm_win_focus_highlight_shutdown(sd);
}

EAPI Eina_Bool
elm_win_focus_highlight_enabled_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->focus_highlight.enabled;
}

EAPI void
elm_win_focus_highlight_style_set(Evas_Object *obj,
                                  const char *style)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   eina_stringshare_replace(&sd->focus_highlight.style, style);
   sd->focus_highlight.changed_theme = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

EAPI const char *
elm_win_focus_highlight_style_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   return sd->focus_highlight.style;
}

EAPI Eina_Bool
elm_win_socket_listen(Evas_Object *obj,
                      const char *svcname,
                      int svcnum,
                      Eina_Bool svcsys)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (!sd->ee) return EINA_FALSE;

   if (!ecore_evas_extn_socket_listen(sd->ee, svcname, svcnum, svcsys))
     return EINA_FALSE;

   return EINA_TRUE;
}

/* windowing specific calls - shall we do this differently? */

EAPI Ecore_X_Window
elm_win_xwindow_get(const Evas_Object *obj)
{
   if (!obj) return 0;

   if (!evas_object_smart_type_check_ptr(obj, WIN_SMART_NAME))
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        return _elm_ee_xwin_get(ee);
     }

   ELM_WIN_CHECK(obj) 0;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, 0);

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

   if (!evas_object_smart_type_check_ptr(obj, WIN_SMART_NAME))
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        return ecore_evas_wayland_window_get(ee);
     }

   ELM_WIN_CHECK(obj) NULL;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);
#if HAVE_ELEMENTARY_WAYLAND
   if (sd->wl.win) return sd->wl.win;
   if (sd->parent) return elm_win_wl_window_get(sd->parent);
#endif
   return NULL;
}

EAPI Eina_Bool
elm_win_trap_set(const Elm_Win_Trap *t)
{
   DBG("old %p, new %p", trap, t);

   if ((t) && (t->version != ELM_WIN_TRAP_VERSION))
     {
        CRITICAL("trying to set a trap version %lu while %lu was expected!",
                 t->version, ELM_WIN_TRAP_VERSION);
        return EINA_FALSE;
     }

   trap = t;
   return EINA_TRUE;
}
