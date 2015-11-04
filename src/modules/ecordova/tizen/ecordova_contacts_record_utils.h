#ifndef _ECORDOVA_CONTACTS_RECORD_UTILS_H
#define _ECORDOVA_CONTACTS_RECORD_UTILS_H

#include <Eina.h>

#include "ecordova_contacts_service.h"

Eina_Bool get_str(contacts_record_h, unsigned int, char **);
Eina_Bool get_str_p(contacts_record_h, unsigned int, const char **);
Eina_Bool set_str(contacts_record_h, unsigned int, const char *);

Eina_Bool get_int(contacts_record_h, unsigned int, int *);
Eina_Bool set_int(contacts_record_h, unsigned int, int);

Eina_Bool get_bool(contacts_record_h, unsigned int, Eina_Bool *);
Eina_Bool set_bool(contacts_record_h, unsigned int, Eina_Bool);

Eina_Bool clear_all_contact_record(contacts_record_h, unsigned int);

#endif
