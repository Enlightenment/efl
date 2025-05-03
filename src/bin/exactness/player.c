#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#ifdef HAVE_DLSYM
# include <dlfcn.h>
#endif

#ifdef HAVE_FORK
# ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
# endif
# ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# ifdef HAVE_SYS_SYSINFO_H
#  include <sys/sysinfo.h>
# endif
#endif

#ifdef _WIN32
# include <evil_private.h> /* setenv */
# undef HAVE_DLSYM
#endif

#include <Eina.h>
#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Ecore_File.h>
#include <Ecore_Con.h>
#include <Elementary.h>

#include "common.h"

#define CMD_LINE_MAX 256
#define IMAGE_FILENAME_EXT ".png"
#define PAUSE_KEY_STR "F2"

#define DBG(...) EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)

static int _log_domain = -1;

typedef enum
{
   FTYPE_UNKNOWN,
   FTYPE_DIR,
   FTYPE_REC = FTYPE_DIR,
   FTYPE_EXU,
   FTYPE_REMOTE
} File_Type;

static File_Type _dest_type = FTYPE_UNKNOWN;
static Eina_Stringshare *_dest = NULL;
static Exactness_Unit *_dest_unit = NULL;

static File_Type _src_type = FTYPE_UNKNOWN;
static Eina_Stringshare *_src_filename = NULL;
static Exactness_Unit *_src_unit = NULL;

static const char *_test_name = NULL;
static int _verbose = 0;

static Eina_List *_evas_list = NULL;

static Eina_List *_cur_event_list = NULL;

static int _cur_shot_id = 0;
static Eina_Bool _shot_needed = EINA_FALSE;
static Eina_Bool _scan_objects = EINA_FALSE, _disable_shots = EINA_FALSE, _stabilize_shots = EINA_FALSE;

static Eina_Debug_Session *_last_debug_session = NULL;
static int _last_debug_src_cid = 0;
static int _take_shot_op = EINA_DEBUG_OPCODE_INVALID;

static Eina_Bool _stabilization_timer_cb(void *);
static double _speed = 1.0;

static Eina_Bool _exit_required = EINA_FALSE;
static Eina_Bool _pause_request = EINA_FALSE;
static Eina_Bool _playing_status = EINA_FALSE;
static Eina_Bool _ready_to_write = EINA_FALSE;

static Exactness_Image *
_snapshot_shot_get(Evas *e)
{
   Exactness_Image *ex_img;
   Evas_Object *snapshot;
   void *pixels;
   int w, h, nb_bytes;

   if (!e) return NULL;

   evas_output_size_get(e, &w, &h);
   if ((w < 1) || (h < 1)) return NULL;

   snapshot = efl_key_data_get(e, "_snapshot");
   if (!snapshot)
     {
        snapshot = evas_object_image_filled_add(e);
        if (snapshot)
          {
             evas_object_image_snapshot_set(snapshot, EINA_TRUE);
             evas_object_geometry_set(snapshot, 0, 0, w, h);
             efl_gfx_entity_visible_set(snapshot, EINA_TRUE);
             efl_key_data_set(e, "_snapshot", snapshot);
          }
        return NULL;
     }

   pixels = evas_object_image_data_get(snapshot, EINA_FALSE);
   if (!pixels) return NULL;

   ex_img = malloc(sizeof(*ex_img));
   nb_bytes = w * h * 4;
   ex_img->w = w;
   ex_img->h = h;
   ex_img->pixels = malloc(nb_bytes);
   memcpy(ex_img->pixels, pixels, nb_bytes);
   return ex_img;
}

static void
_evas_render_post_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   if (_shot_needed)
     {
        Evas_Event_Render_Post *post = event->info;
        void *e_data = efl_key_data_get(event->object, "_shot");

        // Nothing was updated, so let's not bother sending nothingness
        if (post && !post->updated_area) return;
        Exactness_Image *ex_shot = efl_key_data_get(event->object, "_last_shot");
        if (!ex_shot) ex_shot = _snapshot_shot_get(event->object);
        if (!ex_shot) return;

        efl_key_data_set(event->object, "_last_shot", NULL);

        if (e_data)
          {
             if (_dest_type == FTYPE_DIR)
               {
                  char *filename = e_data;
                  Eo *o = evas_object_image_add(event->object);
                  evas_object_image_size_set(o, ex_shot->w, ex_shot->h);
                  evas_object_image_data_set(o, ex_shot->pixels);
                  INF("Shot taken (%s).\n", filename);
                  if (!evas_object_image_save(o, filename, NULL, NULL))
                    {
                       printf("Cannot save widget to <%s>\n", filename);
                    }
                  free(filename);
               }
             else if (_dest_type == FTYPE_EXU)
               {
                  Exactness_Image *ex_img = e_data;
                  memcpy(ex_img, ex_shot, sizeof(Exactness_Image));
                  ex_shot->pixels = NULL;
                  INF("Shot taken (in %s).\n", _dest);
               }
             else if (_dest_type == FTYPE_REMOTE)
               {
                  int len = sizeof(int) + sizeof(int) + ex_shot->w * ex_shot->h * 4;
                  char *buf = alloca(len);
                  char *tmp = buf;
                  STORE_INT(tmp, ex_shot->w);
                  STORE_INT(tmp, ex_shot->h);
                  memcpy(tmp, ex_shot->pixels, ex_shot->w * ex_shot->h * 4);
                  eina_debug_session_send(_last_debug_session, _last_debug_src_cid, _take_shot_op, buf, len);
               }
          }
        exactness_image_free(ex_shot);
        efl_key_data_set(event->object, "_shot", NULL);
        evas_object_del(efl_key_data_get(event->object, "_snapshot"));
        efl_key_data_set(event->object, "_snapshot", NULL);
        /* This part is needed when the shot is needed at the end of the scenario.
         * As it is async, we need to wait for the shot termination. */
        _shot_needed = EINA_FALSE;
        if (_exit_required) ecore_main_loop_quit();
     }
}

