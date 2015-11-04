#ifndef ECORDOVA_FILE_TESTS_H
#define ECORDOVA_FILE_TESTS_H

#include <Ecordova.h>

#include <Ecore_File.h>

#include <check.h>

#include <stdbool.h>

Ecordova_DirectoryEntry *_create_tmpdir(Eina_Tmpstr **);
Eina_Bool _timeout_cb(void *);
Eina_Bool _entry_get_cb(void *, Eo *, const Eo_Event_Description *, void *);
Eina_Bool _error_cb(void *, Eo *, const Eo_Event_Description *, void *);

bool directoryentry_get(Ecordova_DirectoryEntry *directory_entry, const char *, Ecordova_FileFlags, Ecordova_DirectoryEntry **);
bool fileentry_get(Ecordova_DirectoryEntry *directory_entry, const char *, Ecordova_FileFlags, Ecordova_FileEntry **);
void check_exists(const char *url);
void check_doesnt_exist(const char *url);
Ecordova_DirectoryEntry *directoryentry_new(const char *, const char *);

bool entry_remove(Ecordova_Entry *);
bool entry_do(Ecordova_Entry *, const Eo_Event_Description *, void(*)());

Ecordova_FileEntry *create_tmpfile(Eina_Tmpstr **, unsigned char **, size_t *);
Ecordova_FileEntry *create_tmpfile_size(Eina_Tmpstr **, unsigned char **, size_t);
bool fileentry_file_get(Ecordova_FileEntry *, Ecordova_File **);
unsigned char * generate_random_buffer(size_t*);
unsigned char * generate_random_buffer_size(size_t);

bool filereader_read(Ecordova_File *file, char **content, size_t *length);

#endif
