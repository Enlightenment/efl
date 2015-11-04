#ifndef _ECORDOVA_CONTACTORGANIZATION_PRIVATE_H
#define _ECORDOVA_CONTACTORGANIZATION_PRIVATE_H

#include "ecordova_private.h"
#include "ecordova_contactorganization.eo.h"

#include <contacts.h>

typedef struct _Ecordova_ContactOrganization_Data Ecordova_ContactOrganization_Data;

/**
 * Ecordova.ContactOrganization private data
 */
struct _Ecordova_ContactOrganization_Data
{
   Eo                *obj;
   int                id;
   contacts_record_h  record;
};

Eina_Bool ecordova_contactorganization_import(Ecordova_ContactOrganization *, contacts_record_h);
Eina_Bool ecordova_contactorganization_export(Ecordova_ContactOrganization *, contacts_record_h);
Ecordova_ContactOrganization *ecordova_contactorganization_clone(Ecordova_ContactOrganization *);

#endif
