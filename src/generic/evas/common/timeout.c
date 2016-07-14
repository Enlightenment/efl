#ifdef _WIN32
# include <stdio.h>
# include <windows.h>
# include <process.h>

unsigned int
_timeout(void *arg)
{
   int s (int)arg;
   Sleep(s * 1000);
   _Exit(-1);
   _endthreadex(0);
   return 0;
}

void
timeout_init(int seconds)
{
   unsigned int id;
   _beginthreadex( NULL, 0, _timeout, (void *)seconds, 0, &id);
}
#else
# include <unistd.h>
# include <signal.h>

static void
_timeout(int val)
{
   _exit(-1);
   if (val) return;
}

void
timeout_init(int seconds)
{
   signal(SIGALRM, _timeout);
   alarm(seconds);
}
#endif