static void
_shot_do(Evas *e)
{
   void *e_data = NULL;
   if (!e) return;

   if (!_disable_shots)
     {
        if (_dest_type == FTYPE_DIR)
          {
             int dir_name_len;
             char *filename;

             dir_name_len = strlen(_dest) + 1; /* includes space of a '/' */
             filename = malloc(strlen(_test_name) + strlen(IMAGE_FILENAME_EXT) +
                   dir_name_len + 8); /* also space for serial */

             sprintf(filename, "%s/%s%c%03d%s", _dest, _test_name,
                   SHOT_DELIMITER, _cur_shot_id, IMAGE_FILENAME_EXT);
             e_data = filename;
          }
        else if (_dest_type == FTYPE_EXU)
          {
             Exactness_Image *ex_img = malloc(sizeof(*ex_img));

             if (ex_img)
               {
                  memset(ex_img, 0, sizeof(*ex_img));
                  _dest_unit->imgs = eina_list_append(_dest_unit->imgs, ex_img);
                  _dest_unit->nb_shots++;
                  e_data = ex_img;
               }
          }
        else if (_dest_type == FTYPE_REMOTE)
          {
             e_data = e;
          }
     }
   efl_key_data_set(e, "_shot", e_data);
   _shot_needed = EINA_TRUE;
   Efl_Event ev;
   ev.info = NULL;
   ev.object = e;
   _evas_render_post_cb(NULL, &ev);

   if (_scan_objects && _dest_type == FTYPE_EXU)
     {
        Eina_Iterator *iter;
        Eo *obj;
        Exactness_Objects *e_objs = calloc(1, sizeof(*e_objs));
        iter = eo_objects_iterator_new();
        EINA_ITERATOR_FOREACH(iter, obj)
          {
             if (!efl_isa(obj, EFL_CANVAS_OBJECT_CLASS) &&
                   !efl_isa(obj, EFL_CANVAS_SCENE_INTERFACE)) continue;
             Exactness_Object *e_obj = calloc(1, sizeof(*e_obj));
             Eo *parent = efl_parent_get(obj);
             e_obj->id = (long long)(intptr_t)obj;
             if (efl_isa(parent, EFL_CANVAS_OBJECT_CLASS) ||
                   efl_isa(parent, EFL_CANVAS_SCENE_INTERFACE))
               {
                  e_obj->parent_id = (long long)(intptr_t)efl_parent_get(obj);
               }
             else
               {
                  e_obj->parent_id = 0;
               }
             e_obj->kl_name = eina_stringshare_add(efl_class_name_get(obj));
             if (efl_isa(obj, EFL_CANVAS_OBJECT_CLASS))
               {
                  Eina_Size2D sz = efl_gfx_entity_size_get(obj);
                  e_obj->w = sz.w;
                  e_obj->h = sz.h;
                  Eina_Position2D pos = efl_gfx_entity_position_get(obj);
                  e_obj->x = pos.x;
                  e_obj->y = pos.y;
               }
             e_objs->objs = eina_list_append(e_objs->objs, e_obj);
          }
        eina_iterator_free(iter);
        _dest_unit->objs = eina_list_append(_dest_unit->objs, e_objs);
     }
}

