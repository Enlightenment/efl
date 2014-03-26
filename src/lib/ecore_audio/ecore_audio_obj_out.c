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

#define MY_CLASS ECORE_AUDIO_OBJ_OUT_CLASS
#define MY_CLASS_NAME "Ecore_Audio_Out"

static Eina_Bool _write_cb(void *data)
{
  Eo *eo_obj = data;
  Eo *in;

  Ecore_Audio_Output *out_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_CLASS);
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  ssize_t written, bread;
  float buf[1024];

  if (!ea_obj->vio || !ea_obj->vio->vio->write)
    return EINA_FALSE;

  /* FIXME: Multiple inputs */
  in = eina_list_data_get(out_obj->inputs);

  eo_do(in, ecore_audio_obj_in_read(buf, 4*1024, &bread));

  if (bread == 0) {
      ea_obj->paused = EINA_TRUE;
      out_obj->write_idler = NULL;
      return EINA_FALSE;
  }
  written = ea_obj->vio->vio->write(ea_obj->vio->data, eo_obj, buf, bread);

  if (written != bread)
    ERR("Short write");

  return EINA_TRUE;
}

EOLIAN static Eina_Bool
_ecore_audio_out_input_attach(Eo *eo_obj, Ecore_Audio_Output *obj, Eo *input)
{
  Ecore_Audio_Input *in;

  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  in = eo_data_scope_get(input, ECORE_AUDIO_OBJ_IN_CLASS);

  if (in->output == eo_obj)
    return EINA_FALSE;

  if (in->output) eo_do(in->output, ecore_audio_obj_out_input_detach(input, NULL));
  in->output = eo_obj;

  /* TODO: Send event */

  obj->inputs = eina_list_append(obj->inputs, input);

  if (obj->need_writer &&
     ea_obj->vio && ea_obj->vio->vio->write)
    obj->write_idler = ecore_idler_add(_write_cb, eo_obj);


  return EINA_TRUE;
}

EOLIAN static Eina_Bool
_ecore_audio_out_input_detach(Eo *eo_obj, Ecore_Audio_Output *obj, Eo *input)
{
  Ecore_Audio_Input *in;

  in = eo_data_scope_get(input, ECORE_AUDIO_OBJ_IN_CLASS);

  if (in->output != eo_obj)
    return EINA_FALSE;

  in->output = NULL;

  /* TODO: Check type is input
   * Get private data
   * Send event */

  obj->inputs = eina_list_remove(obj->inputs, input);

  return EINA_TRUE;
}

EOLIAN static Eina_List *
_ecore_audio_out_inputs_get(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Output *obj)
{
   return obj->inputs;
}

static void _free_vio(Ecore_Audio_Object *ea_obj)
{
  if (ea_obj->vio->free_func)
    ea_obj->vio->free_func(ea_obj->vio->data);

  free(ea_obj->vio);
  ea_obj->vio = NULL;
}

EOLIAN static void
_ecore_audio_out_ecore_audio_vio_set(Eo *eo_obj, Ecore_Audio_Output *_pd EINA_UNUSED, Ecore_Audio_Vio *vio, void *data, eo_base_data_free_func free_func)
{
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  if (ea_obj->vio)
    _free_vio(ea_obj);

  if (!vio)
    return;

  ea_obj->vio = calloc(1, sizeof(Ecore_Audio_Vio_Internal));
  ea_obj->vio->vio = vio;
  ea_obj->vio->data = data;
  ea_obj->vio->free_func = free_func;
}

EOLIAN static void
_ecore_audio_out_eo_base_constructor(Eo *eo_obj, Ecore_Audio_Output *obj)
{
  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

  obj->need_writer = EINA_TRUE;
}

EOLIAN static void
_ecore_audio_out_eo_base_destructor(Eo *eo_obj, Ecore_Audio_Output *obj)
{
  Eina_List *cur, *tmp;
  Eo *in;

  EINA_LIST_FOREACH_SAFE(obj->inputs, cur, tmp, in) {
      eo_do(eo_obj, ecore_audio_obj_out_input_detach(in, NULL));
  }

  eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

#include "ecore_audio_out.eo.c"
