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

#include <sys/types.h>
#ifdef HAVE_SYS_SYSINFO_H
# include <sys/sysinfo.h>
#endif

#include <Eina.h>
#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_Getopt.h>
#include <Ecore_Con.h>
#include <Elementary.h>

#include "common.h"

#define STABILIZE_KEY_STR "F1"
#define SHOT_KEY_STR "F2"
#define SAVE_KEY_STR "F3"

#define DBG(...) EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)

static int _log_domain = -1;

static const char *_out_filename = NULL;

static Eina_List *_evas_list = NULL;
static unsigned int _last_evas_id = 0;

static Exactness_Unit *_unit = NULL;

static char *_shot_key = NULL;
static unsigned int _last_timestamp = 0.0;

static Exactness_Action_Type
_event_pointer_type_get(Efl_Pointer_Action t)
{
   switch(t)
     {
      case EFL_POINTER_ACTION_IN: return EXACTNESS_ACTION_MOUSE_IN;
      case EFL_POINTER_ACTION_OUT: return EXACTNESS_ACTION_MOUSE_OUT;
      case EFL_POINTER_ACTION_DOWN: return EXACTNESS_ACTION_MULTI_DOWN;
      case EFL_POINTER_ACTION_UP: return EXACTNESS_ACTION_MULTI_UP;
      case EFL_POINTER_ACTION_MOVE: return EXACTNESS_ACTION_MULTI_MOVE;
      case EFL_POINTER_ACTION_WHEEL: return EXACTNESS_ACTION_MOUSE_WHEEL;
      default: return EXACTNESS_ACTION_UNKNOWN;
     }
}

static void
_output_write()
{
   if (_unit) exactness_unit_file_write(_unit, _out_filename);
}

static void
_add_to_list(Exactness_Action_Type type, unsigned int n_evas, unsigned int timestamp, void *data, int len)
{
   if (_unit)
     {
        const Exactness_Action *prev_v = eina_list_last_data_get(_unit->actions);
        if (prev_v)
          {
             if (prev_v->type == type &&
                   timestamp == _last_timestamp &&
                   prev_v->n_evas == n_evas &&
                   (!len || !memcmp(prev_v->data, data, len))) return;
          }
        INF("Recording %s\n", _exactness_action_type_to_string_get(type));
        Exactness_Action *act =  malloc(sizeof(*act));
        act->type = type;
        act->n_evas = n_evas;
        act->delay_ms = timestamp - _last_timestamp;
        _last_timestamp = timestamp;
        if (len)
          {
             act->data = malloc(len);
             memcpy(act->data, data, len);
          }
        _unit->actions = eina_list_append(_unit->actions, act);
     }
}

static int
_evas_id_get(Evas *e)
{
   return (intptr_t)efl_key_data_get(e, "__evas_id");
}

