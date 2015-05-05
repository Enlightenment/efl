#include "eina_debug.h"

#ifdef EINA_HAVE_DEBUG

void
_eina_debug_monitor_service_greet(void)
{
   const char *hello = "HELO";
   unsigned int version = 1;
   unsigned int msize = 4 + 4 + 4;
   unsigned int pid = getpid();
   unsigned char buf[16];
   memcpy(buf +  0, &msize, 4);
   memcpy(buf +  4, hello, 4);
   memcpy(buf +  8, &version, 4);
   memcpy(buf + 12, &pid, 4);
   write(_eina_debug_monitor_service_fd, buf, sizeof(buf));
}
#endif
