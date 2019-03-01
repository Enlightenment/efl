#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1

#include <Ecore.h>

#include "ecore_private.h"

#ifdef _WIN32
#else
# include <sys/time.h>
# include <sys/resource.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <sys/types.h>
# include <unistd.h>
# include <fcntl.h>
# include <signal.h>
# include <sys/socket.h>
# ifdef HAVE_PRCTL
#  include <sys/prctl.h>
# endif
# ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
# endif
# ifndef HAVE_CLEARENV
extern char **environ;
# endif
#endif

#define MY_CLASS EFL_EXE_CLASS

typedef struct _Efl_Exe_Data Efl_Exe_Data;

struct _Efl_Exe_Data
{
   Efl_Core_Env *env;
   int exit_signal;
   Efl_Exe_Flags flags;
#ifdef _WIN32
   struct {
      Eo *in_handler, *out_handler;
      Eina_Bool can_read : 1;
      Eina_Bool eos_read : 1;
      Eina_Bool can_write : 1;
   } fd;
#else
   Eina_Promise *promise;
   Eo *exit_handler;
   pid_t pid;
   struct {
      int in, out, exited_read, exited_write;
      Eo *in_handler, *out_handler;
      Eina_Bool can_read : 1;
      Eina_Bool eos_read : 1;
      Eina_Bool can_write : 1;
   } fd;
#endif
   Eina_Bool exit_called : 1;
   Eina_Bool run : 1;
};

//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#else
static const signed char primap[EFL_TASK_PRIORITY_ULTRA + 1] =
{
   10, // EFL_TASK_PRIORITY_NORMAL
   19, // EFL_TASK_PRIORITY_BACKGROUND
   15, // EFL_TASK_PRIORITY_LOW
    5, // EFL_TASK_PRIORITY_HIGH
    0  // EFL_TASK_PRIORITY_ULTRA
};

static void
_close_fds(Efl_Exe_Data *pd)
{
   if (pd->fd.in >= 0) close(pd->fd.in);
   if (pd->fd.out >= 0) close(pd->fd.out);
   if (pd->fd.exited_read >= 0) close(pd->fd.exited_read);
   if (pd->fd.exited_write >= 0) close(pd->fd.exited_write);
   pd->fd.in = -1;
   pd->fd.out = -1;
   pd->fd.exited_read = -1;
   pd->fd.exited_write = -1;
}

static void
_exec(const char *cmd, Efl_Exe_Flags flags)
{
   char use_sh = 1, *buf = NULL, **args = NULL;

   // Try to avoid wrapping the exe call with /bin/sh -c.
   // We conservatively search for certain shell meta characters,
   // If we don't find them, we can call the exe directly.
   if (!strpbrk(cmd, "|&;<>()$\\\"'*?#"))
     {
        char *token, pre_command = 1;
        int num_tokens = 0, len;

        len = strlen(cmd);
        buf = alloca(len + 1);
        strcpy(buf, cmd);
        buf[len] = 0;

        token = strtok(buf, " \t\n\v");
        while (token)
          {
             if (token[0] == '~') break;
             if (pre_command)
               {
                  if (token[0] == '[') break;
                  if (strchr(token, '=')) break;
                  else pre_command = 0;
               }
             num_tokens++;
             token = strtok(NULL, " \t\n\v");
          }
        if ((!token) && (num_tokens))
          {
             int i = 0;

             len = strlen(cmd);
             buf = alloca(len + 1);
             strcpy(buf, cmd);
             buf[len] = 0;

             token = strtok(buf, " \t\n\v");
             use_sh = 0;
             args = alloca((num_tokens + 1) * sizeof(char *));
             for (i = 0; i < num_tokens; i++)
               {
                  if (token) args[i] = token;
                  token = strtok(NULL, " \t\n\v");
               }
             args[num_tokens] = NULL;
          }
     }
# ifdef HAVE_PRCTL
   if (flags & EFL_EXE_FLAGS_EXIT_WITH_PARENT)
     prctl(PR_SET_PDEATHSIG, SIGTERM);
# endif

   if (flags & EFL_EXE_FLAGS_GROUP_LEADER) setsid();
   if (use_sh) // We have to use a shell to run this.
     {
        errno = 0;
        execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
     }
   else
     { // We can run this directly.
        if (!args)
          {
             ERR("arg[0] is NULL!");
             return;
          }
        errno = 0;
        if (args[0]) execvp(args[0], args);
     }
}

