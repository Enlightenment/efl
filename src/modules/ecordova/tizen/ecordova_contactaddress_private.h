#ifndef _ECORDOVA_CONTACTADDRESS_PRIVATE_H
#define _ECORDOVA_CONTACTADDRESS_PRIVATE_H

#include "ecordova_private.h"
#include "ecordova_contactaddress.eo.h"

#include <contacts.h>

typedef struct _Ecordova_ContactAddress_Data Ecordova_ContactAddress_Data;

/**
 * Ecordova.ContactAddress private data
 */
struct _Ecordova_ContactAddress_Data
{
   Eo                *obj;
   int                id;
   contacts_record_h  record;
};

bool ecordova_contactaddress_import(Ecordova_ContactAddress *, contacts_record_h);
bool ecordova_contactaddress_export(Ecordova_ContactAddress *, contacts_record_h);
Ecordova_ContactAddress *ecordova_contactaddress_clone(Ecordova_ContactAddress *);

#endif
