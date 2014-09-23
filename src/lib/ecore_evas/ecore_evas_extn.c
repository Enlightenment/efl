#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include "Ecore_Evas.h"

EAPI int ECORE_EVAS_EXTN_CLIENT_ADD = 0;
EAPI int ECORE_EVAS_EXTN_CLIENT_DEL = 0;

void
_ecore_evas_extn_init(void)
{
   ECORE_EVAS_EXTN_CLIENT_ADD = ecore_event_type_new();
   ECORE_EVAS_EXTN_CLIENT_DEL = ecore_event_type_new();
}

void
_ecore_evas_extn_shutdown(void)
{
   ECORE_EVAS_EXTN_CLIENT_ADD = 0;
   ECORE_EVAS_EXTN_CLIENT_DEL = 0;
}
