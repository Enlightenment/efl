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

#include "ecore_audio_private.h"

EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_BASE_ID = EO_NOOP;

#define MY_CLASS ECORE_AUDIO_OBJ_OUT_CLASS
#define MY_CLASS_NAME "ecore_audio_obj_out"

static void _input_attach(Eo *eo_obj, void *_pd, va_list *list)
{
  Ecore_Audio_Output *obj = _pd;
  Ecore_Audio_Input *in;

  Eo *input = va_arg(*list, Eo *);
  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  in = eo_data_get(input, ECORE_AUDIO_OBJ_IN_CLASS);

  if (ret)
    *ret = EINA_FALSE;

  if (in->output == eo_obj)
    return;

  if (in->output) eo_do(in->output, ecore_audio_obj_out_input_detach(input, NULL));
  in->output = eo_obj;

  /* TODO: Check type is input
   * Get private data
   * Send event */

  obj->inputs = eina_list_append(obj->inputs, input);

  if (ret)
    *ret = EINA_TRUE;
}

static void _input_detach(Eo *eo_obj, void *_pd, va_list *list)
{
  Ecore_Audio_Output *obj = _pd;
  Ecore_Audio_Input *in;

  Eo *input = va_arg(*list, Eo *);
  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  in = eo_data_get(input, ECORE_AUDIO_OBJ_IN_CLASS);

  if (ret)
    *ret = EINA_FALSE;

  if (in->output != eo_obj)
    return;

  in->output = NULL;

  /* TODO: Check type is input
   * Get private data
   * Send event */

  obj->inputs = eina_list_remove(obj->inputs, input);

  if (ret)
    *ret = EINA_TRUE;
}

static void _inputs_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Output *obj = _pd;

  Eina_List **inputs = va_arg(*list, Eina_List **);

  if (inputs)
    *inputs = obj->inputs;
}


static void _constructor(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

}

static void _destructor(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
  const Ecore_Audio_Output *obj = _pd;
  Eina_List *cur, *tmp;
  Eo *in;

  EINA_LIST_FOREACH_SAFE(obj->inputs, cur, tmp, in) {
      eo_do(eo_obj, ecore_audio_obj_out_input_detach(in, NULL));
  }

  eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

static void _class_constructor(Eo_Class *klass)
{
  const Eo_Op_Func_Description func_desc[] = {
      /* Virtual functions of parent class implemented in this class */
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),

      /* Specific functions to this class */
      EO_OP_FUNC(ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_ATTACH), _input_attach),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_DETACH), _input_detach),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUTS_GET), _inputs_get),

      EO_OP_FUNC_SENTINEL
  };

  eo_class_funcs_set(klass, func_desc);
}

#define S(val) "Sets the " #val " of the output."
#define G(val) "Gets the " #val " of the output."

static const Eo_Op_Description op_desc[] = {
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_ATTACH, "Add an input."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_DETACH, "Delete an input."),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUTS_GET, "Get the connected inputs."),
    EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
    EO_VERSION,
    MY_CLASS_NAME,
    EO_CLASS_TYPE_REGULAR,
    EO_CLASS_DESCRIPTION_OPS(&ECORE_AUDIO_OBJ_OUT_BASE_ID, op_desc, ECORE_AUDIO_OBJ_OUT_SUB_ID_LAST),
    NULL,
    sizeof(Ecore_Audio_Output),
    _class_constructor,
    NULL
};

EO_DEFINE_CLASS(ecore_audio_obj_out_class_get, &class_desc, ECORE_AUDIO_OBJ_CLASS, NULL);
