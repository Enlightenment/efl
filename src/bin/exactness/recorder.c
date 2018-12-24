#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <sys/types.h>
#ifdef HAVE_SYS_SYSINFO_H
# include <sys/sysinfo.h>
#endif

#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif
#include <Eina.h>
#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_Getopt.h>
#include <Ecore_Con.h>
#include <Elementary.h>
#include <Exactness.h>

#include <exactness_private.h>

#define MAX_PATH 1024
#define STABILIZE_KEY_STR "F1"
#define SHOT_KEY_STR "F2"
#define SAVE_KEY_STR "F3"

static Evas *(*_evas_new)(void) = NULL;
static const char *_out_filename = NULL;
static const char *_test_name = NULL;
static int _verbose = 0;

static Eina_List *_evas_list = NULL;
static unsigned int _last_evas_id = 0;

static Exactness_Unit *_unit = NULL;

static char *_shot_key = NULL;
static unsigned int _last_timestamp = 0.0;

static void
_printf(int verbose, const char *fmt, ...)
{
   va_list ap;
   if (!_verbose || verbose > _verbose) return;

   va_start(ap, fmt);
   vprintf(fmt, ap);
   va_end(ap);
}

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
        _printf(1, "Recording %s\n", _exactness_action_type_to_string_get(type));
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

   _printf(2, "Calling \"%s\" timestamp=<%u>\n", _exactness_action_type_to_string_get(evt), timestamp);

   switch (action)
     {
      case EFL_POINTER_ACTION_MOVE:
          {
             double rad = 0, radx = 0, rady = 0, pres = 0, ang = 0, fx = 0, fy = 0;
             int tool = efl_input_pointer_tool_get(evp);
             Eina_Position2D pos = efl_input_pointer_position_get(evp);
             Exactness_Action_Multi_Move t = { tool, pos.x, pos.y, rad, radx, rady, pres, ang, fx, fy };
             if (n_evas >= 0) _add_to_list(evt, n_evas, timestamp, &t, sizeof(t));
             break;
          }
      case EFL_POINTER_ACTION_DOWN: case EFL_POINTER_ACTION_UP:
          {
             double rad = 0, radx = 0, rady = 0, pres = 0, ang = 0, fx = 0, fy = 0;
             int b = efl_input_pointer_button_get(evp);
             int tool = efl_input_pointer_tool_get(evp);
             Eina_Position2D pos = efl_input_pointer_position_get(evp);
             Efl_Pointer_Flags flags = efl_input_pointer_button_flags_get(evp);
             Exactness_Action_Multi_Event t = { tool, b, pos.x, pos.y, rad, radx, rady, pres, ang,
                  fx, fy, flags };
             if (n_evas >= 0) _add_to_list(evt, n_evas, timestamp, &t, sizeof(t));
             break;
          }
      case EFL_POINTER_ACTION_IN: case EFL_POINTER_ACTION_OUT:
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
             _printf(2, "Take Screenshot: %s timestamp=<%u>\n", __func__, timestamp);
             _add_to_list(EXACTNESS_ACTION_TAKE_SHOT, n_evas, timestamp, NULL, 0);
             return;
          }
        if (!strcmp(key, STABILIZE_KEY_STR))
          {
             _printf(2, "Stabilize: %s timestamp=<%u>\n", __func__, timestamp);
             _add_to_list(EXACTNESS_ACTION_STABILIZE, n_evas, timestamp, NULL, 0);
             return;
          }
        if (!strcmp(key, SAVE_KEY_STR))
          {
             _output_write();
             _printf(2, "Save events: %s timestamp=<%u>\n", __func__, timestamp);
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
        t.key = eina_stringshare_add(efl_input_key_get(evk));
        t.string = eina_stringshare_add(efl_input_key_string_get(evk));
        t.compose = eina_stringshare_add(efl_input_key_compose_get(evk));
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
   if (!_evas_new) return NULL;
   e = _evas_new();
   if (e)
     {
        _printf(1, "New Evas\n");
        _evas_list = eina_list_append(_evas_list, e);
        efl_key_data_set(e, "__evas_id", (void *)(intptr_t)_last_evas_id++);
        efl_event_callback_array_add(e, _event_pointer_callbacks(), e);
     }
   return e;
}

