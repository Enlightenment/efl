#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

#define MY_CLASS ECTOR_RENDERER_IMAGE_MIXIN


static void
_ector_renderer_image_buffer_set(Eo *obj EINA_UNUSED,
                                 Ector_Renderer_Image_Data *pd,
                                 Ector_Buffer *buffer)
{
   pd->buffer = buffer;
}


#include "ector_renderer_image.eo.c"
