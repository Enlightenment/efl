//Compile with:
//gcc -g -Wall `pkg-config --cflags --libs ecore` -o ecore_pipe_simple_example ecore_pipe_simple_example.c

#include <unistd.h>
#include <Ecore.h>

static void
do_lengthy_task(Ecore_Pipe *pipe)
{
   int i, j;
   char *buffer;
   for (i = 0; i < 20; i++)
     {
        sleep(1);
        buffer = malloc(sizeof(char) * i);
        for (j = 0; j < i; j++)
           buffer[j] = 'a' + j;
        ecore_pipe_write(pipe, buffer, i);
        free(buffer);
     }
   ecore_pipe_write(pipe, "close", 5);
}

static void
handler(void *data, void *buf, unsigned int len)
{
   char *str = malloc(sizeof(char) * len + 1);
   memcpy(str, buf, len);
   str[len] = '\0';
   printf("received %d bytes\n", len);
   printf("content: %s\n", (const char*)str);
   free(str);
   if (len && !strncmp(buf, "close", len < 5 ? len : 5))
     {
        printf("close requested\n");
        ecore_main_loop_quit();
     }
}

int
main (int argc, char *argv[])
{
   Ecore_Pipe *pipe;
   pid_t child_pid;

   ecore_init();

   pipe = ecore_pipe_add(handler, NULL);

   child_pid = fork();
   if(!child_pid)
     {
        ecore_pipe_read_close(pipe);
        do_lengthy_task(pipe);
     }
   else
     {
        ecore_pipe_write_close(pipe);
        ecore_main_loop_begin();
     }

   ecore_pipe_del(pipe);
   ecore_shutdown();

   return 0;
}
