#include "ecore_private.h"
#include "Ecore.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

static void _ecore_signal_callback_set(int sig, void (*func)(int sig));
static void _ecore_signal_callback_ignore(int sig);
static void _ecore_signal_callback_sigchld(int sig);
static void _ecore_signal_callback_sigusr1(int sig);
static void _ecore_signal_callback_sigusr2(int sig);
static void _ecore_signal_callback_sighup(int sig);
static void _ecore_signal_callback_sigquit(int sig);
static void _ecore_signal_callback_sigint(int sig);
static void _ecore_signal_callback_sigterm(int sig);
static void _ecore_signal_callback_sigpwr(int sig);

static int sigchld_count = 0;
static int sigusr1_count = 0;
static int sigusr2_count = 0;
static int sighup_count = 0;
static int sigquit_count = 0;
static int sigint_count = 0;
static int sigterm_count = 0;
static int sigpwr_count = 0;

static int sig_count = 0;

void
_ecore_signal_shutdown(void)
{
   _ecore_signal_callback_set(SIGPIPE, SIG_DFL);
   _ecore_signal_callback_set(SIGALRM, SIG_DFL);
   _ecore_signal_callback_set(SIGCHLD, SIG_DFL);
   _ecore_signal_callback_set(SIGUSR1, SIG_DFL);
   _ecore_signal_callback_set(SIGUSR2, SIG_DFL);
   _ecore_signal_callback_set(SIGHUP,  SIG_DFL);
   _ecore_signal_callback_set(SIGQUIT, SIG_DFL);
   _ecore_signal_callback_set(SIGINT,  SIG_DFL);
   _ecore_signal_callback_set(SIGTERM, SIG_DFL);
   #ifdef SIGPWR
   _ecore_signal_callback_set(SIGPWR,  SIG_DFL);
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
}

void
_ecore_signal_init(void)
{
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
}

int
_ecore_signal_count_get(void)
{
   return sig_count;
}

void
_ecore_signal_call(void)
{
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
	     _ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, e,
			      _ecore_event_signal_exit_free, NULL);
	  }
	sigterm_count--;
	sig_count--;
     }
   while (sigpwr_count > 0)
     {
	Ecore_Event_Signal_Power *e;
	
	e = _ecore_event_signal_power_new();
	if (e)
	  {
	     _ecore_event_add(ECORE_EVENT_SIGNAL_POWER, e,
			      _ecore_event_signal_power_free, NULL);
	  }
	sigpwr_count--;
	sig_count--;
     }
}

static void
_ecore_signal_callback_set(int sig, void (*func)(int sig))
{
   struct sigaction  sa;

   sa.sa_handler = func;
   sa.sa_flags = SA_RESTART;
   sigemptyset(&sa.sa_mask);
   sigaction(sig, &sa, NULL);   
}

static void
_ecore_signal_callback_ignore(int sig)
{
}

static void
_ecore_signal_callback_sigchld(int sig)
{
   sigchld_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigusr1(int sig)
{
   sigusr1_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigusr2(int sig)
{
   sigusr2_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sighup(int sig)
{
   sighup_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigquit(int sig)
{
   sigquit_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigint(int sig)
{
   sigint_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigterm(int sig)
{
   sigterm_count++;
   sig_count++;
}

static void
_ecore_signal_callback_sigpwr(int sig)
{
   sigpwr_count++;
   sig_count++;
}
