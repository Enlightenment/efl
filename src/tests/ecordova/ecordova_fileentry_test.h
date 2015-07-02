#ifndef _ECORDOVA_FILEENTRY_TEST_H
#define _ECORDOVA_FILEENTRY_TEST_H

#include <Ecordova.h>

#include <check.h>

#include <stdbool.h>

void ecordova_fileentry_test(TCase *);
Ecordova_FileEntry *create_tmpfile(Eina_Tmpstr **, unsigned char **, size_t *);
Ecordova_FileEntry *create_tmpfile_size(Eina_Tmpstr **, unsigned char **, size_t);
bool fileentry_file_get(Ecordova_FileEntry *, Ecordova_File **);
unsigned char * generate_random_buffer(size_t*);
unsigned char * generate_random_buffer_size(size_t);

#endif
