#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_capture_private.h"

#define MY_CLASS ECORDOVA_CAPTURE_CLASS
#define MY_CLASS_NAME "Ecordova_Capture"

static Eo_Base *
_ecordova_capture_eo_base_constructor(Eo *obj, Ecordova_Capture_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_capture_constructor(Eo *obj EINA_UNUSED,
                              Ecordova_Capture_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_capture_eo_base_destructor(Eo *obj,
                                     Ecordova_Capture_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_capture_audio_capture(Eo *obj EINA_UNUSED,
                                Ecordova_Capture_Data *pd EINA_UNUSED,
                                const Ecordova_Capture_AudioOptions *options EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_capture_image_capture(Eo *obj EINA_UNUSED,
                                Ecordova_Capture_Data *pd EINA_UNUSED,
                                const Ecordova_Capture_ImageOptions *options EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_capture_video_capture(Eo *obj EINA_UNUSED,
                                Ecordova_Capture_Data *pd EINA_UNUSED,
                                const Ecordova_Capture_VideoOptions *options EINA_UNUSED)
{
   ERR("Not implemented.");
}

#include "ecordova_capture.eo.c"
