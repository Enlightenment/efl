#ifndef ELM_SYSTRAY_WATCHER_H
#define ELM_SYSTRAY_WATCHER_H

Eina_Bool _elm_systray_watcher_status_notifier_item_register(const char *obj);

Eina_Bool _elm_systray_watcher_init(void);

void _elm_systray_watcher_shutdown(void);

#endif
