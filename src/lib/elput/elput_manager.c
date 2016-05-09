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

EAPI int
elput_manager_open(Elput_Manager *manager, const char *path, int flags)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(manager->interface, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, -1);

   if (flags < 0) flags = O_RDWR;

   if (manager->interface->open)
     return manager->interface->open(manager, path, flags);

   return -1;
}

EAPI void
elput_manager_close(Elput_Manager *manager, int fd)
{
   EINA_SAFETY_ON_NULL_RETURN(manager);
   EINA_SAFETY_ON_NULL_RETURN(manager->interface);

   if (manager->interface->close)
     manager->interface->close(manager, fd);
}

EAPI Eina_Bool
elput_manager_vt_set(Elput_Manager *manager, int vt)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(manager->interface, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((vt < 0), EINA_FALSE);

   if (manager->interface->vt_set)
     return manager->interface->vt_set(manager, vt);

   return EINA_FALSE;
}
