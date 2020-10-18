#ifndef _ETHUMB_PLUGIN_H_
#define _ETHUMB_PLUGIN_H_

#include <Evas.h>
#include <Ecore_Evas.h>
#include <Ethumb.h>

#include <ethumb_api.h>

typedef struct _Ethumb_Plugin Ethumb_Plugin;

struct _Ethumb_Plugin
{
#define ETHUMB_PLUGIN_API_VERSION (1U)
   unsigned int version;
   const char *name;
   const char **extensions;
   void *(*thumb_generate)(Ethumb *);
   void (*thumb_cancel)(Ethumb *, void *);
};

ETHUMB_API Eina_Bool ethumb_plugin_register(const Ethumb_Plugin *plugin);
ETHUMB_API Eina_Bool ethumb_plugin_unregister(const Ethumb_Plugin *plugin);

ETHUMB_API void ethumb_calculate_aspect_from_ratio(Ethumb *e, float ia, int *w, int *h);
ETHUMB_API void ethumb_calculate_aspect(Ethumb *e, int iw, int ih, int *w, int *h);
ETHUMB_API void ethumb_calculate_fill_from_ratio(Ethumb *e, float ia, int *fx, int *fy, int *fw, int *fh);
ETHUMB_API void ethumb_calculate_fill(Ethumb *e, int iw, int ih, int *fx, int *fy, int *fw, int *fh);
ETHUMB_API Eina_Bool ethumb_plugin_image_resize(Ethumb *e, int w, int h);
ETHUMB_API Eina_Bool ethumb_image_save(Ethumb *e);
ETHUMB_API void ethumb_finished_callback_call(Ethumb *e, int result);
ETHUMB_API Evas * ethumb_evas_get(const Ethumb *e);
ETHUMB_API Ecore_Evas * ethumb_ecore_evas_get(const Ethumb *e);

#ifdef __cplusplus
}
#endif

#endif /* _ETHUMB_PLUGIN_H_ */
