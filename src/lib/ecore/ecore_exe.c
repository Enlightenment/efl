#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Efl.h>

#include "Ecore.h"
#include "ecore_private.h"

#define MY_CLASS ECORE_EXE_CLASS

#include "ecore_exe_private.h"

/* TODO: Something to let people build a command line and does auto escaping -
 *
 * ecore_exe_snprintf()
 *
 *   OR
 *
 * cmd = ecore_exe_comand_parameter_append(cmd, "firefox");
 * cmd = ecore_exe_comand_parameter_append(cmd, "http://www.foo.com/bar.html?baz=yes");
 * each parameter appended is one argument, and it gets escaped, quoted, and
 * appended with a preceding space.  The first is the command off course.
 */

struct _ecore_exe_dead_exe
{
   pid_t pid;
   char *cmd;
};

EAPI int ECORE_EXE_EVENT_ADD = 0;
EAPI int ECORE_EXE_EVENT_DEL = 0;
EAPI int ECORE_EXE_EVENT_DATA = 0;
EAPI int ECORE_EXE_EVENT_ERROR = 0;

Eina_List *_ecore_exe_exes = NULL;

EAPI void
ecore_exe_run_priority_set(int pri)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   _impl_ecore_exe_run_priority_set(pri);
}

EAPI int
ecore_exe_run_priority_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   return _impl_ecore_exe_run_priority_get();
}

EAPI Ecore_Exe *
ecore_exe_run(const char *exe_cmd,
              const void *data)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   return ecore_exe_pipe_run(exe_cmd, 0, data);
}

EAPI Ecore_Exe *
ecore_exe_pipe_run(const char     *exe_cmd,
                   Ecore_Exe_Flags flags,
                   const void     *data)
{
   Ecore_Exe *ret = eo_add(MY_CLASS, NULL, ecore_obj_exe_command_set(exe_cmd, flags));
   if (ret)
     {
        Ecore_Exe_Data *pd = eo_data_scope_get(ret, MY_CLASS);
        pd->data = (void *) data;
     }

   return ret;
}

EOLIAN static void
_ecore_exe_command_set(Eo *obj EINA_UNUSED, Ecore_Exe_Data *pd, const char *cmd, Ecore_Exe_Flags flags)
{
   pd->cmd = strdup(cmd);
   pd->flags = flags;
}

EOLIAN static void
_ecore_exe_command_get(Eo *obj EINA_UNUSED, Ecore_Exe_Data *pd, const char **cmd, Ecore_Exe_Flags *flags)
{
   if (cmd) *cmd = pd->cmd;
   if (flags) *flags = pd->flags;
}

EOLIAN static Eo *
_ecore_exe_eo_base_finalize(Eo *obj, Ecore_Exe_Data *exe)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   obj = eo_do_super(obj, MY_CLASS, eo_finalize());

   if (!obj)
      return obj;

   return _impl_ecore_exe_eo_base_finalize(obj, exe);
}

EAPI void
ecore_exe_callback_pre_free_set(Ecore_Exe   *obj,
                                Ecore_Exe_Cb func)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;
   exe->pre_free_cb = func;
}

EAPI Eina_Bool
ecore_exe_send(Ecore_Exe  *obj,
               const void *data,
               int         size)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(EINA_FALSE);
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return EINA_FALSE;

   if (exe->close_stdin)
   {
      ERR("Ecore_Exe %p stdin is closed! Cannot send %d bytes from %p",
          exe, size, data);
      return EINA_FALSE;
   }

   return _impl_ecore_exe_send(obj, exe, data, size);
}

EAPI void
ecore_exe_close_stdin(Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;

   exe->close_stdin = 1;
}

EAPI void
ecore_exe_auto_limits_set(Ecore_Exe *obj,
                          int        start_bytes,
                          int        end_bytes,
                          int        start_lines,
                          int        end_lines)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;

   _impl_ecore_exe_auto_limits_set(obj, exe, start_bytes, end_bytes, start_lines, end_lines);
}

EAPI Ecore_Exe_Event_Data *
ecore_exe_event_data_get(Ecore_Exe      *obj,
                         Ecore_Exe_Flags flags)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return NULL;

   return _impl_ecore_exe_event_data_get(obj, exe, flags);
}

EAPI void
ecore_exe_tag_set(Ecore_Exe  *obj,
                  const char *tag)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;

   IF_FREE(exe->tag);
   if (tag)
     exe->tag = strdup(tag);
   else
     exe->tag = NULL;
}

EAPI const char *
ecore_exe_tag_get(const Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return NULL;

   return exe->tag;
}

EAPI void *
ecore_exe_free(Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return NULL;

   void *data = exe->data;
   eo_del(obj);

   return data;
}

