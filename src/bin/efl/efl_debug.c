/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#include <Eina.h>
#include <Ecore.h>

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

#define EXTRACT(_buf, pval, sz) \
{ \
   memcpy(pval, _buf, sz); \
   _buf += sz; \
}
#define _EVLOG_INTERVAL 0.2

static int               _evlog_max_times = 0;
static Ecore_Timer      *_evlog_fetch_timer = NULL;
static FILE             *_evlog_file = NULL;

static int _cl_stat_reg_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _cid_from_pid_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _prof_on_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _prof_off_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _cpufreq_on_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _cpufreq_off_opcode = EINA_DEBUG_OPCODE_INVALID;
static int _evlog_get_opcode = EINA_DEBUG_OPCODE_INVALID;

static Eina_Debug_Session *_session = NULL;

static int _cid = 0;

static int my_argc = 0;
static char **my_argv = NULL;

static Eina_Debug_Error
_evlog_get_cb(Eina_Debug_Session *session EINA_UNUSED, int src EINA_UNUSED, void *buffer, int size)
{
   static int received_times = 0;
   unsigned char *d = buffer;
   unsigned int *overflow = (unsigned int *)(d + 0);
   unsigned char *p = d + 4;
   unsigned int blocksize = size - 4;

   if(++received_times <= _evlog_max_times)
     {
        if ((_evlog_file) && (blocksize > 0))
          {
             unsigned int header[3];

             header[0] = 0xffee211;
             header[1] = blocksize;
             header[2] = *overflow;
             if (fwrite(header, 1, 12, _evlog_file) < 12 ||
                   fwrite(p, 1, blocksize, _evlog_file) < blocksize)
                printf("Error writing bytes to evlog file\n");
          }
     }

   if(received_times == _evlog_max_times)
     {
        printf("Received last evlog response\n");
        if (_evlog_file) fclose(_evlog_file);
        _evlog_file = NULL;
        ecore_main_loop_quit();
     }

   return EINA_DEBUG_OK;
}

static Eina_Bool
_cb_evlog(void *data EINA_UNUSED)
{
   static int sent_times = 0;
   Eina_Bool ret = ECORE_CALLBACK_RENEW;
   if(++sent_times <= _evlog_max_times)
         eina_debug_session_send(_session, _cid, _evlog_get_opcode, NULL, 0);

   if(sent_times == _evlog_max_times)
     {
        eina_debug_session_send(_session, _cid, _cpufreq_off_opcode, NULL, 0);
        ecore_timer_del(_evlog_fetch_timer);
        _evlog_fetch_timer = NULL;
        ret = ECORE_CALLBACK_CANCEL;
     }

   return ret;
}

static Eina_Debug_Error
_cid_get_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer, int size EINA_UNUSED)
{
   _cid = *(int *)buffer;

   const char *op_str = my_argv[1];
   Eina_Bool quit = EINA_TRUE;

   if ((!strcmp(op_str, "pon")) && (3 <= (my_argc - 1)))
     {
        int freq = atoi(my_argv[3]);
        eina_debug_session_send(_session, _cid, _prof_on_opcode, &freq, sizeof(int));
     }
   else if (!strcmp(op_str, "poff"))
      eina_debug_session_send(_session, _cid, _prof_off_opcode,  NULL, 0);
   else if (!strcmp(op_str, "evlogon") && (3 <= (my_argc - 1)))
     {
        double max_time;
        sscanf(my_argv[3], "%lf", &max_time);
        _evlog_max_times = max_time > 0 ? (max_time/_EVLOG_INTERVAL+1) : 1;
        eina_debug_session_send(_session, 0, _cl_stat_reg_opcode, NULL, 0);
        printf("Evlog request will be sent %d times\n", _evlog_max_times);
        eina_debug_session_send(_session, _cid, _cpufreq_on_opcode,  NULL, 0);

        /* Creating the evlog file and setting the timer */
        char path[4096];
        int pid = atoi(my_argv[2]);
        snprintf(path, sizeof(path), "%s/efl_debug_evlog-%ld.log",
              getenv("HOME"), (long)pid);
        _evlog_file = fopen(path, "wb");
        _evlog_fetch_timer = ecore_timer_add(_EVLOG_INTERVAL, _cb_evlog, NULL);

        quit = EINA_FALSE;
     }
   else if (!strcmp(op_str, "evlogoff"))
        eina_debug_session_send(_session, _cid, _cpufreq_off_opcode,  NULL, 0);

   if(quit)
        ecore_main_loop_quit();

   return EINA_DEBUG_OK;
}