static void
_feed_event(Exactness_Action_Type type, unsigned int n_evas, void *data)
{
   static Evas_Object *rect = NULL;
   static unsigned int rect_evas;

   Eo *e = eina_list_nth(_evas_list, n_evas);

   if (rect && rect_evas != n_evas)
     {
        efl_del(rect);
        rect = NULL;
     }
   if (_verbose && !rect)
     {
        rect = evas_object_rectangle_add(e);
        evas_object_repeat_events_set(rect, EINA_TRUE);
        evas_object_color_set(rect, 255, 0, 0, 255);
        evas_object_resize(rect, 15, 15);
        evas_object_layer_set(rect, 100);
        evas_object_show(rect);
        rect_evas = n_evas;
     }

   switch (type)
     {
      case EXACTNESS_ACTION_MOUSE_IN:
           {
              INF("Mouse in\n");
              DBG("%s evas_event_feed_mouse_in n_evas=<%d>\n", __func__, n_evas);
              if (e) evas_event_feed_mouse_in(e, time(NULL), NULL);
              break;
           }
      case EXACTNESS_ACTION_MOUSE_OUT:
           {
              INF("Mouse out\n");
              DBG("%s evas_event_feed_mouse_out n_evas=<%d>\n", __func__, n_evas);
              if (e) evas_event_feed_mouse_out(e, time(NULL), NULL);
              break;
           }
      case EXACTNESS_ACTION_MOUSE_WHEEL:
           {
              Exactness_Action_Mouse_Wheel *t = data;
              INF("Mouse wheel\n");
              DBG("%s evas_event_feed_mouse_wheel n_evas=<%d>\n", __func__, n_evas);
              if (e) evas_event_feed_mouse_wheel(e, t->direction, t->z, time(NULL), NULL);

              break;
           }
      case EXACTNESS_ACTION_MULTI_DOWN:
           {
              Exactness_Action_Multi_Event *t = data;
              DBG("%s evas_event_feed_multi_down n_evas=<%d>\n", __func__, n_evas);
              if (!t->d)
                {
                   if (e) evas_event_feed_mouse_down(e, t->b, t->flags, time(NULL), NULL);
                   if (rect) evas_object_color_set(rect, 255, 255, 0, 255);
                }
              else
                {
                   if (e) evas_event_feed_multi_down(e,
                         t->d, t->x, t->y, t->rad,
                         t->radx, t->rady, t->pres, t->ang, t->fx, t->fy,
                         t->flags, time(NULL), NULL);
                }

              break;
           }
      case EXACTNESS_ACTION_MULTI_UP:
           {
              Exactness_Action_Multi_Event *t = data;
              DBG("%s evas_event_feed_multi_up n_evas=<%d>\n", __func__, n_evas);
              if (!t->d)
                {
                   if (e) evas_event_feed_mouse_up(e, t->b, t->flags, time(NULL), NULL);
                   if (rect) evas_object_color_set(rect, 255, 0, 0, 255);
                }
              else
                {
                   if (e) evas_event_feed_multi_up(e,
                         t->d, t->x, t->y, t->rad,
                         t->radx, t->rady, t->pres, t->ang, t->fx, t->fy,
                         t->flags, time(NULL), NULL);
                }

              break;
           }
      case EXACTNESS_ACTION_MULTI_MOVE:
           {
              Exactness_Action_Multi_Move *t = data;
              DBG("%s evas_event_feed_multi_move n_evas=<%d>\n", __func__, n_evas);
              if (!t->d)
                {
                   if (e) evas_event_feed_mouse_move(e, t->x, t->y, time(NULL), NULL);
                   if (rect)
                     {
                        evas_object_move(rect, t->x, t->y);
                        evas_object_color_set(rect, 255, 0, 0, 255);
                     }
                }
              else
                {
                   if (e) evas_event_feed_multi_move(e,
                         t->d, t->x, t->y, t->rad,
                         t->radx, t->rady, t->pres, t->ang, t->fx, t->fy,
                         time(NULL), NULL);
                }

              break;
           }
      case EXACTNESS_ACTION_KEY_DOWN:
           {
              Exactness_Action_Key_Down_Up *t = data;
              DBG("%s evas_event_feed_key_down n_evas=<%d>\n", __func__, n_evas);
              if (e)
                 evas_event_feed_key_down_with_keycode(e,
                       t->keyname, t->key, t->string,
                       t->compose, time(NULL), NULL, t->keycode);
              break;
           }
      case EXACTNESS_ACTION_KEY_UP:
           {
              Exactness_Action_Key_Down_Up *t = data;
              DBG("%s evas_event_feed_key_up n_evas=<%d>\n", __func__, n_evas);
              if (e) evas_event_feed_key_up_with_keycode(e,
                    t->keyname, t->key, t->string,
                    t->compose, time(NULL), NULL, t->keycode);

              break;
           }
      case EXACTNESS_ACTION_TAKE_SHOT:
           {
              DBG("%s take shot n_evas=<%d>\n", __func__, n_evas);
              if (rect) evas_object_color_set(rect, 0, 0, 255, 255);
              _cur_shot_id++;
              if (_dest_type != FTYPE_UNKNOWN && e) _shot_do(e);
              break;
           }
      case EXACTNESS_ACTION_EFL_EVENT:
           {
              Exactness_Action_Efl_Event *t = data;
              Eina_Bool found = EINA_FALSE;
              Eina_List *itr;
              EINA_LIST_FOREACH(_evas_list, itr, e)
                {
                   Eo *o = efl_name_find(e, t->wdg_name);
                   if (o)
                     {
                        DBG("%s EFL event invoke %s on %s\n",
                              __func__, t->event_name, t->wdg_name);
                        Efl_Event_Description d;
                        found = EINA_TRUE;
                        memset(&d, 0, sizeof(d));
                        d.name = t->event_name;
                        d.legacy_is = EINA_TRUE;
                        efl_event_callback_legacy_call(o, &d, NULL);
                     }
                }
              if (!found) fprintf(stderr, "Failed finding %s.\n", t->wdg_name);
              break;
           }
      case EXACTNESS_ACTION_CLICK_ON:
           {
              Exactness_Action_Click_On *t = data;
              Eina_List *itr;
              Eo *o;
              n_evas = 0;
              EINA_LIST_FOREACH(_evas_list, itr, e)
                {
                   o = efl_name_find(e, t->wdg_name);
                   if (o) goto wdg_found;
                   n_evas++;
                }
              o = NULL;
wdg_found:
              if (o)
                {
                   Eina_Size2D sz = efl_gfx_entity_size_get(o);
                   Eina_Position2D pos = efl_gfx_entity_position_get(o);
                   int x = pos.x + (sz.w / 2);
                   int y = pos.y + (sz.h / 2);
                   Exactness_Action_Multi_Move *d_move = calloc(1, sizeof(*d_move));
                   Exactness_Action_Multi_Event *d_event = calloc(1, sizeof(*d_event));
                   Exactness_Action *act, *prev_act = eina_list_data_get(_cur_event_list);

                   DBG("%s click on %s\n", __func__, t->wdg_name);
                   act = calloc(1, sizeof(*act));
                   act->type = EXACTNESS_ACTION_MULTI_MOVE;
                   act->delay_ms = 100;
                   act->n_evas = n_evas;
                   act->data = d_move;
                   d_move->x = x;
                   d_move->y = y;
                   _cur_event_list = eina_list_append_relative(_cur_event_list,
                         act, prev_act);
                   prev_act = act;

                   act = calloc(1, sizeof(*act));
                   act->type = EXACTNESS_ACTION_MULTI_DOWN;
                   act->delay_ms = 100;
                   act->n_evas = n_evas;
                   act->data = d_event;
                   d_event->b = 1;
                   _cur_event_list = eina_list_append_relative(_cur_event_list,
                         act, prev_act);
                   prev_act = act;

                   act = calloc(1, sizeof(*act));
                   act->type = EXACTNESS_ACTION_MULTI_UP;
                   act->delay_ms = 100;
                   act->n_evas = n_evas;
                   d_event = calloc(1, sizeof(*d_event));
                   act->data = d_event;
                   d_event->b = 1;
                   _cur_event_list = eina_list_append_relative(_cur_event_list,
                         act, prev_act);
                }
              else fprintf(stderr, "Failed finding %s.\n", t->wdg_name);
              break;
           }
      case EXACTNESS_ACTION_STABILIZE:
           {
              DBG("%s stabilize\n", __func__);
              if (rect) evas_object_color_set(rect, 255, 165, 0, 255);
              ecore_timer_add(0.1, _stabilization_timer_cb, NULL);
              break;
           }
      default: /* All non-input events are not handeled */
         break;
     }
}

