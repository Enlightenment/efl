#if HAVE_CONFIG_H
#include <config.h>
#endif


#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Eio.h>
#include <Ecore.h>

void done_cb(void *data, void *value EINA_UNUSED)
{
    Eio_Job *job = data;
    printf("%s done listing files.\n", __FUNCTION__);
    ecore_main_loop_quit();
    eo_unref(job);
}

void error_cb(void *data, Eina_Error error)
{
    Eio_Job *job = data;
    EINA_SAFETY_ON_NULL_RETURN(error);
    const char *msg = eina_error_msg_get(error);
    printf("%s error: %s\n", __FUNCTION__, msg);
    ecore_main_loop_quit();

    eo_unref(job);
}

void filter_cb(void *data, const Eo_Event *event)
{
    Eio_Filter_Name_Data *event_info = event->info;
    static Eina_Bool should_filter = EINA_FALSE;

    printf("Filtering file %s\n", event_info->file);

    should_filter = !should_filter;
    event_info->filter = should_filter;
}

// Progress used to be the "Eio_Main_Cb" family of callbacks in the legacy API.
void progress_cb(void *data, const char *filename)
{
    EINA_SAFETY_ON_NULL_RETURN(filename);
    printf("%s listing filename: %s\n", __FUNCTION__, filename);
}

void list_files(void *data)
{
    Eina_Promise *promise;
    const char *path = data;

    Eio_Job *job = eo_add(EIO_JOB_CLASS, NULL);
    eo_event_callback_add(job, EIO_JOB_EVENT_FILTER_NAME, (Eo_Event_Cb)&filter_cb, NULL);
    promise = eio_job_file_ls(job, path);
    eina_promise_progress_cb_add(promise, (Eina_Promise_Progress_Cb)&progress_cb, NULL, NULL);
    eina_promise_then(promise, &done_cb, &error_cb, job);
}

int main(int argc, char const *argv[])
{
    eio_init();
    ecore_init();

    const char *path = getenv("HOME");

    if (argc > 1)
        path = argv[1];

    Ecore_Job *job = ecore_job_add(&list_files, path);

    ecore_main_loop_begin();

    ecore_shutdown();
    eio_shutdown();
    return 0;
}
