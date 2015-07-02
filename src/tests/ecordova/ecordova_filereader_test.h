#ifndef _ECORDOVA_FILEREADER_TEST_H
#define _ECORDOVA_FILEREADER_TEST_H

#include <Ecordova.h>

#include <check.h>

#include <stdbool.h>

void ecordova_filereader_test(TCase *);
bool filereader_read(Ecordova_File *file, char **content, size_t *length);

#endif
