
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <stdlib.h>

#include "callback.eo.h"

struct _Callback_Data {};
typedef struct _Callback_Data Callback_Data;

#define MY_CLASS CALLBACK_CLASS

static void _callback_default_constructor(Eo *obj EINA_UNUSED, Callback_Data *pd EINA_UNUSED)
{
  eo_do_super(obj, MY_CLASS, eo_constructor());
}

static void _callback_constructor(Eo *obj EINA_UNUSED, Callback_Data *pd EINA_UNUSED, Ecore_Cb cb, void *data)
{
  eo_do_super(obj, MY_CLASS, eo_constructor());
  cb(data);
}

static void _callback_constructor2(Eo *obj EINA_UNUSED, Callback_Data *pd EINA_UNUSED, Ecore_Cb cb, void *data
                                   , Ecore_Cb cb2 EINA_UNUSED)
{
  eo_do_super(obj, MY_CLASS, eo_constructor());
  cb(data);
}

static void _callback_onecallback(Eo *obj EINA_UNUSED, Callback_Data *pd EINA_UNUSED, Ecore_Cb cb, void *data)
{
  cb(data);
}

static void _callback_twocallback(Eo *obj EINA_UNUSED, Callback_Data *pd EINA_UNUSED, Ecore_Cb cb, void *data
                                  , Ecore_Cb  cb2 EINA_UNUSED)
{
  cb(data);
}

#include "callback.eo.c"