static void
_exe_exit_eval(Eo *obj, Efl_Exe_Data *pd)
{
   if ((pd->fd.out == -1) && /*(pd->fd.in == -1) &&*/
       (pd->fd.exited_read == -1) && (!pd->exit_called))
     {
        pd->exit_called = EINA_TRUE;
        if (pd->promise)
          {
             Eina_Promise *p = pd->promise;
             int exit_code = efl_task_exit_code_get(obj);
             if ((exit_code != 0) && (!(efl_task_flags_get(obj) &
                                        EFL_TASK_FLAGS_NO_EXIT_CODE_ERROR)))
               {
                  Eina_Error err = exit_code + 1000000;
                  // Code  Meaning                        Example             Comments
                  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------
                  // 1     Catchall for general errors    let "var1 = 1/0"    Miscellaneous errors, such as "divide by zero" and other impermissible operations
                  // 2     Misuse of shell builtins       empty_function() {} Missing keyword or command, or permission problem (and diff return code on a failed binary file comparison).
                  // 126   Command invoked cannot execute /dev/null           Permission problem or command is not an executable
                  // 127   "command not found"            illegal_command     Possible problem with $PATH or a typo
                  // 128   Invalid argument to exit       exit 3.14159        exit takes only integer args in the range 0 - 255 (see first footnote)
                  // 128+n Fatal error signal "n"         kill -9 $PPID       $? returns 137 (128 + 9)
                  // 130   Script terminated by Control-C Ctl-C               Control-C is fatal error signal 2, (130 = 128 + 2, see above)
                  // 255*  Exit status out of range       exit -1             exit takes only integer args in the range 0 - 255
                  //
                  // According to the above table, exit codes 1 - 2,
                  // 126 - 165, and 255 [1] have special meanings, and
                  // should therefore be avoided for user-specified exit
                  // parameters. Ending a script with exit 127 would
                  // certainly cause confusion when troubleshooting (is
                  // the error code a "command not found" or a user-defined
                  // one?). However, many scripts use an exit 1 as a general
                  // bailout-upon-error. Since exit code 1 signifies so many
                  // possible errors, it is not particularly useful in
                  // debugging.
                  if      (exit_code == 1  ) err = EBADF;
                  else if (exit_code == 2  ) err = EDOM;
                  else if (exit_code == 126) err = ENOEXEC;
                  else if (exit_code == 127) err = ENOENT;
                  else if (exit_code == 128) err = EINVAL;
                  else if (exit_code == 129) err = EFAULT;
                  else if (exit_code == 130) err = EINTR;
                  else if ((exit_code >= 131) && (exit_code <= 165)) err = EFAULT;
                  eina_promise_reject(p, err);
               }
             else eina_promise_resolve(p, eina_value_int_init(exit_code));
          }
     }
}

static void
_cb_exe_exit_read(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   Efl_Exe_Data *pd = efl_data_scope_get(obj, MY_CLASS);
   Ecore_Signal_Pid_Info pinfo;

   if (!pd) return;
   if (read(pd->fd.exited_read, &pinfo, sizeof(Ecore_Signal_Pid_Info)) ==
       sizeof(Ecore_Signal_Pid_Info))
     {
        Efl_Task_Data *td = efl_data_scope_get(obj, EFL_TASK_CLASS);
        if (td)
          {
             td->exited = EINA_TRUE;
             td->exit_code = pinfo.exit_code;
             pd->exit_signal = pinfo.exit_signal;
          }
     }
   // we don't need this fd and handler anymore now we code exit status
   close(pd->fd.exited_read);
   pd->fd.exited_read = -1;
   efl_del(pd->exit_handler);
   pd->exit_handler = NULL;
   _exe_exit_eval(obj, pd);
   // XXX: autodel of object here is the sensible easy thing to do in C
   // because then you can just run exe's and not have to listen to them exit
   // and do your own del every time - they will then not leak and just
   // self-cleanup without needing a del of the obj on run. but other
   // languages don't like this, so if you dont care to listen to end/death
   // and then del/unref the obj there... always del/unref it immediately.
}

static void
_cb_exe_out(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   efl_io_reader_can_read_set(obj, EINA_TRUE);
}

