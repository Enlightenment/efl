#ifndef EFL_GFX_VG_VALUE_PROVIDER_H
#define EFL_GFX_VG_VALUE_PROVIDER_H

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_vg_private.h"

struct _Efl_Gfx_Vg_Value_Provider_Data
{
   Eo* obj;
   Efl_Gfx_Vg_Value_Provider_Flags flag;

   Eina_Stringshare *keypath;

   Eina_Matrix4 *m;
   struct {
      int r;
      int g;
      int b;
      int a;
   } fill;
   struct {
      int r;
      int g;
      int b;
      int a;
      double width;
   } stroke;
};

typedef struct _Efl_Gfx_Vg_Value_Provider_Data Efl_Gfx_Vg_Value_Provider_Data;

#endif
