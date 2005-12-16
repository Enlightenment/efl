/* Timer example.
 */

#include <Ecore.h>

#include <stdlib.h>
#include <stdio.h>

Ecore_Exe *exe0 = NULL;
Ecore_Exe *exe1 = NULL;
Ecore_Exe *exe2 = NULL;
Ecore_Exe *exe3 = NULL;
 
static int
exe_data(void *data, int type, void *event)
{
   Ecore_Event_Exe_Data *ev;
   int i;

   ev = event;
   printf("  [*] DATA RET EXE %p - %p [%i bytes]\n", ev->exe, ev->data, ev->size);

   if (ev->lines)
      {
         int i;
	       
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
exe_exit(void *data, int type, void *event)
{
   Ecore_Event_Exe_Exit *ev;

   ev = event;
   printf("  [*] EXE EXIT: %p\n", ev->exe);
   return 1;
}

int main(int argc, char **argv) {
   ecore_init();
   ecore_event_handler_add(ECORE_EVENT_EXE_DATA, exe_data, NULL);
   ecore_event_handler_add(ECORE_EVENT_EXE_EXIT, exe_exit, NULL);
   exe0 = ecore_exe_run("/bin/uname -a", NULL);
   
   exe1 = ecore_exe_pipe_run("/bin/sh",
			    ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE,
			    NULL);
   ecore_exe_pipe_write(exe1, "ls\n", 3);
   exe2 = ecore_exe_pipe_run("/usr/bin/find / -print",
			    ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_READ_LINE_BUFFERED,
			    NULL);
   exe3 = ecore_exe_pipe_run("/bin/cat",
			    ECORE_EXE_PIPE_WRITE,
			    NULL);
   ecore_exe_pipe_write(exe3, "ls\n", 3);
   printf("  [*] exe0 = %p (/bin/uname -a)\n", exe0);
   printf("  [*] exe1 = %p (echo \"ls\" | /bin/sh)\n", exe1);
   printf("  [*] exe2 = %p (/usr/bin/find / -print)\n", exe2);
   printf("  [*] exe3 = %p (echo \"ls\" | /bin/cat)\n", exe3);
   ecore_main_loop_begin();
   ecore_shutdown();
   return 0;
}
