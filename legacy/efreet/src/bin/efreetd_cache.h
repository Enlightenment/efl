#ifndef __EFREETD_CACHE_H
#define __EFREETD_CACHE_H

void cache_desktop_dir_add(const char *dir);
void cache_icon_dir_add(const char *dir);
void cache_icon_ext_add(const char *ext);

Eina_Bool cache_init(void);
Eina_Bool cache_shutdown(void);

#endif
