#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
# include <evil_private.h> /* mkdir */
#endif

#include "eio_suite.h"
#include "eio_test_common.h"

#ifndef O_BINARY
# define O_BINARY 0
#endif

static const char *good_dirs[] =
     {
        "eio_file_ls_simple_dir",
        "b."
     };

unsigned int default_rights = DEFAULT_RIGHTS;

const char *files[] =
     {
        ".hidden_file",
        "~$b@:-*$a!{}",
        "$b$a",
        "normal_file"
     };

Eina_Tmpstr*
get_full_path(const char* tmpdirname, const char* filename)
{
    char full_path[PATH_MAX] = "";
    eina_str_join(full_path, sizeof(full_path), '/', tmpdirname, filename);
    return eina_tmpstr_add(full_path);
}

Eina_Tmpstr*
get_eio_test_file_tmp_dir()
{
   Eina_Tmpstr *tmp_dir;

   Eina_Bool created = eina_file_mkdtemp("EioFileTestXXXXXX", &tmp_dir);

   if (!created)
     {
        return NULL;
     }

   return tmp_dir;
}

Eina_Tmpstr*
create_test_dirs(Eina_Tmpstr *test_dirname)
{
   int i, fd;
   int count = sizeof(good_dirs) / sizeof(const char *);
   fail_if(test_dirname == NULL);

   for (i = 0; i != count; ++i)
     {
        Eina_Tmpstr *dirname = get_full_path(test_dirname, good_dirs[i]);
        fail_if(mkdir(dirname, default_rights) != 0);
        eina_tmpstr_del(dirname);
     }
   count = sizeof(files) / sizeof(const char *);
   for (i = 0; i != count; ++i)
     {
        Eina_Tmpstr *filename = get_full_path(test_dirname, files[i]);
        fd = open(filename, O_RDWR | O_BINARY | O_CREAT, default_rights);
        fail_if(fd < 0);
        fail_if(close(fd) != 0);
        eina_tmpstr_del(filename);
     }
   Eina_Tmpstr *nested_dirname = get_full_path(test_dirname, good_dirs[0]);
   for (i = 0; i != count; ++i)
     {
        Eina_Tmpstr *filename = get_full_path(nested_dirname, files[i]);
        fd = open(filename, O_RDWR | O_BINARY | O_CREAT, default_rights);
        fail_if(fd < 0);
        fail_if(close(fd) != 0);
        eina_tmpstr_del(filename);
     }
   return nested_dirname;
}
