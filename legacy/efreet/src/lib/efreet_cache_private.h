#ifndef EFREET_CACHE_PRIVATE_H
#define EFREET_CACHE_PRIVATE_H

EAPI Eet_Data_Descriptor *efreet_desktop_edd(void);
#ifdef ICON_CACHE
EAPI Eet_Data_Descriptor *efreet_icon_theme_edd(Eina_Bool include_dirs);
EAPI Eet_Data_Descriptor *efreet_icon_fallback_edd(Eina_Bool include_dirs);
#endif

#endif
