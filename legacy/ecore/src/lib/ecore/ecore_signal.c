#include "ecore_private.h"
#include "Ecore.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

typedef void (*Signal_Handler)(int sig, siginfo_t *si, void *foo);

static void _ecore_signal_callback_set(int sig, Signal_Handler func); 
static void _ecore_signal_callback_ignore(int sig, siginfo_t *si, void *foo);
static void _ecore_signal_callback_sigchld(int sig, siginfo_t *si, void *foo);
static void _ecore_signal_callback_sigusr1(int sig, siginfo_t *si, void *foo);
static void _ecore_signal_callback_sigusr2(int sig, siginfo_t *si, void *foo);
static void _ecore_signal_callback_sighup(int sig, siginfo_t *si, void *foo);
static void _ecore_signal_callback_sigquit(int sig, siginfo_t *si, void *foo);
static void _ecore_signal_callback_sigint(int sig, siginfo_t *si, void *foo);
static void _ecore_signal_callback_sigterm(int sig, siginfo_t *si, void *foo);
static void _ecore_signal_callback_sigpwr(int sig, siginfo_t *si, void *foo);

#ifdef SIGRTMIN
static void _ecore_signal_callback_sigrt(int sig, siginfo_t *si, void *foo);
#endif

static volatile sig_atomic_t sig_count = 0;
static volatile sig_atomic_t sigchld_count = 0;
static volatile sig_atomic_t sigusr1_count = 0;
static volatile sig_atomic_t sigusr2_count = 0;
static volatile sig_atomic_t sighup_count = 0;
static volatile sig_atomic_t sigquit_count = 0;
static volatile sig_atomic_t sigint_count = 0;
static volatile sig_atomic_t sigterm_count = 0;

static volatile siginfo_t sigchld_info = {0};
static volatile siginfo_t sigusr1_info = {0};
static volatile siginfo_t sigusr2_info = {0};
static volatile siginfo_t sighup_info = {0};
static volatile siginfo_t sigquit_info = {0};
static volatile siginfo_t sigint_info = {0};
static volatile siginfo_t sigterm_info = {0};

#ifdef SIGPWR
static volatile sig_atomic_t sigpwr_count = 0;
static volatile siginfo_t sigpwr_info = {0};
#endif

#ifdef SIGRTMIN
static volatile sig_atomic_t *sigrt_count = NULL;
static volatile siginfo_t *sigrt_info = NULL;
#endif

void
_ecore_signal_shutdown(void)
{
#ifdef SIGRTMIN
   int i, num = SIGRTMAX - SIGRTMIN;
#endif

   _ecore_signal_callback_set(SIGPIPE, (Signal_Handler) SIG_DFL);
   _ecore_signal_callback_set(SIGALRM, (Signal_Handler) SIG_DFL);
   _ecore_signal_callback_set(SIGCHLD, (Signal_Handler) SIG_DFL);
   _ecore_signal_callback_set(SIGUSR1, (Signal_Handler) SIG_DFL);
   _ecore_signal_callback_set(SIGUSR2, (Signal_Handler) SIG_DFL);
   _ecore_signal_callback_set(SIGHUP,  (Signal_Handler) SIG_DFL);
   _ecore_signal_callback_set(SIGQUIT, (Signal_Handler) SIG_DFL);
   _ecore_signal_callback_set(SIGINT,  (Signal_Handler) SIG_DFL);
   _ecore_signal_callback_set(SIGTERM, (Signal_Handler) SIG_DFL);
#ifdef SIGPWR
   _ecore_signal_callback_set(SIGPWR, (Signal_Handler) SIG_DFL);
#endif
   sigchld_count = 0;
   sigusr1_count = 0;
   sigusr2_count = 0;
   sighup_count = 0;
   sigquit_count = 0;
   sigint_count = 0;
   sigterm_count = 0;
   sigpwr_count = 0;
   sig_count = 0;

#ifdef SIGRTMIN
   for (i = 0; i < num; i++) {
      _ecore_signal_callback_set(SIGRTMIN + i, (Signal_Handler) SIG_DFL);
      sigrt_count[i] = 0;
   }

   if (sigrt_count) {
      free((sig_atomic_t *) sigrt_count);
      sigrt_count = NULL;
   }

   if (sigrt_info) {
      free((siginfo_t *) sigrt_info);
      sigrt_info = NULL;
   }
#endif
}