static void
_event_pointer_cb(void *data, const Efl_Event *event)
{
   Eo *eo_e = data;
   Eo *evp = event->info;
   if (!evp) return;

   int timestamp = efl_input_timestamp_get(evp);
   int n_evas = _evas_id_get(eo_e);
   Efl_Pointer_Action action = efl_input_pointer_action_get(evp);
   Exactness_Action_Type evt = _event_pointer_type_get(action);

   if (!timestamp) return;

  DBG("Calling \"%s\" timestamp=<%u>\n", _exactness_action_type_to_string_get(evt), timestamp);

   switch (action)
     {
      case EFL_POINTER_ACTION_MOVE:
          {
             double rad = 0, radx = 0, rady = 0, pres = 0, ang = 0, fx = 0, fy = 0;
             int tool = efl_input_pointer_touch_id_get(evp);
             Eina_Position2D pos = efl_input_pointer_position_get(evp);
             Exactness_Action_Multi_Move t = { tool, pos.x, pos.y, rad, radx, rady, pres, ang, fx, fy };
             if (n_evas >= 0) _add_to_list(evt, n_evas, timestamp, &t, sizeof(t));
             break;
          }
      case EFL_POINTER_ACTION_DOWN:
      case EFL_POINTER_ACTION_UP:
          {
             double rad = 0, radx = 0, rady = 0, pres = 0, ang = 0, fx = 0, fy = 0;
             int b = efl_input_pointer_button_get(evp);
             int tool = efl_input_pointer_touch_id_get(evp);
             Eina_Position2D pos = efl_input_pointer_position_get(evp);
             Efl_Pointer_Flags flags = efl_input_pointer_button_flags_get(evp);
             Exactness_Action_Multi_Event t = { tool, b, pos.x, pos.y, rad, radx, rady, pres, ang,
                  fx, fy, (Evas_Button_Flags)flags };
             if (n_evas >= 0) _add_to_list(evt, n_evas, timestamp, &t, sizeof(t));
             break;
          }
      case EFL_POINTER_ACTION_IN:
      case EFL_POINTER_ACTION_OUT:
          {
             if (n_evas >= 0) _add_to_list(evt, n_evas, timestamp, NULL, 0);
             break;
          }
      case EFL_POINTER_ACTION_WHEEL:
          {
             Eina_Bool horiz = efl_input_pointer_wheel_horizontal_get(evp);
             int z = efl_input_pointer_wheel_delta_get(evp);
             Exactness_Action_Mouse_Wheel t = { horiz, z };
             if (n_evas >= 0) _add_to_list(evt, n_evas, timestamp, &t, sizeof(t));
             break;
          }
      default:
        break;
     }
}

static void
_event_key_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Key *evk = event->info;
   Eo *eo_e = data;
   if (!evk) return;
   const char *key = efl_input_key_name_get(evk);
   int timestamp = efl_input_timestamp_get(evk);
   unsigned int n_evas = _evas_id_get(eo_e);
   Exactness_Action_Type evt = EXACTNESS_ACTION_KEY_UP;

   if (efl_input_key_pressed_get(evk))
     {
        if (!strcmp(key, _shot_key))
          {
             DBG("Take Screenshot: %s timestamp=<%u>\n", __func__, timestamp);
             _add_to_list(EXACTNESS_ACTION_TAKE_SHOT, n_evas, timestamp, NULL, 0);
             return;
          }
        if (!strcmp(key, STABILIZE_KEY_STR))
          {
             DBG("Stabilize: %s timestamp=<%u>\n", __func__, timestamp);
             _add_to_list(EXACTNESS_ACTION_STABILIZE, n_evas, timestamp, NULL, 0);
             return;
          }
        if (!strcmp(key, SAVE_KEY_STR))
          {
             _output_write();
             DBG("Save events: %s timestamp=<%u>\n", __func__, timestamp);
             return;
          }
        evt = EXACTNESS_ACTION_KEY_DOWN;
     }
   else
     {
        if (!strcmp(key, _shot_key) || !strcmp(key, SAVE_KEY_STR) || !strcmp(key, STABILIZE_KEY_STR)) return;
     }
   if (_unit)
     {  /* Construct duplicate strings, free them when list if freed */
        Exactness_Action_Key_Down_Up t;
        t.keyname = eina_stringshare_add(key);
        t.key = eina_stringshare_add(efl_input_key_sym_get(evk));
        t.string = eina_stringshare_add(efl_input_key_string_get(evk));
        t.compose = eina_stringshare_add(efl_input_key_compose_string_get(evk));
        t.keycode = efl_input_key_code_get(evk);
        _add_to_list(evt, n_evas, timestamp, &t, sizeof(t));
     }
}

