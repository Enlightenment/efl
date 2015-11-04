#ifndef _ECORDOVA_CONTACT_PRIVATE_H
#define _ECORDOVA_CONTACT_PRIVATE_H

#include "ecordova_private.h"
#include "ecordova_contactaddress.eo.h"
#include "ecordova_contactfield.eo.h"
#include "ecordova_contactname.eo.h"
#include "ecordova_contactorganization.eo.h"
#include "ecordova_contact.eo.h"

#include <contacts.h>

typedef struct _Ecordova_Contact_Data Ecordova_Contact_Data;

/**
 * Ecordova.Contact private data
 */
struct _Ecordova_Contact_Data
{
   Eo                   *obj;
   contacts_record_h     record;
   int                   id;
   Ecordova_ContactName *name;
   Eina_Array           *phone_numbers;
   Eina_Array           *emails;
   Eina_Array           *addresses;
   Eina_Array           *ims;
   Eina_Array           *organizations;
   Eina_Array           *photos;
   Eina_Array           *categories;
   Eina_Array           *urls;
};

Eina_Bool ecordova_contact_import(Ecordova_Contact *, contacts_record_h);
Eina_Bool ecordova_contact_export(Ecordova_Contact *, contacts_record_h);

#endif
