#ifndef EFREET_CACHE_PRIVATE_H
#define EFREET_CACHE_PRIVATE_H

#define EFREET_CACHE_VERSION "efreet//version"
#ifdef ICON_CACHE
#define EFREET_CACHE_ICON_FALLBACK "efreet//fallback"
#endif

Eina_Bool efreet_cache_check(Eet_File **ef, const char *path, int major);
void *efreet_cache_close(Eet_File *ef);

EAPI Eet_Data_Descriptor *efreet_version_edd(void);
EAPI Eet_Data_Descriptor *efreet_desktop_edd(void);
#ifdef ICON_CACHE
EAPI Eet_Data_Descriptor *efreet_icon_theme_edd(void);
EAPI Eet_Data_Descriptor *efreet_icons_edd(Eina_Bool include_dirs);
EAPI Eet_Data_Descriptor *efreet_icons_fallback_edd(Eina_Bool include_dirs);
#endif

#endif