static Eina_Bool
_feed_event_timer_cb(void *data EINA_UNUSED)
{
   if (_pause_request) return ECORE_CALLBACK_CANCEL;
   Exactness_Action *act = eina_list_data_get(_cur_event_list);
   if (act) _feed_event(act->type, act->n_evas, act->data);

   _cur_event_list = eina_list_next(_cur_event_list);
   if (!_cur_event_list)
     {  /* Finished reading all events */
        _exit_required = EINA_TRUE;
        if (!_shot_needed) ecore_main_loop_quit();
     }
   else
     {
        if (act && act->type != EXACTNESS_ACTION_STABILIZE)
          {
             act = eina_list_data_get(_cur_event_list);
             if (act)
               {
                  DBG("  %s timer_time=<%f>\n", __func__, act->delay_ms / 1000.0);
                  ecore_timer_add(act->delay_ms / 1000.0, _feed_event_timer_cb, NULL);
               }
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_stabilization_timer_cb(void *data EINA_UNUSED)
{
   Eina_List *itr;
   Evas *e;
#define STAB_MAX 5
   static int need_more = STAB_MAX;
   DBG("Not stable yet!\n");
   EINA_LIST_FOREACH(_evas_list, itr, e)
     {
        if (!e) continue;
        Exactness_Image *last_img = efl_key_data_get(e, "_last_stab_shot");
        Exactness_Image *cur_img = _snapshot_shot_get(e);
        if (!last_img || exactness_image_compare(last_img, cur_img, NULL)) need_more = STAB_MAX;
        exactness_image_free(last_img);
        efl_key_data_set(e, "_last_stab_shot", cur_img);
     }
   EINA_LIST_FOREACH(_evas_list, itr, e)
     {
        if (!need_more)
          {
             evas_object_del(efl_key_data_get(e, "_snapshot"));
             efl_key_data_set(e, "_snapshot", NULL);
          }
     }
   if (!need_more)
     {
        _playing_status = EINA_FALSE;
        if (_src_type != FTYPE_REMOTE && !_pause_request)
          {
             Exactness_Action *act = eina_list_data_get(_cur_event_list);
             if (act)
               {
                  DBG("  %s timer_time=<%f>\n", __func__, act->delay_ms / 1000.0);
                  ecore_timer_add(act->delay_ms / 1000.0, _feed_event_timer_cb, NULL);
               }
          }
        need_more = STAB_MAX;
        return ECORE_CALLBACK_CANCEL;
     }
   need_more--;
   return ECORE_CALLBACK_RENEW;
}

static void
_main_loop_mouse_in_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   int n_evas = EXTRACT_INT(buf);
   _feed_event(EXACTNESS_ACTION_MOUSE_IN, n_evas, NULL);
}

static void
_main_loop_mouse_out_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   int n_evas = EXTRACT_INT(buf);
   _feed_event(EXACTNESS_ACTION_MOUSE_OUT, n_evas, NULL);
}

static void
_main_loop_mouse_wheel_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   Exactness_Action_Mouse_Wheel t;
   int n_evas = EXTRACT_INT(buf);
   t.direction = EXTRACT_INT(buf);
   t.z = EXTRACT_INT(buf);
   _feed_event(EXACTNESS_ACTION_MOUSE_WHEEL, n_evas, &t);
}

