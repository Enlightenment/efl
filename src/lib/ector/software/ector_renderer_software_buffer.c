#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include "Ector_Software.h"
#include "ector_private.h"
#include "ector_software_private.h"

#define MY_CLASS ECTOR_RENDERER_SOFTWARE_BUFFER_CLASS

typedef struct
{
   Ector_Renderer_Data *base;
   Software_Rasterizer *surface;
   Ector_Buffer *eo_buffer;
} Ector_Renderer_Software_Buffer_Data;


EOLIAN static void
_ector_renderer_software_buffer_buffer_set(Eo *obj, Ector_Renderer_Software_Buffer_Data *pd, Ector_Buffer *buf)
{
   _eo_xrefplace(&pd->eo_buffer, buf, obj);
}

EOLIAN static Ector_Buffer *
_ector_renderer_software_buffer_buffer_get(Eo *obj EINA_UNUSED, Ector_Renderer_Software_Buffer_Data *pd)
{
   return pd->eo_buffer;
}

EOLIAN static Eina_Bool
_ector_renderer_software_buffer_ector_renderer_software_fill(Eo *obj, Ector_Renderer_Software_Buffer_Data *pd)
{
   Ector_Software_Buffer *buffer = eo_data_scope_get(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   ector_software_rasterizer_buffer_set(pd->surface, buffer);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_ector_renderer_software_buffer_ector_renderer_prepare(Eo *obj, Ector_Renderer_Software_Buffer_Data *pd)
{
   if (!pd->surface)
     pd->surface = eo_data_xref(pd->base->surface, ECTOR_SOFTWARE_SURFACE_CLASS, obj);

   return EINA_TRUE;
}

EOLIAN static unsigned int
_ector_renderer_software_buffer_ector_renderer_crc_get(Eo *obj, Ector_Renderer_Software_Buffer_Data *pd)
{
   Ector_Software_Buffer_Base_Data *buffer = eo_data_scope_get(pd->eo_buffer, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   unsigned int crc;

   crc = ector_renderer_crc_get(eo_super(obj, MY_CLASS));
   crc = eina_crc((const char *) buffer, sizeof(*buffer), crc, EINA_FALSE);
   if (pd->surface)
     crc = eina_crc((const char *) pd->surface, sizeof(*pd->surface), crc, EINA_FALSE);

   return crc;
}

EOLIAN static void
_ector_renderer_software_buffer_efl_object_destructor(Eo *obj, Ector_Renderer_Software_Buffer_Data *pd)
{
   eo_data_xunref(pd->base->surface, pd->surface, obj);
   efl_destructor(eo_super(obj, MY_CLASS));
}

#include "ector_renderer_software_buffer.eo.c"
