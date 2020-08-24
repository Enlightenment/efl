#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#ifdef _WIN32
# include <io.h> /* close(), unlink() */
#else
# include <unistd.h> /* close(), unlink() */
#endif

#include <Eina.h>
#include <Eet.h>

#include "eet_suite.h"
#include "eet_test_common.h"

static Eina_Bool open_worker_stop;
static Eina_Condition open_worker_cond;
static Eina_Lock open_worker_mutex;

static void *
_open_close_worker(void *path, Eina_Thread tid EINA_UNUSED)
{
   static Eina_Bool first = EINA_TRUE;

   while (!open_worker_stop)
     {
        Eet_File *ef = eet_open((char const *)path, EET_FILE_MODE_READ);
        if (ef == NULL)
          return "eet_open() failed";
        else
          {
             Eet_Error err_code = eet_close(ef);
             if (err_code != EET_ERROR_NONE)
               return "eet_close() failed";
          }

        if (first)
          {
             eina_lock_take(&open_worker_mutex);
             eina_condition_broadcast(&open_worker_cond);
             eina_lock_release(&open_worker_mutex);
             first = EINA_FALSE;
          }
     }

   return NULL;
}

EFL_START_TEST(eet_test_cache_concurrency)
{
   const char *buffer = "test data";
   Eet_File *ef;
   void *thread_ret;
   unsigned int n;
   Eina_Thread thread;
   Eina_Tmpstr *tmpfile = NULL;
   Eina_Bool r;
   int tmpfd;

   eina_threads_init();

   eina_lock_new(&open_worker_mutex);
   eina_condition_new(&open_worker_cond, &open_worker_mutex);

   /* create a file to test with */
   /* tmpfile will be created in temporary directory (with eina_environment_tmp) */
   tmpfd = eina_file_mkstemp("eet_suite_testXXXXXX", &tmpfile);
   fail_if(-1 == tmpfd);
   fail_if(!!close(tmpfd));
   ef = eet_open(tmpfile, EET_FILE_MODE_WRITE);
   fail_if(!ef);
   fail_if(!eet_write(ef, "keys/tests", buffer, strlen(buffer) + 1, 0));

   eina_lock_take(&open_worker_mutex);
   /* start a thread that repeatedly opens and closes a file */
   open_worker_stop = 0;
   r = eina_thread_create(&thread, EINA_THREAD_NORMAL, -1, _open_close_worker, tmpfile);
   fail_unless(r);

   eina_condition_wait(&open_worker_cond);
   eina_lock_release(&open_worker_mutex);

   /* clear the cache repeatedly in this thread */
   for (n = 0; n < 20000; ++n)
     {
        eet_clearcache();
     }

   /* join the other thread, and fail if it returned an error message */
   open_worker_stop = 1;
   thread_ret = eina_thread_join(thread);
   fail_unless(thread_ret == NULL, (char const *)thread_ret);

   eet_close(ef);

   fail_if(unlink(tmpfile) != 0);

   eina_threads_shutdown();
   eina_tmpstr_del(tmpfile);
}
EFL_END_TEST

void eet_test_cache(TCase *tc)
{
   tcase_add_test(tc, eet_test_cache_concurrency);
}
