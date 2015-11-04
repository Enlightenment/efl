#ifndef _ECORDOVA_CONTACTFIELD_PRIVATE_H
#define _ECORDOVA_CONTACTFIELD_PRIVATE_H

#include "ecordova_private.h"
#include "ecordova_contactfield.eo.h"
#include "ecordova_contacts_service.h"

typedef enum _Ecordova_ContactField_Property
{
   ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID,
   ECORDOVA_CONTACTFIELD_PROPERTY_ID,
   ECORDOVA_CONTACTFIELD_PROPERTY_TYPE,
   ECORDOVA_CONTACTFIELD_PROPERTY_LABEL,
   ECORDOVA_CONTACTFIELD_PROPERTY_VALUE,
   ECORDOVA_CONTACTFIELD_PROPERTY_PREF,
   ECORDOVA_CONTACTFIELD_PROPERTY_COUNT
} Ecordova_ContactField_Property;

typedef struct _Ecordova_ContactField_Metadata
{
   const char * const *uri;
   const unsigned int *ids[ECORDOVA_CONTACTFIELD_PROPERTY_COUNT];
   char *(*type2label)(int type, const char *custom);
   int(*label2type)(const char *label);
} Ecordova_ContactField_Metadata;

typedef struct _Ecordova_ContactField_Data Ecordova_ContactField_Data;

/**
 * Ecordova.ContactField private data
 */
struct _Ecordova_ContactField_Data
{
   Eo                             *obj;
   int                             id;
   char                           *type;
   char                           *value;
   Eina_Bool                       pref;
   Ecordova_ContactField_Metadata  metadata;
};

Eina_Bool ecordova_contactfield_import(Ecordova_ContactField *, contacts_record_h, const Ecordova_ContactField_Metadata);
Eina_Bool ecordova_contactfield_export(Ecordova_ContactField *, contacts_record_h, const Ecordova_ContactField_Metadata);

char *ecordova_contactnumber_type2label(int, const char *);
int ecordova_contactnumber_label2type(const char *);
char *ecordova_contactemail_type2label(int, const char *);
int ecordova_contactemail_label2type(const char *);
char *ecordova_contactmessenger_type2label(int, const char *);
int ecordova_contactmessenger_label2type(const char *);
char *ecordova_contactimage_type2label(int, const char *);
int ecordova_contactimage_label2type(const char *);
char *ecordova_contacturl_type2label(int, const char *);
int ecordova_contacturl_label2type(const char *);

#endif
