#ifndef _ECORDOVA_CONTACTS_RECORD_UTILS_H
#define _ECORDOVA_CONTACTS_RECORD_UTILS_H

#include <Eina.h>
#include <contacts.h>
#include <stdbool.h>

bool get_str(contacts_record_h, unsigned int, char **);
bool get_str_p(contacts_record_h, unsigned int, const char **);
bool set_str(contacts_record_h, unsigned int, const char *);

bool get_int(contacts_record_h, unsigned int, int *);
bool set_int(contacts_record_h, unsigned int, int);

bool get_bool(contacts_record_h, unsigned int, Eina_Bool *);
bool set_bool(contacts_record_h, unsigned int, Eina_Bool);

bool clear_all_contact_record(contacts_record_h, unsigned int);

#endif