// note: "hold" event comes from above (elm), not below (ecore)
EFL_CALLBACKS_ARRAY_DEFINE(_event_pointer_callbacks,
      { EFL_EVENT_POINTER_MOVE, _event_pointer_cb },
      { EFL_EVENT_POINTER_DOWN, _event_pointer_cb },
      { EFL_EVENT_POINTER_UP, _event_pointer_cb },
      { EFL_EVENT_POINTER_IN, _event_pointer_cb },
      { EFL_EVENT_POINTER_OUT, _event_pointer_cb },
      { EFL_EVENT_POINTER_WHEEL, _event_pointer_cb },
      { EFL_EVENT_FINGER_MOVE, _event_pointer_cb },
      { EFL_EVENT_FINGER_DOWN, _event_pointer_cb },
      { EFL_EVENT_FINGER_UP, _event_pointer_cb },
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
        efl_key_data_set(e, "__evas_id", (void *)(intptr_t)_last_evas_id++);
        efl_event_callback_array_add(e, _event_pointer_callbacks(), e);
     }
   return e;
}

static void
_setup_unit(void)
{
   if (_unit) return;

   _unit = calloc(1, sizeof(*_unit));
}

static Eina_Bool
_setup_fonts_dir(const char *fonts_dir)
{
   if (fonts_dir)
     {
        Eina_Tmpstr *fonts_conf_name = NULL;
        if (!ecore_file_exists(fonts_dir))
          {
             fprintf(stderr, "Unable to find fonts directory %s\n", fonts_dir);
             return EINA_FALSE;
          }
        Eina_List *dated_fonts = ecore_file_ls(fonts_dir);
        char *date_dir;
        _unit->fonts_path = strdup(eina_list_last_data_get(dated_fonts));
        EINA_LIST_FREE(dated_fonts, date_dir) free(date_dir);
        if (_unit->fonts_path)
          {
             int tmp_fd = eina_file_mkstemp("/tmp/fonts_XXXXXX.conf", &fonts_conf_name);
             if (tmp_fd < 0) return EINA_FALSE;
             FILE *tmp_f = fdopen(tmp_fd, "wb");
             fprintf(tmp_f,
                   "<?xml version=\"1.0\"?>\n<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n<fontconfig>\n"
                   "<dir prefix=\"default\">%s/%s</dir>\n</fontconfig>\n",
                   fonts_dir, _unit->fonts_path);
             fclose(tmp_f);
             close(tmp_fd);

             setenv("FONTCONFIG_FILE", fonts_conf_name, 1);
          }
     }
   return EINA_TRUE;
}

static void
_setup_shot_key(void)
{
   if (!_shot_key) _shot_key = getenv("SHOT_KEY");
   if (!_shot_key) _shot_key = SHOT_KEY_STR;
}

static void
_setup_ee_creation(void)
{
   ecore_evas_callback_new_set(_my_evas_new);
   _last_timestamp = ecore_time_get() * 1000;
}

#ifdef HAVE_DLSYM
# define ORIGINAL_CALL_T(t, name, ...) \
   t (*_original_init_cb)(); \
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

   if (ex_is_original_app() && original_return == 1)
     {
        _log_domain = eina_log_domain_register("exactness_recorder", NULL);

        _out_filename = getenv("EXACTNESS_DEST");
        _setup_unit();
        if (!_setup_fonts_dir(getenv("EXACTNESS_FONTS_DIR")))
          return -1;

        _setup_shot_key();
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
     _output_write();
   (void)original_return;
}

EAPI Eina_Value*
efl_loop_begin(Eo *obj)
{
   Eina_Value *original_return;
   ORIGINAL_CALL_T(Eina_Value*, "efl_loop_begin", obj);
   if (ex_is_original_app())
     _output_write();
   return original_return;
}

EAPI int
eina_shutdown(void)
{
   int original_return;
   static Eina_Bool output_written = EINA_FALSE;
   ORIGINAL_CALL("eina_shutdown")
   if (ex_is_original_app() && original_return == 1 && !output_written)
     {
        output_written = EINA_TRUE;
        _output_write();
     }

   return original_return;
}
