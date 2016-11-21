//Compile with:
// gcc -o ecore_file_download_example ecore_file_download_example.c `pkg-config --libs --cflags ecore eina ecore-file`

#include <stdio.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>

/*
 * ecore_file_download() example
 *
 * compile with:
 * gcc ecore_file_download_example.c `pkg-config --libs --cflags ecore-file` \
 *     -o ecore_file_download_example
 *
 */

#define URL      "http://www.kernel.org/pub/linux/kernel/v1.0/linux-1.0.tar.gz"
#define DST      "linux-1.0.tar.gz"
#define DST_MIME "[x-gzip]linux-1.0.tar.gz"

static Ecore_File_Download_Job *job = NULL;

void
completion_cb(void *data EINA_UNUSED, const char *file EINA_UNUSED, int status)
{
   printf("Done (status: %d)\n", status);
   job = NULL;
   ecore_main_loop_quit();
}

int
progress_cb(void *data EINA_UNUSED, const char *file EINA_UNUSED,
            long int dltotal, long int dlnow,
            long int ultotal EINA_UNUSED, long int ulnow EINA_UNUSED)
{
   printf("Progress: %ld/%ld\n", dlnow, dltotal);
   return ECORE_FILE_PROGRESS_CONTINUE; //  continue the download
}

int
main(void)
{
   double start;
   Eina_Hash *headers = NULL;

   eina_init();
   ecore_init();
   ecore_file_init();

   if (ecore_file_exists(DST))
     ecore_file_unlink(DST);

   if (ecore_file_exists(DST_MIME))
     ecore_file_unlink(DST_MIME);

   start = ecore_time_get();

   if (ecore_file_download(URL, DST, completion_cb, progress_cb, NULL, &job))
     {
        printf("Download started successfully:\n  URL: %s\n  DEST: %s\n  JOB: %p\n", URL, DST, job);
        ecore_main_loop_begin();
        if (job)
          {
             printf("\nMain loop aborted! Abort download!\n");
             ecore_file_download_abort(job);
             job = NULL;
          }
        else
          {
             printf("\nTime elapsed: %f seconds\n", ecore_time_get() - start);
             printf("Downloaded %lld bytes\n", ecore_file_size(DST));
          }
     }
   else
     {
        printf("Error, can't start download\n");
        goto done;
     }

   headers = eina_hash_string_small_new(NULL);
   eina_hash_add(headers, "Content-type", "application/x-gzip");

   if (ecore_file_download_full(URL, DST_MIME, completion_cb, progress_cb, NULL, &job, headers))
     {
        printf("Download started successfully:\n  URL: %s\n  DEST: %s\n  JOB: %p\n", URL, DST_MIME, job);
        ecore_main_loop_begin();
        if (job)
          {
             printf("\nMain loop aborted! Abort download!\n");
             ecore_file_download_abort(job);
             job = NULL;
          }
        else
          {
             printf("\nTime elapsed: %f seconds\n", ecore_time_get() - start);
             printf("Downloaded %lld bytes\n", ecore_file_size(DST_MIME));
          }
     }
   else
     {
        printf("Error, can't start download\n");
        goto done;
     }

done:
   if (headers) eina_hash_free(headers);
   ecore_file_shutdown();
   ecore_shutdown();
   eina_shutdown();
   return 0;
}

