#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Eio.h>
#include <Ecore.h>

void error_cb(void *data, Eina_Error *error)
{
    EINA_SAFETY_ON_NULL_RETURN(error);
    EINA_SAFETY_ON_NULL_RETURN(data);

    const char *msg = eina_error_msg_get(*error);
    EINA_LOG_ERR("error: %s", msg);

    ecore_main_loop_quit();
}

void done_closing_cb(int *result EINA_UNUSED)
{
    printf("%s closed file.\n", __FUNCTION__);

    ecore_main_loop_quit();
}

void closing_job(Eio_Job *job, Eina_File *file)
{
    Eina_Promise *promise = NULL;
    printf("%s Will close the file...\n", __FUNCTION__);
    eio_job_file_close(job, file, &promise);
    eina_promise_then(promise, (Eina_Promise_Cb)&done_closing_cb, (Eina_Promise_Error_Cb)&error_cb, job);
}

void done_open_cb(void *data, Eina_File **file)
{
    EINA_SAFETY_ON_NULL_RETURN(data);
    EINA_SAFETY_ON_NULL_RETURN(file);
    EINA_SAFETY_ON_NULL_RETURN(*file);

    Eio_Job *job = data;

    const char *name = eina_file_filename_get(*file);
    printf("%s opened file %s\n", __FUNCTION__, name);

    closing_job(job, *file);
}

void open_file(const char *path)
{
    Eina_Promise *promise;

    Eio_Job *job = eo_add(EIO_JOB_CLASS, NULL);
    eio_job_file_open(job, path, EINA_FALSE, &promise);
    eina_promise_then(promise, (Eina_Promise_Cb)&done_open_cb, (Eina_Promise_Error_Cb)&error_cb, job);

    eo_unref(job);
}

int main(int argc, char const *argv[])
{
    eio_init();
    ecore_init();

    const char *path = getenv("HOME");

    if (argc > 1)
        path = argv[1];

    open_file(path);

    ecore_main_loop_begin();

    ecore_shutdown();
    eio_shutdown();
    return 0;
}
