#include <Ecore.h>
#include <unistd.h>

struct context {
     Ecore_Fd_Handler *handler;
     Ecore_Timer *timer;
};

static void
_fd_prepare_cb(void *data, Ecore_Fd_Handler *handler)
{
   printf("prepare_cb called.\n");
}

static Eina_Bool
_fd_handler_cb(void *data, Ecore_Fd_Handler *handler)
{
   struct context *ctxt = data;
   char buf[1024];
   size_t nbytes;
   int fd;

   if (ecore_main_fd_handler_active_get(handler, ECORE_FD_ERROR))
     {
	printf("An error has occurred. Stop watching this fd and quit.\n");
	ecore_main_loop_quit();
	ctxt->handler = NULL;
	return ECORE_CALLBACK_CANCEL;
     }

   fd = ecore_main_fd_handler_fd_get(handler);
   nbytes = read(fd, buf, sizeof(buf));
   if (nbytes == 0)
     {
	printf("Nothing to read, exiting...\n");
	ecore_main_loop_quit();
	ctxt->handler = NULL;
	return ECORE_CALLBACK_CANCEL;
     }
   buf[nbytes - 1] = '\0';

   printf("Read %zd bytes from input: \"%s\"\n", nbytes - 1, buf);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_timer_cb(void *data)
{
   printf("Timer expired after 5 seconds...\n");

   return ECORE_CALLBACK_RENEW;
}

int main(int argc, char **argv)
{
   struct context ctxt = {0};

   if (!ecore_init())
     {
	printf("ERROR: Cannot init Ecore!\n");
	return -1;
     }

   ctxt.handler = ecore_main_fd_handler_add(STDIN_FILENO,
					    ECORE_FD_READ | ECORE_FD_ERROR,
					    _fd_handler_cb,
					    &ctxt, NULL, NULL);
   ecore_main_fd_handler_prepare_callback_set(ctxt.handler, _fd_prepare_cb, &ctxt);
   ctxt.timer = ecore_timer_add(5, _timer_cb, &ctxt);

   printf("Starting the main loop. Type anything and hit <enter> to "
	  "activate the fd_handler callback, or CTRL+d to shutdown.\n");

   ecore_main_loop_begin();

   if (ctxt.handler)
     ecore_main_fd_handler_del(ctxt.handler);

   if (ctxt.timer)
     ecore_timer_del(ctxt.timer);

   ecore_shutdown();

   return 0;
}
