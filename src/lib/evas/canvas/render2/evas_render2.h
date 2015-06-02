#ifndef EVAS_RENDER2_H
#define EVAS_RENDER2_H
#include "evas_common_private.h"
#include "evas_private.h"
#include <math.h>
#include <assert.h>
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

Eina_Bool  _evas_render2(Eo *eo_e, Evas_Public_Data *e);
Eina_List *_evas_render2_updates(Eo *eo_e, Evas_Public_Data *e);
Eina_List *_evas_render2_updates_wait(Eo *eo_e, Evas_Public_Data *e);
void       _evas_norender2(Eo *eo_e, Evas_Public_Data *e);
void       _evas_render2_idle_flush(Eo *eo_e, Evas_Public_Data *e);
void       _evas_render2_sync(Eo *eo_e, Evas_Public_Data *e);
void       _evas_render2_dump(Eo *eo_e, Evas_Public_Data *e);

#endif