static void
_main_loop_multi_down_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   Exactness_Action_Multi_Event t;
   int n_evas = EXTRACT_INT(buf);
   t.d = EXTRACT_INT(buf);
   t.b = EXTRACT_INT(buf);
   t.x = EXTRACT_INT(buf);
   t.y = EXTRACT_INT(buf);
   t.rad = EXTRACT_DOUBLE(buf);
   t.radx = EXTRACT_DOUBLE(buf);
   t.rady = EXTRACT_DOUBLE(buf);
   t.pres = EXTRACT_DOUBLE(buf);
   t.ang = EXTRACT_DOUBLE(buf);
   t.fx = EXTRACT_DOUBLE(buf);
   t.fy = EXTRACT_DOUBLE(buf);
   t.flags = EXTRACT_INT(buf);
   _feed_event(EXACTNESS_ACTION_MULTI_DOWN, n_evas, &t);
}

static void
_main_loop_multi_up_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   Exactness_Action_Multi_Event t;
   int n_evas = EXTRACT_INT(buf);
   t.d = EXTRACT_INT(buf);
   t.b = EXTRACT_INT(buf);
   t.x = EXTRACT_INT(buf);
   t.y = EXTRACT_INT(buf);
   t.rad = EXTRACT_DOUBLE(buf);
   t.radx = EXTRACT_DOUBLE(buf);
   t.rady = EXTRACT_DOUBLE(buf);
   t.pres = EXTRACT_DOUBLE(buf);
   t.ang = EXTRACT_DOUBLE(buf);
   t.fx = EXTRACT_DOUBLE(buf);
   t.fy = EXTRACT_DOUBLE(buf);
   t.flags = EXTRACT_INT(buf);
   _feed_event(EXACTNESS_ACTION_MULTI_UP, n_evas, &t);
}

static void
_main_loop_multi_move_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   Exactness_Action_Multi_Move t;
   int n_evas = EXTRACT_INT(buf);
   t.d = EXTRACT_INT(buf);
   t.x = EXTRACT_INT(buf);
   t.y = EXTRACT_INT(buf);
   t.rad = EXTRACT_DOUBLE(buf);
   t.radx = EXTRACT_DOUBLE(buf);
   t.rady = EXTRACT_DOUBLE(buf);
   t.pres = EXTRACT_DOUBLE(buf);
   t.ang = EXTRACT_DOUBLE(buf);
   t.fx = EXTRACT_DOUBLE(buf);
   t.fy = EXTRACT_DOUBLE(buf);
   _feed_event(EXACTNESS_ACTION_MULTI_MOVE, n_evas, &t);
}

static void
_main_loop_key_down_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   Exactness_Action_Key_Down_Up t;
   int n_evas = EXTRACT_INT(buf);
   t.keyname = EXTRACT_STRING(buf);
   t.key = EXTRACT_STRING(buf);
   t.string = EXTRACT_STRING(buf);
   t.compose = EXTRACT_STRING(buf);
   t.keycode = EXTRACT_INT(buf);
   _feed_event(EXACTNESS_ACTION_KEY_DOWN, n_evas, &t);
}

static void
_main_loop_key_up_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   Exactness_Action_Key_Down_Up t;
   int n_evas = EXTRACT_INT(buf);
   t.keyname = EXTRACT_STRING(buf);
   t.key = EXTRACT_STRING(buf);
   t.string = EXTRACT_STRING(buf);
   t.compose = EXTRACT_STRING(buf);
   t.keycode = EXTRACT_INT(buf);
   _feed_event(EXACTNESS_ACTION_KEY_UP, n_evas, &t);
}

