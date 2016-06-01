#include <unistd.h>
#include <signal.h>

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