static void
_cb_exe_in(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   efl_io_writer_can_write_set(obj, EINA_TRUE);
}

static Eina_Value
_run_cancel_cb(Efl_Loop_Consumer *consumer, void *data EINA_UNUSED, Eina_Error error)
{
   if (error == ECANCELED) efl_task_end(consumer);

   return eina_value_error_init(error);
}

static void
_run_clean_cb(Efl_Loop_Consumer *consumer EINA_UNUSED,
              void *data,
              const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Exe_Data *pd = data;
   pd->promise = NULL;
}

#endif

//////////////////////////////////////////////////////////////////////////


EOLIAN static void
_efl_exe_env_set(Eo *obj EINA_UNUSED, Efl_Exe_Data *pd, Efl_Core_Env *env)
{
   if (pd->env == env) return;

   if (!pd->env)
     efl_unref(pd->env);
   pd->env = env;
   if (pd->env)
     efl_ref(pd->env);
}

EOLIAN static Efl_Core_Env*
_efl_exe_env_get(const Eo *obj EINA_UNUSED, Efl_Exe_Data *pd)
{
   return pd->env;
}

EOLIAN static void
_efl_exe_signal(Eo *obj EINA_UNUSED, Efl_Exe_Data *pd, Efl_Exe_Signal sig)
{
#ifdef _WIN32
#else
   int s = 0;
   if (pd->pid == -1) return;

   switch (sig)
     {
      case EFL_EXE_SIGNAL_INT:  s = SIGINT;  break;
      case EFL_EXE_SIGNAL_QUIT: s = SIGQUIT; break;
      case EFL_EXE_SIGNAL_TERM: s = SIGTERM; break;
      case EFL_EXE_SIGNAL_KILL: s = SIGKILL; break;
      case EFL_EXE_SIGNAL_CONT: s = SIGCONT; break;
      case EFL_EXE_SIGNAL_STOP: s = SIGSTOP; break;
      case EFL_EXE_SIGNAL_HUP:  s = SIGHUP;  break;
      case EFL_EXE_SIGNAL_USR1: s = SIGUSR1; break;
      case EFL_EXE_SIGNAL_USR2: s = SIGUSR2; break;
      default: return;
     }
   kill(pd->pid, s);
#endif
}

EOLIAN static void
_efl_exe_exe_flags_set(Eo *obj EINA_UNUSED, Efl_Exe_Data *pd, Efl_Exe_Flags flags)
{
   pd->flags = flags;
}

EOLIAN static Efl_Exe_Flags
_efl_exe_exe_flags_get(const Eo *obj EINA_UNUSED, Efl_Exe_Data *pd)
{
   return pd->flags;
}

EOLIAN static void
_efl_exe_efl_task_priority_set(Eo *obj EINA_UNUSED, Efl_Exe_Data *pd, Efl_Task_Priority priority)
{
   efl_task_priority_set(efl_super(obj, MY_CLASS), priority);
#ifdef _WIN32
#else
   int p = 0;

   if (pd->pid != -1)
     {
        if ((priority >= EFL_TASK_PRIORITY_NORMAL) &&
            (priority <= EFL_TASK_PRIORITY_ULTRA))
          p = primap[priority];
     }
   setpriority(PRIO_PROCESS, pd->pid, p);
#endif
}

EOLIAN static Efl_Task_Priority
_efl_exe_efl_task_priority_get(const Eo *obj EINA_UNUSED, Efl_Exe_Data *pd)
{
   Efl_Task_Priority pri = EFL_TASK_PRIORITY_NORMAL;

#ifdef _WIN32
#else
   int p, i, dist = 0x7fffffff, d;

   if (pd->pid == -1)
     return efl_task_priority_get(efl_super(obj, MY_CLASS));
   // p is -20 -> 19
   errno = 0;
   p = getpriority(PRIO_PROCESS, pd->pid);
   if (errno != 0)
     return efl_task_priority_get(efl_super(obj, MY_CLASS));

   // find the closest matching priority in primap
   for (i = EFL_TASK_PRIORITY_NORMAL; i <= EFL_TASK_PRIORITY_ULTRA; i++)
     {
        d = primap[i] - p;
        if (d < 0) d = -d;
        if (d < dist)
          {
             pri = i;
             dist = d;
          }
     }

   Efl_Task_Data *td = efl_data_scope_get(obj, EFL_TASK_CLASS);
   if (td) td->priority = pri;
#endif
   return pri;
}

