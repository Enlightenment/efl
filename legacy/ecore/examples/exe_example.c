/* Timer example.
 */

#include <Ecore.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int                 size = 0;
int                 exe_count = 0;
int                 data_count = 0;
int                 line_count = 0;
double              then = 0.0, now = 0.0;
Ecore_Exe          *exe0 = NULL;
Ecore_Exe          *exe1 = NULL;
Ecore_Exe          *exe2 = NULL;
Ecore_Exe          *exe3 = NULL;
Ecore_Exe          *exe4 = NULL;

static int
exe_data(void *data, int type, void *event)
{
   Ecore_Exe_Event_Data *ev;
   int                 i;

   ev = event;
   printf("  [*] DATA RET EXE %p - %p [%i bytes]\n", ev->exe, ev->data,
	  ev->size);

   if (ev->lines)
     {
	int                 i;

	for (i = 0; ev->lines[i].line != NULL; i++)
	  {
	     printf("%d %s\n", ev->lines[i].size, ev->lines[i].line);
	  }
     }
   else
     {
	for (i = 0; i < ev->size; i++)
	   putchar(((unsigned char *)ev->data)[i]);
     }
   printf("\n");
   return 1;
}

static int
exe_data_count(void *data, int type, void *event)
{
   Ecore_Exe_Event_Data *ev;
   int                 i;

   ev = event;

   if (ev->lines)
     {
	for (i = 0; ev->lines[i].line != NULL; i++)
	   line_count++;
	/* printf("%d ", i);   This is for testing the number of lines per event. */
     }

   data_count += ev->size;
   if (data_count >= size)
     {
	now = ecore_time_get();
	printf
	   ("\n\nApproximate data rate - %f bytes/second (%d lines and %d bytes (x2) in %f seconds).\n",
	    ((double)data_count * 2.0) / (now - then), line_count, data_count,
	    now - then);
	if (data_count != size)
	   printf("Size discrepency of %d bytes.\n", size - data_count);
	printf("\n");

	/* Since there does not seem to be anyway to convince /bin/cat to finish... */
	ecore_exe_terminate(exe0);
     }

   return 1;
}

static int
exe_exit(void *data, int type, void *event)
{
   Ecore_Exe_Event_Del *ev;

   ev = event;
   printf("  [*] EXE EXIT: %p\n", ev->exe);
   exe_count--;
   if (exe_count <= 0)
      ecore_main_loop_quit();
   return 1;
}

int
timer_once(void *data)
{
   int                 argc;
   char              **argv;
   int                 i = 1;

   ecore_app_args_get(&argc, &argv);
   ecore_event_handler_add(ECORE_EXE_EVENT_DATA, exe_data_count, NULL);
   ecore_event_handler_add(ECORE_EXE_EVENT_ERROR, exe_data_count, NULL);
   printf("FILE : %s\n", argv[i]);
   exe0 =
      ecore_exe_pipe_run("/bin/cat",
			 ECORE_EXE_PIPE_WRITE | ECORE_EXE_PIPE_READ |
			 ECORE_EXE_PIPE_READ_LINE_BUFFERED, NULL);

   if (exe0)
     {
	struct stat         s;

	exe_count++;
	if (stat(argv[i], &s) == 0)
	  {
	     int                 fd;

	     size = s.st_size;
	     if ((fd = open(argv[i], O_RDONLY)) != -1)
	       {
		  char                buf[1024];
		  int                 length;

		  then = ecore_time_get();
		  while ((length = read(fd, buf, 1024)) > 0)
		     ecore_exe_send(exe0, buf, length);
		  close(fd);
	       }
	  }
	/* FIXME: Fuckit, neither of these will actually cause /bin/cat to shut down.  What the fuck does it take? */
	ecore_exe_send(exe0, "\004", 1);	/* Send an EOF. */
	ecore_exe_close_stdin(exe0);	/* /bin/cat should stop when it's stdin closes. */
     }

   return 0;
}

int
main(int argc, char **argv)
{
   ecore_app_args_set(argc, (const char **)argv);

   ecore_init();
   ecore_event_handler_add(ECORE_EXE_EVENT_DEL, exe_exit, NULL);

   if (argc == 1)
     {
	ecore_event_handler_add(ECORE_EXE_EVENT_DATA, exe_data, NULL);
	ecore_event_handler_add(ECORE_EXE_EVENT_ERROR, exe_data, NULL);
	exe0 = ecore_exe_run("/bin/uname -a", NULL);
	if (exe0)
	   exe_count++;

	exe1 = ecore_exe_pipe_run("/bin/sh",
				  ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE,
				  NULL);
	if (exe1)
	  {
	     exe_count++;
	     ecore_exe_send(exe1, "ls\n", 3);
	     ecore_exe_send(exe1, "exit\n", 5);
	  }

	exe2 = ecore_exe_pipe_run("/usr/bin/find . -print",
				  ECORE_EXE_PIPE_READ |
				  ECORE_EXE_PIPE_READ_LINE_BUFFERED, NULL);
	if (exe2)
	   exe_count++;

	exe3 = ecore_exe_pipe_run("/bin/cat", ECORE_EXE_PIPE_WRITE, NULL);
	if (exe3)
	  {
	     exe_count++;
	     ecore_exe_send(exe3, "ls\n", 3);
	  }
	exe4 = ecore_exe_pipe_run(".libs/output_tester",
				  ECORE_EXE_PIPE_READ |
				  ECORE_EXE_PIPE_READ_LINE_BUFFERED |
				  ECORE_EXE_PIPE_ERROR |
				  ECORE_EXE_PIPE_ERROR_LINE_BUFFERED, NULL);
	if (exe4)
	   exe_count++;

	printf("  [*] exe0 = %p (/bin/uname -a)\n", exe0);
	printf("  [*] exe1 = %p (echo \"ls\" | /bin/sh)\n", exe1);
	printf("  [*] exe2 = %p (/usr/bin/find / -print)\n", exe2);
	printf("  [*] exe3 = %p (echo \"ls\" | /bin/cat)\n", exe3);
	printf("  [*] exe4 = %p (.libs/output_tester)\n", exe4);
     }
   else
      ecore_timer_add(0.5, timer_once, NULL);

   if ((exe_count > 0) || (argc > 1))
      ecore_main_loop_begin();

   ecore_shutdown();
   return 0;
}
