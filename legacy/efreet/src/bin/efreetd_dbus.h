#ifndef __EFREETD_DBUS_H
#define __EFREETD_DBUS_H

void send_signal_icon_cache_update(Eina_Bool update);
void send_signal_desktop_cache_update(Eina_Bool update);

Eina_Bool dbus_init(void);
Eina_Bool dbus_shutdown(void);

#endif