EOLIAN static Eina_Future *
_efl_exe_efl_task_run(Eo *obj, Efl_Exe_Data *pd)
{
#ifdef _WIN32
   return EINA_FALSE;
#else
   Eo *loop;
   Efl_Task_Data *tdl, *td = efl_data_scope_get(obj, EFL_TASK_CLASS);
   const char *cmd;
   int devnull;
   int pipe_stdin[2];
   int pipe_stdout[2];
   int pipe_exited[2];
   int ret;

   if (pd->run) return NULL;
   if (pd->pid != -1) return NULL;
   if (!td) return NULL;

   // get a cmdline to run
   cmd = efl_core_command_line_command_get(obj);
   if (!cmd) return NULL;

   ret = pipe(pipe_exited);
   if (EINA_UNLIKELY(ret != 0))
     {
        const int error = errno;
        ERR("pipe() failed: %s", strerror(error));
        return NULL;
     }

   pd->fd.exited_read = pipe_exited[0];
   eina_file_close_on_exec(pd->fd.exited_write,  EINA_TRUE);
   pd->fd.exited_write = pipe_exited[1];
   eina_file_close_on_exec(pd->fd.exited_read,  EINA_TRUE);

   if (td->flags & EFL_TASK_FLAGS_USE_STDIN)
     {
        ret = pipe(pipe_stdin);
        if (EINA_UNLIKELY(ret != 0))
          {
             const int error = errno;
             ERR("pipe() failed: %s", strerror(error));
             return NULL;
          }
        pd->fd.in = pipe_stdin[1];
        if (fcntl(pd->fd.in, F_SETFL, O_NONBLOCK) < 0)
          ERR("can't set pipe to NONBLOCK");
        eina_file_close_on_exec(pd->fd.in, EINA_TRUE);
        pd->fd.in_handler =
          efl_add(EFL_LOOP_HANDLER_CLASS, obj,
                  efl_loop_handler_fd_set(efl_added, pd->fd.in),
                  efl_event_callback_add
                    (efl_added, EFL_LOOP_HANDLER_EVENT_WRITE, _cb_exe_in, obj));
     }
   if (td->flags & EFL_TASK_FLAGS_USE_STDOUT)
     {
        ret = pipe(pipe_stdout);
        if (EINA_UNLIKELY(ret != 0))
          {
             const int error = errno;
             ERR("pipe() failed: %s", strerror(error));
             return NULL;
          }
        pd->fd.out = pipe_stdout[0];
        if (fcntl(pd->fd.out, F_SETFL, O_NONBLOCK) < 0)
          ERR("can't set pipe to NONBLOCK");
        eina_file_close_on_exec(pd->fd.out, EINA_TRUE);
        pd->fd.out_handler =
          efl_add(EFL_LOOP_HANDLER_CLASS, obj,
                  efl_loop_handler_fd_set(efl_added, pd->fd.out),
                  efl_event_callback_add
                    (efl_added, EFL_LOOP_HANDLER_EVENT_READ, _cb_exe_out, obj),
                  efl_loop_handler_active_set
                    (efl_added, EFL_LOOP_HANDLER_FLAGS_READ));
     }

   _ecore_signal_pid_lock();
   pd->pid = fork();
   if (pd->pid != 0)
     {
        // parent process is here inside this if block
        if (td->flags & EFL_TASK_FLAGS_USE_STDIN)  close(pipe_stdin[0]);
        if (td->flags & EFL_TASK_FLAGS_USE_STDOUT) close(pipe_stdout[1]);
        // fork failed... close up and clean and release locks
        if (pd->pid == -1)
          {
             _close_fds(pd);
             _ecore_signal_pid_unlock();
             return NULL;
          }
        // register this pid in the core sigchild/pid exit code watcher
        _ecore_signal_pid_register(pd->pid, pd->fd.exited_write);
        pd->exit_handler =
          efl_add(EFL_LOOP_HANDLER_CLASS, obj,
                  efl_loop_handler_fd_set(efl_added, pd->fd.exited_read),
                  efl_event_callback_add(efl_added,
                                         EFL_LOOP_HANDLER_EVENT_READ,
                                         _cb_exe_exit_read, obj),
                  efl_loop_handler_active_set(efl_added,
                                              EFL_LOOP_HANDLER_FLAGS_READ));
        _ecore_signal_pid_unlock();
        pd->run = EINA_TRUE;
        pd->promise = efl_loop_promise_new(obj);
        return efl_future_then(obj, eina_future_new(pd->promise),
                               .data = pd,
                               .error = _run_cancel_cb,
                               .free = _run_clean_cb);
     }
   // this code is in the child here, and is temporary setup until we
   // exec() the child to replace everything.

   if (td->flags & EFL_TASK_FLAGS_USE_STDIN)  close(pipe_stdin[1]);
   if (td->flags & EFL_TASK_FLAGS_USE_STDOUT) close(pipe_stdout[0]);
   // set priority of self
   if ((td->priority >= EFL_TASK_PRIORITY_NORMAL) &&
       (td->priority <= EFL_TASK_PRIORITY_ULTRA))
     setpriority(PRIO_PROCESS, 0, primap[td->priority]);

   // if we want to hide or use any of the stdio, close the fd's
   if ((td->flags & EFL_TASK_FLAGS_USE_STDIN) ||
       (pd->flags & EFL_EXE_FLAGS_HIDE_IO))
     close(STDIN_FILENO);
   if ((td->flags & EFL_TASK_FLAGS_USE_STDOUT) ||
       (pd->flags & EFL_EXE_FLAGS_HIDE_IO))
     close(STDOUT_FILENO);
   if ((pd->flags & EFL_EXE_FLAGS_HIDE_IO))
     close(STDERR_FILENO);

   if (!(td->flags & EFL_TASK_FLAGS_USE_STDIN) &&
       (pd->flags & EFL_EXE_FLAGS_HIDE_IO))
     {
        // hide stdin
        devnull = open("/dev/null", O_RDONLY);
        dup2(devnull, STDIN_FILENO);
        close(devnull);
     }
   else if ((td->flags & EFL_TASK_FLAGS_USE_STDIN))
     {
        // hook up stdin to the pipe going to the parent
        dup2(pipe_stdin[0], STDIN_FILENO);
        close(pipe_stdin[0]);
     }

   if (!(td->flags & EFL_TASK_FLAGS_USE_STDOUT) &&
       (pd->flags & EFL_EXE_FLAGS_HIDE_IO))
     {
        // hide stdout
        devnull = open("/dev/null", O_WRONLY);
        dup2(devnull, STDOUT_FILENO);
        close(devnull);
     }
   else if ((td->flags & EFL_TASK_FLAGS_USE_STDOUT))
     {
        // hook up stdout to the pipe going to the parent
        dup2(pipe_stdout[1], STDOUT_FILENO);
        close(pipe_stdout[1]);
     }

   if ((pd->flags & EFL_EXE_FLAGS_HIDE_IO))
     {
        // hide stderr
        devnull = open("/dev/null", O_WRONLY);
        dup2(devnull, STDERR_FILENO);
        close(devnull);
     }

   if (!(loop = efl_provider_find(obj, EFL_LOOP_CLASS))) exit(1);

   if (!(tdl = efl_data_scope_get(loop, EFL_TASK_CLASS))) exit(1);

   // clear systemd notify socket... only relevant for systemd world,
   // otherwise shouldn't be trouble
   putenv("NOTIFY_SOCKET=");

   // actually setenv the env object (clear what was there before so it is
   // the only env there)
   if (pd->env)
     {
        Eina_Iterator *itr;
        const char *key;

# ifdef HAVE_CLEARENV
        clearenv();
# else
        environ = NULL;
# endif
        itr = efl_core_env_content_get(pd->env);

        EINA_ITERATOR_FOREACH(itr, key)
          {
             setenv(key, efl_core_env_get(pd->env, key) , 1);
          }
       efl_unref(pd->env);
       pd->env = NULL;
     }

   // actually execute!
   _exec(cmd, pd->flags);
   // we couldn't exec... uh oh. HAAAAAAAALP!
   if ((errno == EACCES)  || (errno == EINVAL) || (errno == ELOOP) ||
       (errno == ENOEXEC) || (errno == ENOMEM))
     exit(126);
   exit(127);
   return NULL;
#endif
}

