#include "elput_private.h"

static Elput_Interface *_ifaces[] =
{
#if defined(HAVE_SYSTEMD) || defined(HAVE_ELOGIND)
   &_logind_interface,
#endif
   NULL,
};

static Eina_Bool
_cb_key_down(void *data, int type EINA_UNUSED, void *event)
{
   Elput_Manager *em;
   Ecore_Event_Key *ev;
   int code = 0, vt = 0;

   em = data;
   ev = event;
   code = (ev->keycode - 8);

   if ((ev->modifiers & ECORE_EVENT_MODIFIER_CTRL) &&
       (ev->modifiers & ECORE_EVENT_MODIFIER_ALT) &&
       (code >= KEY_F1) && (code <= KEY_F8))
     {
        vt = (code - KEY_F1 + 1);
        if (em->interface->vt_set)
          {
             if (!em->interface->vt_set(em, vt))
               ERR("Failed to switch to virtual terminal %d", vt);
          }
     }

   return ECORE_CALLBACK_RENEW;
}

EAPI Elput_Manager *
elput_manager_connect(const char *seat, unsigned int tty)
{
   Elput_Interface **it;

   for (it = _ifaces; *it != NULL; it++)
     {
        Elput_Interface *iface;
        Elput_Manager *em;

        iface = *it;
        if (iface->connect(&em, seat, tty))
          return em;
     }

   return NULL;
}

EAPI void
elput_manager_disconnect(Elput_Manager *manager)
{
   EINA_SAFETY_ON_NULL_RETURN(manager);
   EINA_SAFETY_ON_NULL_RETURN(manager->interface);

   if (manager->input.thread)
     {
        ecore_thread_cancel(manager->input.thread);
        manager->del = 1;
        return;
     }

   if (manager->interface->disconnect)
     manager->interface->disconnect(manager);
}

EAPI int
elput_manager_open(Elput_Manager *manager, const char *path, int flags)
{
   int ret = -1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(manager->interface, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, -1);

   if (flags < 0) flags = O_RDWR;

   if (manager->interface->open)
     {
        ret = manager->interface->open(manager, path, flags);
        if (ret)
          {
             manager->vt_hdlr =
               ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
                                       _cb_key_down, manager);
             manager->vt_fd = ret;
          }
     }

   return ret;
}

EAPI void
elput_manager_close(Elput_Manager *manager, int fd)
{
   EINA_SAFETY_ON_NULL_RETURN(manager);
   EINA_SAFETY_ON_NULL_RETURN(manager->interface);

   if (fd == manager->vt_fd)
     {
        if (manager->vt_hdlr) ecore_event_handler_del(manager->vt_hdlr);
        manager->vt_hdlr = NULL;
     }

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

EAPI void
elput_manager_window_set(Elput_Manager *manager, unsigned int window)
{
   EINA_SAFETY_ON_NULL_RETURN(manager);

   manager->window = window;
}

EAPI const Eina_List *
elput_manager_seats_get(Elput_Manager *manager)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, NULL);
   return manager->input.seats;
}
