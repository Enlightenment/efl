#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Ecore.h>
#include <Eio.h>
#include <Eet.h>

#include "eio_suite.h"

Eina_File *ee;

static void
_done_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_open_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, Eina_File *ef)
{
   ee = ef;
   ecore_main_loop_quit();
}

static Eina_Bool
_filter_cb(void *data, Eio_File *handler EINA_UNUSED, void *map, size_t length)
{
   char *str = data;
   size_t len = strlen(str);

   fail_if(len != length);
   fail_if(memcmp(data, map, length) != 0);
   return EINA_TRUE;
}

static void
_map_cb(void *datai EINA_UNUSED, Eio_File *handler EINA_UNUSED, void *map EINA_UNUSED, size_t length EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   fprintf(stderr, "Error:%s\n", strerror(error));
   ecore_main_loop_quit();
}

EFL_START_TEST(eio_test_map_simple)
{
   int fd;
   const char *file = "eio_map_exampleXXXXXX.txt";
   const char *data = "This is the data to save in file";
   Eio_File *ef;
   Eina_Tmpstr *file_path;

   fd = eina_file_mkstemp(file, &file_path);
   fail_if(fd < 0);
   fail_if(write(fd, data, strlen(data)) != (ssize_t)strlen(data));
   close(fd);

   ef = eio_file_open(file_path, EINA_FALSE, _open_cb, _error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_file_map_all(ee, EINA_FILE_POPULATE, _filter_cb, _map_cb,
                    _error_cb, data);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_file_map_new(ee, EINA_FILE_WILLNEED, 0, strlen(data), _filter_cb,
                         _map_cb, _error_cb, data);
   ecore_main_loop_begin();
   fail_if(!ef);

   ef = eio_file_close(ee, _done_cb, _error_cb, NULL);
   ecore_main_loop_begin();
   fail_if(!ef);

   eina_tmpstr_del(file_path);
}
EFL_END_TEST

void
eio_test_map(TCase *tc)
{
   tcase_add_test(tc, eio_test_map_simple);
}