EOLIAN static void
_ecore_exe_eo_base_destructor(Eo *obj, Ecore_Exe_Data *exe)
{
   eo_do_super(obj, ECORE_EXE_CLASS, eo_destructor());

   _impl_ecore_exe_eo_base_destructor(obj, exe);
}

EAPI void
ecore_exe_event_data_free(Ecore_Exe_Event_Data *e)
{
   if (!e) return;
   IF_FREE(e->lines);
   IF_FREE(e->data);
   free(e);
}

EAPI pid_t
ecore_exe_pid_get(const Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return -1;

   return exe->pid;
}

EAPI const char *
ecore_exe_cmd_get(const Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   const char *ret = NULL;

   eo_do(obj, ecore_obj_exe_command_get(&ret, NULL));

   return ret;
}

EAPI void *
ecore_exe_data_get(const Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return NULL;

   return exe->data;
}

EAPI void *
ecore_exe_data_set(Ecore_Exe *obj,
                   void      *data)
{
   void *ret;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return NULL;

   ret = exe->data;
   exe->data = data;
   return ret;
}

EAPI Ecore_Exe_Flags
ecore_exe_flags_get(const Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return 0;

   return exe->flags;
}

EAPI void
ecore_exe_pause(Ecore_Exe *obj)
{
   eo_do(obj, efl_control_suspend_set(EINA_TRUE));
}

EAPI void
ecore_exe_continue(Ecore_Exe *obj)
{
   eo_do(obj, efl_control_suspend_set(EINA_FALSE));
}

EOLIAN static void
_ecore_exe_efl_control_suspend_set(Eo *obj EINA_UNUSED, Ecore_Exe_Data *exe, Eina_Bool suspend)
{
   EINA_MAIN_LOOP_CHECK_RETURN;

   if (suspend)
     {
        _impl_ecore_exe_pause(obj, exe);
     }
   else
     {
        _impl_ecore_exe_continue(obj, exe);
     }
}

EAPI void
ecore_exe_interrupt(Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;

   _impl_ecore_exe_interrupt(obj, exe);
}

EAPI void
ecore_exe_quit(Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;

   _impl_ecore_exe_quit(obj, exe);
}

EAPI void
ecore_exe_terminate(Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;

   _impl_ecore_exe_terminate(obj, exe);
}

EAPI void
ecore_exe_kill(Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;

   _impl_ecore_exe_kill(obj, exe);
}

EAPI void
ecore_exe_signal(Ecore_Exe *obj,
                 int        num)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;

   _impl_ecore_exe_signal(obj, exe, num);
}

EAPI void
ecore_exe_hup(Ecore_Exe *obj)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
   if (!eo_isa(obj, MY_CLASS))
      return;

   _impl_ecore_exe_hup(obj, exe);
}

void
_ecore_exe_init(void)
{
   ECORE_EXE_EVENT_ADD = ecore_event_type_new();
   ECORE_EXE_EVENT_DEL = ecore_event_type_new();
   ECORE_EXE_EVENT_DATA = ecore_event_type_new();
   ECORE_EXE_EVENT_ERROR = ecore_event_type_new();
}

void
_ecore_exe_shutdown(void)
{
   Ecore_Exe *exe = NULL;
   Eina_List *l1, *l2;
   EINA_LIST_FOREACH_SAFE(_ecore_exe_exes, l1, l2, exe)
      ecore_exe_free(exe);
}

Ecore_Exe *
_ecore_exe_find(pid_t pid)
{
   Eina_List *itr;
   Ecore_Exe *obj;

   EINA_LIST_FOREACH(_ecore_exe_exes, itr, obj)
   {
      Ecore_Exe_Data *exe = eo_data_scope_get(obj, MY_CLASS);
      if (exe->pid == pid)
        return obj;
   }
   return NULL;
}

void *
_ecore_exe_event_del_new(void)
{
   Ecore_Exe_Event_Del *e;

   e = calloc(1, sizeof(Ecore_Exe_Event_Del));
   return e;
}

void
_ecore_exe_event_del_free(void *data EINA_UNUSED,
                          void *ev)
{
   Ecore_Exe_Event_Del *e;

   e = ev;
   if (e->exe)
     ecore_exe_free(e->exe);
   free(e);
}

void
_ecore_exe_event_exe_data_free(void *data EINA_UNUSED,
                               void *ev)
{
   Ecore_Exe_Event_Data *e;

   e = ev;
   ecore_exe_event_data_free(e);
}

Ecore_Exe_Event_Add *
_ecore_exe_event_add_new(void)
{
   Ecore_Exe_Event_Add *e;

   e = calloc(1, sizeof(Ecore_Exe_Event_Add));
   return e;
}

void
_ecore_exe_event_add_free(void *data EINA_UNUSED,
                          void *ev)
{
   Ecore_Exe_Event_Add *e;

   e = ev;
   free(e);
}

#include "ecore_exe.eo.c"