static int
_prg_invoke(const char *full_path, int argc, char **argv)
{
   Eina_Value *ret__;
   int real__;

   void (*efl_main)(void *data, const Efl_Event *ev);
   int (*elm_main)(int argc, char **argv);
   int (*c_main)(int argc, char **argv);
   Eina_Module *h = eina_module_new(full_path);
   if (!h || !eina_module_load(h))
     {
        fprintf(stderr, "Failed loading %s.\n", full_path);
        if (h) eina_module_free(h);
        return EINA_FALSE;
     }
   efl_main = eina_module_symbol_get(h, "efl_main");
   elm_main = eina_module_symbol_get(h, "elm_main");
   c_main = eina_module_symbol_get(h, "main");
   _evas_new = eina_module_symbol_get(h, "evas_new");
   if (!_evas_new)
     {
        fprintf(stderr, "Failed loading symbol 'evas_new' from %s.\n", full_path);
        eina_module_free(h);
        return 1;
     }
   if (efl_main)
     {
        elm_init(argc, argv);
        efl_event_callback_add(efl_main_loop_get(), EFL_LOOP_EVENT_ARGUMENTS, efl_main, NULL);
        ret__ = efl_loop_begin(efl_main_loop_get());
        real__ = efl_loop_exit_code_process(ret__);
        elm_shutdown();
     }
   else if (elm_main)
     {
        elm_init(argc, argv);
        real__ = elm_main(argc, argv);
        elm_shutdown();
     }
   else if (c_main)
     {
        real__ = c_main(argc, argv);
     }
   else
     {
        fprintf(stderr, "Failed loading symbol 'efl_main', 'elm_main' or 'main' from %s.\n", full_path);
        eina_module_free(h);
        real__ = 1;
     }
   return real__;
}

static Eina_Stringshare *
_prg_full_path_guess(const char *prg)
{
   char full_path[MAX_PATH];
   if (strchr(prg, '/')) return eina_stringshare_add(prg);
   char *paths = strdup(getenv("PATH"));
   Eina_Stringshare *ret = NULL;
   while (paths && *paths && !ret)
     {
        char *real_path;
        char *colon = strchr(paths, ':');
        if (colon) *colon = '\0';

        sprintf(full_path, "%s/%s", paths, prg);
        real_path = ecore_file_realpath(full_path);
        if (*real_path)
          {
             ret = eina_stringshare_add(real_path);
             // check if executable
          }
        free(real_path);

        paths += strlen(paths);
        if (colon) paths++;
     }
   return ret;
}

static Eina_Bool
_mkdir(const char *dir)
{
   if (!ecore_file_exists(dir))
     {
        const char *cur = dir + 1;
        do
          {
             char *slash = strchr(cur, '/');
             if (slash) *slash = '\0';
             if (!ecore_file_exists(dir) && !ecore_file_mkdir(dir)) return EINA_FALSE;
             if (slash) *slash = '/';
             if (slash) cur = slash + 1;
             else cur = NULL;
          }
        while (cur);
     }
   return EINA_TRUE;
}

