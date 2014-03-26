#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif
#include <ctype.h>
#include <errno.h>

#include <Eo.h>
#include "ecore_audio_private.h"

#define MY_CLASS ECORE_AUDIO_OBJ_CLASS
#define MY_CLASS_NAME "Ecore_Audio"

EOLIAN static void
_ecore_audio_name_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Object *obj, const char *name)
{
  eina_stringshare_replace(&obj->name, name);
}

EOLIAN static const char*
_ecore_audio_name_get(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Object *obj)
{
  return obj->name;
}

EOLIAN static void
_ecore_audio_paused_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Object *obj, Eina_Bool paused)
{
  obj->paused = paused;
}

EOLIAN static Eina_Bool
_ecore_audio_paused_get(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Object *obj)
{
  return obj->paused;
}

EOLIAN static void
_ecore_audio_volume_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Object *obj, double volume)
{
  obj->volume = volume;
}

EOLIAN static double
_ecore_audio_volume_get(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Object *obj)
{
  return obj->volume;
}

EOLIAN static void
_ecore_audio_eo_base_constructor(Eo *eo_obj, Ecore_Audio_Object *obj)
{
  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

  obj->volume = 1.0;
}

#include "ecore_audio.eo.c"
