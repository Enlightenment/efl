//Compile with:
// gcc -o ecore_job_example ecore_job_example.c `pkg-config --libs --cflags ecore`

#include <Ecore.h>
#include <unistd.h>

static void
_job_print_cb(void *data)
{
   char *str = data;

   printf("%s\n", str);
}

static void
_job_quit_cb(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();
}

int
main(void)
{
   Ecore_Job *job1, *job2, *job3, *job_quit;
   char *str1 = "Job 1 started.";
   char *str2 = "Job 2 started.";
   char *str3 = "Job 3 started.";

   if (!ecore_init())
     {
        printf("ERROR: Cannot init Ecore!\n");
        return -1;
     }

   job1 = ecore_job_add(_job_print_cb, str1);
   job2 = ecore_job_add(_job_print_cb, str2);
   job3 = ecore_job_add(_job_print_cb, str3);

   job_quit = ecore_job_add(_job_quit_cb, NULL);

   (void)job1;
   (void)job3;
   (void)job_quit;

   printf("Created jobs 1, 2, 3 and quit.\n");

   if (job2)
     {
        char *str;
        str = ecore_job_del(job2);
        job2 = NULL;
        printf("Deleted job 2. Its data was: \"%s\"\n", str);
     }

   ecore_main_loop_begin();
   ecore_shutdown();

   return 0;
}