static const Ecore_Getopt optdesc = {
  "exactness_record",
  "%prog [options] <-v|-t|-h> command",
  PACKAGE_VERSION,
  "(C) 2017 Enlightenment",
  "BSD",
  "A scenario recorder for EFL based applications.\n"
  "\tF1 - Request stabilization\n"
  "\tF2 - Request shot\n"
  "\tF3 - Request to save the scenario\n",
  1,
  {
    ECORE_GETOPT_STORE_STR('t', "test", "Name of the filename where to store the test."),
    ECORE_GETOPT_STORE_STR('f', "fonts-dir", "Specify a directory of the fonts that should be used."),
    ECORE_GETOPT_COUNT('v', "verbose", "Turn verbose messages on."),

    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

int main(int argc, char **argv)
{
   char *dest = NULL, *eq;
   char *fonts_dir = NULL;
   int pret = 1, opt_args = 0;
   Eina_Bool want_quit = EINA_FALSE;

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(dest),
     ECORE_GETOPT_VALUE_STR(fonts_dir),
     ECORE_GETOPT_VALUE_INT(_verbose),

     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_NONE
   };

   eina_init();
   ecore_init();

   opt_args = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (opt_args < 0)
     {
        fprintf(stderr, "Failed parsing arguments.\n");
        goto end;
     }
   if (want_quit) goto end;

   /* Check for a sentinel */
   if (argv[opt_args] && !strcmp(argv[opt_args], "--")) opt_args++;

   /* Check for env variables */
   do
     {
        eq = argv[opt_args] ? strchr(argv[opt_args], '=') : NULL;
        if (eq)
          {
             char *var = malloc(eq - argv[opt_args] + 1);
             memcpy(var, argv[opt_args], eq - argv[opt_args]);
             var[eq - argv[opt_args]] = '\0';
             setenv(var, eq + 1, 1);
             opt_args++;
          }
     } while (eq);
   _out_filename = eina_stringshare_add(dest);

   if (!_out_filename)
     {
        fprintf(stderr, "no test file specified\n");
        goto end;
     }
   else
     {
        char *slash = strrchr(_out_filename, '/');
        if (slash) _test_name = strdup(slash + 1);
        else _test_name = strdup(_out_filename);
        char *dot = strrchr(_test_name, '.');
        if (dot) *dot = '\0';
        if (slash)
          {
             *slash = '\0';
             if (!_mkdir(_out_filename))
               {
                  fprintf(stderr, "Can't create %s\n", _out_filename);
                  goto end;
               }
             *slash = '/';
          }
     }
   if (strcmp(_out_filename + strlen(_out_filename) - 4,".exu"))
     {
        fprintf(stderr, "A file with a exu extension is required - %s invalid\n", _out_filename);
        goto end;
     }

   if (strcmp(_out_filename + strlen(_out_filename) - 4,".exu"))
     {
        fprintf(stderr, "A file with a exu extension is required - %s invalid\n", _out_filename);
        goto end;
     }

   if (!argv[opt_args])
     {
        fprintf(stderr, "no program specified\nUse -h for more information\n");
        goto end;
     }

   efl_object_init();
   evas_init();

   if (!_unit)
     {
        _unit = calloc(1, sizeof(*_unit));
     }

   if (fonts_dir)
     {
        Eina_Tmpstr *fonts_conf_name = NULL;
        if (!ecore_file_exists(fonts_dir))
          {
             fprintf(stderr, "Unable to find fonts directory %s\n", fonts_dir);
             goto end;
          }
        Eina_List *dated_fonts = ecore_file_ls(fonts_dir);
        char *date_dir;
        _unit->fonts_path = strdup(eina_list_last_data_get(dated_fonts));
        EINA_LIST_FREE(dated_fonts, date_dir) free(date_dir);
        if (_unit->fonts_path)
          {
             int tmp_fd = eina_file_mkstemp("/tmp/fonts_XXXXXX.conf", &fonts_conf_name);
             dprintf(tmp_fd,
                   "<?xml version=\"1.0\"?>\n<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n<fontconfig>\n"
                   "<dir prefix=\"default\">%s/%s</dir>\n</fontconfig>\n",
                   fonts_dir, _unit->fonts_path);
             close(tmp_fd);

             setenv("FONTCONFIG_FILE", fonts_conf_name, 1);
          }
     }

   /* Replace the current command line to hide the Exactness part */
   int len = argv[argc - 1] + strlen(argv[argc - 1]) - argv[opt_args];
   memcpy(argv[0], argv[opt_args], len);
   memset(argv[0] + len, 0, MAX_PATH - len);

   int i;
   for (i = opt_args; i < argc; i++)
     {
        if (i != opt_args)
          {
             argv[i - opt_args] = argv[0] + (argv[i] - argv[opt_args]);
          }
        _printf(1, "%s ", argv[i - opt_args]);
     }
   _printf(1, "\n");

   if (!_shot_key) _shot_key = getenv("SHOT_KEY");
   if (!_shot_key) _shot_key = SHOT_KEY_STR;

   ecore_evas_callback_new_set(_my_evas_new);
   _last_timestamp = ecore_time_get() * 1000;
   pret = _prg_invoke(_prg_full_path_guess(argv[0]), argc - opt_args, argv);

   _output_write();
   //free_events(_events_list, EINA_TRUE);
   //_events_list = NULL;

   pret = 0;
end:
   eina_shutdown();
   return pret;
}
