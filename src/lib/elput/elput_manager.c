#include "elput_private.h"

static Elput_Interface *_ifaces[] =
{
#ifdef HAVE_SYSTEMD
   &_logind_interface,
#endif
   NULL, // launcher
   NULL, // direct
   NULL,
};

EAPI Elput_Manager *
elput_manager_connect(const char *seat, unsigned int tty, Eina_Bool sync)
{
   Elput_Interface **it;

   for (it = _ifaces; *it != NULL; it++)
     {
        Elput_Interface *iface;
        Elput_Manager *em;

        iface = *it;
        if (iface->connect(&em, seat, tty, sync))
          return em;
     }

   return NULL;
}

EAPI void
elput_manager_disconnect(Elput_Manager *manager)
{
   EINA_SAFETY_ON_NULL_RETURN(manager);
   EINA_SAFETY_ON_NULL_RETURN(manager->interface);

   if (manager->interface->disconnect)
     manager->interface->disconnect(manager);
}
