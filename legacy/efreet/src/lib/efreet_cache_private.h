#ifndef EFREET_CACHE_PRIVATE_H
#define EFREET_CACHE_PRIVATE_H

EAPI Eet_Data_Descriptor *efreet_desktop_edd_init(void);
#ifdef ICON_CACHE
EAPI Eet_Data_Descriptor *efreet_icon_theme_edd(void);
EAPI Eet_Data_Descriptor *efreet_icon_fallback_edd(void);
#endif

#endif
