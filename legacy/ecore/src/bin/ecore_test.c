#include "config.h"

#include <Ecore.h>
#include <assert.h>
#include <unistd.h>

const char *called = NULL;

static const char *idler_str = "idler";
static const char *idle_enterer_str = "idler_enterer";
static const char *idle_exiter_str = "idler_exiter";
static const char *timer1_str = "timer 1";
static const char *timer2_str = "timer 2";
static const char *pipe_read_str = "pipe read";

int count;
Ecore_Pipe *the_pipe;

Eina_Bool timer_one(void *data __UNUSED__)
{
   fprintf(stderr, "timer 1\n");
   assert(called == pipe_read_str);
   called = timer1_str;
   ecore_pipe_write(the_pipe, "b", 1);

   count++;
   if (count == 10)
     {
       ecore_main_loop_quit();
       return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool timer_two(void *data __UNUSED__)
{
   fprintf(stderr, "timer 2\n");
   assert(called == timer1_str);
   called = timer2_str;

   return EINA_TRUE;
}

Eina_Bool idle_enterer_one(void *data __UNUSED__)
{
   fprintf(stderr, "idle enterer!\n");
   switch (count)
   {
   default:
     assert(called == timer2_str);
     break;
   case 1:
     assert(called == timer1_str);
     break;
   case 0:
     assert(called == NULL);
   }
   called = idle_enterer_str;
   return EINA_TRUE;
}

Eina_Bool idler_one(void *data __UNUSED__)
{
   fprintf(stderr, "idler!\n");
   assert(called == idle_enterer_str);
   called = idler_str;
   if (count == 0)
     ecore_timer_add(0.0, timer_two, NULL);
   return EINA_TRUE;
}

Eina_Bool idle_exiter_one(void *data __UNUSED__)
{
   fprintf(stderr, "idle exiter!\n");
   assert(called == idler_str);
   called = idle_exiter_str;
   return EINA_TRUE;
}

void pipe_read(void *data __UNUSED__, void *buffer __UNUSED__, unsigned int nbyte __UNUSED__)
{
   fprintf(stderr, "pipe read\n");
   assert(called == idle_exiter_str);
   called = pipe_read_str;
}

int main(int argc __UNUSED__, char **argv __UNUSED__)
{
   ecore_init();

   the_pipe = ecore_pipe_add(&pipe_read, NULL);
   ecore_pipe_write(the_pipe, "a", 1);

   ecore_timer_add(0.0, timer_one, NULL);

   ecore_idle_enterer_add(&idle_enterer_one, NULL);
   ecore_idler_add(&idler_one, NULL);
   ecore_idle_exiter_add(&idle_exiter_one, NULL);

   ecore_main_loop_begin();

   /* glib main loop exits on an idle enterer */
   assert(called == idle_enterer_str);

   ecore_shutdown();
   return 0;
}