static void
_main_loop_take_shot_cb(Eina_Debug_Session *session, int srcid, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   int n_evas = EXTRACT_INT(buf);
   _feed_event(EXACTNESS_ACTION_TAKE_SHOT, n_evas, NULL);
   _last_debug_session = session;
   _last_debug_src_cid = srcid;
}

static void
_main_loop_efl_event_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   Exactness_Action_Efl_Event t;
   t.wdg_name = EXTRACT_STRING(buf);
   t.event_name = EXTRACT_STRING(buf);
   _feed_event(EXACTNESS_ACTION_EFL_EVENT, 0, &t);
}

static void
_main_loop_click_on_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   Exactness_Action_Click_On t;
   t.wdg_name = EXTRACT_STRING(buf);
   _feed_event(EXACTNESS_ACTION_CLICK_ON, 0, &t);
}

static void
_main_loop_stabilize_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   _feed_event(EXACTNESS_ACTION_STABILIZE, 0, NULL);
}

static void
_main_loop_finish_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   ecore_main_loop_quit();
}

WRAPPER_TO_XFER_MAIN_LOOP(_mouse_in_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_mouse_out_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_mouse_wheel_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_multi_down_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_multi_up_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_multi_move_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_key_down_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_key_up_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_take_shot_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_efl_event_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_click_on_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_stabilize_cb)
WRAPPER_TO_XFER_MAIN_LOOP(_finish_cb)

EINA_DEBUG_OPCODES_ARRAY_DEFINE(_debug_ops,
     {"Exactness/Actions/Mouse In", NULL, &_mouse_in_cb},
     {"Exactness/Actions/Mouse Out", NULL, &_mouse_out_cb},
     {"Exactness/Actions/Mouse Wheel", NULL, &_mouse_wheel_cb},
     {"Exactness/Actions/Multi Down", NULL, &_multi_down_cb},
     {"Exactness/Actions/Multi Up", NULL, &_multi_up_cb},
     {"Exactness/Actions/Multi Move", NULL, &_multi_move_cb},
     {"Exactness/Actions/Key Down", NULL, &_key_down_cb},
     {"Exactness/Actions/Key Up", NULL, &_key_up_cb},
     {"Exactness/Actions/Take Shot", &_take_shot_op, &_take_shot_cb},
     {"Exactness/Actions/EFL Event", NULL, &_efl_event_cb},
     {"Exactness/Actions/Click On", NULL, &_click_on_cb},
     {"Exactness/Actions/Stabilize", NULL, &_stabilize_cb},
     {"Exactness/Actions/Finish", NULL, &_finish_cb},
     {NULL, NULL, NULL}
);

static Eina_Bool
_src_feed(void *data EINA_UNUSED)
{
   if (!_evas_list) return EINA_TRUE;
   _cur_event_list = _src_unit->actions;
   Exactness_Action *act = eina_list_data_get(_cur_event_list);

   if (act && act->delay_ms)
     {
        DBG("  Waiting <%f>\n", act->delay_ms / 1000.0);
        ecore_timer_add(act->delay_ms / 1000.0, _feed_event_timer_cb, NULL);
     }
   else
     {
        _feed_event_timer_cb(NULL);
     }
   return EINA_FALSE;
}

static Eina_Bool
_src_open()
{
   if (_src_type != FTYPE_REMOTE)
     {
        Eina_List *itr, *itr2;
        Exactness_Action *act;
        DBG("<%s> Source file is <%s>\n", __func__, _src_filename);
        if (_src_type == FTYPE_EXU)
          {
             _src_unit = exactness_unit_file_read(_src_filename);
             _ready_to_write = EINA_TRUE;
          }
        if (!_src_unit) return EINA_FALSE;
        if (_stabilize_shots)
          {
             Exactness_Action_Type last_action_type = EXACTNESS_ACTION_UNKNOWN;
             EINA_LIST_FOREACH_SAFE(_src_unit->actions, itr, itr2, act)
               {
                  if (act->type == EXACTNESS_ACTION_TAKE_SHOT &&
                        last_action_type != EXACTNESS_ACTION_STABILIZE)
                    {
                       Exactness_Action *act2 = calloc(1, sizeof(*act2));
                       act2->type = EXACTNESS_ACTION_STABILIZE;
                       _src_unit->actions = eina_list_prepend_relative(_src_unit->actions, act2, act);
                    }
                  last_action_type = act->type;
               }
          }
        if (EINA_DBL_NONZERO(_speed) && (!EINA_DBL_EQ(_speed, 1)))
          {
             EINA_LIST_FOREACH(_src_unit->actions, itr, act)
                act->delay_ms /= _speed;
          }
     }
   else
     {
        eina_debug_opcodes_register(NULL, _debug_ops(), NULL, NULL);
     }
   return EINA_TRUE;
}

