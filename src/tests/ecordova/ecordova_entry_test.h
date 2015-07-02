#ifndef _ECORDOVA_ENTRY_TEST_H
#define _ECORDOVA_ENTRY_TEST_H

#include <Ecordova.h>

#include <check.h>

#include <stdbool.h>

void ecordova_entry_test(TCase *);
bool entry_remove(Ecordova_Entry *);
bool entry_do(Ecordova_Entry *, const Eo_Event_Description *, void(*)());

#endif
