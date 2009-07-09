#ifndef _ETHUMB_PLUGIN_H_
#define _ETHUMB_PLUGIN_H_

#include <Ethumb.h>
#include <Eina.h>
#include <Evas.h>
#include <Ecore_Evas.h>

typedef struct _Ethumb_Plugin Ethumb_Plugin;

struct _Ethumb_Plugin
{
   const char **extensions;
   void (*generate_thumb)(Ethumb *);
};

void ethumb_calculate_aspect_from_ratio(Ethumb *e, float ia, int *w, int *h);
void ethumb_calculate_aspect(Ethumb *e, int iw, int ih, int *w, int *h);
void ethumb_calculate_fill_from_ratio(Ethumb *e, float ia, int *fx, int *fy, int *fw, int *fh);
void ethumb_calculate_fill(Ethumb *e, int iw, int ih, int *fx, int *fy, int *fw, int *fh);
Eina_Bool ethumb_plugin_image_resize(Ethumb *e, int w, int h);
Eina_Bool ethumb_image_save(Ethumb *e);
void ethumb_finished_callback_call(Ethumb *e, int result);
Evas * ethumb_evas_get(const Ethumb *e);
Ecore_Evas * ethumb_ecore_evas_get(const Ethumb *e);

#endif /* _ETHUMB_PLUGIN_H_ */
