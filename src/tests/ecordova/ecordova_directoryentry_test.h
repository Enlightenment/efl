#ifndef _ECORDOVA_DIRECTORYENTRY_TEST_H
#define _ECORDOVA_DIRECTORYENTRY_TEST_H

#include <Ecordova.h>

#include <Ecore_File.h>

#include <check.h>

#include <stdbool.h>

void ecordova_directoryentry_test(TCase *);
Ecordova_DirectoryEntry *_create_tmpdir(Eina_Tmpstr **);
Eina_Bool _timeout_cb(void *);
Eina_Bool _entry_get_cb(void *, Eo *, const Eo_Event_Description *, void *);
Eina_Bool _error_cb(void *, Eo *, const Eo_Event_Description *, void *);

bool directoryentry_get(Ecordova_DirectoryEntry *directory_entry, const char *, Ecordova_FileFlags, Ecordova_DirectoryEntry **);
bool fileentry_get(Ecordova_DirectoryEntry *directory_entry, const char *, Ecordova_FileFlags, Ecordova_FileEntry **);
void check_exists(const char *url);
void check_doesnt_exist(const char *url);
Ecordova_DirectoryEntry *directoryentry_new(const char *, const char *, const char *);

#endif