void
_ecore_signal_init(void)
{
#ifdef SIGRTMIN
   int i, num = SIGRTMAX - SIGRTMIN;
#endif

   _ecore_signal_callback_set(SIGPIPE, _ecore_signal_callback_ignore);
   _ecore_signal_callback_set(SIGALRM, _ecore_signal_callback_ignore);
   _ecore_signal_callback_set(SIGCHLD, _ecore_signal_callback_sigchld);
   _ecore_signal_callback_set(SIGUSR1, _ecore_signal_callback_sigusr1);
   _ecore_signal_callback_set(SIGUSR2, _ecore_signal_callback_sigusr2);
   _ecore_signal_callback_set(SIGHUP,  _ecore_signal_callback_sighup);
   _ecore_signal_callback_set(SIGQUIT, _ecore_signal_callback_sigquit);
   _ecore_signal_callback_set(SIGINT,  _ecore_signal_callback_sigint);
   _ecore_signal_callback_set(SIGTERM, _ecore_signal_callback_sigterm);
#ifdef SIGPWR
   _ecore_signal_callback_set(SIGPWR,  _ecore_signal_callback_sigpwr);
#endif

#ifdef SIGRTMIN
   sigrt_count = calloc(1, sizeof(sig_atomic_t) * num);
   assert(sigrt_count);

   sigrt_info = calloc(1, sizeof(siginfo_t) * num);
   assert(sigrt_info);

   for (i = 0; i < num; i++)
      _ecore_signal_callback_set(SIGRTMIN + i, _ecore_signal_callback_sigrt);
#endif
}

int
_ecore_signal_count_get(void)
{
   return sig_count;
}

void
_ecore_signal_call(void)
{
#ifdef SIGRTMIN
   int i, num = SIGRTMAX - SIGRTMIN;
#endif

   while (sigchld_count > 0)
     {
	pid_t pid;
	int status;
	
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	  {
	     Ecore_Event_Exe_Exit *e;
	     
	     e = _ecore_event_exe_exit_new();
	     if (e)
	       {
		  if (WIFEXITED(status))
		    {
		       e->exit_code = WEXITSTATUS(status);
		       e->exited = 1;
		    }
		  else if (WIFSIGNALED(status))
		    {
		       e->exit_signal = WTERMSIG(status);
		       e->signalled = 1;
		    }
		  e->pid = pid;
		  e->exe = _ecore_exe_find(pid);
          e->data = sigchld_info;
		  _ecore_event_add(ECORE_EVENT_EXE_EXIT, e, 
				   _ecore_event_exe_exit_free, NULL);
	       }
	  }
	sigchld_count--;
	sig_count--;
     }
   while (sigusr1_count > 0)
     {
	Ecore_Event_Signal_User *e;
	
	e = _ecore_event_signal_user_new();
	if (e)
	  {
	     e->number = 1;
	     e->data = sigusr1_info;	 
	     _ecore_event_add(ECORE_EVENT_SIGNAL_USER, e,
			      _ecore_event_signal_user_free, NULL);
	  }
	sigusr1_count--;
	sig_count--;
     }
   while (sigusr2_count > 0)
     {
	Ecore_Event_Signal_User *e;
	
	e = _ecore_event_signal_user_new();
	if (e)
	  {
	     e->number = 2;
	     e->data = sigusr2_info;	 
	     _ecore_event_add(ECORE_EVENT_SIGNAL_USER, e,
			      _ecore_event_signal_user_free, NULL);
	  }
	sigusr2_count--;
	sig_count--;
     }
   while (sighup_count > 0)
     {
	Ecore_Event_Signal_Hup *e;
	
	e = _ecore_event_signal_hup_new();
	if (e)
	  {
	    e->data = sighup_info;
	     _ecore_event_add(ECORE_EVENT_SIGNAL_HUP, e,
			      _ecore_event_signal_hup_free, NULL);
	  }
	sighup_count--;
	sig_count--;
     }
   while (sigquit_count > 0)
     {
	Ecore_Event_Signal_Exit *e;
	
	e = _ecore_event_signal_exit_new();
	if (e)
	  {
	     e->quit = 1;
	     e->data = sigquit_info;
	     _ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, e,
			      _ecore_event_signal_exit_free, NULL);
	  }
	sigquit_count--;
	sig_count--;
     }
   while (sigint_count > 0)
     {
	Ecore_Event_Signal_Exit *e;
	
	e = _ecore_event_signal_exit_new();
	if (e)
	  {
	     e->interrupt = 1;
	     e->data = sigint_info;
	     _ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, e,
			      _ecore_event_signal_exit_free, NULL);
	  }
	sigint_count--;
	sig_count--;
     }
   while (sigterm_count > 0)
     {
	Ecore_Event_Signal_Exit *e;
	
	e = _ecore_event_signal_exit_new();
	if (e)
	  {
	     e->terminate = 1;
	     e->data = sigterm_info;
	     _ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, e,
			      _ecore_event_signal_exit_free, NULL);
	  }
	sigterm_count--;
	sig_count--;
     }