EOLIAN static void
_efl_exe_efl_task_end(Eo *obj EINA_UNUSED, Efl_Exe_Data *pd)
{
#ifdef _WIN32
#else
   if (pd->pid == -1) return;
   kill(pd->pid, SIGINT);
#endif
}

EOLIAN static int
_efl_exe_exit_signal_get(const Eo *obj EINA_UNUSED, Efl_Exe_Data *pd)
{
   return pd->exit_signal;
}

EOLIAN static Efl_Object *
_efl_exe_efl_object_constructor(Eo *obj, Efl_Exe_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
#ifdef _WIN32
#else
   pd->pid = -1;
   pd->fd.in = -1;
   pd->fd.out = -1;
   pd->fd.exited_read = -1;
#endif
   pd->fd.can_write = EINA_TRUE;
   pd->flags = EFL_EXE_FLAGS_EXIT_WITH_PARENT;
   pd->exit_signal = -1;
   return obj;
}

EOLIAN static void
_efl_exe_efl_object_destructor(Eo *obj, Efl_Exe_Data *pd)
{
#ifdef _WIN32
#else
   if (pd->promise)
     ERR("Exe being destroyed while child has not exited yet.");
   if (pd->fd.exited_read >= 0)
     {
        _ecore_signal_pid_lock();
        _ecore_signal_pid_unregister(pd->pid, pd->fd.exited_read);
        _ecore_signal_pid_unlock();
        close(pd->fd.exited_read);
        pd->fd.exited_read = -1;
        efl_del(pd->exit_handler);
        pd->exit_handler = NULL;
     }
   if (pd->fd.in_handler) efl_del(pd->fd.in_handler);
   if (pd->fd.out_handler) efl_del(pd->fd.out_handler);
   pd->fd.in_handler = NULL;
   pd->fd.out_handler = NULL;
   _close_fds(pd);
#endif
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_exe_efl_io_closer_close(Eo *obj, Efl_Exe_Data *pd)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(obj), EBADF);
   efl_io_writer_can_write_set(obj, EINA_FALSE);
   efl_io_reader_can_read_set(obj, EINA_FALSE);
   efl_io_reader_eos_set(obj, EINA_TRUE);