static Eina_Debug_Error
_clients_info_added_cb(Eina_Debug_Session *session EINA_UNUSED, int src EINA_UNUSED, void *buffer, int size)
{
   char *buf = buffer;
   while(size)
     {
        int cid, pid, len;
        EXTRACT(buf, &cid, sizeof(int));
        EXTRACT(buf, &pid, sizeof(int));
        /* We dont need client notifications on evlog */
        if(!_evlog_fetch_timer)
           printf("Added: CID: %d - PID: %d - Name: %s\n", cid, pid, buf);
        len = strlen(buf) + 1;
        buf += len;
        size -= (2 * sizeof(int) + len);
     }
   return EINA_DEBUG_OK;
}

static Eina_Debug_Error
_clients_info_deleted_cb(Eina_Debug_Session *session EINA_UNUSED, int src EINA_UNUSED, void *buffer, int size)
{
   char *buf = buffer;
   while(size)
     {
        int cid;
        EXTRACT(buf, &cid, sizeof(int));
        size -= sizeof(int);

        /* If client deleted dont send anymore evlog requests */
        if(_evlog_fetch_timer)
          {
             if(_cid == cid)
               {
                  printf("Evlog debugged App closed (CID: %d), stopping evlog\n", cid);
                  ecore_timer_del(_evlog_fetch_timer);
                  _evlog_fetch_timer = NULL;
                  fclose(_evlog_file);
                  _evlog_file = NULL;
                  ecore_main_loop_quit();
               }
          }
        else
           printf("Deleted: CID: %d\n", cid);
     }
   return EINA_DEBUG_OK;
}

static void
_ecore_thread_dispatcher(void *data)
{
   eina_debug_dispatch(_session, data);
}

Eina_Debug_Error
_disp_cb(Eina_Debug_Session *session EINA_UNUSED, void *buffer)
{
   ecore_main_loop_thread_safe_call_async(_ecore_thread_dispatcher, buffer);
   return EINA_DEBUG_OK;
}

static void
_args_handle(Eina_Bool flag)
{
   if (!flag) exit(0);
   eina_debug_session_dispatch_override(_session, _disp_cb);;

   const char *op_str = my_argv[1];
   if (op_str && !strcmp(op_str, "list"))
     {
        eina_debug_session_send(_session, 0, _cl_stat_reg_opcode, NULL, 0);
     }
   else if (2 <= my_argc - 1)
     {
        int pid = atoi(my_argv[2]);
        eina_debug_session_send(_session, 0, _cid_from_pid_opcode, &pid, sizeof(int));
     }
}

static const Eina_Debug_Opcode ops[] =
{
     {"daemon/observer/client/register", &_cl_stat_reg_opcode,   NULL},
     {"daemon/observer/slave_added",   NULL,                  &_clients_info_added_cb},
     {"daemon/observer/slave_deleted", NULL,                  &_clients_info_deleted_cb},
     {"daemon/info/cid_from_pid",      &_cid_from_pid_opcode,  &_cid_get_cb},
     {"profiler/on",                   &_prof_on_opcode,       NULL},
     {"profiler/off",                  &_prof_off_opcode,      NULL},
     {"cpufreq/on",                    &_cpufreq_on_opcode,      NULL},
     {"cpufreq/off",                   &_cpufreq_off_opcode,     NULL},
     {"evlog/get",                     &_evlog_get_opcode,     _evlog_get_cb},
     {NULL, NULL, NULL}
};

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   eina_init();
   ecore_init();

   my_argc = argc;
   my_argv = argv;

   _session = eina_debug_local_connect(EINA_TRUE);
   if (!_session)
     {
        fprintf(stderr, "ERROR: Cannot connect to debug daemon.\n");
        return -1;
     }
   eina_debug_opcodes_register(_session, ops, _args_handle);

   ecore_main_loop_begin();

   ecore_shutdown();
   eina_shutdown();

   return 0;
}
