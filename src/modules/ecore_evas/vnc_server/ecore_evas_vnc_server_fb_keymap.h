#include <rfb/rfb.h>
#include <Eina.h>

#ifndef ECORE_EVAS_VNC_SERVER_FB_KEY_MAP_H
#define ECORE_EVAS_VNC_SERVER_FB_KEY_MAP_H

Eina_Bool ecore_evas_vnc_server_keysym_to_fb_translate(rfbKeySym key,
                                                       const char **key_name,
                                                       const char **key_str,
                                                       const char **compose);

#endif
