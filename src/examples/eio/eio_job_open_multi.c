#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>
#include <Eio.h>
#include <Ecore.h>

void error_cb(void *data, Eina_Error error)
{
    EINA_SAFETY_ON_NULL_RETURN(error);
    EINA_SAFETY_ON_NULL_RETURN(data);

    const char *msg = eina_error_msg_get(error);
    EINA_LOG_ERR("error: %s", msg);

    Eio_Job *job = data;
    eo_unref(job);

    ecore_main_loop_quit();
}

void done_closing_cb(void *data, void* value EINA_UNUSED)
{
    EINA_SAFETY_ON_NULL_RETURN(data);

    Eina_Iterator* result = value;

    printf("%s closed file.\n", __FUNCTION__);

    Eio_Job *job = data;
    eo_unref(job);

    ecore_main_loop_quit();
}

void closing_job(Eio_Job *job, Eina_File *file1, Eina_File *file2)
{
    Eina_Promise *promise;
    Eina_Promise *tasks[3] = {NULL, NULL, NULL};

    printf("%s Closing files.\n", __FUNCTION__);
    eio_job_file_close(job, file1, &tasks[0]);
    eio_job_file_close(job, file2, &tasks[1]);
    promise = eina_promise_all(eina_carray_iterator_new((void**)&tasks[0]));
    eina_promise_then(promise, &done_closing_cb, &error_cb, job);
}

void done_open_cb(void *data, Eina_Iterator **iterator)
{
    EINA_SAFETY_ON_NULL_RETURN(data);
    EINA_SAFETY_ON_NULL_RETURN(iterator);
    EINA_SAFETY_ON_NULL_RETURN(*iterator);
    Eio_Job *job = data;

    Eina_File **file = NULL;
    Eina_File **files = calloc(sizeof(Eina_File*),2);
    int i = 0;
    while (eina_iterator_next(*iterator, (void**)&file))
    {
        files[i] = *file;
        const char *name = eina_file_filename_get(*file);
        printf("%s opened file %s\n", __FUNCTION__, name);
        i++;
    }
    closing_job(job, files[0], files[1]);
    free(files);
}

void open_file(const char *path, const char *path2)
{
    Eina_Promise *promise;
    Eina_Promise *tasks[3] = {NULL, NULL, NULL};

    Eio_Job *job = eo_add(EIO_JOB_CLASS, NULL);
    tasks[0] = eio_job_file_open(job, path, EINA_FALSE);
    tasks[1] = eio_job_file_open(job, path2, EINA_FALSE);
    promise = eina_promise_all(eina_carray_iterator_new((void**)&tasks[0]));
    eina_promise_then(promise, (Eina_Promise_Cb)&done_open_cb, (Eina_Promise_Error_Cb)&error_cb, job);
}

int main(int argc, char const *argv[])
{
    eio_init();
    ecore_init();

    const char *path = getenv("HOME");
    const char *path2 = "./";

    if (argc > 1)
        path = argv[1];
    if (argc > 2)
        path2 = argv[2];

    open_file(path, path2);

    ecore_main_loop_begin();

    ecore_shutdown();
    eio_shutdown();
    return 0;
}
