#include "Ecore.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

static void         ecore_event_signal_free(void *event);
static void         ecore_event_signal_handle_sigchld(int num);
static void         ecore_event_signal_handle_sigusr1(int num);
static void         ecore_event_signal_handle_sigusr2(int num);
static void         ecore_event_signal_handle_sighup(int num);
static void         ecore_event_signal_handle_sigpipe(int num);
static void         ecore_event_signal_handle_sigsegv(int num);
static void         ecore_event_signal_handle_sigfpe(int num);
static void         ecore_event_signal_handle_sigill(int num);
static void         ecore_event_signal_handle_sigbus(int num);

#ifdef HAVE_SIGSTKFLT
static void         ecore_event_signal_handle_sigstkflt(int num);
#endif
#ifdef HAVE_SIGPWR
static void         ecore_event_signal_handle_sigpwr(int num);
#endif
static void         ecore_event_signal_handle_sigchld(int num);
static void         ecore_event_signal_handle_all(pid_t pid);

static int          signal_chld_count = 0;
static int          signal_usr1_count = 0;
static int          signal_usr2_count = 0;
static int          signal_hup_count = 0;

/* freeing stuff */
static void
ecore_event_signal_free(void *event)
{
  FREE(event);
}

/* signal handlers we can return from and add to signal recieved counts */
static void
ecore_event_signal_handle_sigchld(int num)
{
  signal_chld_count++;
  return;
  num = 0;
}

static void
ecore_event_signal_handle_sigusr1(int num)
{
  signal_usr1_count++;
  return;
  num = 0;
}

static void
ecore_event_signal_handle_sigusr2(int num)
{
  signal_usr2_count++;
  return;
  num = 0;
}

static void
ecore_event_signal_handle_sighup(int num)
{
  signal_hup_count++;
  return;
  num = 0;
}

/* signals to ignore */
static void
ecore_event_signal_handle_sigpipe(int num)
{
  return;
  num = 0;
}

/* signal handlers we cant return from - so handle here */
static void
ecore_event_signal_handle_sigsegv(int num)
{
  for (;;)
    {
      fprintf(stderr, "EEEEEEEEK SEGV - waiting 10 seconds\n");
      sleep(10);
    }
  /* EEK - can't return - bad */
  abort();
  num = 0;
}

static void
ecore_event_signal_handle_sigfpe(int num)
{
  /* EEK - can't return - bad */
  abort();
  num = 0;
}

static void
ecore_event_signal_handle_sigill(int num)
{
  /* EEK - can't return - bad */
  abort();
  num = 0;
}

static void
ecore_event_signal_handle_sigbus(int num)
{
  /* EEK - can't return - bad */
  abort();
  num = 0;
}

#ifdef HAVE_SIGSTKFLT
static void
ecore_event_signal_handle_sigstkflt(int num)
{
  /* EEK - can't return - bad */
  abort();
  return;
  num = 0;
}
#endif

static void
ecore_event_signal_handle_sigint(int num)
{
  exit(0);
  return;
  num = 0;
}

static void
ecore_event_signal_handle_sigquit(int num)
{
  exit(0);
  return;
  num = 0;
}

static void
ecore_event_signal_handle_sigabrt(int num)
{
  abort();
  return;
  num = 0;
}

static void
ecore_event_signal_handle_sigalrm(int num)
{
  return;
  num = 0;
}

static void
ecore_event_signal_handle_sigterm(int num)
{
  exit(0);
  return;
  num = 0;
}

#ifdef HAVE_SIGPWR
static void
ecore_event_signal_handle_sigpwr(int num)
{
  exit(0);
  return;
  num = 0;
}
#endif

static void
ecore_event_signal_handle_all(pid_t pid_pass)
{
  int                 status;
  pid_t               pid;

  if (signal_chld_count > 0)
    {
      while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
	  if (WIFEXITED(status))
	    {
	      Ecore_Event_Child  *e;
	      int                 code;

	      code = WEXITSTATUS(status);
	      e = NEW(Ecore_Event_Child, 1);
	      e->pid = pid;
	      e->exit_code = code;
	      ecore_add_event(ECORE_EVENT_CHILD, e, ecore_event_signal_free);
	    }
	}
      signal_chld_count = 0;
    }
  while (signal_usr1_count > 0)
    {
      Ecore_Event_User   *e;

      e = NEW(Ecore_Event_User, 1);
      e->num = 0;
      e->hup = 0;
      ecore_add_event(ECORE_EVENT_USER, e, ecore_event_signal_free);
      signal_usr1_count--;
    }
  while (signal_hup_count > 0)
    {
      Ecore_Event_User   *e;

      e = NEW(Ecore_Event_User, 1);
      e->num = 0;
      e->hup = 1;
      ecore_add_event(ECORE_EVENT_USER, e, ecore_event_signal_free);
      signal_hup_count--;
    }
  return;
  pid_pass = 0;
}

int
ecore_event_signal_events_pending(void)
{
  return (signal_chld_count + signal_usr1_count + signal_hup_count);
}

void
ecore_event_signal_init(void)
{
  struct sigaction    sa;

  ecore_add_event_pid(0, ecore_event_signal_handle_all);

  sa.sa_handler = ecore_event_signal_handle_sigchld;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sigusr1;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGUSR1, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sigusr2;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGUSR2, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sighup;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGHUP, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sigpipe;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGPIPE, &sa, (struct sigaction *)0);
/*
   sa.sa_handler = ecore_event_signal_handle_sigsegv;
   sa.sa_flags = SA_RESTART;
   sigemptyset(&sa.sa_mask);
   sigaction(SIGSEGV, &sa, (struct sigaction *)0);
*/
  sa.sa_handler = ecore_event_signal_handle_sigfpe;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGFPE, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sigill;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGILL, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sigbus;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGBUS, &sa, (struct sigaction *)0);
#ifdef HAVE_SIGSTKFLT
  sa.sa_handler = ecore_event_signal_handle_sigstkflt;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGSTKFLT, &sa, (struct sigaction *)0);
#endif
  sa.sa_handler = ecore_event_signal_handle_sigint;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sigquit;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGQUIT, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sigabrt;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGABRT, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sigalrm;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGALRM, &sa, (struct sigaction *)0);

  sa.sa_handler = ecore_event_signal_handle_sigterm;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGTERM, &sa, (struct sigaction *)0);
#ifdef HAVE_SIGPWR
  sa.sa_handler = ecore_event_signal_handle_sigpwr;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGPWR, &sa, (struct sigaction *)0);
#endif
}