#ifdef SIGPWR
   while (sigpwr_count > 0)
     {
	Ecore_Event_Signal_Power *e;
	
	e = _ecore_event_signal_power_new();
	if (e)
	  {
	     e->data = sigpwr_info;
	     _ecore_event_add(ECORE_EVENT_SIGNAL_POWER, e,
			      _ecore_event_signal_power_free, NULL);
	  }
	sigpwr_count--;
	sig_count--;
     }
#endif

#ifdef SIGRTMIN
   for (i = 0; i < num; i++)
      while (sigrt_count[i] > 0) {
         Ecore_Event_Signal_Realtime *e;

         if ((e = _ecore_event_signal_realtime_new())) {
            e->data = sigrt_info[i];
            _ecore_event_add(ECORE_EVENT_SIGNAL_REALTIME, e,
                             _ecore_event_signal_realtime_free, NULL);
         }

         sigrt_count[i]--;
         sig_count--;
      }
#endif
}

static void
_ecore_signal_callback_set(int sig, Signal_Handler func)
{
   struct sigaction  sa;

   sa.sa_sigaction = func;
   sa.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&sa.sa_mask);
   sigaction(sig, &sa, NULL); 
}

static void
_ecore_signal_callback_ignore(int sig, siginfo_t *si, void *foo)
{
}

static void
_ecore_signal_callback_sigchld(int sig, siginfo_t *si, void *foo)
{
   sigchld_info = *si;
   sigchld_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigusr1(int sig, siginfo_t *si, void *foo)
{
   sigusr1_info = *si;
   sigusr1_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigusr2(int sig, siginfo_t *si, void *foo)
{
   sigusr2_info = *si;
   sigusr2_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sighup(int sig, siginfo_t *si, void *foo)
{
   sighup_info = *si;
   sighup_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigquit(int sig, siginfo_t *si, void *foo)
{
   sigquit_info = *si;
   sigquit_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigint(int sig, siginfo_t *si, void *foo)
{
   sigint_info = *si;
   sigint_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigterm(int sig, siginfo_t *si, void *foo)
{
   sigterm_info = *si;
   sigterm_count++;
   sig_count++;
}

#ifdef SIGPWR
static void
_ecore_signal_callback_sigpwr(int sig, siginfo_t *si, void *foo)
{
   sigpwr_info = *si;
   sigpwr_count++;
   sig_count++;
}
#endif

#ifdef SIGRTMIN
static void
_ecore_signal_callback_sigrt(int sig, siginfo_t *si, void *foo)
{
   sigrt_info[sig - SIGRTMIN] = *si;
   sigrt_count[sig - SIGRTMIN]++;
   sig_count++;
}
#endif