#ifdef _WIN32
#else
   if (pd->fd.in >= 0) close(pd->fd.in);
   if (pd->fd.out >= 0) close(pd->fd.out);
   if (pd->fd.exited_read >= 0) close(pd->fd.exited_read);
   if (pd->fd.in_handler) efl_del(pd->fd.in_handler);
   if (pd->fd.out_handler) efl_del(pd->fd.out_handler);
   pd->fd.in = -1;
   pd->fd.out = -1;
   pd->fd.exited_read = -1;
   pd->fd.in_handler = NULL;
   pd->fd.out_handler = NULL;
#endif
   return 0;
}

EOLIAN static Eina_Bool
_efl_exe_efl_io_closer_closed_get(const Eo *obj EINA_UNUSED, Efl_Exe_Data *pd)
{
#ifdef _WIN32
   return EINA_FALSE;
#else
   if ((pd->fd.in == -1) && (pd->fd.out == -1)) return EINA_TRUE;
#endif
   return EINA_FALSE;
}

EOLIAN static Eina_Error
_efl_exe_efl_io_reader_read(Eo *obj, Efl_Exe_Data *pd, Eina_Rw_Slice *rw_slice)
{
#ifdef _WIN32
   return EINVAL;
#else
   ssize_t r;

   errno = 0;
   if (pd->fd.out == -1) goto err;

   do
     {
        errno = 0;
        r = read(pd->fd.out, rw_slice->mem, rw_slice->len);
        if (r == -1)
          {
             if (errno == EINTR) continue;
             goto err;
          }
     }
   while (r == -1);

   rw_slice->len = r;
   if (r == 0)
     {
        efl_io_reader_can_read_set(obj, EINA_FALSE);
        efl_io_reader_eos_set(obj, EINA_TRUE);
        close(pd->fd.out);
        pd->fd.out = -1;
        efl_del(pd->fd.out_handler);
        pd->fd.out_handler = NULL;
        _exe_exit_eval(obj, pd);
        return EPIPE;
     }
   return 0;
err:
   if ((pd->fd.out != -1) && (errno != EAGAIN))
     {
        close(pd->fd.out);
        pd->fd.out = -1;
        efl_del(pd->fd.out_handler);
        pd->fd.out_handler = NULL;
     }
   rw_slice->len = 0;
   rw_slice->mem = NULL;
   efl_io_reader_can_read_set(obj, EINA_FALSE);
   _exe_exit_eval(obj, pd);
   return EINVAL;
#endif
}