static void
_old_shots_rm_cb(const char *name, const char *path, void *data)
{
   const char *prefix = data;
   unsigned int len = strlen(prefix);
   if (!strncmp(name, prefix, len) && (strlen(name) > len) && (name[len] == SHOT_DELIMITER))
     {
        unsigned int length = strlen(path) + strlen(name) + 2;
        char *buf = alloca(length);
        snprintf(buf, length, "%s/%s", path, name);
        if (unlink(buf))
          {
             printf("Failed deleting '%s/%s': ", path, name);
             perror("");
          }
     }
}

static void
_evas_del_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_List *p = eina_list_data_find_list(_evas_list, event->object);
   eina_list_data_set(p, NULL);
}

static void
_event_key_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Key *evk = event->info;
   if (!evk) return;
   const char *key = efl_input_key_name_get(evk);

   if (!strcmp(key, PAUSE_KEY_STR) && efl_input_key_pressed_get(evk))
     {
        _pause_request = !_pause_request;
        if (_pause_request) INF("Pausing scenario\n");
        else
          {
             INF("Playing scenario\n");
             if (!_playing_status)
                _feed_event_timer_cb(NULL);
          }
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(_evas_callbacks,
      { EFL_EVENT_DEL, _evas_del_cb },
      { EFL_CANVAS_SCENE_EVENT_RENDER_POST, _evas_render_post_cb },
      { EFL_EVENT_KEY_DOWN, _event_key_cb },
      { EFL_EVENT_KEY_UP, _event_key_cb }
      )

static Evas *
_my_evas_new(int w EINA_UNUSED, int h EINA_UNUSED)
{
   Evas *e;
   e = evas_new();
   if (e)
     {
        INF("New Evas\n");
        _evas_list = eina_list_append(_evas_list, e);
        efl_event_callback_array_add(e, _evas_callbacks(), NULL);
     }
   return e;
}

static Eina_Bool
_setup_dest_type(const char *dest, Eina_Bool external_injection)
{
   if (dest)
     {
        _dest = eina_stringshare_add(dest);
        if (!strcmp(_dest + strlen(_dest) - 4,".exu"))
          {
             _dest_type = FTYPE_EXU;
             char *path = ecore_file_dir_get(dest);

             if (!ecore_file_mkpath(path))
               {
                  fprintf(stderr, "Path for %s cannot be created\n", _dest);
                  free(path);
                  return EINA_FALSE;
               }
             free(path);
          }
        else
          {
             _dest_type = FTYPE_DIR;
             if (!ecore_file_mkpath(_dest))
               {
                  fprintf(stderr, "Directory %s cannot be created\n", _dest);
                  return EINA_FALSE;
               }
          }
     }
   if (external_injection)
     {
        _src_type = FTYPE_REMOTE;
        if (_dest_type == FTYPE_UNKNOWN) _dest_type = FTYPE_REMOTE;
     }
   return EINA_TRUE;
}

static void
_setup_names(const char *src)
{
   if (src)
     {
        _src_filename = eina_stringshare_add(src);
        if (!strcmp(_src_filename + strlen(_src_filename) - 4,".exu"))
          {
             _src_type = FTYPE_EXU;
             if (_dest_type == FTYPE_UNKNOWN)
               {
                  _dest_type = FTYPE_EXU;
                  _dest = "./output.exu";
               }
          }
        char *slash = strrchr(_src_filename, '/');
        if (slash) _test_name = strdup(slash + 1);
        else _test_name = strdup(_src_filename);
        char *dot = strrchr(_test_name, '.');
        if (dot) *dot = '\0';
     }
}

static void
_setup_dest_unit(void)
{
   if (_dest_type == FTYPE_EXU) _dest_unit = calloc(1, sizeof(*_dest_unit));

}

static void
_remove_old_shots(void)
{
   if (_dest_type == FTYPE_DIR && _test_name)
      eina_file_dir_list(_dest, 0, _old_shots_rm_cb, (void *)_test_name);
}

static Eina_Bool
_setup_font_settings(const char *fonts_dir)
{
   const char *chosen_fonts = NULL;
   if (_src_unit && _src_unit->fonts_path)
     {
        char buf[PATH_MAX];
        if (!fonts_dir) fonts_dir = "./fonts";
        snprintf(buf, PATH_MAX, "%s/%s", fonts_dir, _src_unit->fonts_path);
        if (!ecore_file_exists(buf))
          {
             fprintf(stderr, "Unable to use the fonts path '%s' provided in %s\n",
                   _src_unit->fonts_path, _src_filename);
             return EINA_FALSE;
          }
        chosen_fonts = _src_unit->fonts_path;
     }
   if (fonts_dir)
     {
        Eina_Tmpstr *fonts_conf_name = NULL;
        if (!ecore_file_exists(fonts_dir))
          {
             fprintf(stderr, "Unable to find fonts directory %s\n", fonts_dir);
             return EINA_FALSE;
          }
        if (!chosen_fonts)
          {
             Eina_List *dated_fonts = ecore_file_ls(fonts_dir);
             char *date_dir;
             chosen_fonts = eina_stringshare_add(eina_list_last_data_get(dated_fonts));
             EINA_LIST_FREE(dated_fonts, date_dir) free(date_dir);
          }
        if (chosen_fonts)
          {
             int tmp_fd = eina_file_mkstemp("/tmp/fonts_XXXXXX.conf", &fonts_conf_name);
             if (tmp_fd < 0) return EINA_FALSE;
             FILE *tmp_f = fdopen(tmp_fd, "wb");
             fprintf(tmp_f,
                   "<?xml version=\"1.0\"?>\n<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n<fontconfig>\n"
                   "<dir prefix=\"default\">%s/%s</dir>\n</fontconfig>\n",
                   fonts_dir, chosen_fonts);
             fclose(tmp_f);
             close(tmp_fd);

             setenv("FONTCONFIG_FILE", fonts_conf_name, 1);
          }
     }
   return EINA_TRUE;
}

static void
_setup_ee_creation(void)
{
   ecore_evas_callback_new_set(_my_evas_new);
   if (_src_type != FTYPE_REMOTE)
      ecore_idler_add(_src_feed, NULL);
}

static void
_write_unit_file(void)
{
   if (_dest && _dest_unit && _ready_to_write)
     {
        Exactness_Unit *tmp = NULL;

        EINA_SAFETY_ON_NULL_RETURN(_src_unit);
        if (_src_type == FTYPE_EXU)
          {
             tmp = exactness_unit_file_read(_src_filename);
             _dest_unit->actions = tmp->actions;
          }
        exactness_unit_file_write(_dest_unit, _dest);
     }
}
#ifdef HAVE_DLSYM
# define ORIGINAL_CALL_T(t, name, ...) \
   t (*_original_init_cb)(...); \
   _original_init_cb = dlsym(RTLD_NEXT, name); \
   original_return = _original_init_cb(__VA_ARGS__);
#else
# define ORIGINAL_CALL_T(t, name, ...) \
   printf("THIS IS NOT SUPPORTED ON WINDOWS\n"); \
   abort();
#endif

#define ORIGINAL_CALL(name, ...) \
   ORIGINAL_CALL_T(int, name, __VA_ARGS__)

EAPI int
eina_init(void)
{
   int original_return;

   ORIGINAL_CALL("eina_init");

   ex_set_original_envvar();

   if (original_return == 1)
     {
        const char *dest = getenv("EXACTNESS_DEST");
        const char *external_injection = getenv("EXACTNESS_EXTERNAL_INJECTION");
        const char *src = getenv("EXACTNESS_SRC");
        const char *fonts_dir = getenv("EXACTNESS_FONTS_DIR");
        const char *speed = getenv("EXACTNESS_SPEED");

        _scan_objects = !!getenv("EXACTNESS_SCAN_OBJECTS");
        _disable_shots = !!getenv("EXACTNESS_DISABLE_SHOTS");
        _stabilize_shots = !!getenv("EXACTNESS_STABILIZE_SHOTS");
        _verbose = !!getenv("EXACTNESS_VERBOSE");
        if (speed)
          _speed = eina_convert_strtod_c(speed, NULL);

        _log_domain = eina_log_domain_register("exactness_player", NULL);
        if (!_setup_dest_type(dest, !!external_injection))
          return 0;
        _setup_names(src);
        _setup_dest_unit();
        _remove_old_shots();

        if (!_src_open())
          {
             fprintf(stderr, "Unable to read source file\n");
             return 0;
          }
        if (!_setup_font_settings(fonts_dir))
          return 0;
     }

   return original_return;
}

EAPI int
ecore_evas_init(void)
{
   int original_return;

   ORIGINAL_CALL("ecore_evas_init")

   if (ex_is_original_app() && original_return == 1)
     {
        _setup_ee_creation();
     }

   return original_return;
}

//hook, to hook in our theme
EAPI int
elm_init(int argc, char **argv)
{
   int original_return;
   ORIGINAL_CALL("elm_init", argc, argv)

   if (ex_is_original_app() && original_return == 1)
     ex_prepare_elm_overlay();

   return original_return;
}

EAPI void
ecore_main_loop_begin(void)
{
   int original_return;
   ORIGINAL_CALL("ecore_main_loop_begin")
   if (ex_is_original_app())
     _write_unit_file();
   (void)original_return;
}

EAPI Eina_Value*
efl_loop_begin(Eo *obj)
{
   Eina_Value *original_return;
   ORIGINAL_CALL_T(Eina_Value*, "efl_loop_begin", obj);
   if (ex_is_original_app())
     _write_unit_file();
   return original_return;
}

EAPI int
eina_shutdown(void)
{
   int original_return;
   static Eina_Bool output_written = EINA_FALSE;
   ORIGINAL_CALL("eina_shutdown")
   if (ex_is_original_app() &&original_return == 1 && !output_written)
     {
        output_written = EINA_TRUE;
        _write_unit_file();
     }

   return original_return;
}
