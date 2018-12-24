#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif
#include <Eina.h>
#include <Eet.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Elementary.h>

#include <Exactness.h>

#include "exactness_private.h"

typedef struct
{
   Eina_Debug_Session *session;
   int srcid;
   void *buffer;
   unsigned int size;
} _Main_Loop_Info;

#define WRAPPER_TO_XFER_MAIN_LOOP(foo) \
static void \
_intern_main_loop ## foo(void *data) \
{ \
   _Main_Loop_Info *info = data; \
   _main_loop ## foo(info->session, info->srcid, info->buffer, info->size); \
   free(info->buffer); \
   free(info); \
} \
static Eina_Bool \
foo(Eina_Debug_Session *session, int srcid, void *buffer, int size) \
{ \
   _Main_Loop_Info *info = calloc(1, sizeof(*info)); \
   info->session = session; \
   info->srcid = srcid; \
   info->size = size; \
   if (info->size) \
     { \
        info->buffer = malloc(info->size); \
        memcpy(info->buffer, buffer, info->size); \
     } \
   ecore_main_loop_thread_safe_call_async(_intern_main_loop ## foo, info); \
   return EINA_TRUE; \
}

#ifndef WORDS_BIGENDIAN
#define SWAP_64(x) x
#define SWAP_32(x) x
#define SWAP_16(x) x
#define SWAP_DBL(x) x
#else
#define SWAP_64(x) eina_swap64(x)
#define SWAP_32(x) eina_swap32(x)
#define SWAP_16(x) eina_swap16(x)
#define SWAP_DBL(x) SWAP_64(x)
#endif

#define EXTRACT_INT(_buf) \
({ \
   int __i; \
   memcpy(&__i, _buf, sizeof(int)); \
   _buf += sizeof(int); \
   SWAP_32(__i); \
})

#define STORE_INT(_buf, __i) \
{ \
   int __i2 = SWAP_32(__i); \
   memcpy(_buf, &__i2, sizeof(int)); \
   _buf += sizeof(int); \
}

#define STORE_DOUBLE(_buf, __d) \
{ \
   double __d2 = SWAP_DBL(__d); \
   memcpy(_buf, &__d2, sizeof(double)); \
   _buf += sizeof(double); \
}

#define STORE_STRING(_buf, __s) \
{ \
   int __len = (__s ? strlen(__s) : 0) + 1; \
   if (__s) memcpy(_buf, __s, __len); \
   else *_buf = '\0'; \
   _buf += __len; \
}

static Eina_Stringshare *_src_filename = NULL;
static Exactness_Unit *_src_unit = NULL;
static int _verbose = 0;

static Eina_Debug_Session *_session = NULL;
static int _cid = -1, _pid = -1;
static Eina_List *_cur_event_list = NULL;

static int _all_apps_get_op = EINA_DEBUG_OPCODE_INVALID;
static int _mouse_in_op = EINA_DEBUG_OPCODE_INVALID;
static int _mouse_out_op = EINA_DEBUG_OPCODE_INVALID;
static int _mouse_wheel_op = EINA_DEBUG_OPCODE_INVALID;
static int _multi_down_op = EINA_DEBUG_OPCODE_INVALID;
static int _multi_up_op = EINA_DEBUG_OPCODE_INVALID;
static int _multi_move_op = EINA_DEBUG_OPCODE_INVALID;
static int _key_down_op = EINA_DEBUG_OPCODE_INVALID;
static int _key_up_op = EINA_DEBUG_OPCODE_INVALID;
static int _take_shot_op = EINA_DEBUG_OPCODE_INVALID;
static int _efl_event_op = EINA_DEBUG_OPCODE_INVALID;
static int _click_on_op = EINA_DEBUG_OPCODE_INVALID;
static int _stabilize_op = EINA_DEBUG_OPCODE_INVALID;
static int _finish_op = EINA_DEBUG_OPCODE_INVALID;

static Eina_Bool _all_apps_get_cb(Eina_Debug_Session *, int , void *, int);

EINA_DEBUG_OPCODES_ARRAY_DEFINE(_debug_ops,
      {"Daemon/Client/register_observer", &_all_apps_get_op, NULL},
      {"Daemon/Client/added", NULL, &_all_apps_get_cb},
      {"Exactness/Actions/Mouse In", &_mouse_in_op, NULL},
      {"Exactness/Actions/Mouse Out", &_mouse_out_op, NULL},
      {"Exactness/Actions/Mouse Wheel", &_mouse_wheel_op, NULL},
      {"Exactness/Actions/Multi Down", &_multi_down_op, NULL},
      {"Exactness/Actions/Multi Up", &_multi_up_op, NULL},
      {"Exactness/Actions/Multi Move", &_multi_move_op, NULL},
      {"Exactness/Actions/Key Down", &_key_down_op, NULL},
      {"Exactness/Actions/Key Up", &_key_up_op, NULL},
      {"Exactness/Actions/Take Shot", &_take_shot_op, NULL},
      {"Exactness/Actions/EFL Event", &_efl_event_op, NULL},
      {"Exactness/Actions/Click On", &_click_on_op, NULL},
      {"Exactness/Actions/Stabilize", &_stabilize_op, NULL},
      {"Exactness/Actions/Finish", &_finish_op, NULL},
      {NULL, NULL, NULL}
      );

static void
_printf(int verbose, const char *fmt, ...)
{
   va_list ap;
   if (!_verbose || verbose > _verbose) return;

   va_start(ap, fmt);
   vprintf(fmt, ap);
   va_end(ap);
}

static void
_feed_event(Exactness_Action_Type type, unsigned int n_evas, void *data)
{
   switch (type)
     {
      case EXACTNESS_ACTION_MOUSE_IN:
           {
              _printf(1, "Mouse in\n");
              _printf(2, "%s evas_event_feed_mouse_in n_evas=<%d>\n", __func__, n_evas);
              eina_debug_session_send(_session, _cid, _mouse_in_op, &n_evas, sizeof(int));
              break;
           }
      case EXACTNESS_ACTION_MOUSE_OUT:
           {
              _printf(1, "Mouse out\n");
              _printf(2, "%s evas_event_feed_mouse_out n_evas=<%d>\n", __func__, n_evas);
              eina_debug_session_send(_session, _cid, _mouse_out_op, &n_evas, sizeof(int));
              break;
           }
      case EXACTNESS_ACTION_MOUSE_WHEEL:
           {
              Exactness_Action_Mouse_Wheel *t = data;
              int len = 3*sizeof(int);
              char *buf = malloc(len), *tmp = buf;
              _printf(1, "Mouse wheel\n");
              _printf(2, "%s evas_event_feed_mouse_wheel n_evas=<%d>\n", __func__, n_evas);
              STORE_INT(tmp, n_evas);
              STORE_INT(tmp, t->direction);
              STORE_INT(tmp, t->z);
              eina_debug_session_send(_session, _cid, _mouse_wheel_op, buf, len);
              break;
           }
      case EXACTNESS_ACTION_MULTI_DOWN:
      case EXACTNESS_ACTION_MULTI_UP:
           {
              Exactness_Action_Multi_Event *t = data;
              int len = 5*sizeof(int)+7*sizeof(double)+sizeof(int);
              char *buf = malloc(len), *tmp = buf;
              _printf(2, "%s %s n_evas=<%d>\n", __func__,
                    type == EXACTNESS_ACTION_MULTI_DOWN ? "evas_event_feed_multi_down" :
                    "evas_event_feed_multi_up", n_evas);
              STORE_INT(tmp, n_evas);
              STORE_INT(tmp, t->d);
              STORE_INT(tmp, t->b);
              STORE_INT(tmp, t->x);
              STORE_INT(tmp, t->y);
              STORE_DOUBLE(tmp, t->rad);
              STORE_DOUBLE(tmp, t->radx);
              STORE_DOUBLE(tmp, t->rady);
              STORE_DOUBLE(tmp, t->pres);
              STORE_DOUBLE(tmp, t->ang);
              STORE_DOUBLE(tmp, t->fx);
              STORE_DOUBLE(tmp, t->fy);
              STORE_INT(tmp, t->flags);
              eina_debug_session_send(_session, _cid,
                    type == EXACTNESS_ACTION_MULTI_DOWN ? _multi_down_op : _multi_up_op,
                    buf, len);
              break;
           }
      case EXACTNESS_ACTION_MULTI_MOVE:
           {
              Exactness_Action_Multi_Move *t = data;
              int len = 4*sizeof(int)+7*sizeof(double);
              char *buf = malloc(len), *tmp = buf;
              _printf(2, "%s evas_event_feed_multi_move n_evas=<%d>\n", __func__, n_evas);
              STORE_INT(tmp, n_evas);
              STORE_INT(tmp, t->d);
              STORE_INT(tmp, t->x);
              STORE_INT(tmp, t->y);
              STORE_DOUBLE(tmp, t->rad);
              STORE_DOUBLE(tmp, t->radx);
              STORE_DOUBLE(tmp, t->rady);
              STORE_DOUBLE(tmp, t->pres);
              STORE_DOUBLE(tmp, t->ang);
              STORE_DOUBLE(tmp, t->fx);
              STORE_DOUBLE(tmp, t->fy);
              eina_debug_session_send(_session, _cid, _multi_move_op, buf, len);
              break;
           }
      case EXACTNESS_ACTION_KEY_DOWN:
      case EXACTNESS_ACTION_KEY_UP:
           {
              Exactness_Action_Key_Down_Up *t = data;
              int len = 2*sizeof(int) + 4;
              len += t->keyname ? strlen(t->keyname) : 0;
              len += t->key ? strlen(t->key) : 0;
              len += t->string ? strlen(t->string) : 0;
              len += t->compose ? strlen(t->compose) : 0;
              char *buf = malloc(len), *tmp = buf;
              _printf(2, "%s %s n_evas=<%d>\n", __func__,
                    type == EXACTNESS_ACTION_KEY_DOWN ? "evas_event_feed_key_down " :
                    "evas_event_feed_key_up", n_evas);
              STORE_INT(tmp, n_evas);
              STORE_STRING(tmp, t->keyname);
              STORE_STRING(tmp, t->key);
              STORE_STRING(tmp, t->string);
              STORE_STRING(tmp, t->compose);
              STORE_INT(tmp, t->keycode);
              eina_debug_session_send(_session, _cid,
                    type == EXACTNESS_ACTION_KEY_DOWN ? _key_down_op : _key_up_op,
                    buf, len);
              break;
           }
      case EXACTNESS_ACTION_TAKE_SHOT:
           {
              _printf(2, "%s take shot n_evas=<%d>\n", __func__, n_evas);
              eina_debug_session_send(_session, _cid, _take_shot_op, &n_evas, sizeof(int));
              break;
           }
      case EXACTNESS_ACTION_EFL_EVENT:
           {
              Exactness_Action_Efl_Event *t = data;
              int len = 0;
              len += t->wdg_name ? strlen(t->wdg_name) : 0;
              len += t->event_name ? strlen(t->event_name) : 0;
              char *buf = malloc(len), *tmp = buf;
              _printf(2, "%s %s\n", __func__, "EFL event");
              STORE_STRING(tmp, t->wdg_name);
              STORE_STRING(tmp, t->event_name);
              eina_debug_session_send(_session, _cid, _efl_event_op, buf, len);
              break;
           }
      case EXACTNESS_ACTION_CLICK_ON:
           {
              Exactness_Action_Click_On *t = data;
              int len = 0;
              len += t->wdg_name ? strlen(t->wdg_name) : 0;
              char *buf = malloc(len), *tmp = buf;
              _printf(2, "%s %s\n", __func__, "Click On");
              STORE_STRING(tmp, t->wdg_name);
              eina_debug_session_send(_session, _cid, _click_on_op, buf, len);
              break;
           }
      case EXACTNESS_ACTION_STABILIZE:
           {
              _printf(2, "%s stabilize\n", __func__);
              eina_debug_session_send(_session, _cid, _stabilize_op, NULL, 0);
              break;
           }
      default: /* All non-input events are not handeled */
         break;
     }
}

static Eina_Bool
_feed_event_timer_cb(void *data EINA_UNUSED)
{
   Exactness_Action *act = eina_list_data_get(_cur_event_list);
   _feed_event(act->type, act->n_evas, act->data);

   _cur_event_list = eina_list_next(_cur_event_list);

   if (!_cur_event_list)
     {  /* Finished reading all events */
        eina_debug_session_send(_session, _cid, _finish_op, NULL, 0);
        ecore_main_loop_quit();
     }
   else
     {
        Exactness_Action *cur_act = eina_list_data_get(_cur_event_list);
        ecore_timer_add(cur_act->delay_ms / 1000.0, _feed_event_timer_cb, NULL);
     }
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_src_open()
{
   double diff_time = 0; /* Time to wait before feeding the first event */

   _printf(2, "<%s> Source file is <%s>\n", __func__, _src_filename);
   if (!strcmp(_src_filename + strlen(_src_filename) - 4,".exu"))
     {
        _src_unit = exactness_unit_file_read(_src_filename);
     }
   else if (!strcmp(_src_filename + strlen(_src_filename) - 4,".rec"))
     {
        _src_unit = legacy_rec_file_read(_src_filename);
     }
   if (!_src_unit) return EINA_FALSE;
   _cur_event_list = _src_unit->actions;
   Exactness_Action *act = eina_list_data_get(_cur_event_list);

   if (act->delay_ms)
     {
        _printf(2, "  Waiting <%f>\n", diff_time);
        ecore_timer_add(act->delay_ms / 1000.0, _feed_event_timer_cb, NULL);
     }
   else
     {
        _feed_event_timer_cb(NULL);
     }
   return EINA_TRUE;
}

static void
_main_loop_all_apps_get_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   char *buf = buffer;
   int chosen_cid = -1;
   if (_cid != -1) return;
   while (size > 0)
     {
        int cid, pid, len;
        cid = EXTRACT_INT(buf);
        pid = EXTRACT_INT(buf);
        if (_pid != -1)
          {
             if (_pid == pid)
               {
                  _cid = cid;
                  _src_open();
                  return;
               }
          }
        else
          {
             if (!strcmp(buf, "exactness_play"))
               {
                  if (chosen_cid != -1)
                    {
                       fprintf(stderr, "Need to specify a PID - too much choice\n");
                       return;
                    }
                  chosen_cid = cid;
               }
          }
        len = strlen(buf) + 1;
        buf += len;
        size -= (2 * sizeof(int) + len);
     }
   if (chosen_cid != -1)
     {
        _cid = chosen_cid;
        _src_open();
     }
}

WRAPPER_TO_XFER_MAIN_LOOP(_all_apps_get_cb)

static void
_ops_ready_cb(void *data EINA_UNUSED, Eina_Bool status)
{
   static Eina_Bool on = EINA_FALSE;
   if (status)
     {
        if (!on)
          {
             eina_debug_session_send(_session, 0, _all_apps_get_op, NULL, 0);
          }
        on = EINA_TRUE;
     }
}

static const Ecore_Getopt optdesc = {
  "exactness_inject",
  "%prog [options] <-v|-p|-t|-h> command",
  PACKAGE_VERSION,
  "(C) 2018 Enlightenment",
  "BSD",
  "A scenario events injector for EFL based applications.",
  1,
  {
    ECORE_GETOPT_STORE_STR('t', "test", "Test to run on the given application"),
    ECORE_GETOPT_STORE_INT('p', "pid", "PID of the application to connect to"),
    ECORE_GETOPT_STORE_INT('r', "remote-port", "Port to connect remotely to the daemon. Local connection if not specified"),
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
   int opt_args = 0, real__ = 1, port = -1;
   char *src = NULL;
   Eina_Value *ret__;
   Eina_Bool want_quit = EINA_FALSE;

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(src),
     ECORE_GETOPT_VALUE_INT(_pid),
     ECORE_GETOPT_VALUE_INT(port),
     ECORE_GETOPT_VALUE_INT(_verbose),

     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_NONE
   };

   eina_init();
   eet_init();
   ecore_init();

   opt_args = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (opt_args < 0)
     {
        fprintf(stderr, "Failed parsing arguments.\n");
        goto end;
     }
   if (want_quit) goto end;

   if (!src)
     {
        fprintf(stderr, "no test file specified\n");
        goto end;
     }
   _src_filename = eina_stringshare_add(src);

   if (port == -1)
      _session = eina_debug_local_connect(EINA_TRUE);
   else
      _session = eina_debug_remote_connect(port);
   eina_debug_opcodes_register(_session, _debug_ops(), _ops_ready_cb, NULL);

   elm_init(argc, argv);
   ret__ = efl_loop_begin(efl_main_loop_get());
   real__ = efl_loop_exit_code_process(ret__);
   elm_shutdown();
end:
   eet_shutdown();
   eina_shutdown();
   return real__;
}

