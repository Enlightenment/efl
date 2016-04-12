#ifndef _EIO_TEST_COMMON_H
#define _EIO_TEST_COMMON_H

#include <Eina.h>

#define DEFAULT_RIGHTS 0755;
extern unsigned int default_rights;
extern const char *files[];

Eina_Tmpstr* get_full_path(const char* tmpdirname, const char* filename);

Eina_Tmpstr* get_eio_test_file_tmp_dir();

Eina_Tmpstr* create_test_dirs(Eina_Tmpstr *test_dirname);

#endif