EOLIAN static void
_efl_exe_efl_io_reader_can_read_set(Eo *obj, Efl_Exe_Data *pd, Eina_Bool can_read)
{
   Eina_Bool old = efl_io_reader_can_read_get(obj);
   if (old == can_read) return;
   pd->fd.can_read = can_read;
   if (!pd->fd.out_handler) return;
   if (can_read)
     efl_loop_handler_active_set(pd->fd.out_handler, 0);
   else
     efl_loop_handler_active_set(pd->fd.out_handler,
                                 EFL_LOOP_HANDLER_FLAGS_READ);
   efl_event_callback_call(obj, EFL_IO_READER_EVENT_CAN_READ_CHANGED, (void*) (uintptr_t) can_read);
}

EOLIAN static Eina_Bool
_efl_exe_efl_io_reader_can_read_get(const Eo *obj EINA_UNUSED, Efl_Exe_Data *pd)
{
   return pd->fd.can_read;
}

EOLIAN static void
_efl_exe_efl_io_reader_eos_set(Eo *obj, Efl_Exe_Data *pd, Eina_Bool is_eos)
{
   Eina_Bool old = efl_io_reader_eos_get(obj);
   if (old == is_eos) return;

   pd->fd.eos_read = is_eos;
   if (!is_eos) return;
   if (pd->fd.out_handler)
     efl_loop_handler_active_set(pd->fd.out_handler, 0);
   efl_event_callback_call(obj, EFL_IO_READER_EVENT_EOS, NULL);
}

EOLIAN static Eina_Bool
_efl_exe_efl_io_reader_eos_get(const Eo *obj EINA_UNUSED, Efl_Exe_Data *pd)
{
   return pd->fd.eos_read;
}

EOLIAN static Eina_Error
_efl_exe_efl_io_writer_write(Eo *obj, Efl_Exe_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
{
#ifdef _WIN32
   return EINVAL;
#else
   ssize_t r;

   errno = 0;
   if (pd->fd.in == -1) goto err;

   do
     {
        errno = 0;
        r = write(pd->fd.in, slice->mem, slice->len);
        if (r == -1)
          {
             if (errno == EINTR) continue;
             goto err;
          }
     }
   while (r == -1);

   if (remaining)
     {
        remaining->len = slice->len - r;
        remaining->bytes = slice->bytes + r;
     }
   slice->len = r;

   if ((slice) && (slice->len > 0))
     efl_io_writer_can_write_set(obj, EINA_FALSE);
   if (r == 0)
     {
        close(pd->fd.in);
        pd->fd.in = -1;
        efl_del(pd->fd.in_handler);
        pd->fd.in_handler = NULL;
        _exe_exit_eval(obj, pd);
        return EPIPE;
     }
   return 0;
err:
   if ((pd->fd.in != -1) && (errno != EAGAIN))
     {
        close(pd->fd.in);
        pd->fd.in = -1;
        efl_del(pd->fd.in_handler);
        pd->fd.in_handler = NULL;
     }
   if (remaining) *remaining = *slice;
   slice->len = 0;
   slice->mem = NULL;
   efl_io_writer_can_write_set(obj, EINA_FALSE);
   _exe_exit_eval(obj, pd);
   return EINVAL;
#endif
}

EOLIAN static void
_efl_exe_efl_io_writer_can_write_set(Eo *obj, Efl_Exe_Data *pd, Eina_Bool can_write)
{
   Eina_Bool old = efl_io_writer_can_write_get(obj);
   if (old == can_write) return;
   pd->fd.can_write = can_write;
   if (can_write)
     efl_loop_handler_active_set(pd->fd.in_handler, 0);
   else
     efl_loop_handler_active_set(pd->fd.in_handler,
                                 EFL_LOOP_HANDLER_FLAGS_WRITE);
   efl_event_callback_call(obj, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, (void*) (uintptr_t) can_write);
}

EOLIAN static Eina_Bool
_efl_exe_efl_io_writer_can_write_get(const Eo *obj EINA_UNUSED, Efl_Exe_Data *pd)
{
   return pd->fd.can_write;
}

//////////////////////////////////////////////////////////////////////////

#include "efl_exe.eo.c"
