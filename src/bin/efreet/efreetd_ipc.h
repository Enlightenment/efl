#ifndef __EFREETD_IPC_H
#define __EFREETD_IPC_H

void send_signal_icon_cache_update(Eina_Bool update);
void send_signal_desktop_cache_update(Eina_Bool update);

Eina_Bool ipc_init(void);
Eina_Bool ipc_shutdown(void);

#endif
