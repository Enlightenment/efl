#ifndef _ECORDOVA_CONTACTNAME_PRIVATE_H
#define _ECORDOVA_CONTACTNAME_PRIVATE_H

#include "ecordova_private.h"
#include "ecordova_contactname.eo.h"

#include <contacts.h>

typedef struct _Ecordova_ContactName_Data Ecordova_ContactName_Data;

/**
 * Ecordova.ContactName private data
 */
struct _Ecordova_ContactName_Data
{
   Eo                *obj;
   contacts_record_h  record;
};

Eina_Bool ecordova_contactname_import(Ecordova_ContactName *, contacts_record_h);
Eina_Bool ecordova_contactname_export(Ecordova_ContactName *, contacts_record_h);
Ecordova_ContactName *ecordova_contactname_clone(Ecordova_ContactName *);

#endif
