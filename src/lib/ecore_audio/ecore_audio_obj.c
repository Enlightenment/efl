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

EAPI Eo_Op ECORE_AUDIO_OBJ_BASE_ID = EO_NOOP;

#define MY_CLASS ECORE_AUDIO_OBJ_CLASS
#define MY_CLASS_NAME "Ecore_Audio"

static void _name_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  Ecore_Audio_Object *obj = _pd;

  const char *name = va_arg(*list, const char *);

  eina_stringshare_replace(&obj->name, name);
}

static void _name_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Object *obj = _pd;

  const char **name = va_arg(*list, const char **);

  if (name)
    *name = obj->name;
}

static void _paused_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  Ecore_Audio_Object *obj = _pd;

  Eina_Bool paused = va_arg(*list, int);

  obj->paused = paused;
}

static void _paused_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Object *obj = _pd;

  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  if (ret)
    *ret = obj->paused;
}

static void _volume_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  Ecore_Audio_Object *obj = _pd;

  double volume = va_arg(*list, double);

  obj->volume = volume;
}

static void _volume_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Object *obj = _pd;

  double *ret = va_arg(*list, double *);

  if (ret)
    *ret = obj->volume;
}

static void _constructor(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
  Ecore_Audio_Object *obj = _pd;

  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

  obj->volume = 1.0;

}

static void _class_constructor(Eo_Class *klass)
{
  const Eo_Op_Func_Description func_desc[] = {
      /* Virtual functions of parent class implemented in this class */
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      //EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),

      /* Specific functions to this class */
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_NAME_SET), _name_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_NAME_GET), _name_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_PAUSED_SET), _paused_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_PAUSED_GET), _paused_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_VOLUME_SET), _volume_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_VOLUME_GET), _volume_get),
      EO_OP_FUNC_SENTINEL
  };

  eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_NAME_SET, "Sets the name of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_NAME_GET, "Gets the name of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_PAUSED_SET, "Sets the paused stated of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_PAUSED_GET, "Gets the paused stated of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_VOLUME_SET, "Sets the volume of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_VOLUME_GET, "Gets the volume of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_SET, "Sets the source of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_GET, "Gets the source of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_FORMAT_SET, "Sets the format of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_FORMAT_GET, "Gets the format of the object."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_SUB_ID_VIO_SET, "Sets virtual IO callbacks for this object."),
    EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
    EO_VERSION,
    MY_CLASS_NAME,
    EO_CLASS_TYPE_REGULAR,
    EO_CLASS_DESCRIPTION_OPS(&ECORE_AUDIO_OBJ_BASE_ID, op_desc, ECORE_AUDIO_OBJ_SUB_ID_LAST),
    NULL,
    sizeof(Ecore_Audio_Object),
    _class_constructor,
    NULL
};

EO_DEFINE_CLASS(ecore_audio_obj_class_get, &class_desc, EO_BASE_CLASS, NULL);
