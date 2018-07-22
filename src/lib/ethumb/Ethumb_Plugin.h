#ifndef _ETHUMB_PLUGIN_H_
#define _ETHUMB_PLUGIN_H_

#include <Evas.h>
#include <Ecore_Evas.h>
#include <Ethumb.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

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

EAPI Eina_Bool ethumb_plugin_register(const Ethumb_Plugin *plugin);
EAPI Eina_Bool ethumb_plugin_unregister(const Ethumb_Plugin *plugin);

EAPI void ethumb_calculate_aspect_from_ratio(Ethumb *e, float ia, int *w, int *h);
EAPI void ethumb_calculate_aspect(Ethumb *e, int iw, int ih, int *w, int *h);
EAPI void ethumb_calculate_fill_from_ratio(Ethumb *e, float ia, int *fx, int *fy, int *fw, int *fh);
EAPI void ethumb_calculate_fill(Ethumb *e, int iw, int ih, int *fx, int *fy, int *fw, int *fh);
EAPI Eina_Bool ethumb_plugin_image_resize(Ethumb *e, int w, int h);
EAPI Eina_Bool ethumb_image_save(Ethumb *e);
EAPI void ethumb_finished_callback_call(Ethumb *e, int result);
EAPI Evas * ethumb_evas_get(const Ethumb *e);
EAPI Ecore_Evas * ethumb_ecore_evas_get(const Ethumb *e);

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif /* _ETHUMB_PLUGIN_H_ */
